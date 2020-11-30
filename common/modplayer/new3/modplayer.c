   /////////////////////////////////
  // Fast Module Player GP32 ver.//
 // Aj0 & Sla, Gods' Maze 2002  //
/////////////////////////////////

// $Date: 2002/04/02 13:53:44 $  
// $Id: directsound.c,v 1.21 2002/04/02 13:53:44 ajo Exp $

  //////////////
 // Includes // 
//////////////


#include "gpdef.h"
#include "gpstdlib.h" 
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpmm.h"
#include "gpstdio.h"

#include "debug.h"
#include "modplayer.h"

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#ifndef true
#define false FALSE
#define true TRUE
#endif

#define max_channels 	2
#define max_sfx_channels  4


#ifdef VARIABLE_SAMPLERATE

int samplerate = 32000;
int samples_tick= 32000/50;
int buffer_half_size = 32000/50;

#define buffer_half_size_max  (44*1024/50)

#else

#define samplerate 	(38*1000)
#define samples_tick	samplerate/50

// we have an interrupt per tick 

#define buffer_half_size     samples_tick
#define buffer_half_size_max samples_tick

#endif




  //////////
 // Data //
//////////

unsigned short *buffer=NULL;
unsigned short *buffer_end=NULL;

///////////////////////////////////////////////////////////////////////////////
// GP32 Driver
///////////////////////////////////////////////////////////////////////////////

#define NUMBUFFERS	6				/* number of buffers */
#define BUFFERSIZE	120				/* buffer size in milliseconds */

int *play_pos;
int *play_chan;
int buffer_size;
int writting_buffer=0;


#define BUFFER1_SIZE (samples_tick*2*2)


///////////////////////////////////////////////////////////////////////////////
// MIXER DATA VARIABLES   (iwram)
///////////////////////////////////////////////////////////////////////////////
output_channel channels_R[max_channels];
output_channel channels_L[max_channels];




#define MAX_VOLUME 64

u16 volume_table [MAX_VOLUME][256];

s16 *sound_buffer_L;
s16 *sound_buffer_R;

int where_buf;


///////////////////////////////////////////////////////////////////////////////
// SFX PLAYER VARIABLES   
///////////////////////////////////////////////////////////////////////////////

	sfx_channel sfx_channels[max_sfx_channels];
	int sfx_oldest_age=0;
	
	

///////////////////////////////////////////////////////////////////////////////
// MODULE PLAYER VARIABLES   
///////////////////////////////////////////////////////////////////////////////

#include "period.h"

	
	mod_channel_info *mod_patterns;
	u8		mod_patterns_n;
	output_channel	mod_instrument[31];
	u32		freqtable[4096];

	u8  *mod_pattern_list;
	
//static module data	
	int mod_channels_num;		// mod num
	int mod_restart_pattern;
	int mod_played_patterns;


//playing module data	

	int mod_pattern_n=0;
	int mod_speed;
	int mod_next_change;
	int mod_playing=false;
	int mod_pattern_row=0;	
	int mod_tempo=128;
	int mod_arpeggio_pos=0;
	
	mod_channel_info *mod_pattern;
	
output_channel *mod_channels[]={	&channels_L[0],&channels_R[0],
				&channels_L[1],&channels_R[1]
#if max_channels>2
				,&channels_L[2],&channels_R[2]
#endif

#if max_channels>3
				,&channels_L[3],&channels_R[3]
#endif

				};									  									  
									  

  ///////////////////////////
 // Functions implemented //
///////////////////////////
__inline u16 calc_period(u8 finetune, u16 period) {
	
	int i;
	
	if (finetune==0) return period;
	
	for (i=0;(i<60)&&(PeriodTable[0][i]>period);i++);
	
	return PeriodTable[finetune][i];	
	
}

__inline u16 calc_arpeggio_period(u8 finetune, u16 period,int mod) {
	

	int i;
	
	for (i=0;(i<60)&&(PeriodTable[finetune][i]>period);i++);
	
	if ((i+mod)>=60) return PeriodTable[finetune][59];
	if ((i+mod)<0) return PeriodTable[finetune][0];
	
	return PeriodTable[finetune][i+mod];	
	
}


