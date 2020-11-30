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

extern block items;
extern block additems;
extern block modelspr;
extern block menus;
extern block logomenu;
extern block shop_bg;

extern unsigned char waitboom;
extern unsigned char waitmess;
extern s16 messy;
extern signed char messsy;
extern char *mess; //подсказка
extern block textindex[100]; //ссылки на строки текста 0,1...100

extern struct BFont text_font;
extern struct BFont big_font;
extern struct BFont big_black_font;
extern struct BFont big_green_font;
extern struct BFont big_red_font;

extern char stroka[300];

extern block obloka; //под взрывы
extern block enbull; //под вражеские снаряды

extern screen bckg; //задний фон

extern struct hero man;
//extern unsigned int tic; //для задержки по таймеру

extern s16 coins;	//монетки
extern s16 hearts;	//сердечки
extern s16 keys;	//ключи
extern s16 beams;	//шишки
extern s16 shields;	//защита
extern s16 power;	//энергия
extern s16 times;	//время
extern u32 score,hiscore; //очки
extern s16 continues;	//продолжения
extern signed char maxjump;	//высота прыжка
extern char maxspeed;	//максимальная скорость
extern char glass;	//очки
extern char shboots;	//шиповки
extern char spboots;	//скоростные башмаки
extern char jetpack;	//ранец
extern char kolun;	//колун

extern s16 end,pause,key_f,key_b;
extern s16 left,right,up,down,jump,fire;
extern s16 t_left,t_right,t_up,t_down,t_jump,t_fire;

struct sshop shop[12]={
	{05,NULL,NULL,16,16,&items,256*8*4,7},	//cone
	{10,NULL,NULL,16,16,&items,256*8,7},		//"Honey"
	{25,NULL,NULL,16,16,&items,256*8*5,7},	//"Time"
	{99,NULL,NULL,16,16,&items,256*8*2,7},	//"+1 Live"
	{100,NULL,NULL,16,16,&additems,0,1},		//"+1 Continue"
	{30,NULL,NULL,16,16,&items,256*8*6,7},	//"Super Power"
	{50,NULL,NULL,16,16,&additems,256*12,1},	//"Super Shield"
	{110,NULL,NULL,16,16,&additems,256*8,1},	//"Super Lens"
	{120,NULL,NULL,16,16,&additems,256*2,1},	//"Super Jump"
	{175,NULL,NULL,16,16,&additems,256*4,1},	//"Super Thorn Boots"
	{145,NULL,NULL,16,16,&additems,256*10,1},	//"Super Crash Boots"
	{250,NULL,NULL,16,16,&additems,256*6,1}	//"Super Jet"
};
/*struct sshop shop[12]={
	{05,"Cone",16,16,&items,256*8*4,7},
	{10,"Honey",16,16,&items,256*8,7},
	{25,"Time",16,16,&items,256*8*5,7},
	{99,"+1 Live",16,16,&items,256*8*2,7},
	{100,"+1 Continue",16,16,&additems,0,1},
	{30,"Super Power",16,16,&items,256*8*6,7},
	{50,"Super Shield",16,16,&additems,256*12,1},
	{110,"Super Lens",16,16,&additems,256*8,1},
	{120,"Super Jump",16,16,&additems,256*2,1},
	{175,"Super Thorn Boots",16,16,&additems,256*4,1},
	{145,"Super Crash Boots",16,16,&additems,256*10,1},
	{250,"Super Jet",16,16,&additems,256*6,1}
};
*/

void buy_item(int i);
int can_buy(int i);
void shopmess(char *m,unsigned char w);

