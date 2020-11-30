#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpstdio.h"

#include "bfont.h"
#include "../gpmain.h"

#include "debug.h"

extern GPDRAWSURFACE gpDraw;
extern char g_string[100];
extern char tmp_string[256];
int GetLib(char *, unsigned char *);
unsigned int SizeLib(char *);

int d_xpos=0,d_ypos=10;
void dp(int i);

#define bmp_offset 1078
//load font.bmp 256 colours

int LoadFont(char *font_name,struct BFont *font, unsigned char *color)
{
	int i,x1,y1,c0,c1,c2,c3;

	ERR_CODE err_code;
	//F_HANDLE h_file;
	F_HANDLE h_rfile;
	unsigned long n_dummy;

	//bitmap font file size
	unsigned long font_file_size;

	unsigned char * font_raw;
	//position within the temp raw font data
	unsigned long fpos,fpos2;

	unsigned int cx,cy,mark_color, shift_char;
	unsigned int x,t,line_width,line_chars_len;
	//int y;
	
	BGFONTINFO	font_info;
	
	//грузим системный шрифт, т.е. не грузим ничего
	//а тока узнаем его параметры
	if ( gp_str_func.compare((char *)font_name, (const char *)"system")==0 )
	{
		font->type=2;
		GpSysFontGet(&font_info);
		font->width = font_info.eng_w;
		font->kor_w = font_info.kor_w; // блин... пришлось добавить
		font->height = font_info.eng_h-1;
		font->chargap = font_info.chargap;
		font->linegap = font_info.linegap;
		font->source = NULL;
		font->color[0] = color[0];
		return SM_OK;
	}
/*	else
	{	
		//пропорциональный шрифт грузим
		font->type=0;
	}


	err_code = GpFileGetSize(font_name, &font_file_size);
	//GpTextOut(NULL, &gpDraw, 0, 0, ( char*)"Looking for bitmap font", 0x0);

	err_code = GpFileOpen(font_name, OPEN_R, &h_rfile);
	if (err_code != SM_OK)
	{
		return 1;
	}


	font->source = (unsigned char*)gp_mem_func.malloc((int)font_file_size-bmp_offset+8);
	font_raw= (unsigned char*)gp_mem_func.malloc((int)font_file_size-bmp_offset+8);

	//1078 - offset from the start to the RAW pic data
	err_code = GpFileSeek(h_rfile, FROM_BEGIN, bmp_offset, (long*)&n_dummy);
	if (err_code != SM_OK)
 	{
		return 1;
	}

	err_code = GpFileRead(h_rfile, font_raw, font_file_size-bmp_offset, (ulong*)&n_dummy);
	if (err_code != SM_OK)
	{
		return 1;
	}

	//расстояние м-ду символами и стоками по умолчанию
	font->chargap = 1;
	font->linegap = 2; //0

	//getting the font's parameters (colours, transp, sizes)	
	fpos = 0;

	font->width = (unsigned char)*(font_raw + fpos++);
	font->height = (unsigned char)*(font_raw + fpos++);
	cx = (unsigned char)*(font_raw + fpos++);
	cy = (unsigned char)*(font_raw + fpos++);

	if (cx<=1) cx = 1;
	if (cy<=1) cy = 1;
	if (font->width<=3) font->width=3;
	if (font->height<=3) font->height = 3;
	//ширина в строчки пичкелов в пикселах
	//и вообще размер будущеей матрицы
	font->mw = (line_width = font->width*cx);
	font->mh = font->height*cy;
	//кол-во байтов м-ду символами в 2х ближних строках
	line_chars_len = line_width*font->height;

	//skip a line to get transp an dmark colours
	fpos += (cx*font->width - 4);
	//transparent colour
	font->transparent = *(font_raw + fpos++);
	//the colour which marks the end of charas
	mark_color = *(font_raw + fpos++);
	//сдвиг... для неполных шрифтов
	shift_char = *(font_raw + fpos++);
	if (cx*cy == 256)
		shift_char = 0;	//у полных шрифтов нет сдвига

	//skip a line to get набор цветов
	fpos += (cx*font->width - 3);
	for (i=0; i<4; i++)
	{
		font->color[i] = *(font_raw + fpos++);
	}
	c0=font->color[0];
	c1=font->color[1];
	c2=font->color[2];
	c3=font->color[3];

	//loading all fonts characters
	for (y1=0; y1 < cy; ++y1)
	{
		for (x1=0; x1 < cx; ++x1)
		{
			i=x1+y1*cx; //номер чара
			//начало символа, его...1й левый верхний байт
			fpos= x1 * font->width + y1 * line_chars_len;

			//calculating max char's width
			font->c[i + shift_char].width = font->width;
			font->c[i + shift_char].height = font->height;
			font->c[i + shift_char].x = x1 * font->width;
			font->c[i + shift_char].y = y1 * font->height;

			for (x=0; x < font->width; x++)
			{
				//looking for the mark_color
				if ( *(font_raw + fpos+x) == mark_color)
				{
					font->c[i + shift_char].width = x;
					break;
				}
			}

			//dp(font->c[i].width);
			//dp(fpos2);
			//__kbhit();
			//__printf("\nch %d [%dx%d]",i,font->c[i].width,font->c[i].height);
			//если символ не нулевой и не пуст, то копируем данные
		}

	}

	//а теперь готовим матрицу шрифтов - копируем с переворачиванием и меняем по ходу цвета как надо
	fpos2 = 0;
	for (x1=0; x1 < font->mw; ++x1)
	{
		for (y1= (font->mh - 1); y1 >= 0; --y1)
		{
			t= *(font_raw+ x1 + y1*line_width );

			if (t==c0)
				t = color[0];
			else if (t==c1)
				t = color[1];
			else if (t==c2)
				t = color[2];
			else if (t==c3)
				t = color[3];
			else
				t = font->transparent;

			//заношу повернутый символ
			*(font->source + fpos2++) = (char)t;
		}
	}

	//заменить цвета на указанные
	font->color[0]=color[0];
	font->color[1]=color[1];
	font->color[2]=color[2];
	font->color[3]=color[3];

	//пробел должен быть пробелом! для сдвинутых фонтов
	if (shift_char != 0)
	{
		font->c[32].width = (font->width*2)/3;
		font->c[32].height = 0;
	}

	// проверить чтобы транспарент не совпадал с этими цветами...

	gp_mem_func.free(font_raw);

	GpFileClose(h_rfile);
	return SM_OK;
*/
}

