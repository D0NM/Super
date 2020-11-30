   /////////////////////////////////
  // GBALib - Mod Module Header  //
 // Aj0 & Sla : God's Maze 2002 //
/////////////////////////////////
// $Date: 2002/03/31 20:39:43 $  
// $Id: mod.h,v 1.5 2002/03/31 20:39:43 ajo Exp $ 

  /////////////
 // Structs //
/////////////

#ifndef __mod_h
#define __mod_h

#include "modtypes.h"

typedef __packed struct {
	char name[22];	// Instrument name
	unsigned short int size;		// Instrument size
	unsigned char finetune;	// We will use 4 lower bits (-8..8)
	unsigned char vol;			// First instrument main vol (0-64)
	unsigned short int loopstart;
	unsigned short int loopsize;
} mod_instrument_info;

typedef __packed struct {
	char song_name[20];		// Module name
	mod_instrument_info instruments[31]; // Instruments info
	unsigned char played_patterns;		// Number of played patterns
	unsigned char restart_pattern;     // pattern where to start from when finished
	unsigned char pattern_list[128];	// (0-63)->pattern for playing
	char song_id[4];		// M.K. o FLT4 = 4 chan (6CHN, 8CHN)
} mod_general_info;

//       Byte  1   Byte  2   Byte  3   Byte 4
//              --------- --------- --------- ---------
//              7654-3210 7654-3210 7654-3210 7654-3210
//              wwww XXXX xxxxxxxxx yyyy ZZZZ zzzzzzzzz
//
//                  wwwwyyyy ( 8 bits) : sample number
//              XXXXxxxxxxxx (12 bits) : sample 'period'
//              ZZZZzzzzzzzz (12 bits) : effect and argument

typedef __packed struct {
	
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
//	u32 effect_arg:8;
//	u32 effect_num:4;
//	u32 instr_number_l:4;
//	u32 period:12;
//	u32 instr_number_h:4;
} mod_channel_info;

#define mod_pattern_4_size  4*4*64;
#define mod_pattern_6_size  6*4*64;
#define mod_pattern_8_size  8*4*64;

#endif