__inline void mod_do_pattern_break (int new_pattern_pos, int pattern_n) {
	int i;
	mod_pattern_row=new_pattern_pos;
	mod_pattern_n=pattern_n;
	if (mod_pattern_n>=mod_played_patterns)  //loop the patterns
    		mod_pattern_n=mod_restart_pattern;
    	
	if (mod_pattern_n>=127) {
    		mod_playing=false;
	        for (i=0;i<mod_channels_num;i++) mod_channels[i]->playing=false;
	}
	else mod_pattern= mod_patterns + ((mod_pattern_list[mod_pattern_n]) * mod_channels_num * 64);    	
}

__inline void mod_player(void) {
	
    int i, instr,period,effect,effect_arg;	
    int new_pattern, new_row,effect_jump;
    u32 mcpy_size;
    u8 *mcpy_src, *mcpy_dst;
    
    if ((--mod_next_change)==0) {
    	
    	mod_next_change=mod_speed;

//                Byte  1   Byte  2   Byte  3   Byte 4
//              --------- --------- --------- ---------
//              7654-3210 7654-3210 7654-3210 7654-3210
//              wwww XXXX xxxxxxxxx yyyy ZZZZ zzzzzzzzz
//
//                  wwwwyyyy ( 8 bits) : sample number
//              XXXXxxxxxxxx (12 bits) : sample 'period'
//              ZZZZzzzzzzzz (12 bits) : effect and argument
    	
    	new_pattern=0;
    	new_row=0;
    	effect_jump=0;
    		    		
	for (i=0;i<mod_channels_num;i++) {    		
			    
    	
    		mod_channels[i]->arpeggio_on=false;
    		
		instr= (mod_pattern->byte1 & 0xF0)|((mod_pattern->byte3 & 0xF0) >> 4);
		period=((mod_pattern->byte1 & 0x0F) << 8) | mod_pattern->byte2;
		    effect=(mod_pattern->byte3&0x0F);
		    effect_arg=(mod_pattern->byte4);
		    
			    					  
			if (!instr)  instr=mod_channels[i]->instr + 1;
   
    		
			if (period) {
				period=calc_period(mod_instrument[instr-1].finetune,period);
    			
    			mcpy_size=sizeof(output_channel);
    			mcpy_src=(u8 *)&mod_instrument[instr-1];
    			mcpy_dst=(u8 *)mod_channels[i];
    				
    			while (mcpy_size--) *mcpy_dst++=*mcpy_src++;
    			mod_channels[i]->increment=freqtable[period];
    			mod_channels[i]->period=period;
   			}
    		    		
			// effect processing   -------------------------------------------------- 
			// effect , effect_arg = xxxx yyyy
    		switch (effect) {     
		    	case 0xF:	if (effect_arg<=0x19) {	// speed change effect ------------------
		    				mod_speed=effect_arg;
		    				mod_next_change=effect_arg;
		    			}
			case 0xC:	// volume change effect ----------------- xxxxyyyy = 0-64
					if (effect_arg>=MAX_VOLUME) effect_arg=MAX_VOLUME-1;  // (0-64) -> (0-63)
					mod_channels[i]->volume=effect_arg;						  	    			
					break;
		    			  
		    	case 0xD:	// pattern break effect ----------------- xxxx*10 + yyyy
					new_row=((effect_arg&0xf0)>>4)*10+(effect_arg&0x0f);
		    			new_pattern=mod_pattern_n+1;
		    			effect_jump=true;
		    			break;
				
			case 0xB:	// pattern table jump effect ------------
					new_row=0;
					new_pattern=effect_arg;						  
					effect_jump=true;
					break;
			case 0x0:	// arpeggio effect ---------------------- xxxx yyyy
						
					mod_channels[i]->arpeggio_table[0]=mod_channels[i]->increment;

					mod_channels[i]->arpeggio_table[1]= freqtable [
					calc_arpeggio_period(mod_channels[i]->finetune,
					mod_channels[i]->period,
					(0xf0&effect_arg)>>4)];
					mod_channels[i]->arpeggio_table[2]= freqtable [
					calc_arpeggio_period(mod_channels[i]->finetune,
					mod_channels[i]->period,
					(0xf&effect_arg))];						  						 
					mod_channels[i]->arpeggio_on=1;
					break;						  						 

		}

    		mod_pattern++;  //move the pointer to the next channel in the pattern 
    		
    	}
    
    	if (effect_jump) 
    		mod_do_pattern_break(new_row,new_pattern);
    	else {
    		mod_pattern_row++; //one pattern line executed
	        if (mod_pattern_row>63)
			mod_do_pattern_break(0,mod_pattern_n+1); // to line 0
        }
    	
    }
    
    // arpeggio processing
    
    for (i=0;i<mod_channels_num;i++) {
    	
    	if (mod_channels[i]->arpeggio_on) 
    		mod_channels[i]->increment=mod_channels[i]->arpeggio_table[mod_arpeggio_pos];
	}
    
    if (++mod_arpeggio_pos>=3) mod_arpeggio_pos=0;

}	
	

