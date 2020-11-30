/* Работа в граф режиме 256цв 320х240 */

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

extern GPDRAWSURFACE gpDraw[2];
extern int nflip;

extern int swi_pal_oper(int , GP_HPALETTE );
extern int swi_chan_pal_fade(int , int , GP_HPALETTE );
extern void swi_pal_addr_get(GP_HPALETTE* , GP_HPALETTE* );

//s16 win_num=0; /*текущее открытое окно*/
//struct {
//	u16 x,y,lx,ly;
//	block save;
//} win[16];

s16 G_x=0, G_y=0; /*поз графич курсора*/
s16 G_lx=8, G_ly=8,G_interval=1;
char char_fgd=17, char_bkgd=0; /*цвет символа и фона*/
block palette;  /*буфер для текущей*/
block palette1; /*буфер для текущей палитры*/
//просчитанные для сатурации
block palettes8;
block palettes16;

/*=====================================================================*/

void InitGraph(void) { /*начальная инициализация палитр и тп*/
	//Clip(0,0,319,239);
	palette=(block)famemalloc(768);
	//временная
	palette1=(block)famemalloc(768);
	//для сатурации
	palettes8=(block)famemalloc(256);
	palettes16=(block)famemalloc(256);
}

void MoveXY(s16 x, s16 y) {	//переместить текстов курсор на экране 320х200*/
	G_x=(s16)x; G_y=(s16)y;
}

//======================================= палитры
#define _SaturA16Col(c) (palette[c*3]+palette[c*3+1]+palette[c*3+2])/(3*19)

#define Satur8Col(c,sc) sc-palettes8[c]
#define SaturA8Col(c) palettes8[c]
#define Satur16Col(c,sc) sc-palettes16[c]
#define SaturA16Col(c) palettes16[c]


u8 NormColor(u16 c)
{
	c /= 8;
	if (c>0x1f)
		return 0x1f;
	return c&0x1f;
}

void PutPalette(block f) {
	static s16 i,k,j=0;

	//GP_HPALETTE	h_pal;
	GP_PALETTEENTRY tmp_entry;
	
	for ( i=k=0; k<256*3; k+=3,++i )
	{
		tmp_entry = ( NormColor(f[k])<<11 ) | ( NormColor(f[k+1])<<6 ) | ( NormColor(f[k+2])<<1 ) | 0;
		GpPaletteEntryChange(i, 1, &tmp_entry, GPC_PAL_NO_REALIZE);
		//просчитываем прозрачность, т.е. яркость
		palettes8[j] = (palettes16[j] = _SaturA16Col(j)) / 2;
		j++;
	}		
	GpPaletteRealize();
	GpPaletteInit();
}
#define fadestep 8
#define fadesteps 32

int GpLcdFade_(int fade_step, GP_HPALETTE old_pal)
{
	GP_HPALETTE tmp_pal, sys_pal;
	int i;
	
	if ( !fade_step ) return 0;
	swi_pal_addr_get(&tmp_pal, &sys_pal);
	if ( old_pal )
		sys_pal = old_pal;
	if ( fade_step > 0 )
	{
		for ( i=0; i<fade_step; i++ )
		{
			if ( !swi_pal_oper(3, sys_pal) )
				return 0;
		}
	}
	else
	{
		fade_step *= (-1);
		for ( i=0; i<fade_step; i++ )
		{
			if ( !swi_pal_oper(4, sys_pal) )
				return 0;
		}
	}
	return 1;
}

int GpLcdFadeNormalize_(GP_HPALETTE basic_pal)
{
	GP_HPALETTE tmp_pal, sys_pal;
	if ( !basic_pal )
	{
		swi_pal_addr_get(&tmp_pal, &sys_pal);
		basic_pal = sys_pal;
	}
	
	return swi_pal_oper(6, basic_pal);
}


//эксперимент
void PaletteOn(block f) {
	int j;
	for (j=0;j<fadesteps;j++) {
		//GpLcdFade(fadestep,NULL);
#ifdef NOEMU
		GpLcdFadeNormalize_(NULL);
#endif
	}
	//GpLcdFadeNormalize(NULL);
}

void PaletteOff(block f) {
//	int j;
//	for (j=fadesteps;j>=0;j--) {
#ifdef NOEMU
	GpLcdFade_(-fadesteps,NULL);
#endif
//	}
}

