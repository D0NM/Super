#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpstdio.h"

#include "gp32\file_dialog.h"
#include "gp32\debug.h"

#include "language.h"
#include "famegraf.h"
#include "all.h"
#include "bfont.h"

extern GPDRAWSURFACE gpDraw[2];
extern int nflip;
extern block menus;	//менюшки... графические.
extern block logomenu;	//для облачков-курсора

extern struct BFont big_font;
extern struct BFont big_black_font;
extern struct BFont big_green_font;
extern struct BFont big_red_font;

extern char stroka[300];


//char savegame_ini_file[]="savegame.ini";
extern char savegame_ini_file[];

struct st_svgame {
	u32 random;
	u32 money;
	u32 level;
	u32 CRC;
	u32 highscore;
};

struct st_svgame savegame[20];	//20 слотов сохр. для бонус уровней тоже...

u16 savegames;	//есть ли сохраненные игры

void check_savegame(void);
void save_savegame(void);

// - показать слот для выбора
int select_savegame(int t);
// показ высших очков для демо
int show_highscore(void);


//-- для внутреннего пользования 
void init_savegame(void);
void encode_savegame(void);
void decode_savegame(void);
//- из famelib
//void _fmeminv(block s, u32 n);

void check_savegame(void)
{
// загружает сохраненные игры или инициализирует
	ERR_CODE err_code;
	F_HANDLE h_rfile;
	unsigned long n_dummy;

	//грузим language.ini файл

	err_code = GpFileOpen(savegame_ini_file, OPEN_R, &h_rfile);
	if (err_code != SM_OK)
	{
		//если нет INI файла или там ошибка то 
		init_savegame();
		save_savegame();
		return;
	}

	//ini файл открыт, считаем язык
	err_code = GpFileRead(h_rfile, savegame, sizeof(savegame), (ulong*)&n_dummy);
	if (err_code != SM_OK)
	{
		//если пустой файл то
		init_savegame();
		save_savegame();
		return;
	}

	_fmeminv((block)savegame,sizeof(savegame));

	//закроем INI файл 
	GpFileClose(h_rfile);
}

void init_savegame(void)
{
//инициализируем пустой savegame ерундой.
	int i;
	savegames = 0;
	for (i=0; i<20; i++)
	{
		savegame[i].highscore = (i+1)*10000/*+random(500)*/;
		savegame[i].level = 0;	//i+2;
		savegame[i].money = random(1000);
		savegame[i].random = random(64000);
	}
}

int select_savegame(int t)
{
// показать список сохраненных слотов... перемещение по ним
// arg t - 0=load, 1= continue
// A, START = accept, B = cancel
	int i,j,l,keydata,op=0;

	//защита от залипания клавиши
	GpKeyGetEx(&keydata);
	while (keydata)
	{
		GpKeyGetEx(&keydata);
		Delay(50);
	}


	//цикл выбора
	l = i = 0;

	keydata = 0;
	while(1)
	{
		//фон
		//GpRectFill(NULL, &gpDraw[nflip], 0, 0, 320, 240, 8);
		do_strip(0);
		show_strip(0);

		//название операции - типа сэйв или продолж
		if (t) {	//save
			GpTransBlt(NULL,&gpDraw[nflip],
				44,8, 232,24, (unsigned char*)menus,
				0,144,
				320,200,
				0);
		} else
		{	//continue
			GpTransBlt(NULL,&gpDraw[nflip],
				72,8, 176,24, (unsigned char*)menus,
				0,24,
				320,200,
				0);
		}

#define ssg_py 42

		//выводим строки файлов
		for (j=0; j<6; j++)
		{
			//cursor
			if ( j == i )
			{
				GpRectFill(NULL, &gpDraw[nflip], 0, j*32 + ssg_py +2, 320, 24, 136); //синий
				GpTransBlt(NULL,&gpDraw[nflip], //верх
					op, j*32 + ssg_py -6, 320,8,(unsigned char*)logomenu,
					0,184,
					320,200,
					0);
				GpTransBlt(NULL,&gpDraw[nflip], //верх
					op-320, j*32 + ssg_py -6, 320,8,(unsigned char*)logomenu,
					0,184,
					320,200,
					0);

				GpTransBlt(NULL,&gpDraw[nflip], //низ
					op, j*32 + ssg_py + 24 +2, 320,8,(unsigned char*)logomenu,
					0,192,
					320,200,
					0);
				GpTransBlt(NULL,&gpDraw[nflip], //низ
					op-320, j*32 + ssg_py + 24 +2, 320,8,(unsigned char*)logomenu,
					0,192,
					320,200,
					0);
				op--;	//скролл курсора - облаков
				if (op < 0)
					op = 320;
			}

			//слот сохранёнки
			if ( savegame[j].level == 0 )
			{
				BTextOut(NULL, &gpDraw[nflip], (i==j)?&big_red_font:&big_font, 16+8+12, j*32 + ssg_py+4, t?(char*)"FREE SAVE SLOT":(char*)"START NEW GAME");
			}
			else
			{
				gp_str_func.sprintf(stroka, "LEVEL %1u, $%u",savegame[j].level,savegame[j].money);
				BTextOut(NULL, &gpDraw[nflip], (i==j)?&big_red_font:&big_green_font, 16+32+4, j*32 + ssg_py+4, (char*)stroka);
			}
		}

		FlipAndShow();

		//ждем нажатия клавиши
		GpKeyGetEx(&keydata);
#define menu_cycles_wait 2
		//keyboard
		if ( keydata & GPC_VK_FR ) //press FR
		{
			i = 5;
			l = 0;
			//SE(sfx_cursor);
		} else
		if ( keydata & GPC_VK_FL ) //press FL
		{
			l = i = 0;
			//SE(sfx_cursor);
		} else
		if ( keydata & GPC_VK_UP ) //press UP
		{
			if (l>menu_cycles_wait ) {
				i--;
				if (i<0)
				{
					i = 5;
				}
				l = 0;
				SE(sfx_cursor);
			}
		} else
		if ( keydata & GPC_VK_DOWN ) //press DOWN
		{
			if (l>menu_cycles_wait ) {
				i++;
				if (i > 5)
				{
					i = 0;
				}
				l = 0;
				SE(sfx_cursor);
			}
		} else
		if ( (keydata & GPC_VK_FA) || (keydata & GPC_VK_START) ) //press A or START = accept
		{
			//выбран...
			SE(sfx_OK);
			return i;
		} else
		if ( keydata & GPC_VK_FB ) //press B = cancel
		{
			//не выбран...
			SE(sfx_cancel);
			return -1;
		}
		l++;
		Delay(45);
	}
}