__inline void mix_channel(output_channel *channel, s16 *buffer_out) {
	
	u32 position, increment,loopend;
	u32 pos, loopstart, length, loop;
	u8 volume;
	s8 *sample;

    		position=channel->position;	   //copy channel parameters to local variables
    		increment=channel->increment;
    		loopend=channel->loopend;
    		loopstart=channel->looppos;
    		loop=channel->loopsize;
    		length=channel->length;
    		sample=channel->sample;         	volume=channel->volume;

    		for (pos=0;pos<buffer_half_size;pos++) {               	        		
                *buffer_out +=
                     volume_table[volume]
                     [(u8)(sample[position>>16])];
                buffer_out+=2;
		//take the integer part of the fixed point
                         
                position+=increment; //increment the channel sample position
                                
                if (position >= length) { // loop or not.......
                	if (loop) {
                		position=loopstart;  //restart to loop start position
                		length=loopend;      //loop playback set to loopend
                	} else { 
                		channel->playing=false; //we are'nt looping... stop the channel playback
                		pos=buffer_half_size;   // exit the channel mixing loop "for"
                	}
                }
        }
           	 
       	channel->position=position;	 //write back the changed parameters of the channel
	channel->length=length;
}	

__inline void mix_sfx_channel(sfx_channel *channel, s16 *buffer_out) {
	
	u32 position, increment,loopend;
	u32 pos, loopstart, length, loop;
	u8 volume;
	s8 *sample;

    		position=channel->position;	   //copy channel parameters to local variables
    		increment=channel->increment;
    		loopend=channel->loopend;
    		loopstart=channel->looppos;
    		loop=channel->loopsize;
    		length=channel->length;
    		sample=channel->sample;         	volume=channel->volume;

    		for (pos=0;pos<buffer_half_size;pos++) {               	        		
               
                *buffer_out++ +=                       //pan should be done...
                     volume_table[volume]
                     [(u8)(sample[position>>16])];
                *buffer_out++ +=
                     volume_table[volume]
                     [(u8)(sample[position>>16])];
                     
		//take the integer part of the fixed point
                         
                position+=increment; //increment the channel sample position
                                
                if (position >= length) { // loop or not.......
                	if (loop) {
                		position=loopstart;  //restart to loop start position
                		length=loopend;      //loop playback set to loopend
                	} else { 
                		channel->playing=false; //we are'nt looping... stop the channel playback
                		pos=buffer_half_size;   // exit the channel mixing loop "for"
                	}
                }
        }
           	 
       	channel->position=position;	 //write back the changed parameters of the channel
	channel->length=length;
}	