void doshop(s16 maxit) {
//магазин
// A = buy, B = cancel
	int i,j,k,keydata,x=0,y=0;

	//защита от залипания клавиши
/*	GpKeyGetEx(&keydata);
	while (keydata)
	{
		GpKeyGetEx(&keydata);
		Delay(50);
	}
*/
	waitmess=0;
	//инструкцию показать
	shopmess(textindex[45],40);

	//цикл выбора
	i=0;

	//!!! УБРАТЬ ПОТОМ ЭТО ДЛЯ ОТЛАДКИ
#ifndef RELEASE
	//maxit = 12; //random(12);
#endif
#ifndef DEMO
	maxit = min(max(1,maxit),12);
#else
	maxit = min(max(1,maxit),4); //для демки урезаем кол-во товара в магазе
#endif

	keydata = 0;
	while(1)
	{
		//фон
		do_strip(0);
		show_strip(0);
	
		GpTransBlt(NULL,&gpDraw[nflip], //картинка магазина - тележки
			320-264, 0, 264,240,(unsigned char*)shop_bg,
			0,0,
			264,240,
			0);

		//название SHOP
		GpTransBlt(NULL,&gpDraw[nflip],
			//88,16,144,24,(unsigned char*)menus,
			88,240-40,144,24,(unsigned char*)menus,
			176,72,
			320,200,
			0);
#define sh_py 100-11
#define sh_px 81

		//cursor
		GpTransBlt(NULL,&gpDraw[nflip], //верх
			sh_px+x*32 -8, sh_py+32*y -8, 48,48,(unsigned char*)logomenu,
			256,0,
			320,200,
			0);

		waitboom++;

		i = y*4+x;

		//выводим слоты магазина
		for (j=0; j<maxit; j++)
		{
			if ( j == i )
			{
				//выводим текущий предмет покупки с объяснениями
				//название вещи
				gp_str_func.sprintf(stroka, "%s",shop[j].name);
				BTextOut(NULL, &gpDraw[nflip], &big_black_font, 8+1, 27+16+1, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip], can_buy(i)==1?&big_font:(can_buy(i)==-1?&big_red_font:&big_green_font), 8, 27+16, (char*)stroka);
				//цена вещи
				gp_str_func.sprintf(stroka, "$%u",shop[j].price);
				BTextOut(NULL, &gpDraw[nflip], &big_black_font, 8+1, 36+64+1, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip], (coins>=shop[j].price)?&big_font:&big_red_font, 8, 36+64, (char*)stroka);
				//полное название вещи
				gp_str_func.sprintf(stroka, "%s",shop[j].description);
				text_font.color[0] = 8;	//Черный
				BTextOut(NULL, &gpDraw[nflip], &text_font, 8+1, 48+16+1, (char*)stroka);
				//BTextOut(NULL, &gpDraw[nflip], &text_font, 8+1, 50+16-1, (char*)stroka);
				//BTextOut(NULL, &gpDraw[nflip], &text_font, 8-1, 50+16+1, (char*)stroka);
				//text_font.color[0] = 8;	//светло черный
				//BTextOut(NULL, &gpDraw[nflip], &text_font, 8-1, 50+16-1, (char*)stroka);
				text_font.color[0] = 15; //белый
				BTextOut(NULL, &gpDraw[nflip], &text_font, 8, 48+16, (char*)stroka);

				//сама вещь в слоте
				if ( ++k>shop[j].mf ) k=0;
				PutMtb(sh_px+(j&3)*32, sh_py+32*(j/4),
				shop[j].lx,shop[j].ly,32,32,
				*shop[j].buf+shop[j].sm+k*256);
			} else {
				//сама вещь в слоте
				PutMas(sh_px+(j&3)*32 + 8, sh_py+32*(j/4) + 8,
				16,16,
				*shop[j].buf+shop[j].sm);
				//PutMtb(sh_px+(j&3)*32 + 8, sh_py+32*(j/4) + 8,
				//shop[j].lx,shop[j].ly,16,16,
				//*shop[j].buf+shop[j].sm);
			}

		}

		//если есть POWER - тоже выведем
		if ( power ) {
			PutMas16(300,20+20*8,items+256*8*6+((waitboom&7)*256));
		}
		put_score();put_additems();

		//	messy=/*176+40*/4; messsy=/*-5*/3;
		if( waitmess>0 ) {
			waitmess--;
			text_font.color[0] = 8;
			BTextOut(NULL, &gpDraw[nflip], &text_font, 8+1, messy+1, (char*)mess);
			text_font.color[0] = 12+(waitboom&3);
			BTextOut(NULL, &gpDraw[nflip], &text_font, 8, messy, (char*)mess);
			if( messsy>0 || waitmess<10) {
				messy+=(--messsy);
			}
		}

		FlipAndShow();

		Delay(100);
		//ждем нажатия клавиши
		GpKeyGetEx(&keydata);

		//keyboard
		if ( keydata & GPC_VK_UP ) //press UP
		{
			y--;
			if (y<0)
			{
				y = 2;
			}
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_DOWN ) //press DOWN
		{
			y++;
			if (y > 2 )
			{
				y = 0;
			}
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_LEFT ) //press LEFT
		{
			x--;
			if (x<0)
			{
				x = 3;
			}
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_RIGHT ) //press RIGHT
		{
			x++;
			if (x > 3 )
			{
				x = 0;
			}
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_FR ) //press FR
		{
			x = 3;
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_FL ) //press FL
		{
			x = 0;
			SE(sfx_cursor);
		}
		if ( (keydata & GPC_VK_FA) || (keydata & GPC_VK_START) ) //press A or START = accept
		{
			//выбран...
			if (i < maxit)
				buy_item(i);
			else {
				SE(sfx_cancel);
				if (random(10)<8)
					//не могу купить нет такой вещи.
					shopmess(textindex[48],20);
				else
					//инструкция
					shopmess(textindex[45],20);
			}
			Delay(100);
			//return;
		}
		if ( keydata & GPC_VK_FB ) //press B = cancel
		{
			//не выбран...
			SE(sfx_cancel);
			while (keydata)
			{
				GpKeyGetEx(&keydata);
				Delay(50);
			}
			waitmess=0;
			return;
		}

	}
}

