/*****************************************************************************************************
* GP32 SDK Version 1.1 header file : gpfont.h                                                        *
*		The latest patch : 28,June,2001                                                            *
*		SDK Developer : Jstar, Achi                                                                *
*		related library : gpfont.alf                                                               *
*		COPYRIGHT DESCRIPTION														 *
*			The copyright of the source is reserved by GAMEPARK,Inc!!!                            *
*****************************************************************************************************/

#ifndef __GPFONT_H__
#define	__GPFONT_H__

#include "gpdef.h" 
#include "gpgraphic.h"

/* CAUTION 
	target\gpfont_port.h 의 FONT_CHARGAP, FONT_LINEGAP 의 단위는 percentage 이다
	가령 KORFONT_W 가 ENGFONT_W보다 크다면 다음과 같다.
	BGFONTINFO bgfontinfo;
	bgfontinfo.chargap = (kor_w * FONT_CHARGAP) / 100;
*/	
typedef struct tagBGFONTINFO{
	int kor_w;			/* 2byte character (korean) pixel*/
	int kor_h;			/* 2byte character (korean) pixel*/
	int eng_w;			/* 1byte character (english) pixel*/
	int eng_h;			/* 1byte character (english) pixel*/
	int chargap;		/* 문자 사이의 간격 pixel */	
	int linegap;		/* 줄 간격 pixel */
}BGFONTINFO;

void GpFontInit(BGFONTINFO * ptr);

void GpFontResSet(unsigned char * p_kor, unsigned char * p_eng);
unsigned char * GpKorFontResGet(void);
unsigned char * GpEngFontResGet(void);

void GpSysFontGet(BGFONTINFO *fInfo);
int GpTextWidthGet(const char * lpsz);
int GpTextHeightGet(const char * lpsz);
int GpTextLenGet(const char * str);

void GpTextOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y,
              char * sour, unsigned char color);
void GpCharOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, 
              char * sour, unsigned char color);
void GpTextNOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, 
		      char * sour, int nStart, int nString, unsigned char color);
		      
#define GPC_GT_BOTTOM		0x2
#define GPC_GT_HCENTER		0x8
#define GPC_GT_LEFT			0x10
#define GPC_GT_RIGHT		0x20
#define GPC_GT_TOP			0x40
#define GPC_GT_WORDBREAK		0x80		/* 자동 줄바꿈 */
#define GPC_GT_VCENTER		0x100
		      
void GpTextDraw(GPDRAWSURFACE * ptgpds, GPRECT * cRect, unsigned int uFormat, 
		      char * sour, int nStart, int nCount, unsigned char color);

/*
CAUTION :
	You must include gpg_ex??.alf version for using followings!!!
	You can define only your's character code.
	But the number of code must be 256. 
	And code 0 is reserved at null character!!!
	And code 1 is reserved at line feed character!!!
	
CURRENTLY ALLOWED fx_flag
	GPC_GDFX_COPY
	GPC_GDFX_TRANS
	GPC_GDFX_TRANS | GPC_GDFX_RAYPLUS
*/		      
typedef struct tagEXT_FONT{
	int fx_flag;				/* effect flag : refer gpgraphic.h */
	unsigned char * lpsz;		/* customized string */
	unsigned char * pBmFont;		/* customized bitmap font */
	int ex_font_w;
	int ex_font_h;
	int ex_chargap;
	int ex_linegap;
	int color1;
	int color2;
}EXT_FONT;
int GpCustTextOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, EXT_FONT * y_font);

void GpTextOut16(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, char * sour, int color);
void GpCharOut16(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, char * sour, int color);
void GpTextNOut16(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, char * sour, int nStart, int nString, int color);

/******************************************************************************************
* Hangul support API														*
******************************************************************************************/
void GpHAutomatonInit(void);
int GpHAutomatonInput(const char * p_input, char ** p_working);
int GpHAutomatonDelete(char ** p_working);
char * GpHAutomatonBuffered(void);

#endif