/////////////////////////////////////////////////////////////////////////////
//interrupt_mixer()
//  
//		process a tick of the modplay, it has to be called once every 1/50 of second
//     (by mean)
//
//		Usually hooked to a timer, it depends on play_start
//

static volatile int already_mixing=0;
static volatile int used_time=0;

void interrupt_mixer(void) {

	int channel_n;		

	int playing_buffer;
	s16 *writeTo;
	int i;
	
	if (already_mixing) return;   // a lock for not to enter twice the mixin routine
	already_mixing=1;
	

	playing_buffer=(*play_pos-(int)buffer)/BUFFER1_SIZE;
	
	
	while(writting_buffer!=playing_buffer) {
   //    	 *(volatile unsigned long*)0x14A007fc^=0xffffffff; //** the noise dots where because of this

   //	__printf("interrupt_mixer, writting_buffer=0x%x\n",writting_buffer);
		
   //	 	used_time=GpTickCountGet();
		
		writeTo=(s16 *)(((unsigned int)buffer)+writting_buffer*BUFFER1_SIZE);
	//clear the L&R buffers before mixing channels on them
    
    	gp_str_func.memset(writeTo,0,BUFFER1_SIZE);
	//memset(writeTo,0,BUFFER1_SIZE);

    
		for(channel_n=0;channel_n<max_channels;channel_n++) {    //MIX the L & R channels of the module

    		if (channels_L[channel_n].playing)     {	

    			mix_channel(&(channels_L[channel_n]), (s16 *)(writeTo));
    		}
    		if (channels_R[channel_n].playing)
    			mix_channel(&(channels_R[channel_n]), (s16 *)(writeTo+1));

		}
		
		for (channel_n=0;channel_n<max_sfx_channels;channel_n++) {  // MIX the sfx channels
		
			if(sfx_channels[channel_n].playing) {
////****
				mix_sfx_channel(&(sfx_channels[channel_n]),(s16 *)(writeTo));

			}
				
		}
		
		for (i=0;i<BUFFER1_SIZE/2;i++) writeTo[i]=0x8000+writeTo[i];

    	if (mod_playing) mod_player();
 
	
    	//** *(volatile unsigned long*)0x14A007fc^=0xffffffff;  //** the noise dots again
    	
    	writting_buffer=(writting_buffer+1)%NUMBUFFERS; //advance for the next playing buffer.
    	
//    	used_time=GpTickCountGet()-used_time;
    	
        }
        already_mixing=0;
        
}


/////////////////////////////////////////////////////////////////////////////
//switch_off_all_channels(void) 
//  
// Switches off all the playing channels.
//
void switch_off_all_channels(void) {
	
	int i;
		
	for(i=0;i<max_channels;i++) {  //switch off channels
    		channels_R[i].playing=FALSE; 
	    	channels_L[i].playing=FALSE; 
	}	
}


/////////////////////////////////////////////////////////////////////////////
//switch_off_all_sfx_channels(void) 
//  
// Switches off all the playing sfx channels.
//
void switch_off_all_sfx_channels(void) {
	
	int i;
		
	for(i=0;i<max_sfx_channels;i++) {  //switch off channels
    		sfx_channels[i].playing=FALSE;  
	}	
}


void sound_init(void) {
	
	int i,volume;
	
	// GP32 initialization
	
	GpPcmInit ( PCM_S44, PCM_16BIT );

	
	buffer_size=BUFFER1_SIZE*NUMBUFFERS;
	
	buffer=(u16 *)gp_mem_func.zimalloc(buffer_size);
	//gp_str_func.memset(buffer,0,buffer_size);
	//memset(buffer,0,buffer_size);
		
	writting_buffer=2;
	
	// modplayer inicialization
	
	for(i=1;i<4096;i++) 
		freqtable[i]= ((7159091/samplerate)*65536) / (i<<1);
   	        
	switch_off_all_channels();	
	switch_off_all_sfx_channels();
    
	for (volume=0;volume<MAX_VOLUME;volume++) { //build volume table
    		for (i=-128;i<128;i++)
		    	volume_table[volume][((u8)i)]=(((i/*+0x80*/)*volume)<<(8-6))/max_channels ; // unsigned= +0x80
	}

	/* start the PCM playing loop... using the SDK */

}

