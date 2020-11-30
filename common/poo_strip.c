#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpstdio.h"

#include "gp32\gp32.h"
#include "gp32\debug.h"
#include "gp32\24x.h"
#include "gp32\option.h"
#include "gp32\bfont.h"

#include "gp32\file_dialog.h"

#include "famegraf.h"
#include "all.h"

extern GPDRAWSURFACE gpDraw[2];
extern int nflip;

#define maxstrip 20
#define st_still 0
#define st_roll 1
#define st_go 2

s16 nstrip=1;
s16 striply=1;
struct {
	s16 x,y; /*начальная позиция*/
	s16 ly;	/*высота полоски*/
	s16 sx,sy; /**/
	s16 st;
	s16 typ;
} strip[maxstrip];
void show_strip(s16 y);
void do_strip(s16 sx);

extern screen hidscr;
extern char stroka[256];
extern screen bckg; //задний фон

int gettok(char **s, char *d);
int getNtok(char **s);

#ifdef FASTDRAW__
void FastStrip(s16 x, s16 y, s16 lx, s16 ly, block buf, int sx, int sy) {
	//вывод с прозрачным цветом
	//sx sy - смешение от начала в картинке исходн
	//bx by - размеры больш картинки

	register s16 tx, ty;
	register unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpBitBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		sx,sy,320,200);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) ); //2e 239

	buf += sx*200 + 200-(sy+ly);
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty != 0; ty--) {
			*p_buf++ = *buf++;
		}
		p_buf += (240 - ly);
		buf += (200 - ly);
	}
}
#endif

void readstrip(void) {
	//считывание описания для scroll-strip
	// 01strip.def
	s16 i;
	block pos=(block)hidscr;
#ifndef RELEASE
	__printf("Load strips: Start\n");	
#endif	
	striply=nstrip=0;

	for (i=0; i<maxstrip; ++i ) {
		//считаем очередн строку
/*	gp_str_func.memcpy(&stroka,pos,250);
		//0 0  27   -1 0   1
		n=sscanf(stroka,"%d %d %d %d %d %d",
		&strip[i].x,&strip[i].y,&strip[i].ly,
		&strip[i].sx,&strip[i].sy,&strip[i].typ);
*/

		strip[i].x=getNtok(&pos);
		strip[i].y=getNtok(&pos);	
		strip[i].ly=getNtok(&pos);	
		strip[i].sx=getNtok(&pos);	
		strip[i].sy=getNtok(&pos);	
		strip[i].typ=getNtok(&pos);
		
		strip[i].st=0;		

		if ( strip[i].ly <= 0 ) {
			//когда описание кончилось - выход
			break;
		}
		
		++nstrip;
		striply+=strip[i].ly;
	}
	striply-=240;
#ifndef RELEASE
	__printf("Load strips: Finished\n");	
#endif
}

void splitcopy(s16 x, s16 y, s16 lx, s16 ly, s16 sx, s16 sy, screen src) {
	//вывод блока со сдвигом
	s16 lx1,lx2;
	if ( sx+lx>=320 ) {
		lx1=320-sx;
	} else {
		lx1=lx;
	}

	lx2=lx-lx1;

	//CopyBlock(x, y, lx1, ly, dest, sx, sy, src);
#ifdef FASTDRAW__
	FastStrip(x, y, lx1, ly,src,
		sx,sy);
	if ( lx2 )
		FastStrip(x+lx1, y, lx2, ly,src,0,sy);
#else
	GpBitBlt(NULL,&gpDraw[nflip],
		x, y, lx1, ly,(unsigned char*)src,
		sx,sy,
		320,200);
	if ( lx2 )
		//CopyBlock(x+lx1, y, lx2, ly, dest, 0, sy, src);
		GpBitBlt(NULL,&gpDraw[nflip],
			x+lx1, y, lx2, ly,(unsigned char*)src,
			0,sy,
			320,200);
#endif
}

void show_strip(s16 y) {
	static s16 i,j,py;

	j=py=0;
	for ( i=0; i<nstrip; ++i ) {
		py+=strip[i].ly;
		if ( py>y ) {
			j=py-y;
			break;
		}
	}
	/*1я строка*/
	//splitcopy(16, 16, 272, min(160,strip[i].ly), strip[i].x, strip[i].y+strip[i].ly-j, bckg);
	splitcopy(0, 0, 320, min(240,strip[i].ly), strip[i].x, strip[i].y+strip[i].ly-j, bckg);
	if ( nstrip<=1) return;
	py=j;
	for ( ++i; i<nstrip; ++i ) {
		/*1я строка*/
		if ( py+(j=strip[i].ly)>240/*168*/ ) {
			break;
		}
		//splitcopy(16, 16+py, 272, j, strip[i].x, strip[i].y, bckg);
		splitcopy(0, py, 320, j, strip[i].x, strip[i].y, bckg);
		py+=j;
	}
	/*последняя строка*/
	if ( (j=240-py)>0 ) {
		//splitcopy(16, 16+py, 272, min(strip[i].ly,j), strip[i].x, strip[i].y, bckg);
		splitcopy(0, py, 320, min(strip[i].ly,j), strip[i].x, strip[i].y, bckg);
	}
}

void do_strip(s16 sx/*=0*/) {
	s16 i;
	sx=min(sx,319);
	for ( i=0; i<nstrip; ++i ) {
		switch ( strip[i].typ ) {
			case st_roll:
				strip[i].x-=(strip[i].sx-sx/4);
			case st_still:
				break;
			case st_go:
				if ( sx<0 ) {
					if ( (strip[i].st+=sx)<-strip[i].sx ) {
						strip[i].x+=(strip[i].st/strip[i].sx);
						strip[i].st=0;

					}
				} else if ( sx>0 ) {
					if ( (strip[i].st+=sx)>strip[i].sx ) {
						strip[i].x+=(strip[i].st/strip[i].sx);
						strip[i].st=0;

					}
				}
				break;
		}
		if ( strip[i].x>319 ) {
			strip[i].x-=320;
		} else if ( strip[i].x<0 ) {
			strip[i].x+=320;
		}
	}
}
