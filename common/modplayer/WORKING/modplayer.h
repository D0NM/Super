   /////////////////////////////////
  // 		Module player Header //
 // Aj0 & Sla : Gods' Maze 2002 //
/////////////////////////////////

// $Date: 2002/04/02 01:17:18 $  
// $Id: modplayer.h,v 1.11 2002/04/02 01:17:18 ajo Exp $ 

#ifndef __modplayer_h
#define __modplayer_h

#include "mod.h"
#include "gbamod.h"

  ////////////////
 // Structures //
////////////////
typedef struct  {
	
	s8 *sample;
	u32 position;  // fixed point 16 | 16
	u32 looppos;   // fixed point 16 | 16 
	u32 loopend;   // fixed point 16 | 16
	u32 loopsize;  // fixed point 16 | 16
	u32 increment; // fixed point 16 | 16
	u32 length;    // fixed point 16 | 16
	u32 volume;	   // 0-63
	int playing;	
	s8 finetune;
	u8 instr;
	
	u32 period;
	u32 arpeggio_table[3];
	u8 arpeggio_pos;
	u8 arpeggio_on;

} output_channel;


typedef struct  {
	
	s8 *sample;
	u32 position;  // fixed point 16 | 16
	u32 looppos;   // fixed point 16 | 16 
	u32 loopend;   // fixed point 16 | 16
	u32 loopsize;  // fixed point 16 | 16
	u32 increment; // fixed point 16 | 16
	u32 length;    // fixed point 16 | 16
	u32 volume;	   // 0 - 63
	s32 paning;
	s32 pan_increment;
	u32 age;	   // to see what's the oldest sound if we need to take out one.

	int playing;	
	
	

} sfx_channel;



  ///////////////
 // Functions //
///////////////
void interrupt_process(void);
void sound_init(void);


  ///////////////////
 // MOD Functions //
///////////////////

int module_load(u8 *module);
/////////////////////////////////////////////////////////////////////////////
// initialize the structures to play the module


int module_load_gmod(u8 *module);
/////////////////////////////////////////////////////////////////////////////
// the same as module_load but for gmod's


void module_play(int use_timer,int timer_num);
/////////////////////////////////////////////////////////////////////////////
// start playing the module loaded 


void module_stop(void);
/////////////////////////////////////////////////////////////////////////////
// stop playing the module



int module_get_info(int what);
/////////////////////////////////////////////////////////////////////////////
// get info about the player
//
// example:   module_get_info(MODINFO_CHANNEL_PLAYING + channel_number)

#define MODINFO_MAX_CHANNELS 		0
#define MODINFO_USED_TIME    		1
#define MODINFO_NCHANNELS_ACTIVE	2
#define MODINFO_CHANNEL_PLAYING    	3
#define MODINFO_PATTERN_PLAYING    	11
#define MODINFO_ROW_PLAYING			12

#endif


  ///////////////////
 // SFX Functions //
///////////////////
 
int sfx_play(s8 *sfx_data, int lenght, float rate, s8 paning, float pan_increment_50th, u32 volume);
int sfx_stop(int channel);
int sfx_setpan(int channel, s8 paning);

void start_sound(void);
void stop_sound(void);