//можно ли купить эту вещь
int can_buy(int i)
{
	//проверяем - можем ли мы купить вещь
	if (coins < shop[i].price)
		return -1;	//нет денег

	switch (i) {
		case 0:
			//ШИШКА
			if ( beams<99)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 1:
			//МЕД
			if ( man.on<5)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 2:
			//ЧАСЫ
			if (times<5000)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 3:
			//ЖИЗНИ
			if (hearts<9)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 4:
			//continues
			if (continues<9)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 5: //энергия
			if (power<400)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 6: //ЗАЩИТА
			if (shields==0)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 7:	//очки
			if (glass==0)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 8:
			//пружинка
			if (maxjump>-10)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 9:
			//шиповки
			if (shboots==0)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 10:
			//колун
			if (kolun==0)
				return 1;
			else
				return -2; //уже есть до отвалу
		case 11:
			//ранец
			if (jetpack==0)
				return 1;
			else
				return -2; //уже есть до отвалу
	}
	return -2;	//не можем купить - у нас уже есть
}

void buy_item(int i)
{
	int can_buy_it = can_buy(i);
	if ( can_buy_it == 0 )
	{
		SE(sfx_error);
		if (random(10)<8)
			//не могу купить нет такой вещи.
			shopmess(textindex[48],20);
		else
			//инструкция
			shopmess(textindex[45],20);
		return;
	}
	if ( can_buy_it == -1 )
	{
		SE(sfx_error);
		//не могу купить - нет денег
		shopmess(textindex[47],20);
		return;
	}
	if ( can_buy_it == -2 )
	{
		SE(sfx_error);
		//не могу купить уже есть такая вещь
		shopmess(textindex[46],20);
		return;
	}
	SE(sfx_OK);
	//покупаем вещь
	switch (i) {
		case 0:
			//ШИШКА
			if ( beams<99) {
				coins-=shop[i].price;
				++beams;
			}
			break;
		case 1:
			//МЕД
			if ( man.on<5) {
				coins-=shop[i].price;
				++man.on;
			}
			break;
		case 2:
			//ЧАСЫ
			if (times<5000) {
				coins-=shop[i].price;
				times=9999;
			}
			break;
		case 3:
			//ЖИЗНИ
			if (hearts<9) {
				coins-=shop[i].price;
				++hearts;
			}
			break;
		case 4:
			//continues
			if (continues<9) {
				coins-=shop[i].price;
				++continues;
			}
			break;
		case 5: //энергия
			if (power<400) {
				coins-=shop[i].price;
				power=500;
			}
			break;
#ifndef DEMO
		case 6: //ЗАЩИТА
			if (shields==0) {
				coins-=shop[i].price;
				shields=1;
			}
			break;
		case 7:	//очки
			if (glass==0) {
				coins-=shop[i].price;
				glass=1;
			}
			break;
		case 8:
			//пружинка
			if (maxjump>-10) {
				coins-=shop[i].price;
				--maxjump;
			}
			break;
		case 9:
			//шиповки
			if (shboots==0) {
				coins-=shop[i].price;
				shboots=1;
			}
			break;
		case 10:
			//колун
			if (kolun==0) {
				coins-=shop[i].price;
				kolun=1;
			}
			break;
		case 11:
			//ранец
			if (jetpack==0) {
				coins-=shop[i].price;
				jetpack=1;
			}
			//break;
#endif
	}
}

void shopmess(char *m,unsigned char w) {
	waitmess=w; mess=m;
	messy=/*176+40*/-8; messsy=/*-6*/6;
}