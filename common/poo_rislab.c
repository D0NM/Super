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

#include "famegraf.h"
#include "all.h"

#include "gp32\file_dialog.h"
#include "profiler\sdsfonts.h"

extern GPDRAWSURFACE gpDraw[2];
extern int nflip;

extern block level;
extern block blevel;
extern block items;
extern u16 r_x,r_y; //прежние координаты лев. верхн угла окна
extern u16 siz_xlev;
extern u16 siz_ylev;
extern u32 siz_level; //=siz_xlev*siz_ylev;
extern u32 svel,cvel;
extern block fon;
extern s16 striply;

extern s16 scrsdvig,scrsdvig_;
//дл€ вывод элементов в лабиринте сверху
#define maxnup (21*16)
struct strup {
	int x,y; //ссылка на адрес экрана
	int transp; //прозр ли?
	block buf; //ссылка на текущую фазу
};

struct strup upput[maxnup];
s16 nup=-1; //кол-во элементов сверху

extern void show_strip(s16 y);
extern void do_strip(s16 sx);
extern u16 TranspTile[256]; //флани - прозр точки у тайла есть или нет

void rislab(u16 x_, u16 y_) {
	register u16 x,y,sx,sy,i,j;
	register unsigned char *poslab, *poslabt;
	static u16 sdvig0,sdvig,sdvig1,anim_sdvig;
	u16 teki;
	register u16 teksmn,teksw;

	//дл€ скроллинга
	//дл€ плавного сдвига окна
	r_x=((teksw=r_x)+x_)/2; r_y=(r_y+y_)/2;
	teksw=r_x-teksw;

	//разделим координаты
	sx=(r_x&0x0f); x=r_x>>4;
	sy=(r_y&0x0f); y=r_y>>4;
	//позицию первую вычислим
	poslab=(unsigned char *)level+(teksmn=x+y*siz_xlev);
	poslabt=(unsigned char *)blevel+teksmn;

//
	if (y == 0)
		scrsdvig = 0;
	if (y > siz_ylev-(240/16))
		scrsdvig_ = 0;

	//дл€ анимации фона
	if ( ++sdvig0>=4 ) {
		sdvig0=0;
		if ( ++sdvig>=4 ) sdvig=0;
	}
	//дл€ анимации вещей
	sdvig1=(++sdvig1)&7;

	nup=-1; //кол-во элементов сверху

#ifndef RELEASE
	Profile("showstrip",
	do_strip(teksw/*man.sx*/);
	show_strip( (r_y*striply)/((siz_ylev-10)*16) );
	)
#else
	//вывод заднего плана
	do_strip(teksw/*man.sx*/);
	show_strip( (r_y*striply)/((siz_ylev-10)*16) );
#endif

	for ( i=0; i<=15; ++i ) {
		for ( j=0; j<=20; ++j ) {
			if ( *poslab ) { //вывод фона
				//вы€сним анимацию тайла
                                if ( *poslab < 16*8 || *poslab >= 16*13 ) {
                                        anim_sdvig = *poslab;
                                } else if ( *poslab < 16*12 ) {
                                        anim_sdvig = (*poslab + sdvig );
                                } else {
                                        anim_sdvig = *poslab + (sdvig>>1);
                                }
				if ( (*poslabt & 31)==23 || (*poslabt & 31)==24 ) { //вывод снаружи
					upput[++nup].x = j*16-sx;
					upput[nup].y = i*16-sy;
					upput[nup].buf=fon+(upput[nup].transp = anim_sdvig)*256;
				} else {
#ifdef FASTDRAW
					if ( TranspTile[anim_sdvig] )
						PutMas16(j*16-sx,i*16-sy,fon+anim_sdvig * 256);
					else
						PutImg16(j*16-sx,i*16-sy,fon+anim_sdvig * 256);
#else
					GpTransBlt(NULL,&gpDraw[nflip],
						j*16-sx,i*16-sy,16,16,(unsigned char*)fon+anim_sdvig * 256,
						0,0,16,16,0);
#endif
				}
			}
			if ( (teki=*poslabt)>31 ) { //вывод вещи
#ifdef FASTDRAW
				PutMas16(j*16-sx,i*16-sy,items+(((teki>>5)-1)*8+((sdvig1+i+j)&7))*256); //сделал красив ани
#else
				GpTransBlt(NULL,&gpDraw[nflip],
					j*16-sx,i*16-sy,16,16,(unsigned char*)items+(((teki>>5)-1)*8+sdvig1)*256,
					0,0,16,16,0);
#endif
			}
			++poslab; ++poslabt;
		}
		poslab=poslab+(teksw=siz_xlev-(20+1));
		poslabt=poslabt+teksw;
	}
}

void putup(void) {	//вывод верхнего рельева (сверху геро€)
	while ( nup>=0 ) {
#ifdef FASTDRAW
		if ( TranspTile[upput[nup].transp] )
			PutMas16(upput[nup].x,upput[nup].y,(block)upput[nup].buf);
		else
			PutImg16(upput[nup].x,upput[nup].y,(block)upput[nup].buf);
#else
		GpTransBlt(NULL,&gpDraw[nflip],
			upput[nup].x,upput[nup].y,16,16,(unsigned char*)upput[nup].buf,
			0,0,16,16,0);
#endif
		nup--;
	}
}
