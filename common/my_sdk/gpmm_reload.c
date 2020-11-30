#include "gpdef.h"
#include "gpos_internal.h"
#include "gpstdlib.h"
#include "gpmm.h"

#include "gpmm_internal.h"

#include "snd_option.h"

volatile int _snd_srcexist;
SNDMIXER	_sndmixer[MAX_PCMCHANNEL];
unsigned short *_sndmixedbuf[2];
volatile int _pcm_workidx;

extern void swi_set_sndbuffer(unsigned short **, int);
extern int _GpPcmMixingStereo16(unsigned short *, SNDMIXER*,int);
extern int _GpPcmMixingMono16(unsigned short *, SNDMIXER*,int);
extern int _GpPcmMixingStereo8(unsigned short *, SNDMIXER*,int);
extern int _GpPcmMixingMono8(unsigned short *, SNDMIXER*,int);
void _fill_src8_buf(SNDMIXER *mixer, int size);
void _fill_src16_buf(SNDMIXER *mixer, int size);
int (*_pcm_mixing_func)(unsigned short *, SNDMIXER *, int) = NULL;
void (*_pcm_fill_src)(SNDMIXER *mixer, int size) = NULL;

typedef struct tag_GPPCM_ENV{
	int b_initialized;
	PCM_SR sr;
	PCM_BIT bit;
	int real_sr;
}_GPPCM_ENV;
_GPPCM_ENV _gppcm_env = {0, PCM_M11, PCM_8BIT, 11025};

struct tag_iis_info{
	int cmd;
	int addr_mode;
	unsigned int mem_addr;
	unsigned int isr_func;
	int src_bytes;
	int sr_rate;
	int reload_flag;
};
#define IIS_CMD_TX_ON	0x1
#define IIS_CMD_TX_OFF	0x2
#define IIS_CMD_CLK_ON	0x4
#define IIS_CMD_CLK_OFF	0x8
#define IIS_CMD_RELOAD_ON	0x10
#define IIS_CMD_RELOAD_OFF	0x20
#define IIS_CMD_ADDR_SET	0x40
#define IIS_CMD_SRC_COUNT	0x80
#define IIS_CMD_SRC_SET		0x100
#define IIS_CMD_HW_INIT		0x200
#define IIS_CMD_INT_DIS		0x400
#define IIS_CMD_INT_EN		0x800
#define IIS_CMD_DFT_SET		0x1000
#define IIS_CMD_STOP_SET	0x2000
#define IIS_CMD_PLAY_SET	0x4000
#define IIS_CMD_ENV_SET		0x8000

int GpPcmEnvGet(PCM_SR * p_sr, PCM_BIT * p_bit_count, int * p_real_sr)
{
	if ( !_gppcm_env.b_initialized ) return GPC_EPCM_NO_INIT;
	*p_sr = _gppcm_env.sr;
	*p_bit_count = _gppcm_env.bit;
	*p_real_sr = _gppcm_env.real_sr;
	return GPC_EPCM_OK;
}

void GpSoundThreadInit(void)
{
	GP_THREAD * tmp_thread;
	
	ARMDisableInterrupt();
	tmp_thread = &_gp_thread_list[0];
	tmp_thread->t_state = GPOS_STAT_BLOCKED;
	tmp_thread->stack_ptr = _os_init_thread_stack(GpProcSound, (unsigned int*)((char*)tmp_thread->init_stack_ptr + tmp_thread->stk_size));
	ARMEnableInterrupt();
}

void GpSoundThreadAct(void)
{
	unsigned char * s_ptr;
	ARMDisableInterrupt();
	_gp_ret_from_rt = _gp_cur_thread;
	_gp_highest_thread = &_gp_thread_list[0];
	s_ptr = (unsigned char*)_gp_highest_thread->init_stack_ptr;
	if ( !s_ptr )
	{
		_gp_highest_thread->stk_size = 2<<10;
		s_ptr = gp_mem_func.zimalloc(2<<10);
		_gp_highest_thread->init_stack_ptr = (void*)s_ptr;
	}
	s_ptr += _gp_highest_thread->stk_size;	//8byte align
	_gp_highest_thread->stack_ptr = _os_init_thread_stack(GpProcSound, (unsigned int*)s_ptr);			
	GpTaskSW();
	ARMEnableInterrupt();
}

