#ifndef	__GPFINTERNAL_H__
#define	__GPFINTERNAL_H__

#define	_KSUNISIZE	2350
#define	_KSUNIEXSIZE	94
#define	_MAXJASO_FIRST	19
#define	_MAXJASO_MID	21
#define	_MAXJASO_LAST	27

#define _PACKOFF_JM    (_MAXJASO_FIRST<<3)
#define _PACKOFF_JL    (_PACKOFF_JM+(_MAXJASO_MID<<2))

typedef struct tag_BGFONT{
     unsigned char * dest;
     int dx;
     int dy;
     int width;
     int height;
     unsigned char * sour;
     int sx;
     int sy;
     int srcpitch;
     int tgpitch;
     int foregnd;
}_BGFONT;

#endif