void start_sound() {
	GpPcmPlay((unsigned short *)buffer,buffer_size,1);
    	GpPcmLock((unsigned short *)buffer,(int *)&play_chan,(unsigned int *)&play_pos);	
}

void stop_sound() {
		GpPcmRemove((unsigned short*)buffer);
}


u32 big_e_32 (u32 big_e) {
	
		return  (big_e<<24) |
		((big_e<<8)&0x00ff0000) |
		((big_e>>8) &0x0000ff00) |
		(big_e>>24);
		 
	
}
u16 big_e_16 (u16 big_e) {
	
	return	(big_e<<8) |
		(big_e>>8);
		 
	
}

static int timer_used;

/////////////////////////////////////////////////////////////////////////////
//int sfx_play(s8 *sfx_data, int lenght, float rate, s8 paning, float pan_increment_50th, u32 volume);
//  
//	starts playing a sfx
//	
//	sfx_data = pointer to 8bit waveform data
//  lenght = lenght of the waveform data
//  rate = speed to play   1.0 = normal speed  , 0.5 = half speed... 2.0 = double speed... and so on
//  paning = -127 means left, 0 means center, 127 means right
//  pan_increment_50th = increment for the pan variable each 1/50 of second
//  volume = 0..63  the volume of the waveform
//
//  returns: the channel number where the sfx will be playing
//

int sfx_play(s8 *sfx_data, int length, float rate, s8 paning, float pan_increment_50th, u32 volume) {

	int channel_allocated;
	int chan;
	
	
	channel_allocated=-1;  // search for a free channel
	
	for(chan=0;(chan<max_sfx_channels)&(channel_allocated!=1);chan++)
		if (!sfx_channels[chan].playing) channel_allocated=chan;
		
		
	if (volume>63) volume=63;
	
	if (channel_allocated!=-1) {
	
		sfx_channels[channel_allocated].playing = false;
		sfx_channels[channel_allocated].sample = sfx_data;
		sfx_channels[channel_allocated].position = 0;
		sfx_channels[channel_allocated].increment = (int) (rate*65536.0);
		sfx_channels[channel_allocated].length = length;
		sfx_channels[channel_allocated].volume = volume;
		sfx_channels[channel_allocated].paning = paning<<16;
		sfx_channels[channel_allocated].pan_increment = (int) (pan_increment_50th * 65536.0);
		sfx_channels[channel_allocated].age = sfx_oldest_age++;
		sfx_channels[channel_allocated].loopsize = 0;
		sfx_channels[channel_allocated].playing = true;
		
	}
	
	return channel_allocated;
}

/////////////////////////////////////////////////////////////////////////////
//int sfx_stop(int channel)
//
// stops playing a sfx channel
//
// returns: true / false ... if it was stopped or not
//

int sfx_stop(int channel) {
	
	if ((channel>=0)&&(channel<=max_sfx_channels)) {
		
		if (sfx_channels[channel].playing) {
			
			sfx_channels[channel].playing=false;
			return true;
		} else return false;
	
	} else return false;

}
/////////////////////////////////////////////////////////////////////////////
//int sfx_setpan(int channel, s8 paning)
//
// set the panning for a sfx channel
//
// returns: true / false  ... if it was set or not
//

int sfx_setpan(int channel, s8 paning){
	
	if ((channel>=0)&&(channel<=max_sfx_channels)) {
		
		if (sfx_channels[channel].playing) {
			
			sfx_channels[channel].paning=paning<<16;
			return true;
		} else return false;
	
	} else return false;

}


/////////////////////////////////////////////////////////////////////////////
//int sfx_setvolume(int channel, s8 paning)
//
// set the volume for a sfx channel
//
// returns: true / false  ... if it was set or not
//