int FmlLoadFont(char *font_name,struct BFont *font, unsigned char *color)
{
	int i,x1,y1,c0,c1,c2,c3;
	unsigned long fpos,fpos2;
	unsigned char * font_raw;
	unsigned int cx,cy,mark_color, shift_char;
	unsigned int x,t,line_width,line_chars_len;
	unsigned long font_file_size;
	//int y;
	
	//BGFONTINFO	font_info;
	
	//пропорциональный шрифт грузим
	font->type=0;
	//расстояние м-ду символами и стоками по умолчанию
	font->chargap = 1;
	font->linegap = 2;	//0

	font_file_size=SizeLib(font_name);
	font->source = (unsigned char*)gp_mem_func.malloc((int)font_file_size-bmp_offset+8);
	font_raw= (unsigned char*)gp_mem_func.malloc((int)font_file_size+8);

	GetLib(font_name, font_raw);

	//1078 - offset from the start to the RAW pic data
	fpos = bmp_offset;

	//getting the font's parameters (colours, transp, sizes)	
	font->width = (unsigned char)*(font_raw + fpos++);
	font->height = (unsigned char)*(font_raw + fpos++);
	cx = (unsigned char)*(font_raw + fpos++);
	cy = (unsigned char)*(font_raw + fpos++);

	if (cx<=1) cx = 1;
	if (cy<=1) cy = 1;
	if (font->width<=3) font->width=3;
	if (font->height<=3) font->height = 3;

	//__printf("cx %d, cy %d, w %d, h %d !!\n",cx,cy,font->width,font->height);

	//ширина в строчки пичкелов в пикселах
	//и вообще размер будущеей матрицы
	font->mw = (line_width = font->width*cx);
	font->mh = font->height*cy;
	//кол-во байтов м-ду символами в 2х ближних строках
	line_chars_len = line_width*font->height;

	//skip a line to get transp an dmark colours
	fpos += (cx*font->width - 4);
	//transparent colour
	font->transparent = *(font_raw + fpos++);
	//the colour which marks the end of charas
	mark_color = *(font_raw + fpos++);
	//сдвиг... для неполных шрифтов
	shift_char = *(font_raw + fpos++);
	if (cx*cy == 256)
		shift_char = 0;	//у полных шрифтов нет сдвига

	//skip a line to get набор цветов
	fpos += (cx*font->width - 3);
	for (i=0; i<4; i++)
	{
		font->color[i] = *(font_raw + fpos++);
	}
	c0=font->color[0];
	c1=font->color[1];
	c2=font->color[2];
	c3=font->color[3];

	//__printf("schit shirinu\n");

	//loading all fonts characters
	for (y1=0; y1 < cy; ++y1)
	{
		for (x1=0; x1 < cx; ++x1)
		{
			//__printf("y1 %d,x1 %d |",y1,x1);
			i=x1+y1*cx; //номер чара
			//начало символа, его...1й левый верхний байт
			fpos= x1 * font->width + y1 * line_chars_len + bmp_offset;

			//calculating max char's width
			font->c[i + shift_char].width = font->width;
			font->c[i + shift_char].height = font->height;
			font->c[i + shift_char].x = x1 * font->width;
			font->c[i + shift_char].y = y1 * font->height;

			for (x=0; x < font->width; x++)
			{
				//looking for the mark_color
				if ( *(font_raw + fpos+x) == mark_color)
				{
					font->c[i + shift_char].width = x;
					break;
				}
			}

			//dp(font->c[i].width);
			//dp(fpos2);
			//__kbhit();
			//__printf("\nch %d [%dx%d]",i,font->c[i].width,font->c[i].height);
			//если символ не нулевой и не пуст, то копируем данные
		}

	}

	//__printf("gotovim matr cvo-v\n");

	//а теперь готовим матрицу шрифтов - копируем с переворачиванием и меняем по ходу цвета как надо
	fpos2 = 0;
	for (x1=0; x1 < font->mw; ++x1)
	{
		for (y1= (font->mh - 1); y1 >= 0; --y1)
		{
			t= *(font_raw+bmp_offset+ x1 + y1*line_width );

			if (t==c0)
				t = color[0];
			else if (t==c1)
				t = color[1];
			else if (t==c2)
				t = color[2];
			else if (t==c3)
				t = color[3];
			else
				t = font->transparent;

			//заношу повернутый символ
			*(font->source + fpos2++) = (char)t;
		}
	}

	//заменить цвета на указанные
	font->color[0]=color[0];
	font->color[1]=color[1];
	font->color[2]=color[2];
	font->color[3]=color[3];

	//пробел должен быть пробелом! для сдвинутых фонтов
	if (shift_char != 0)
	{
		font->c[32].width = (font->width*2)/3;
		font->c[32].height = 0;
	}

	// проверить чтобы транспарент не совпадал с этими цветами...

	gp_mem_func.free(font_raw);

	return SM_OK;
}