void PaletteWhite(block f) {
//	int j;
//	for (j=fadesteps;j>=0;j--) {
#ifdef NOEMU
	GpLcdFade_(fadesteps,NULL);
#endif
//	}
}

/*
void PaletteOn(block f) { //норм
	static s16 j,i;
	for (j=256;j>=0;j-=fadestep) {
		for (i=0;i<256*3;++i) {
			palette1[i]=(f[i]-j<=0)?0:(f[i]-j);
		}
		PutPalette(palette1);
             	}
	return;
}

void PaletteOff(block f) { //буфер
	static s16 j,i;
	for (j=0;j<=256;j+=fadestep) {
		for (i=0;i<256*3;++i) {
			palette1[i]=(f[i]-j<=0)?0:(f[i]-j);
		}
		PutPalette(palette1);
	}
}
*/

void Saturate(s16 x, s16 y, s16 lx, s16 ly, int start_c) {
	//вывод в одной цветовой гамме (start_c - самый темный и правый цвет из 15)

	register s16 tx, ty;
	register unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		//фигу... блин
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) );
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty !=0; ty--) {
			*p_buf = Satur16Col(*p_buf, start_c);
			p_buf++;
		}
		p_buf += (240 - ly);
	}
}

void PutCSatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c) {
	//вывод с прозрачным цветом

	register s16 tx, ty;
	register unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) ); //2e 239
	buf--;
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty !=0; ty--) {
			if (*(++buf))
				*p_buf = Satur8Col(*buf, start_c);
			p_buf++;
		}
		p_buf += (240 - ly);
	}
}

void PutCSaturr(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c) {
	//вывод с прозрачным цветом

	register s16 tx, ty;
	register unsigned char *p_buf;

	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransLRBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}	

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) );	//239 было ошиб
	buf += (lx-1)*ly -1;
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty !=0; ty--) {
			if (*(++buf))
				*p_buf = Satur8Col(*buf, start_c);
			p_buf++;
		}
		buf -= ly*2;
		p_buf += (240 - ly);
	}
}

//с абрисом
void PutCASatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c) {
	//вывод с прозрачным цветом

	register s16 tx, ty,t;
	register unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) ); //2e 239
	buf--;
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty !=0; ty--) {
			if (*(++buf))
				*p_buf = (t=SaturA16Col(*buf))?start_c-t:8;
			p_buf++;
		}
		p_buf += (240 - ly);
	}
}

void PutCASaturr(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c) {
	//вывод с прозрачным цветом

	register s16 tx, ty,t;
	register unsigned char *p_buf;

	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransLRBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}	

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) );	//239 было ошиб
	buf += (lx-1)*ly-1;
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty !=0; ty--) {
			if (*(++buf))
				*p_buf = (t=SaturA16Col(*buf))?start_c-t:8;
			p_buf++;
		}
		buf -= ly*2;
		p_buf += (240 - ly);
	}
}

//для фона
void PutCESatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c) {
	//вывод с прозрачным цветом

	register s16 tx, ty;
	unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) ); //2e 239
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty !=0; ty--) {
			if (*buf++)
				*p_buf = Satur16Col(*p_buf, start_c);
			p_buf++;
		}
		p_buf += (240 - ly);
	}
}

//для фона
void PutCESaturr(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c) {
	//вывод с прозрачным цветом

	register s16 tx, ty;
	register unsigned char *p_buf;

	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransLRBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}	

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) );	//239 было ошиб
	buf += (lx-1)*ly-1;
	
	for (tx = lx; tx !=0; tx--)
	{
		for (ty = ly; ty !=0; ty--) {
			if (*(++buf))
				*p_buf = Satur16Col(*p_buf, start_c);
			p_buf++;
		}
		buf -= ly*2;
		p_buf += (240 - ly);
	}
}

//- для мульта и больших спрайтов внутри больших картинок
void BltSatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int sx, int sy, int bx, int by,int start_c) {
	//вывод с прозрачным цветом
	//sx sy - смешение от начала в картинке исходн
	//bx by - размеры больш картинки

	register s16 tx, ty;
	register unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		sx,sy,bx,by,0);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) ); //2e 239

	buf += sx*by + by-(sy+ly)-1;
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty != 0; ty--) {
			if (*(++buf))
				*p_buf = Satur8Col(*p_buf, start_c);
			p_buf++;
		}
		p_buf += (240 - ly);
		buf += (by - ly);
	}
}