void encode_savegame(void)
{
	//закодируем сохранку
	int i,p;

	//добавляем сер номер
	GpUserInfoGet(stroka, stroka);
	p = (stroka[0]+stroka[1]+stroka[2]+stroka[3])&255;
	//добавить скорость geepee 8) к рассчету

	for (i=0; i<20; i++)
	{
		savegame[i].highscore = 3 * savegame[i].highscore + 7;
		savegame[i].level *= 901;
		savegame[i].money *= 3;
		savegame[i].random = random(64000);
		savegame[i].CRC = savegame[i].highscore + savegame[i].level + savegame[i].money + savegame[i].random + 123*(i+1) + p;
	}
}

void decode_savegame(void)
{
	//раскодируем сохранку
	int i,p;

	savegames = 0;

	//добавляем сер номер
	GpUserInfoGet(stroka, stroka);
	p = (stroka[0]+stroka[1]+stroka[2]+stroka[3])&255;

	for (i=0; i<20; i++)
	{
		//проверим CRC
		if ( savegame[i].CRC == savegame[i].highscore + savegame[i].level + savegame[i].money + savegame[i].random + 123*(i+1) + p && savegame[i].level <= 20 && savegame[i].money <= 9999 )
		{
			//все ок.
			savegame[i].highscore = (savegame[i].highscore - 7) / 3;
			savegame[i].level /= 901;
			savegame[i].money /= 3;
			savegames++;
			savegame[i].random = random(64000);
			savegame[i].CRC = random(64000);
		}
		else
		{
			//нас взломали или сбой
			//savegame[i].highscore = (i+1)*1000+random(500);
			savegame[i].highscore = (i+1)*10000;
			savegame[i].level = 0;
			savegame[i].money = random(64000);
			savegame[i].random = random(64000);
		}
	}
}

void save_savegame(void)
{
	//запишим имя языка в ини файл
	ERR_CODE err_code;
	F_HANDLE h_rfile;

	_fmeminv((block)savegame,sizeof(savegame));

	err_code = GpFileCreate(savegame_ini_file, ALWAYS_CREATE, &h_rfile);
	if (err_code != SM_OK)
	{
		//если нет INI файла или там ошибка то
		fatalerror("Error: Can't create gpsys\\plusha.sav...");
	}

	//ini файл открыт
	err_code = GpFileWrite(h_rfile, savegame, sizeof(savegame));
	if (err_code != SM_OK)
	{
		fatalerror("Error: Can't save the game...");
	}

	_fmeminv((block)savegame,sizeof(savegame));

	//закроем INI файл 
	GpFileClose(h_rfile);
}

int show_highscore(void)
{
	//проверить по сохраненке какие есть уровни записаны
	//и показать столько 
	int i,j=0,keydata;

	//таблица рекордов пуста (не показываем её)
	if ( savegame[0].highscore == 10000 )
		return 1;

	keydata = 0;
	while( keydata == 0)
	{
		//фон
		do_strip(0);
		show_strip(0);

		//титул
		BTextOut(NULL, &gpDraw[nflip], &big_black_font, 88+2, 8+2, (char*)"HI-SCORE");
		BTextOut(NULL, &gpDraw[nflip], &big_green_font, 88, 8, (char*)"HI-SCORE");

		//выводим строки файлов
		for (i=0; i<7; i++)
		{
			//не показываем рекорды дальше уровней, на которых не были
			if ( savegame[i].highscore == 10000 * (i+1) )
				break;
			gp_str_func.sprintf(stroka, "LEVEL %1u : %07u",i+1,savegame[i].highscore);
			BTextOut(NULL, &gpDraw[nflip], &big_black_font, 20+1, i*29 + ssg_py+1-4, (char*)stroka);
			BTextOut(NULL, &gpDraw[nflip], &big_font, 20, i*29 + ssg_py-4, (char*)stroka);
		}

		FlipAndShow();

		//ждем нажатия клавиши
		GpKeyGetEx(&keydata);
		Delay(50);
		if (++j > 150)
			return 1;	//выход по времени
	}
	return 0;	//выход по нажатию
}