void Put1Chara(GPDRAWTAG * gptag, GPDRAWSURFACE *gpDraw, struct BFont *font, int x1, int y1, unsigned char i)
{
	if (i==0 || i>255 || font->type == 2 || font->c[i].width==0) return;
	//добавить проверку max кол-во символов в шрифте
	GpTransBlt(gptag,
		gpDraw, 
		x1,y1, 
		font->c[i].width, font->c[i].height,
		font->source,
		font->c[i].x, font->c[i].y,
		font->mw,font->mh,
		font->transparent
		);
}

void Put1TrChara(GPDRAWTAG * gptag, GPDRAWSURFACE *gpDraw, struct BFont *font, int x1, int y1, unsigned char i)
{
	if (i==0 || i>255 || font->type == 2 || font->c[i].width==0) return;
	//добавить проверку max кол-во символов в шрифте
	GpBitBlt(gptag,
		gpDraw, 
		x1,y1, 
		font->c[i].width, font->c[i].height,
		font->source,
		font->c[i].x, font->c[i].y,
		font->mw,font->mh
		);
}

void BCharOut(GPDRAWTAG * gptag, GPDRAWSURFACE *ptgpds ,struct BFont *font,int x,int y,char* sour)
{
	int init_x = x;
	int init_def_y;
	
	if (font->type == 2)
	{
		GpCharOut(gptag, ptgpds, x, y, sour, font->color[0]);
		return;
	}
	
	init_def_y = y;
	x = init_x;
	while(* sour != 0)
	{
		Put1Chara(gptag, ptgpds, font, x, init_def_y,(unsigned char)*sour);
		x += font->c[(unsigned char)*sour++].width + font->chargap;
	}
}

