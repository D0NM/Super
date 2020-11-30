#include "gpstdlib.h"
#include "gpfont.h"
#include "gpfinternal.h"

extern const unsigned char JasoTable[3][32];
extern const unsigned char FirstTb01[_MAXJASO_MID+1];
extern const unsigned char FirstTb02[_MAXJASO_MID+1];
extern const unsigned char MidTb01[_MAXJASO_FIRST+1];
extern const unsigned char MidTb02[_MAXJASO_FIRST+1];
extern const unsigned char LastTb[_MAXJASO_MID+1];
extern const unsigned short HCodeTable[_KSUNISIZE];
extern const unsigned short HCodeTableEx[_KSUNIEXSIZE];

extern BGFONTINFO _g_font;
extern int _g_font_h;
extern int _korfont_size;
extern int _engfont_size;
extern unsigned char * _pt_font_kor;
extern unsigned char * _pt_font_eng;

extern GpFontOut(_BGFONT *);

void _processEngChar(GPDRAWTAG * gptag, 
			GPDRAWSURFACE * ptgpds,
			int x,
			int y,
			char data1,
			unsigned char color);
			
void _processKorChar(GPDRAWTAG * gptag,
			GPDRAWSURFACE * ptgpds,
			int x,
			int y,
			char data1,
			char data2,
			unsigned char color);

void GpCharOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, char * sour, unsigned char color)
{
	int init_def_y, init_ext_y;
	char chardata[2];
	init_def_y = y + (_g_font_h - _g_font.kor_h);
	init_ext_y = y + (_g_font_h - _g_font.eng_h);
	x += _g_font.chargap;
	while(* sour != 0)
	{
		if((*sour) & 0x80) //korean
		{
			chardata[0] = *sour;
			sour++;
			chardata[1] = *sour;
			sour++;
			_processKorChar(gptag, ptgpds, x, init_def_y, chardata[0], chardata[1],color);
			x += _g_font.kor_w + _g_font.chargap;
		}
		else
		{
			chardata[0] = *sour;
			sour++;
			_processEngChar(gptag, ptgpds, x, init_ext_y, chardata[0], color);
			x += _g_font.eng_w + _g_font.chargap;
		}
	}
}

void GpTextOut(GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int x,int y,char* sour, unsigned char color)
{
	int limit_x, limit_y;
	int new_x;
	int init_x = x;
	int init_def_y, init_ext_y;
	char chardata[2];
	init_def_y = y + (_g_font_h - _g_font.kor_h);
	init_ext_y = y + (_g_font_h - _g_font.eng_h);
	x = init_x;
	
	if ( gptag )
	{
		limit_x = gptag->clip_x + gptag->clip_w;
		limit_y = gptag->clip_y + gptag->clip_h;
		//limit_x = gptag->clip_w;
		//limit_y = gptag->clip_h;
	}
	else
	{
		limit_x = GPC_LCD_WIDTH;
		limit_y = GPC_LCD_HEIGHT;
	}
	
	while(* sour != 0)
	{
		if((*sour) & 0x80) //korean
		{
			new_x = x + _g_font.kor_w + _g_font.chargap;
			if ( new_x > limit_x )
			{
				x = init_x;
				init_def_y += _g_font_h + _g_font.linegap;
				init_ext_y += _g_font_h + _g_font.linegap;
				new_x = x + _g_font.kor_w + _g_font.chargap;
				if ( init_def_y > limit_y || init_ext_y > limit_y ) break;
			}
			
			chardata[0] = *sour;
			sour++;
			chardata[1] = *sour;
			sour++;
			_processKorChar(gptag, ptgpds, x, init_def_y, chardata[0], chardata[1],color);
			
			x = new_x;
		}
		else if ((*sour) == '\r' || (*sour) == '\n')
		{
			sour++;
			x = init_x;
			init_def_y += _g_font_h + _g_font.linegap;
			init_ext_y += _g_font_h + _g_font.linegap;
			if ((*sour) == '\r' || (*sour) == '\n')
				sour++;
			if ( init_def_y > limit_y || init_ext_y > limit_y ) break;
		}
		else
		{
			chardata[0] = *sour;
			sour++;
			if ( chardata[0] != '\t' )
			{
				new_x = x + _g_font.eng_w + _g_font.chargap;
				if ( new_x > limit_x )
				{
					x = init_x;
					init_def_y += _g_font_h + _g_font.linegap;
					init_ext_y += _g_font_h + _g_font.linegap;
					new_x = x + _g_font.eng_w + _g_font.chargap;
					if ( init_def_y > limit_y || init_ext_y > limit_y ) break;
				}
				_processEngChar(gptag, ptgpds, x, init_ext_y+2, chardata[0], color);
				x = new_x;
			}
			else
				x += (4*(_g_font.eng_w + _g_font.chargap));	
		}	
	}
}