int sfx_setvolume(int channel, int volume){
	
	if (volume<0) volume=0;
	if (volume>63) volume=63;
	
	if ((channel>=0)&&(channel<=max_sfx_channels)) {
		
		if (sfx_channels[channel].playing) {
			
			sfx_channels[channel].volume=volume;
			
			return true;
		} else return false;
	
	} else return false;

}



/////////////////////////////////////////////////////////////////////////////
//module_play(use_timer,timer_num)
//  
//	starts playing the module loaded with module_load,
//	
// use_timer = {true,false}    = hook a timer to interrupt_mixer
// timer_num				   = which timer to hook on..

void module_play(int use_timer,int timer_num) {
	
	mod_speed=6;
	mod_next_change=6;
	mod_playing=true;
	mod_pattern_row=0;
	mod_pattern_n=0;
	mod_pattern = mod_patterns + (mod_pattern_list[0]<<8);
	
	//start a 50 tps timer for mixing ticks
	if (use_timer) {
		//__printf("TB: Memory Available:%d\n", gp_mem_func.availablemem());
         	GpTimerOptSet(timer_num, 50/*55*/, 0, interrupt_mixer);
         	GpTimerSet(timer_num);
		//__printf("TE: Memory Available:%d\n", gp_mem_func.availablemem());
		timer_used=timer_num+1;
	} else timer_num=0;

	
}

void module_stop(){
	
	mod_playing=false;
	switch_off_all_channels();
		
	memset(buffer,0,buffer_size); //** clear the buffer instead of freeing it
	
	if (timer_used)           
	{
		GpTimerKill(timer_used-1);
	}

}



int module_load(u8 *module) {
		
	int i;
	mod_general_info *mod_info;
	u8 *mod_sampledata;
	
	//__printf("inside module_load(module=0x%x)\n",module);	

	mod_info = (mod_general_info *)module;
	mod_patterns = (mod_channel_info *) (module + sizeof(mod_general_info));

	if ((mod_info->song_id[0]!='M')||
		(mod_info->song_id[1]!='.')||
		(mod_info->song_id[2]!='K')||
		(mod_info->song_id[3]!='.')) {
	
		return false;
	}

	mod_channels_num=4;

	mod_patterns_n=0;
	
	for(i=0;i<mod_info->played_patterns;i++)
		if(mod_info->pattern_list[i]>mod_patterns_n) 
			mod_patterns_n=mod_info->pattern_list[i];
	
	mod_sampledata = (u8 *) mod_patterns + (mod_patterns_n+1) * mod_pattern_4_size;  // fixed 4chan size (by now..)
	
	mod_info->song_name[19]=0;

	
	for (i=0;i<31;i++) {
		
		//__printf("inside module_load(), ins=%d\n",i);	
	
		mod_instrument[i].sample= (s8 *) mod_sampledata;
		mod_sampledata+=big_e_16(mod_info->instruments[i].size)*2; //move pointer to next instrument
		mod_instrument[i].position=0;
		mod_instrument[i].length=big_e_16(mod_info->instruments[i].size)*2<<16;
		mod_instrument[i].loopsize=(big_e_16(mod_info->instruments[i].loopsize)-1)*2<<16;
		mod_instrument[i].looppos=big_e_16(mod_info->instruments[i].loopstart)*2<<16;
/*		
		__printf("ins=%d len=%d loopsize=%d looppos=%d\n",i,mod_instrument[i].length>>17,
		mod_instrument[i].loopsize>>17,
		mod_instrument[i].looppos>>17);
*/
		mod_instrument[i].loopend=mod_instrument[i].looppos+mod_instrument[i].loopsize;

		
		mod_instrument[i].volume=(mod_info->instruments[i].vol);
		if (mod_instrument[i].volume>=MAX_VOLUME) mod_instrument[i].volume=MAX_VOLUME-1;
		
		mod_instrument[i].finetune=mod_info->instruments[i].finetune;
		mod_instrument[i].playing=true;
		mod_instrument[i].instr=i;
		
		

	}
	
	mod_pattern_list=(u8 *)mod_info->pattern_list;
	mod_restart_pattern = mod_info->restart_pattern;
	mod_played_patterns = mod_info->played_patterns;		
	if (mod_restart_pattern!=127) mod_restart_pattern--;
	
	//__printf("restart:%d played:%d\n",mod_restart_pattern,mod_played_patterns);
	
	return true;
	
}