int GpPcmPlay(unsigned short* src, int size, int repeatflag/*0 or 1*/)
{
	int i;
	
	if ( !_gppcm_env.b_initialized ) return GPC_EPCM_NO_INIT;
	
	ARMDisableInterrupt();
	for (i = 0 ; i < MAX_PCMCHANNEL ; i++)
	{
		if (_sndmixer[i].i_ptr == NULL)
		{
			_pcm_fill_src(&_sndmixer[i], size);
			_sndmixer[i].i_ptr = src;
			_sndmixer[i].c_ptr = src;
			_sndmixer[i].repeatflag = repeatflag;
			break;
		}	
	}
	if (i == MAX_PCMCHANNEL)
	{
		ARMEnableInterrupt();
		return GPC_EPCM_FULL;
	}
	if (_snd_srcexist & _PCM_SRC_EXIST)
	{
		ARMEnableInterrupt();
		return GPC_EPCM_OK;
	}
	if ( !_snd_srcexist )
	{
		_snd_srcexist |= _PCM_SRC_EXIST;
		GpSoundThreadAct();
	}
	else
		ARMEnableInterrupt();
	return GPC_EPCM_OK;
}

int GpPcmInit(PCM_SR sr, PCM_BIT bit_count)
{
	int i;
	int retval;
	int samplerate;
	struct tag_iis_info _iis_info;
	int _channel_flag = 0;
	
	_gp_os_sched_lock();
	
	_iis_info.cmd = IIS_CMD_DFT_SET;
	swi_iis_operate(&_iis_info);
	
	for (i = 0; i < MAX_PCMCHANNEL; i++)
	{
		_sndmixer[i].i_ptr = _sndmixer[i].c_ptr = NULL;
		_sndmixer[i].i_ptr = _sndmixer[i].c_ptr = 0;
	}
	
	switch (sr)
	{
	case PCM_S11:
		_channel_flag = 1;
	case PCM_M11:
		samplerate = 11025;
		break;
	case PCM_S22:
		_channel_flag = 1;
	case PCM_M22:
		samplerate = 22050;
		break;
	case PCM_S44:
		_channel_flag = 1;
	case PCM_M44:
		samplerate = 44100;
		break;
	default:
		return 0;
	}
	
	_iis_info.cmd = IIS_CMD_ENV_SET | IIS_CMD_RELOAD_ON;

	_iis_info.addr_mode = 0;
	_iis_info.isr_func = (unsigned)_GpIsrSound;
	_iis_info.sr_rate = samplerate;
	retval = swi_iis_operate(&_iis_info);
	
	_pcm_mixing_func = NULL;
	_pcm_fill_src = NULL;
	if (_channel_flag)
	{
		if (bit_count == PCM_8BIT)
		{
			_pcm_mixing_func = _GpPcmMixingStereo8;
			_pcm_fill_src = _fill_src8_buf;
		}
		else if (bit_count == PCM_16BIT)
		{
			_pcm_mixing_func = _GpPcmMixingStereo16;
			_pcm_fill_src = _fill_src16_buf;
		}
		else
			retval = 0;
	}
	else
	{
		if (bit_count == PCM_8BIT)
		{
			_pcm_mixing_func = _GpPcmMixingMono8;
			_pcm_fill_src = _fill_src8_buf;
		}
		else if (bit_count == PCM_16BIT)
		{
			_pcm_mixing_func = _GpPcmMixingMono16;
			_pcm_fill_src = _fill_src16_buf;
		}
		else
			retval = 0;
	}
	
	_snd_srcexist = 0;
	swi_set_sndbuffer(_sndmixedbuf, SNDMIXING_UNIT<<1);
	
	_gp_os_sched_unlock();
	
	GpSoundThreadInit();
	
	if ( retval )
	{
		_gppcm_env.b_initialized = 1;
	}
	_gppcm_env.sr = sr;
	_gppcm_env.bit = bit_count;
	_gppcm_env.real_sr = retval;
	
	return retval;
}

void _fill_src8_buf(SNDMIXER *mixer, int size)
{
	mixer->i_length = size;
	mixer->c_length = size;
}

void _fill_src16_buf(SNDMIXER *mixer, int size)
{
	mixer->i_length = (size>>1);
	mixer->c_length = (size>>1);
}

void GpPcmStop(void)
{
	int i;
	struct tag_iis_info _iis_info;
	ARMDisableInterrupt();
	for ( i=0; i<MAX_PCMCHANNEL; i++ )
	{
		_sndmixer[i].i_ptr = NULL;
		_sndmixer[i].c_ptr = NULL;
	}
	_snd_srcexist = 0;
	
	_iis_info.cmd = IIS_CMD_STOP_SET;
	swi_iis_operate(&_iis_info);
	ARMEnableInterrupt();
}