/******************************************************************************
 ******************************************************************************
 **                                                                          **
 **        DrawMidText                                                       **
 **                                                                          **
 ******************************************************************************
 ******************************************************************************/
void GpTextNOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds, int x, int y, 
			char * sour, int nStart, int nString, unsigned char color)
{
	char	* p_sub;
	
	p_sub = (char *)gp_mem_func.malloc(nString + 1);
	gp_str_func.strncpy((char *)p_sub, (const char *)(sour + nStart), nString);
	p_sub[nString] = 0;
	GpTextOut(gptag,
		ptgpds,
		x,
		y,
		p_sub,
		color);
	gp_mem_func.free((void *)p_sub);
}

/**************************************************************************
_getFontDrawPos
**************************************************************************/
int _getFontDrawPos(GPDRAWTAG * gptag,int buf_h,int dx,int dy,int width,int height,int fw,int fh,_BGFONT * _bgfont)
{
     int clip_l,clip_t,clip_r,clip_b;
     int sx,sy;
     sx=sy=0;
     if (gptag == NULL)
     {
          clip_l = clip_t = 0;
          clip_r = GPC_LCD_WIDTH;
          clip_b = GPC_LCD_HEIGHT;
     }
     else
     {
          clip_l = gptag->clip_x;
          clip_t = gptag->clip_y;
          clip_r = gptag->clip_x + gptag->clip_w;
          clip_b = gptag->clip_y + gptag->clip_h;
     }
     if (dx < clip_l)
     {
          sx = clip_l - dx;
          width -= (clip_l - dx);
          dx = clip_l;
     }
     if (dy < clip_t)
     {
          sy = clip_t - dy;
          height -= clip_t - dy;
          dy = clip_t;
     }
     if (dx+width > clip_r)
          width = clip_r - dx;
     if (dy+height > clip_b)
          height = clip_b - dy;
     if (fw-sx < width)
          width = fw-sx;
     if (fh-sy < height)
          height = fh-sy;
     if (width <= 0 || height <= 0) return 0;
     
     _bgfont->dx = buf_h-dy-height;
     _bgfont->dy =dx;
     _bgfont->width =height; 
     _bgfont->height =width;
     _bgfont->sx = fh-sy-height;
     _bgfont->sy = sx;
     _bgfont->srcpitch = fh;
     _bgfont->tgpitch = ((buf_h>>2) + ((buf_h & 0x3) ? 1 : 0))<<2;
     return 1;
}

/**************************************************************************
ProcessEnglishChar
**************************************************************************/
void _processEngChar(GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int x,int y,char data1,unsigned char color)
{
     _BGFONT _bgfont;
     
     if (_getFontDrawPos(gptag,ptgpds->buf_h,x,y,_g_font.eng_w,_g_font.eng_h,_g_font.eng_w,_g_font.eng_h,&_bgfont))
     {
          _bgfont.dest = (unsigned char*)ptgpds->ptbuffer;
          _bgfont.sour = (unsigned char*)(_pt_font_eng + data1 * _engfont_size);
          _bgfont.foregnd = (int)color;
          GpFontOut(&_bgfont);
     }
}

