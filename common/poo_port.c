//основной файл.. для адапртации функций
#include "famegraf.h"
#include "all.h"

#include "gp32\file_dialog.h"
#include "gp32\debug.h"

//#include "modplayer\modplayer.h"

extern GPDRAWSURFACE gpDraw[2];
extern int nflip;
extern char g_string[MAX_PATH_NAME_LEN];

extern int g_key_pressed;		//list of buttons pressed in curent loop

s16 MaxY;
s16 MaxX;
s16 MinY;
s16 MinX;
u8 CurrentColor;

// Графические функции
void Cls(u8 color)
{
	//очистка экрана
	GpRectFill(NULL, &gpDraw[nflip], 0,0,GPC_LCD_WIDTH,GPC_LCD_HEIGHT, color);

	//логотип
	//GpTextOut(NULL, &gpDraw[nflip], 2, 225, (char*)"U-Book v0.2b (c)2004 FaMe Soft", 0xf);
	//свободн память
	//gp_str_func.sprintf(g_string, "Memory Available:%d", gp_mem_func.availablemem());
	//GpTextOut(NULL, &gpDraw[nflip], 2, 213, g_string, 0);

}

/*void  WVR(void)
{
}
*/
 void  SetColor(s8 c)
{
	CurrentColor = c;
}

void  PutRGB(s8 i, s8 r, s8 g, s8 b)
{
	GP_PALETTEENTRY tmp_entry;
	tmp_entry = ((r & 0x1f)<<11) | ((g & 0x1f)<<6) | ((b & 0x1f)<<1) | (r&0xE0 || g&0xE0 || b&0xE0);
	GpPaletteEntryChange(i, 1, &tmp_entry, 0);
}
void  GetPalette(block p)
{
}

void  Clip(s16 x, s16 y, s16 x1, s16 y1)
{
	MaxY=y1;
	MaxX=x1;
	MinY=y;
	MinX=x;
}

void  PutPixel(s16 x, s16 y)
{
	GpPointSet(&gpDraw[nflip],x,y, CurrentColor);
}

s16   GetPixel(s16 x, s16 y)
{
	unsigned char *p_buf;
	p_buf = (unsigned char *)(gpDraw[nflip].ptbuffer);
	return *(p_buf + x*240 + (239-y));
}

void  Bar(s16 x, s16 y, s16 x1, s16 y1)
{
	GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, CurrentColor);
}

void  PutImg(s16 x, s16 y, s16 x1, s16 y1, block b)
{
	//GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, *b);
	GpBitBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1);

}

void  GetImg(s16 x1, s16 y1, s16 x2, s16 y2, block b)
{
}

void  PutMas(s16 x, s16 y, s16 x1, s16 y1, block b)
{
	GpTransBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1,0);
}

void  PutMasr(s16 x, s16 y, s16 x1, s16 y1, block b)
{
	GpTransLRBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1,0);
}

/* void  PutBlink(s16 x, s16 y, s16 x1, s16 y1, block b)
{
	GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, CurrentColor);
	//GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, *b);
	GpTransBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,
		x1,y1,
		0);

}

void  PutBlinkr(s16 x, s16 y, s16 x1, s16 y1, block b)
{
	GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, CurrentColor);
	//GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, *b);
	GpTransLRBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,
		x1,y1,
		0);
}*/

/*void  PutImg16s16(s16 x, s16 y, block b)
{
	//GpRectFill(NULL,&gpDraw[nflip], x, y, x+16, x+16, *b);
	GpBitBlt(NULL,&gpDraw[nflip], x, y,
		16,16,
		(unsigned char*)b,
		0,0,16,16);

}*/
/*
void  PutMas16(s16 x, s16 y, block b)
{
	//GpRectFill(NULL,&gpDraw[nflip], x, y, x+16, x+16, *b);
	GpTransBlt(NULL,&gpDraw[nflip], x, y,
		16,16,
		(unsigned char*)b,		
		0,0,16,16,0);

}*/

void  PutSMas(s16 x, s16 y, s16 x1, s16 y1,s16 x11, s16 y11, s16 x21, s16 y21, block b)
{
	GpTransBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1,0);
}

void PutSMasr(s16 x, s16 y, s16 x1, s16 y1, s16 x11, s16 y11, s16 x21, s16 y21, block b)
{
	GpTransLRBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1,0);
}

void PutSImg(s16 x, s16 y, s16 x1, s16 y1,s16 x11, s16 y11, s16 x21, s16 y21, block b)
{
	GpBitBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1);
}

/* void  PutSBlink(s16 x, s16 y, s16 x1, s16 y1, s16 x11, s16 y11, s16 x21, s16 y21, block b)
{

	GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, CurrentColor);
	GpTransBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1,0);
}

void  PutSBlinkr(s16 x, s16 y, s16 x1, s16 y1,s16 x11, s16 y11, s16 x21, s16 y21, block b)
{
	GpRectFill(NULL,&gpDraw[nflip], x, y, x1, y1, CurrentColor);
	GpTransBlt(NULL,&gpDraw[nflip], x, y,
		x1,y1,
		(unsigned char*)b,
		0,0,x1,y1,0);
}
*/

unsigned int gettic(void)
{
	return GpTickCountGet();
}


int random(int r)
{
	return r?GpRandN(r):0;
}

void TurnIt(block s, u16 w, u16 h)
{
	char *b;
	char t;
	int x,y;
	int fpos = 0;
	
	b = (char*)gp_mem_func.malloc(w*h+400);
	for (x=0; x < w; ++x)
	{
		for (y= h - 1; y >= 0; --y)
		{
			t= *(s + x + y*w );
			//заношу повернутый символ
			*(b + fpos++) = (char)t;
		}
	}
	gp_str_func.memcpy(s,b,w*h);
	gp_mem_func.free(b);
}

//поворот с подсчетом прозр точек!!
int TurnItT(block s, u16 w, u16 h)
{
	char *b;
	char t;
	int x,y;
	int fpos = 0, n_transp = 0; //кол-во прозрачных точек
	
	b = (char*)gp_mem_func.malloc(w*h+400);
	for (x=0; x < w; ++x)
	{
		for (y= h - 1; y >= 0; --y)
		{
			t= *(s + x + y*w );
			//заношу повернутый символ
			*(b + fpos++) = (char)t;
			if (t == 0)
				n_transp++;
		}
	}
	gp_str_func.memcpy(s,b,w*h);
	gp_mem_func.free(b);
	return n_transp;
}

/*extern char str0[25];
s16 GetLibs(char *s, block f) {
	//для экономного открытия...
	str0[20]=0;
	if (gp_str_func.compare(str0,s)!=0) {
		gp_str_func.strcpy(str0,s);
		return GetLib(s,f);
	}
	return 0;
}*/