/*
//- для мульта и больших спрайтов внутри больших картинок
void BltSatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int sx, int sy, int bx, int by,int start_c) {
	//вывод с прозрачным цветом
	//sx sy - смешение от начала в картинке исходн
	//bx by - размеры больш картинки

	register s16 tx, ty;
	register unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		sx,sy,bx,by,0);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) ); //2e 239

	buf += sx*by + by-(sy+ly)-1;
	
	for (tx = lx; tx !=0; tx--) {
		for (ty = ly; ty != 0; ty--) {
			if (*(++buf))
				*p_buf = Satur8Col(*buf, start_c);
			p_buf++;
		}
		p_buf += (240 - ly);
		buf += (by - ly);
	}
}
*/


void Rectangle(s16 x1, s16 y1, s16 lx, s16 ly) {
	//вывод рамки
	Line(x1,y1,x1,y1+ly-1); Line(x1,y1,x1+lx-1,y1);
	Line(x1+lx-1,y1,x1+lx-1,y1+ly-1); Line(x1,y1+ly-1,x1+lx-1,y1+ly-1);
}

void Swap (s16 *pa,s16 *pb) {
	register s16 t;
	t=*pa; *pa=*pb; *pb=t;
}

void Line(s16 x1, s16 y1, s16 x2, s16 y2)
{
	static s16 d,y,x,dx,dy,Aincr,Bincr,yincr;

	if ( abs(y2-y1)>abs(x2-x1) ) goto Do_dr_lin;

	if (x1>x2) {
		Swap (&x1,&x2);
		Swap (&y1,&y2);
	}

	if (y2>y1)
		yincr=1;
	else
		yincr=-1;

	dx=x2-x1; dy=abs(y2-y1); d=2*dy-dx;
	Aincr=2*(dy-dx);
	Bincr=2*dy;
	x=x1;
	y=y1;

	for (x=x1/*+1*/;x<=x2;x++) {
		if (d>=0) {
			y+=yincr;
			d+=Aincr;
		}
		else
			d+=Bincr;
			PutPixel(x,y);
	}
	return;

Do_dr_lin:
	if (y1>y2) {
		Swap (&x1,&x2);
		Swap (&y1,&y2);
	}

	if (x2>x1)
		yincr=1;
	else
		yincr=-1;

	dy=y2-y1; dx=abs(x2-x1); d=2*dx-dy;
	Aincr=2*(dx-dy);
	Bincr=2*dx;
	x=x1;
	y=y1;

	for (y=y1/*+1*/;y<=y2;y++) {
		if (d>=0) {
			x+=yincr;
			d+=Aincr;
		}
		else
			d+=Bincr;
			PutPixel(x,y);
	}
}

/*void vputs(char * f) { //вывод текстовой строки на экран
	while (*f) {
		PutCh(*f);
		++f;
	}
}*/

/*s16 vprint( char *fmt, ... ) {
	va_list  argptr;
	char str[64];
	s16 cnt;
	va_start( argptr, fmt );
	cnt = vsprintf( str, fmt, argptr );
	vputs( str );
	va_end( argptr );
	return( cnt );
}*/

/*void vputsc(char * f) { //вывод текстовой строки на экран по центру
	MoveXY(159-famestrlen((unsigned char *)f)/2,G_y);
	vputs(f);
}*/

void PutMtb(s16 x,s16 y,s16 lx,s16 ly,s16 mx,s16 my,block buffer) {
	static s16 i,j;
	static s16 tx,ty;
	static s16 ny;
	for (j=0;j<ly;++j) {
		ny=y+(ty=(j*my/ly)); ty=((j+1)*my/ly-ty);
		ty=(ty ? ty : 1);
		for (i=0;i<lx;++i) {
			tx=( i*mx/lx );
			CurrentColor=( *(buffer+i*ly+(lx-j-1) ) );
			if (CurrentColor)	//типа прозрачный 0й не выводим
				Bar( x+tx, ny, ((i+1)*mx/lx)-tx, ty );
		}
	}
	return;
}