void GpPcmRemove(unsigned short * src)
{
	int i,j;
	if ( !src ) return;
	ARMDisableInterrupt();
	for ( i=0,j=0; i<MAX_PCMCHANNEL; i++ )
	{
		if ( _sndmixer[i].i_ptr )
		{
			if ( _sndmixer[i].i_ptr == src )
			{
				_sndmixer[i].i_ptr = NULL;
				_sndmixer[i].c_ptr = NULL;
			}
			else
				j++;
		}
	}
	if ( !j )
	{
		_snd_srcexist = 0;
	}
	ARMEnableInterrupt();
}

int GpPcmLock(unsigned short * p_src, int * idx_buf, unsigned int * addr_of_playing_buf)
{
	int i;
	for(i = 0 ; i < MAX_PCMCHANNEL ; i++)
	{
		if(_sndmixer[i].i_ptr == p_src )
		{
			*idx_buf = i;
			*addr_of_playing_buf = (unsigned int)&(_sndmixer[i].c_ptr);
			return 1;
		}
	}
	return 0;
}

void GpPcmOnlyKill(unsigned short * p_src)
{
	int i;
	for(i = 0 ; i < MAX_PCMCHANNEL ; i++)
	{
		if( (_sndmixer[i].i_ptr) && (_sndmixer[i].i_ptr != p_src) )
		{
			GpPcmRemove(_sndmixer[i].i_ptr);
		}
	}
}

void GpProcSound(void * arg)
{
	int b_started;
	int mixed_size;
	struct tag_iis_info _iis_info;
	
	_pcm_workidx = 0;
	mixed_size = 0;
	
	b_started = 0;
	while ( 1 )
	{
		if ( b_started )
		{
			ARMDisableInterrupt();
			if ( _gp_ret_from_rt->t_id > 0 )
			{
				_gp_highest_thread = _gp_ret_from_rt;
				_gp_ret_from_rt = NULL;
				_gp_cur_thread->t_state = GPOS_STAT_BLOCKED;
			}
			else
			{
				_iis_info.cmd = IIS_CMD_SRC_COUNT;
				_iis_info.src_bytes = mixed_size << 1;
				swi_iis_operate(&_iis_info);
				//rDCON2 = (0x5<<28)|(0x9<<20)|(mixed_size);
				_gp_os_schedule();
			}
			GpTaskSW();			
			ARMEnableInterrupt();

			if ( !mixed_size )
			{
				_iis_info.cmd = IIS_CMD_TX_OFF;
				swi_iis_operate(&_iis_info);
				//rDMASKTRIG2 = 0x4;
			}
		}
		else if ( mixed_size )
		{
			ARMDisableInterrupt();
			_iis_info.cmd = IIS_CMD_PLAY_SET | IIS_CMD_RELOAD_ON;
			swi_iis_operate(&_iis_info);
			/*
			rDMASKTRIG2=0x2;
			
			rIISCON = 0x2e;
			rIISFCON = 0xa00;
			rIISCON = 0x27;
			*/
			ARMEnableInterrupt();
			
			b_started = 1;
		}
		
		if ( _snd_srcexist )
		{
			mixed_size = _pcm_mixing_func(_sndmixedbuf[_pcm_workidx], _sndmixer, SNDMIXING_UNIT);
			if ( !mixed_size )
			{
				_snd_srcexist = 0;
				_iis_info.cmd = IIS_CMD_RELOAD_OFF;
				swi_iis_operate(&_iis_info);
			}
			else
			{
				_iis_info.cmd = IIS_CMD_SRC_COUNT | IIS_CMD_SRC_SET;
				_iis_info.mem_addr = (unsigned int)_sndmixedbuf[_pcm_workidx];
				_iis_info.src_bytes = (mixed_size<<1);
				swi_iis_operate(&_iis_info);
				_pcm_workidx++;
				_pcm_workidx &= 0x1;
			}
		}
		else
		{
			ARMDisableInterrupt();
			mixed_size = 0;
			_iis_info.cmd = IIS_CMD_STOP_SET;
			swi_iis_operate(&_iis_info);
			/*
			rDMASKTRIG2 = 0x4;
			rIISCON = 0x0e;
			rIISFCON = 0x00;
			*/
			if ( _gp_ret_from_rt->t_id > 0 )
			{
				_gp_highest_thread = _gp_ret_from_rt;
				_gp_ret_from_rt = NULL;
				_gp_cur_thread->t_state = GPOS_STAT_NO_OPER;
			}
			else
			{
				_gp_os_schedule();
			}
			GpTaskSW();
			ARMEnableInterrupt();
		}
	}
}

void GpMidiPlay(unsigned char * midisrc,int repeatcount)
{
}

void GpMidiListPlay(unsigned char ** srclist,int listcount)
{
}

void GpMidiStop(void)
{
}

void GpMidiPause(void)
{
}

void GpMidiReplay(void)
{
}

int GpMidiStatusGet(int * played)
{
	return 0;
}
