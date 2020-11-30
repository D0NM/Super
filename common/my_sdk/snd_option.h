#ifndef __option_h__
#define __option_h__

#define CODEC_MOSA		0
#define CODEC_TDA1387	1
#define CODEC_UDA1330	2
#define ID_SOUND_CODEC	CODEC_UDA1330							/* SELECTION */

#define USE_MSB_32FS	0x0
#define USE_MSB_48FS	0x1
#define USE_IIS_32FS	0x2
#define USE_IIS_48FS	0x4

#if (ID_SOUND_CODEC == CODEC_TDA1387 )
	#define USE_384FS_ONLY
	#define ID_CODEC_SET	USE_IIS_48FS						/* SELECTION */
#else
	#if ( ID_SOUND_CODEC == CODEC_MOSA )
		#define ID_CODEC_SET	USE_MSB_32FS						/* SELECTION */
	#else
		#define ID_CODEC_SET 	USE_IIS_32FS						/* SELECTION */
	#endif
#endif

#if (ID_SOUND_CODEC == CODEC_UDA1330)
	#define CHANGE_UDA_VOLUME	1								/* SELECTION */
	#if (CHANGE_UDA_VOLUME )
		#define VALUE_UDA_VOLUME	0
	#endif
#endif

#endif