int module_get_info(int what) {
	int i,n;
	switch (what) {
		case MODINFO_MAX_CHANNELS: 
					return max_channels;
					
		case MODINFO_USED_TIME: 
					while(already_mixing); //wait for the barrel
					return used_time;
					
		case MODINFO_NCHANNELS_ACTIVE: 
					n=0;
					for (i=0;i<max_channels;i++) {
						if (channels_L[i].playing) n++;
						if (channels_R[i].playing) n++;
					}
					return n;
					
		case MODINFO_CHANNEL_PLAYING+0: return channels_L[0].playing;
		case MODINFO_CHANNEL_PLAYING+1: return channels_R[0].playing;
		case MODINFO_CHANNEL_PLAYING+2: return channels_L[1].playing;
		case MODINFO_CHANNEL_PLAYING+3: return channels_R[1].playing;
#if max_channels>2
		case MODINFO_CHANNEL_PLAYING+4: return channels_L[2].playing;
		case MODINFO_CHANNEL_PLAYING+5: return channels_R[2].playing;
#if max_channels>3
		case MODINFO_CHANNEL_PLAYING+6: return channels_L[3].playing;		
		case MODINFO_CHANNEL_PLAYING+7: return channels_R[3].playing;
#endif
#endif		
		case MODINFO_PATTERN_PLAYING:return mod_pattern_n;
		case MODINFO_ROW_PLAYING:return mod_pattern_row;
				
	}
	
	return 0;
}
	

// Code don't really needed, only for loading .gmod file... a special kind of .mod file
/*	
int module_load_gmod(u8 *module) {
		
	int i;
	u8 *mod_sampledata;
	TGBA_MODULE *gmod_info;
 	TGBA_INSTRUMENTS *gmod_instruments;

	gmod_info = (TGBA_MODULE *) module;
			
	mod_patterns = (mod_channel_info *) (module + sizeof(TGBA_MODULE) - (128-gmod_info->n_patterns_played));
	mod_patterns_n=gmod_info->n_patterns_defined;
	mod_channels_num=gmod_info->n_channels;
	
	gmod_instruments= (TGBA_INSTRUMENTS *) 
			(((u8 *)mod_patterns) + (mod_patterns_n) * mod_channels_num * 4 * 64);
	
	mod_sampledata=(u8 *)&gmod_instruments[gmod_info->n_instruments];
	
	for (i=0;i<gmod_info->n_instruments;i++) {  //build instrument table, set volumes to 0-63(max)
		
		
		mod_instrument[i].sample= (s8 *) mod_sampledata;
		mod_sampledata+=gmod_instruments[i].samplesize; //move pointer to next instrument
		mod_instrument[i].position=0;
		mod_instrument[i].length=gmod_instruments[i].samplesize<<16;
		mod_instrument[i].loopsize=(gmod_instruments[i].loopsize-2)<<16;
		mod_instrument[i].looppos=gmod_instruments[i].loopstart<<16;
		mod_instrument[i].loopend=mod_instrument[i].looppos+mod_instrument[i].loopsize;
		
		mod_instrument[i].volume=gmod_instruments[i].volume;
		if (mod_instrument[i].volume>=MAX_VOLUME) mod_instrument[i].volume=MAX_VOLUME-1;
		
		mod_instrument[i].finetune=gmod_instruments[i].finetune;
		mod_instrument[i].playing=true;
		mod_instrument[i].instr=i;

	}

	mod_pattern_list=(u8 *)gmod_info->pattern_list;
	mod_restart_pattern = gmod_info->restart_pattern;
	mod_played_patterns = gmod_info->n_patterns_played;

	return true;
	
	
}

*/			