/*
void PutMtb(s16 x,s16 y,s16 lx,s16 ly,s16 mx,s16 my,block buffer) {
	static s16 i,j;
	static s16 tx,ty;
	static s16 ny;
	for (j=0;j<ly;++j) {
		ny=y+(ty=(j*my/ly)); ty=((j+1)*my/ly-ty);
		ty=(ty ? ty : 1);
		for (i=0;i<lx;++i) {
			tx=( i*mx/lx );
			CurrentColor=( *(buffer+j*lx+i) );
			Bar( x+tx, ny, ((i+1)*mx/lx)-tx, ty );
		}
	}
	return;
}
*/

void PutCMas(s16 x, s16 y, s16 lx, s16 ly, block buf) {
	//вывод с прозрачным цветом

	//рисуем не обрезая
	//PutMas(x,y,lx,ly,buf);
	GpTransBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		
}

void PutCMasr(s16 x, s16 y, s16 lx, s16 ly, block buf) {
	//вывод с прозрачным цветом

	//рисуем не обрезая
	//PutMasr(x,y,lx,ly,buf);
	GpTransLRBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);

}

/*//sets a pixel in 16-bit mode 
void PutPixel(int x, int y, int c)
{
	// you can optimize this function...
	// i did not add here any pixels range conditions
	// checking...
	// x = 0..319      y = 0..239 
	unsigned char *p_buf;
	p_buf = (unsigned char *)(gpDraw[nflip].ptbuffer);
	*(p_buf + x*240 + (239-y)) = c;
}
*/

void PutCBlink(s16 x, s16 y, s16 lx, s16 ly, block buf) {
	//вывод с прозрачным цветом

	s16 tx, ty;
	unsigned char *p_buf;
	
	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) ); //2e 239
	
	for (tx = lx; tx !=0; tx--)
	{
		for (ty = 0; ty < ly; ty++)
		{
			if (*buf++)
				*p_buf = CurrentColor;
			p_buf++;
		}
		p_buf += (240 - ly);
	}
}

void PutCBlinkr(s16 x, s16 y, s16 lx, s16 ly, block buf) {
	//вывод с прозрачным цветом

	s16 tx, ty;
	unsigned char *p_buf;

	if ( x+lx > 320 || x<0 || y<0 || y+ly > 240)
	{
		GpTransLRBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly,0);
		return;
	}	

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+ly) );	//239 было ошиб
	buf += (lx-1)*ly;
	
	for (tx = lx; tx !=0; tx--)
	{
		for (ty = 0; ty < ly; ty++)
		{
			if (*buf++)
				*p_buf = CurrentColor;
			p_buf++;
		}
		buf -= ly*2;
		p_buf += (240 - ly);
	}
}

void PutCImg(s16 x, s16 y, s16 lx, s16 ly, block buf) {
	//вывод без прозрачности

	//рисуем не обрезая
	//PutImg(x,y,lx,ly,buf);
	GpBitBlt(NULL,&gpDraw[nflip], x,y,
		lx,ly,
		(unsigned char*)buf,
		0,0,lx,ly);
}


void CBar(s16 x, s16 y, s16 lx, s16 ly) {
	//вывод ПРЯМОУГОЛЬНИКА с обрезкой
	static s16 x1,y1,nx,ny,nlx,nly,rx,ry;
	x1=x+lx-1; y1=y+ly-1; 
	if ( x1<MinX || y1<MinY || x>MaxX || y>MaxY ) {
		//не рисуем
		return;
	}
	if ( x1<=MaxX && y1<=MaxY && x>=MinX && y>=MinY ) {
		//рисуем не обрезая
		Bar(x,y,lx,ly);
		return;
	}
	//вырезка по экрану
	// X координата
	nlx=lx;	nx=x; ry=rx=0;
	if (x<MinX) {
		nx=MinX;
		rx=(MinX-x);
		nlx=lx-rx;
	}
	if (x1>MaxX) {
		nlx-=(x1-MaxX);
	}

	// Y координата
	nly=ly; ny=y;
	if (y<MinY) {
		ny=MinY;
		ry=(MinY-y);
		nly=ly-ry;
	}
	if (y1>MaxY) {
		nly-=(y1-MaxY);
	}

	Bar(nx,ny,nlx,nly);
}

