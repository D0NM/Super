/* 
   $Date: 2002/04/01 01:27:49 $  
   $Id: gbamod.h,v 1.16 2002/04/01 01:27:49 sla Exp $ 

 Include File for GBA music module file type (GMOD FILES)

*/

#ifndef __GBAMOD_H
#define __GBAMOD_H

#include "modtypes.h"

// definitions for GBA compatibility....
#if defined (__arm) || defined (__thumb)

#define packing 	__packed

#else 
  // for the PC...... no "__packed" supported (ARM SDT stuff..) :)
#define packing 

#endif

// the structures

typedef packing struct {
    SBYTE sampledata[65536];
} TGBA_SAMPLES;

typedef packing struct {
    WORD samplesize;
    SBYTE finetune;
    BYTE volume;
    WORD loopstart;
    WORD loopsize;
} TGBA_INSTRUMENTS;

typedef packing struct {
    // As in MOD files
	BYTE byte1;
    BYTE byte2;
	BYTE byte3;
	BYTE byte4;
} TGBA_CHANNELS;

typedef packing struct {
    TGBA_CHANNELS channels[8];
} TGBA_LINES;

typedef packing struct {
    TGBA_LINES lines[64];
} TGBA_PATTERNS;

typedef packing struct {
    BYTE tempo;											// Initial tempo (128 in mods)
    BYTE speed;											// Initial Speed (6 in mods)
    BYTE n_channels;									// Number of module channels
    BYTE n_patterns_defined;							// Number of patterns defined, 0-63
    BYTE n_patterns_played;								// Number of patterns played, 0-128
    BYTE n_instruments;									// Number of instruments
	BYTE restart_pattern;								// pattern where to start from when finished
    BYTE pattern_list[128];								// List of patterns played, max 
#ifndef __gba    
    TGBA_PATTERNS pattern_table[64];					// Table of used patterns
    TGBA_INSTRUMENTS instruments_table[31];				// Table of used instruments
	TGBA_SAMPLES sample_table[31];						// Table of sample data
#endif
} TGBA_MODULE;

#endif