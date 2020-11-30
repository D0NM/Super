#ifndef __gpmm_internal_h__
#define __gpmm_internal_h__

#define _PCM_SRC_EXIST	0x01
#define _MIDI_SRC_EXIST	0x02
#define MAX_PCMCHANNEL  	4
#define SNDMIXING_UNIT		96
typedef struct tagSNDMIXER{
	unsigned short * i_ptr;
	unsigned short * c_ptr;
	int i_length;
	int c_length;
	int repeatflag;
}SNDMIXER;
extern SNDMIXER	_sndmixer[MAX_PCMCHANNEL];

#endif