void TileBar(s16 x, s16 y, s16 lx, s16 ly, s16 tx, s16 ty, block buf) {
	//вывод ПРЯМОУГОЛЬНИКА мозаикой
	s16 _MaxX=MaxX,_MaxY=MaxY,_MinX=MinX,_MinY=MinY,i,j;
	Clip(x,y,x+lx-1,y+ly-1);
	for ( i=y; i<=y+ly; i+=ty )
		for ( j=x; j<=x+lx; j+=tx ) {
			PutCImg(j,i,tx,ty,buf);
		}
	Clip((MinX=_MinX),(MinY=_MinY),(MaxX=_MaxX),(MaxY=_MaxY));
}

/*
char CharUp(unsigned char i) { //DOS кодировка
	if (i>=224 && i<=239) {
		i-=48;	//склейка русских букв
	}
	if ( (i>=96 && i<=127) || (i>=160 && i<=191) ) {
		return (i-32);
	}
	return i;
}
*/

//быстрый вывод 16х16
void PutMas16(s16 x, s16 y, block buf) {
	//вывод с прозрачным цветом

	register s16 tx, ty;
	unsigned char *p_buf;
	
	if ( x+16 > 320 || x<0 || y<0 || y+16 > 240)
	{
		GpTransBlt(NULL,&gpDraw[nflip], x,y,
		16,16,
		(unsigned char*)buf,
		0,0,16,16,0);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+16) );
	buf--;
	
	for (tx = 16; tx !=0; tx--) {
		for (ty = 16; ty !=0; ty--) {
			if (*(++buf))
				*p_buf = *buf;
			p_buf++;
		}
		p_buf += (240 - 16);
	}
}

//быстрый вывод 16х16
void PutImg16(s16 x, s16 y, block buf) {
	//вывод без прозрачности!

	register s16 tx, ty;
	unsigned char *p_buf;
	
	if ( x+16 > 320 || x<0 || y<0 || y+16 > 240)
	{
		GpBitBlt(NULL,&gpDraw[nflip], x,y,
		16,16,
		(unsigned char*)buf,
		0,0,16,16);
		return;
	}

	p_buf = ( (gpDraw[nflip].ptbuffer) + x*240 + 240-(y+16) );
	
	for (tx = 16; tx !=0; tx--) {
		for (ty = 16; ty !=0; ty--) {
			*p_buf++ = *buf++;
		}
		p_buf += (240 - 16);
	}
}

//выбираем текстовый токен... строку или число. если конец строки - возвр -1
int gettok(char **s, char *d)
{
	char *t;
	if (**s == 0)
		//конец
		return 0;
	while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
	{
		(*s)++;
	}
	t=*s;
	//теперь ищем конец
	while (**s != ' ' && **s != '\t' && **s != '\n' && **s != '\r' && **s != 0 )
	{
		//printf("%c.",**s);
		(*s)++;
	}
	gp_str_func.memcpy(d,t,*s-t);
	gp_str_func.memset(d+(*s-t),0,1);
	//*s += (*s-t);
	
	//__printf("gettok: '%s' ",d);
	while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
	{
		(*s)++;
	}
	return *s-t;
}

int getNtok(char **s)
{
	char *t,*d;
	int i;
	if (**s == 0)
		//конец
		return -1;	
	d=(char*)gp_mem_func.malloc(250);
	while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
	{
		(*s)++;
	}
	t=*s;
	//теперь ищем конец
	while (**s != ' ' && **s != '\t' && **s != '\n' && **s != '\r' && **s != 0 )
	{
		(*s)++;
		//printf("%c-",**s);		
	}
	gp_str_func.memset(d,0,20);
	gp_str_func.memcpy(d,t,*s-t);
	i=fatoi(d);
	//__printf("[%s->%i]",d,i);
	gp_mem_func.free(d);
	//if (*s-t > 3)	//если не ЧИСЛО! А текст, начинающийся с цифр
	//	return 0;
	while (**s == ' ' || **s == '\t' || **s == '\n' || **s == '\r')
	{
		(*s)++;
	}	
	return i;
}

//моя функция преобр-я СТРОКИ в число
int fatoi(char *s)
{
	int i=0, sign=1;
	if (*s == '-')
	{
		s++;
		sign = -1;
	}
	while ( *s >= '0' && *s <= '9')
	{
		i *= 10;
		i += (*s - '0');
		s++;
	}
	return i * sign;
}