/**************************************************************************
ProcessKoreanChar
     data1+data2-->2byte unified korean character
     1>convert to combined character
***************************************************************************/
void _processKorChar(GPDRAWTAG * gptag,GPDRAWSURFACE * ptgpds,int x,int y,char data1,char data2,unsigned char color)
{
	int i;
	int JasoIndex;
	char data_f,data_b;
	int JasoFirst,JasoMid,JasoLast;    //decide first,mid,last index
	int packJF,packJM,packJL;                    //decide package
	unsigned int * srcdata[3];
	_BGFONT _bgfont;

	/*************************************************************************************************************************
	*                                                                                                                       *
	*    ASCII 코드 -> 완성형 한글문자 인덱스                                                                               *
	*                                                                                                                       *
	*************************************************************************************************************************
	*                                                                                                                       *
	* 한글 2350자는 94자씩 25그룹으로 되어있다.                                                                             *
	* 각 그룹은 하위1바이트 128개 문자중 34번째 문자(값으로는 33)부터 94개이다.                                             *
	* 또한 그룹은 상위바이트가 0xB0-0xC8 (25개)로 되어있다.                                                                 *
	* 따라서 JasoIndex를 얻는 공식은 다음과 같다.                                                                           *
	*                                                                                                                       *
	*      JasoIndex=(data1-176)*94 + data2-161;                                                                            *
	*                --------------   ---------                                                                             *
	*                  그룹인덱스     문자인덱스                                                                            *
	*                                                                                                                       *
	*   o 176 = 0xB0 (시작 그룹)                                                                                            *
	*   o 161 = 0x80 (최상위비트 set) + 33                                                                                  *
	*                                                                                                                       *
	*************************************************************************************************************************/
 	if((data1 >= 0xB0) && (data1 <= 0xC8))	/* 표준완성형 */
 	{
 		//JasoIndex = (data1 - 176) * 94 + (data2 & 0x7fff) - 33;	/* ascii code --> unified korean character index */
 		JasoIndex = (data1 - 176) * 94 + (data2 & 0x7fff) - 161;	/* ascii code --> unified korean character index */
		if((JasoIndex < 0) || (JasoIndex >= _KSUNISIZE))	return;
		data_f = HCodeTable[JasoIndex] >> 8;
		data_b = HCodeTable[JasoIndex] & 0x00ff;
 	}
 	else if(data1 == 0xA4)	/* 확장 환성형 */
 	{
 		JasoIndex = (data2 & 0x7fff) - 161;
 		if((JasoIndex < 0) || (JasoIndex >= 94))	return;
		data_f = HCodeTableEx[JasoIndex] >> 8;
		data_b = HCodeTableEx[JasoIndex] & 0x00ff;
	}
	else return;
	
	JasoFirst = JasoTable[0][(data_f & 0x7c)>>2];
	JasoMid = JasoTable[1][((data_f & 0x03)<<3) | ((data_b & 0xe0)>>5)];
	JasoLast = JasoTable[2][data_b & 0x1f];
     
	packJL=LastTb[JasoMid];
	if(JasoLast)        //when last exist
	{
		packJF=FirstTb02[JasoMid];
		packJM=MidTb02[JasoFirst];
	}
	else
	{
		packJF=FirstTb01[JasoMid];
		packJM=MidTb01[JasoFirst];
	}
	if (JasoFirst)
		srcdata[0] = (unsigned int*)(_pt_font_kor + (packJF*_MAXJASO_FIRST+JasoFirst-1)*_korfont_size);
	else
		srcdata[0] = NULL;
	if (JasoMid)
		srcdata[1]=(unsigned int*)(_pt_font_kor + (_PACKOFF_JM+packJM*_MAXJASO_MID+JasoMid-1)*_korfont_size);
	else
		srcdata[1] = NULL;
	if (JasoLast)
		srcdata[2] = (unsigned int*)(_pt_font_kor + (_PACKOFF_JL+packJL*_MAXJASO_LAST+JasoLast-1)*_korfont_size);
	else
		srcdata[2] = NULL;
	for (i=0;i<3;i++)
	{
		if (_getFontDrawPos(gptag,ptgpds->buf_h,x,y,_g_font.kor_w,_g_font.kor_h,_g_font.kor_w,_g_font.kor_h,&_bgfont))
		{
			if (srcdata[i] != NULL)
			{
				_bgfont.dest = ptgpds->ptbuffer;
				_bgfont.sour = (unsigned char*)srcdata[i];
				_bgfont.foregnd = (int)color;
				GpFontOut(&_bgfont);
			}
		}
	} 
}