void BTextOut(GPDRAWTAG * gptag, GPDRAWSURFACE *ptgpds ,struct BFont *font,int x,int y,char* src)
{
	char *sour = src;
	int init_x = x;
	int init_def_y;

	if (font->type == 2)
	{
		GpTextOut(gptag, ptgpds, x, y, sour, font->color[0]);
		return;
	}

	init_def_y = y;
	x = init_x;
	while(* sour != 0)
	{
		if ((*sour) == '\r' || (*sour) == '\n')
		{
			sour++;
			x = init_x;
			init_def_y += font->height + font->linegap;
			if ((*sour) == '\r' || (*sour) == '\n')
				sour++;
		}
		else
		{
			Put1Chara(gptag, ptgpds, font, x, init_def_y,(unsigned char)*sour);
			x += font->c[(unsigned char)*sour++].width + font->chargap;
		}
	}
}


//вычислить ширину строки в пикселях
int BTextWidthGet(struct BFont *font, const char * lpsz)
{
	int result = 0, cur_result = font->chargap;

	if (font->type == 2)
	{
		return GpTextWidthGet(lpsz);
	}

	while(*lpsz != 0)
	{
		if ((*lpsz) == '\r' || (*lpsz) == '\n')
		{
			lpsz++;
			if (cur_result > result)
			{
				result = cur_result;
				cur_result = font->chargap;
			}
			if ((*lpsz) == '\r' || (*lpsz) == '\n')
				lpsz++;
		}
		else
		{
			cur_result += font->c[*lpsz++].width + font->chargap;
		}
	}
	if (cur_result > result)
		result = cur_result;
	if (result <= font->chargap)
		result = 0;
	return result;
}

//Высота текста в пикселях 
int BTextHeightGet(struct BFont *font, const char * lpsz)
{
	int result = font->height;

	if (font->type == 2)
	{
		return GpTextHeightGet(lpsz);
	}

	if (*lpsz == 0)
		return 0;
	while(*lpsz != 0)
	{
		if ((*lpsz) == '\r' || (*lpsz) == '\n')
		{
			lpsz++;
			result += (font->height + font->linegap);
			if ((*lpsz) == '\r' || (*lpsz) == '\n')
			{
				result += (font->height + font->linegap);
				lpsz++;
			}
		}
		else
		{
			lpsz++;
		}
	}
	return result;
}

//длина строки не считая символов ПК и ВК
int BTextLenGet(struct BFont *font, const char * str)
{
	int result = 0;
	
	if (font->type == 2)
	{
		return GpTextLenGet(str);
	}

	while (*str)
	{
		if (*str == '\r' || *str == '\n')
		{
			str++;
			if (result)
				result--;
		}
		else
			str++;
		result++;
	}
	return result;
}

void BTextNOut(GPDRAWTAG * gptag, GPDRAWSURFACE * ptgpds,struct BFont *font, int x, int y, 
	char * sour, int nStart, int nString)
{
	char	* p_sub;
	
	if (font->type == 2)
	{
		GpTextNOut(gptag, ptgpds, x, y, sour, nStart, nString, font->color[0]);
		return;
	}

	p_sub = (char *)gp_mem_func.malloc(nString + 1);
	gp_str_func.strncpy((char *)p_sub, (const char *)(sour + nStart), nString);
	p_sub[nString] = 0;
	BTextOut(gptag,
		ptgpds,
		font,
		x,
		y,
		p_sub);
	gp_mem_func.free((void *)p_sub);
}

