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
#include "modplayer\modplayer.h"

#include "famegraf.h"
#include "all.h"
#include "language.h"

//профилер
#include "profiler\sdsfonts.h"

int main_menu(int i);

extern GPDRAWSURFACE gpDraw[2];
extern int nflip;

unsigned int tic; //для задержки по таймеру

block demobuf; //буфер под демонстр нажатия клавишь
char demo=0; //демонстр 0выключена 1-включена
u16 pdemo;

block ind; //под разные значки и индикаторы

block trase;
block level;
block blevel;

block bfon;
block fon;
block manspr; //образы героя

block mnstrspr; //образы гадов
u32 pmnspr=0; //последний свободн байт

block items;	//спрайты
block additems;
block modelspr;
//block lifts;

block obloka; //под взрывы
block enbull; //под вражеские снаряды

//- для мультиязыковой поддержки
block menus;	//менюшки... графические.
block textdata;	//тексты на разн языках.
block textindex[200]; //ссылки на строки текста 0,1...200

block shop_bg;	//задний фон для магазина 266х240

block logomenu;	//логотип игры названия, FAME, фишки для менюх и т.д
block tiles;	//для TILES

extern unsigned char *mod_source;	//под музыку

screen bckg; //задний фон
screen hidscr; //скрытая страничка

//char serial_pc=1; // 1- обычная писишка, 0 - Trident 512k
//char speedraw; // 1,2 - для быстрой машины, 0 - медленная
//char draw; //отрисовывать ли сейчас что-либо?

char str0[25];
/*const*/ char stroka[300];
extern char g_string[100];

char levnum[4]="01";
s16 curr_level=1;

char files[10][13]={ //файлы под уровни
	"01def.bin",
	"01level.bin",
	"01blevel.bin",
	"01monstr.bin",
	"01monstr.def",
	"01models.bin",
	"01demo.bin",
	"01fon.bin",
	"01bckg.bin",
	"01strip.def"
};

struct levdef leveldef;

//размер уровня
u16 siz_xlev=500;
u16 siz_ylev=60;
u32 siz_level=500*60; //=siz_xlev*siz_ylev;
u32 svel=9,cvel=8;

s16 x,y,sx,sy; //тек положение точки
unsigned char *poslab; //тек поз точки в массиве трассы
unsigned char *poslal; //тек поз точки в массиве фона
unsigned char tekblock;

s16 coins;	//монетки
s16 hearts;	//сердечки
s16 keys;	//ключи
s16 beams;	//шишки
s16 shields;	//защита
s16 power;	//энергия
s16 times;	//время
u32 score; //очки
s16 continues;	//продолжения
signed char maxjump;	//высота прыжка
char maxspeed;	//максимальная скорость
char glass;	//очки
char shboots;	//шиповки
char spboots;	//скоростные башмаки
char jetpack;	//ранец
char kolun;	//колун

char boss;	//признак активности БОССА

u16 nsecret;	//# секретов
u16 nitems;	//# вещей

//герой
struct hero man;

//враги
struct object obj[maxobj];
struct strkobj kobj[maxobj];

unsigned char waitmess=0;
s16 messy;
signed char messsy;
char * mess;
char waitboom=0;
char waitfire=0;
struct boom booms[maxboom];
struct strbullets bullets[maxbullet];

struct monstr monsters[maxmonstr];
int nmonstr=0; //текущ кол-во видов монстров

struct strmodels models[maxmodel];

u16 k;
s16 end,pause,key_f,key_b;
s16 left,right,up,down,jump,fire;
s16 t_left,t_right,t_up,t_down,t_jump,t_fire;
int man_can_jump; //может ли медведь прыгать
u16 r_x,r_y; //прежние координаты лев. верхн угла окна

//есть ли звук и т.д. (больше для отладки надо)
s16 sound_dev=1,sound_mix,sound_vol,sound_state;

// - save data
struct st_svgame {
	u32 random;
	u32 money;
	u32 level;
	u32 CRC;
	u32 highscore;
};
extern struct st_svgame savegame[20];	//6 слотов сохр. + 20 4hiscore

extern int keydata;

//extern u8 my_module;

extern char path_to_data_file[];

//признак если есть бонус уровни для игры
int installed_bonus = 0;

//-- шрифты
struct BFont big_font;
struct BFont text_font;
struct BFont score_font;
struct BFont big_red_font;
struct BFont big_green_font;
struct BFont big_black_font;

s16 scrsdvig,scrsdvig_;

void main_p(void) {
	int i,k;

	unsigned char font_colors[]={15,20,24,30};
	unsigned char score_font_colors[]={15,48,28,8};
	unsigned char font_red_colors[]={12,34,40,43};
	unsigned char font_green_colors[]={80,86,88,110};
	unsigned char font_black_colors[]={28,29,30,8};

	GpRelativePathSet(path_to_data_file);
	//для проверки на путь запуска
	gp_str_func.strcpy(stroka,GpAppPathGet(NULL));

	InitGraph();
	LoadFont("system", &text_font, (unsigned char*)&font_colors);

	//загрузка звуков
	if ( !SetLib("graph") ) {
		fatalerror("Error: Game data not found...");
	}
	
	GetLib("plusha.act",palette);
	PutPalette(palette);
	FmlLoadFont("big_font.bfm", &big_font, (unsigned char*)&font_colors);

	//выбор и загрузка языка
	menus=(block)famemalloc(320*200);	//многоязыковы менюхи
	textdata=(block)famemalloc(320*200);	//для текста на разн языках. !!! уменьшить размер выел-й памяти

	//проверка на воровство игры и что она из FXE запущена
	// gp:\GAME\PLUSHA
#ifndef RELEASE
#ifdef RELEASE
	//gp_str_func.strcpy(stroka,GpAppPathGet(NULL));
	if ( stroka[0]==0 || (stroka[7]!='E' && stroka[7]!='e') ) {
		fatalerror("Error: Game data not found...");
		//не работает с крунчером
	}
#endif
#endif
	check_language();

	if ( !SetLib("sound") ) {
		fatalerror("Error: Sound data not found...");
	}
	load_SE();
//	gp_str_func.strcpy(stroka,GpAppPathGet(&i));
//#ifndef RELEASE
//	k=GpUserInfoGet(g_string, g_string);
//	sprintf(stroka,"DEBUG ID:\nAP:'%s',N%d\nUI: N%d, '%s'\n",GpAppPathGet(&i),i,k,g_string);
//	fatalerror(stroka);
//#endif

	//выясним есть бонус или нет
//#ifndef DEMO
	installed_bonus = SetLib("bonus\\graph");
//#endif

	//считаю конфиг из файла
	if ( !SetLib("graph") ) {
		fatalerror("Error: Game data not found...");
	}

#ifdef DEMO
	if (SizeLib("pooh.cfg") > 0 || SizeLib("tukki.bin") > 0 || installed_bonus)
	{
		fatalerror("Error: Data files not found...");
	}
#else
	if (SizeLib("pooh.cfg") <= 0 )
	{
		fatalerror("Error: Data files not found...");
	}
#endif
	//FmlLoadFont("big_font.bfm", &big_font, (unsigned char*)&font_colors);
	//big_font.chargap = 0;
	FmlLoadFont("score.bfm", &score_font, (unsigned char*)&score_font_colors);
	FmlLoadFont("big_font.bfm", &big_red_font, (unsigned char*)&font_red_colors);
	FmlLoadFont("big_font.bfm", &big_green_font, (unsigned char*)&font_green_colors);
	FmlLoadFont("big_font.bfm", &big_black_font, (unsigned char*)&font_black_colors);
	loading();

	logomenu=(block)famemalloc(320*200);	//картинки для меню
	GetLib("logomenu.bin",(block)logomenu);
	TurnIt(logomenu,320,200);

	shop_bg=(block)famemalloc(SizeLib("shop_bg.bin"));	//фон для магазина
	GetLib("shop_bg.bin",(block)shop_bg);
	TurnIt(shop_bg,264,240);

	loading();

	obloka=(block)famemalloc(SizeLib("obloka.bin"));
	GetLib("obloka.bin",(block)obloka);
	for (i=0; i<30; i++) {
		TurnIt(obloka+i*16*16,16,16);
	}
	for (i=0; i<5; i++) {
		TurnIt(obloka+0x3200+i*8*16,16,8);
	}
	for (i=0; i<4; i++) {
		TurnIt(obloka+0x3480+i*8*8,8,8);
	}

	loading();

	//--
	enbull=(block)famemalloc(SizeLib("enbull.bin"));
	GetLib("enbull.bin",(block)enbull);
	//lifts=(block)famemalloc(SizeLib("lift.bin"));
	//GetLib("lift.bin",(block)lifts);
	items=(block)famemalloc(SizeLib("items.bin"));
	GetLib("items.bin",(block)items);
	for (i=0; i<56; i++) {
		TurnIt(items+i*16*16,16,16);
	}
	additems=(block)famemalloc(SizeLib("additm.bin"));

	loading();
	GetLib("additm.bin",(block)additems);
	for (i=0; i<14; i++) {
		TurnIt(additems+i*16*16,16,16);
	}

	modelspr=(block)famemalloc(SizeLib("models.bin"));
	GetLib("models.bin",(block)modelspr);
	for (i=0; i<6; i++) {
		TurnIt(modelspr+i*16*16,16,16);
	}
	for (i=0; i<3; i++) {
		TurnIt(modelspr+6*256+i*32*32,32,32);
	}
	for (i=0; i<2; i++) {
		TurnIt(modelspr+6*256+32*32*3+i*16*32,16,32);
	}
	loading();

	ind=(block)famemalloc(SizeLib("honey.bin"));
	GetLib("honey.bin",(block)ind);
	for (i=0; i<7; i++) {
		TurnIt(ind+i*8*8,8,8);
	}	
	manspr=(block)famemalloc(320*240); //!!! 32k а если он будет больше?
	demobuf=(block)famemalloc(maxdemo);
	fon=(block)famemalloc(256*256); //256 тайлов теперь мона!
	bckg=famemalloc(320*200+4);

	mnstrspr=(block)famemalloc(320*200*4);	//в 4 раза больше места для спрайтов монстров
	level=(block)famemalloc(maxsiz_level);
	blevel=(block)famemalloc(maxsiz_level);
	hidscr=(block)famemalloc(320*240); //добавил

	tiles=(block)famemalloc(SizeLib("tiles.bin"));
	GetLib("tiles.bin",(block)tiles);
	
	mod_source=(u8*)famemalloc(250*1024); //250k max для Пуховых модов - добавил

	//очистка начальная
	/*gp_str_func.memset(level,0,maxsiz_level);
	gp_str_func.memset(blevel,0,maxsiz_level);
	gp_str_func.memset(demobuf,0,maxdemo);*/

	for ( i=0; i<maxobj; ++i ) {
		//инициализируем массив с монстрами
		kobj[i].n=-1;
	}
	for ( i=0; i<maxmodel; ++i ) {
		//инициализируем массив с моделями
		models[i].typ=-1;
		models[i].on0=models[i].name0[0]=models[i].name[0]=0;
	}
	loading();

	//считаем сохр-е игры
	check_savegame();

#ifdef RELEASE
	PaletteWhite(palette);
	SetLib("mult"); //мульт интро
	do_mult("fame.scr",0);
	PaletteOff(palette);
#endif
	if (installed_bonus)
		SetLib("bonus\\mult"); //мульт интро но уже из бонуса 8))
	do_mult("intro.scr",1);
	PaletteOff(palette);

	curr_level = end = 0;
	i = 0;

	while ( end ==0 ) {
		//выкл муз
		//!stop_sound();
		modstop();

		//создание фона для меню
		SetLib("graph");
		//фон- strip для
		GetLib("menu_bg.bin",(block)bckg);
		TurnIt((block)bckg, 320, 200);
		GetLib("menu_str.def",(block)hidscr);
		readstrip();

mainmenu_continue_sound:
		modsetup("intro.fms", 4, 0 ,0, 0, 0 );
		//!start_sound();

mainmenu_continue:
		//чистим от спрайтов экран меню
		for ( k=0; k<maxboom; ++k ) {
			booms[k].typ = -1;
		}

		i=main_menu(i);

		SE(sfx_OK);
		switch ( i ) {
			case 0:
				//начать Игру
				//Delay(600);
				//!stop_sound();
				modstop();//выкл муз

				coins=demo=0;
				curr_level=1;

				while ( is_sfx_playing() );
				f_game();
				curr_level=0;
				i = end=0;
				break;
			case 1:
				// продолжаем игру - считываем сохраненную гаму
				coins=demo=0;
				i = select_savegame(0);
				if ( i>=0 ) {
#ifndef RELEASE
					curr_level = (i+2);
					if( curr_level<1 || curr_level>20 ) {
						curr_level = 1;
					} else {
						coins = 777;
					}
#else
					curr_level = savegame[i].level;
					if(curr_level<1 || curr_level>7 )
					{
						if (!installed_bonus || (curr_level>20 && curr_level<1) )
							curr_level = 1;
					} else {
						coins = savegame[i].money;
					}
#endif
				}
				else {
					//нажали отмену
					i = 1;
					PaletteOff(palette);
					//Delay(100);
					goto mainmenu_continue;
					break;
				}
				//Delay(600);
				//!stop_sound();
				modstop();//выкл муз

				f_game();
				curr_level=0;
				i = end=0;
				break;

			case 2:	//изменить язык
				modstop();//выкл муз
				while ( is_sfx_playing() );
				//Delay(600);
				//!stop_sound();


				select_language();
				i = 2;
				PaletteOff(palette);

				goto mainmenu_continue_sound;
				break;

			case 4:	//демо
				if ( show_highscore() ) {
					demo=1;
	
					//выкл муз
					//!stop_sound();
					modstop();
#ifndef DEMO
					if ( ++curr_level>3 ) {
						curr_level=1;
					}
#else
					curr_level=1;
#endif

					while ( is_sfx_playing() );
					f_game();
					i = end = 0;
				} else {
					PaletteOff(palette);
					i = end = 0;
					goto mainmenu_continue;
				}
				break;

			case 3:	//выход в дос
				end=1;
				break;
		}
	}
	//Delay(600);
	while ( is_sfx_playing() );

	//Thank you for playing!
	fatalerror(textindex[10]);
	return;
}

void fatalerror(char *t) {
	GP_PALETTEENTRY tmp_entry;
	//выход по фатальной ошибке
	//!stop_sound();
	modstop();	//выкл муз

	SetLib("");

	//подгоним палитру
	tmp_entry = 0;
	GpPaletteEntryChange(8, 1, &tmp_entry, GPC_PAL_NO_REALIZE);
	tmp_entry = ( 0X1F << 11 ) | ( 0X1F << 6 ) | ( 0X1F << 1 ) | 0;
	GpPaletteEntryChange(15, 1, &tmp_entry, GPC_PAL_NO_REALIZE);
	GpPaletteRealize();

	Clrs(15,nflip);	//БЕЛЫЙ
	GpTextOut(NULL, &gpDraw[nflip], 14, 8, (char*)  "Super Plusha         WWW.FAMESOFT.RU", 229);
#ifdef DEMO
	//логотип
	GpTextOut(NULL, &gpDraw[nflip], 14, 224-11, (char*)"DEMO Ver 1.86 (c)2002-2004 FaMe Soft", 229);
#else
	GpTextOut(NULL, &gpDraw[nflip], 14, 224-11, (char*)"Version 1.86  (c)2002-2004 FaMe Soft", 229);
#endif
	text_font.color[0] = 8;	//Черный
	BTextOut(NULL, &gpDraw[nflip], &text_font,(320-BTextWidthGet(&text_font,t))/2, 112, t);
	FlipAndShow();
	start_sound();
	SE(sfx_error);
	Delay(2500);
	GpAppExit();
}

//главное меню
int main_menu(int i) {
	//int keydata;
	s16 j=0,s=0,x,y,x1,y1,x2,y2;

#define nmenx 75
#define nmeny 114+8

	//позиция логотипа - медведя над меню
	x2 = 168; y2 = 16;
	x= 84; y= 16; y1 = -4; x1 = 104;
	keydata = r_x = 0; r_y = 0; siz_ylev = 240; //для летящих иньянев.
	text_font.color[0] = 8;	//Черный

	while ( 1 ) {
		//--рисуем фон меню - скроллируем
		do_strip(-y2);
		show_strip(s);
		if ( s < i*50)
			s++;
		if ( s > i*50)
			s--;
		//-- конец фона скролла

		// FAME SOFT
		GpTransBlt(NULL,&gpDraw[nflip],
			1,240-31,66,30,(unsigned char*)logomenu,
			183,90,320,200,0);
		// MeGaGP logo
		GpTransBlt(NULL,&gpDraw[nflip],
			320-32,240-31,31,30,(unsigned char*)logomenu,
			152,90,320,200,0);
		//если есть бонус уровни
		if (installed_bonus)
			GpTransBlt(NULL,&gpDraw[nflip],
			320-32,240-31-15,31,17,(unsigned char*)logomenu,
			249,103,320,200,0);

		//летучий медведь и логотипы там всякие
		if (x == x1 && y == y1) {
			x = 84 - random(20) + 10;
			y = 5 - random(10) + 16;
			//дым из топки
			addboom(x1+ 122,y1+77,16,16,4+random(3),3+random(3),b_boom,obloka+256*10);
		} else {
			if ( x1 < x) x1++;
			if ( x1 > x) x1--;
			if ( y1 < y) y1++;
			if ( y1 > y) y1--;
		}
		doboom();
		GpTransBlt(NULL,&gpDraw[nflip],
			x1,y1,151,120,(unsigned char*)logomenu,
			0,0,320,200,0);
		//мырг глазами
		if (abs(x-x1) >4 && abs(y-y1) >4) {
			GpTransBlt(NULL,&gpDraw[nflip],
			x1+22,y1+37,51,52,(unsigned char*)logomenu,
			256,48,320,200,0);
		}
		//огонь из горелки
		if ( abs(x-x1) < 3 && abs(y-y1) < 3) {
			GpTransBlt(NULL,&gpDraw[nflip],
			x1+121,y1+75,27,13,(unsigned char*)logomenu,
			200,0,320,200,0);
		}
		//метеориты типа
		if ( abs(x-x1) < 3 && abs(y-y1) < 3) {
			addboom(0,random(200)-20,16,16,4+random(16),0-random(2),b_crash,items+256*8*6+256*random(7));
		}
		//логотип SUPER PLUSHA
		GpTransBlt(NULL,&gpDraw[nflip],
			0-x2,4+y2,152,64,(unsigned char*)logomenu,
			0,120,320,200,0);
		GpTransBlt(NULL,&gpDraw[nflip],
			152+x2,4+y2,168,64,(unsigned char*)logomenu,
			152,120,320,200,0);

		if (x2 == 8) SE(sfx_magic);	//при сдвижке тутула - магия звук
		if (x2>0) x2 -= 8;
		if (y2>0) y2--;

		//меню
		//cursor
		GpRectFill(NULL, &gpDraw[nflip], 68, i*28 + nmeny , 184, 24, 136); //синий
		GpTransBlt(NULL,&gpDraw[nflip], //верх
			68, i*28 + nmeny - 8, 184,8,(unsigned char*)logomenu,
			0,184,320,200,0);
		GpTransBlt(NULL,&gpDraw[nflip], //низ
			68, i*28 + nmeny + 24, 184,8,(unsigned char*)logomenu,
			0,192,320,200,0);
		GpTransBlt(NULL,&gpDraw[nflip], //лево
			68-8, i*28 + nmeny, 8,24,(unsigned char*)logomenu,
			304,0,320,200,0);
		GpTransBlt(NULL,&gpDraw[nflip], //право
			68+184, i*28 + nmeny, 8,24,(unsigned char*)logomenu,
			312,0,320,200,0);

		//пункты меню
		//start game
		GpTransBlt(NULL,&gpDraw[nflip],
			72,0*28 + nmeny,176,24,(unsigned char*)menus,
			0,0,320,200,0);
		//continue game
		GpTransBlt(NULL,&gpDraw[nflip],
			72,1*28 + nmeny,176,24,(unsigned char*)menus,
			0,24,320,200,0);
		//change language
		GpTransBlt(NULL,&gpDraw[nflip],
			72,2*28 + nmeny,176,24,(unsigned char*)menus,
			0,48,320,200,0);
		//exit (reboot)
		GpTransBlt(NULL,&gpDraw[nflip],
			88,3*28 + nmeny,144,24,(unsigned char*)menus,
			176,0,320,200,0);
#ifdef DEMO
		//"Demo version" над меню
		BTextOut(NULL, &gpDraw[nflip], &text_font,100, 64, "Demo Version");
#endif

		FlipAndShow();
		//цикл меню
 		//ждем нажатия клавиши
		GpKeyGetEx(&keydata);
#define menu_cycles_wait 2
		//keyboard
		if ( (keydata & GPC_VK_FA) || (keydata & GPC_VK_START) ) //press A or START
		{
			if (j>menu_cycles_wait) {
				SE(sfx_OK);
				break;
			}
		} else
		if ( keydata & GPC_VK_FR ) //press FR
		{
			i = 3;
			j = 0;
			//SE(sfx_cursor);
		} else
		if ( keydata & GPC_VK_FL ) //press FL
		{
			j = i = 0;
			//SE(sfx_cursor);
		} else
		if ( keydata & GPC_VK_UP ) //press UP
		{
			if (j>menu_cycles_wait) {
				if (--i<0)
				{
					i = 3;
				}
				j = 0;
				SE(sfx_cursor);
			}
		} else
		if ( keydata & GPC_VK_DOWN ) //press DOWN
		{
			if (j>menu_cycles_wait) {
				if (++i > 3)
				{
					i = 0;
				}
				j = 0;
				SE(sfx_cursor);
			}
		}

		if ( y2 == 15 )	//один раз проявить картинку
			PaletteOn(palette);
		
		Delay(45);

		if ( ++j > 300 ) {	//выход на ДЕМО
			return 4;
		}
	}
	return i;
}


//модуль собственно game
//=============================================================
void f_game(void) { //проба скролла и объектов
	static s16 x,y,sx,sy,i,ingame_menu;
	int transition, bonus_5=0,j;
	//__printf("f_game: start\n");
	
	shields=kolun=glass=shboots=jetpack=end=0; //монетки,продолжения
	continues=1; //продолжения
	maxjump=-8; //высота прыжка
	maxspeed=4; //макс. скорость

	while ( continues-- ) {
		man.s=s_end;
		//hearts=3; //сердечки
		beams=6; //ШИШКИ
		hearts=score=0;	//очки
		hearts++; beams--;
		hearts++; hearts++;

		while ( hearts>0 ) {
			//пока не кончились жизни
			if ( man.s==s_end ) {

				PaletteOff(palette);
				//проверка score - можно ли на 5й секретный уровень
				if ( curr_level == 5) {
					if (bonus_5 != 1) {
//#ifdef RELEASE
						if (score > 3*10000 || score == 0) {
							bonus_5 = 1;
						} else {
							curr_level++;
							bonus_5 = 2;
						}
//#endif
					}
				} else {
					//не выводим надписи про секреты
					bonus_5 = 0;
				}
				//если есть бонус уровни и выше 7го..то читаем их.
				if (installed_bonus && !demo && curr_level>7) {
					SetLib("bonus\\mult"); //мульт на начало уровня
				} else {
					SetLib("mult"); //мульт на начало уровня
				}
				if ( demo ) {
					//TileBar(0,0,320,200,64,64,(block)tiles+random(4)*64*64);
					do_mult("intro.scr",1);
				} else {
					gp_str_func.sprintf(stroka, "%02ulevel.scr",curr_level);
					if (SizeLib(stroka) <=0 ) {
						do_mult("intro.scr",1);
					} else {
						//если тока начали - мульт можно перескочить если нет то низя
						//смотрим на score
#ifndef RELEASE
						do_mult(stroka,1);
#else
						do_mult(stroka,score?0:1);
#endif
					}
				}
				if ( bonus_5 == 1) {
					BTextOut(NULL, &gpDraw[nflip], &big_green_font, 56, 0, "SECRET LEVEL!");
					BTextOut(NULL, &gpDraw[nflip], &big_green_font, 56, 220, "SECRET LEVEL!");
				} else
				if ( bonus_5 == 2) {
					BTextOut(NULL, &gpDraw[nflip], &big_green_font, 35, 0, "NO SECRET LEVEL");
					BTextOut(NULL, &gpDraw[nflip], &big_green_font, 35, 220, "NO SECRET LEVEL");
					bonus_5 = 0; //остальные уровни без надписей бонус
				} else
				if ( installed_bonus && !demo && curr_level>7) {
					BTextOut(NULL, &gpDraw[nflip], &big_green_font, 16, 0, "EXTRA BONUS LEVEL");
					BTextOut(NULL, &gpDraw[nflip], &big_green_font, 16, 220, "EXTRA BONUS LEVEL");
					bonus_5 = 0; //остальные уровни без надписей бонус
				}


				Saturate(0,0,320,240,142);

				//покажим заголовок - название уровня
				//TileBar(0,0,320,200,64,64,(block)tiles+random(4)*64*64);
				gp_str_func.sprintf(stroka, "LEVEL %u",curr_level);
				BTextOut(NULL, &gpDraw[nflip], &big_black_font, 101+2, 28+2, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip], &big_font, 101, 28, (char*)stroka);

				if (demo) {
					//выводим надпись DEMO
					GpTransBlt(NULL,&gpDraw[nflip],
					116/*230*/,200,88,24,(unsigned char*)menus,
					232,120,320,200,0);
				}


				if ( installed_bonus && !demo && curr_level>7) {
					//для плугинового бонуса тока номер ур-ня
					// и жесткое назв
/*					i = 8;
					gp_str_func.strcpy(stroka, leveldef.mname0);
					BTextOut(NULL, &gpDraw[nflip], &big_black_font,2+(320-BTextWidthGet(&big_font, (char*)stroka ))/2, 2+76, (char*)stroka);
					BTextOut(NULL, &gpDraw[nflip], &big_green_font,(320-BTextWidthGet(&big_font, (char*)stroka ))/2, 76, (char*)stroka );

					gp_str_func.strcpy(stroka, leveldef.name);
					// black
					text_font.color[0] = 8;
					BTextOut(NULL, &gpDraw[nflip], &text_font,1+(320-BTextWidthGet(&text_font, (char*)stroka ))/2, 1+86+30, (char*)stroka );
					// white
					text_font.color[0] = 15;
					BTextOut(NULL, &gpDraw[nflip], &text_font,(320-BTextWidthGet(&text_font, (char*)stroka ))/2, 86+30, (char*)stroka );

					//обнулим супер вещи!! для кажд дополн уровня!
					shields=kolun=glass=shboots=jetpack=0; //монетки,продолжения
					maxjump=-8; //высота прыжка
*/
				} else {
					//для обычного ур-ня его имя
					i = curr_level;
					BTextOut(NULL, &gpDraw[nflip], &big_black_font,2+(320-BTextWidthGet(&big_font,(char*)textindex[i+49]))/2, 2+76, (char*)textindex[i+49]);
					BTextOut(NULL, &gpDraw[nflip], &big_green_font,(320-BTextWidthGet(&big_font,(char*)textindex[i+49]))/2, 76, (char*)textindex[i+49]);
					// black
					text_font.color[0] = 8;
					BTextOut(NULL, &gpDraw[nflip], &text_font,1+(320-BTextWidthGet(&text_font,(char*)textindex[i+59]))/2, 1+86+BTextHeightGet(&big_black_font,(char*)textindex[i+49]), (char*)textindex[i+59]);
					// white
					text_font.color[0] = 15;
					BTextOut(NULL, &gpDraw[nflip], &text_font,(320-BTextWidthGet(&text_font,(char*)textindex[i+59]))/2, 86+BTextHeightGet(&big_black_font,(char*)textindex[i+49]), (char*)textindex[i+59]);
					FlipAndShow();
					PaletteOn(palette);
				}
	

				//tic=gettic()+18*3;

				readlev();

				if ( installed_bonus && !demo && curr_level>7) {
					//для плугинового бонуса тока номер ур-ня
					// и жесткое назв
					BTextOut(NULL, &gpDraw[nflip], &big_black_font,2+(320-BTextWidthGet(&big_font, (char*)leveldef.name ))/2, 2+76, (char*)leveldef.name);
					BTextOut(NULL, &gpDraw[nflip], &big_green_font,(320-BTextWidthGet(&big_font, (char*)leveldef.name ))/2, 76, (char*)leveldef.name);

					// black
					text_font.color[0] = 8;
					BTextOut(NULL, &gpDraw[nflip], &text_font,1+(320-BTextWidthGet(&text_font, (char*)leveldef.mname0 ))/2, 1+86+30, (char*)leveldef.mname0 );
					// white
					text_font.color[0] = 15;
					BTextOut(NULL, &gpDraw[nflip], &text_font,(320-BTextWidthGet(&text_font, (char*)leveldef.mname0 ))/2, 86+30, (char*)leveldef.mname0 );

					//обнулим супер вещи!! для кажд дополн уровня!
					shields=kolun=glass=shboots=jetpack=0; //монетки,продолжения
					maxjump=-8; //высота прыжка					
					FlipAndShow();
					PaletteOn(palette);
					Delay(2000);
				}

				Delay(1000);
				SE(sfx_coin);
				//выведем hiscore после паузы
				gp_str_func.sprintf(stroka, "HI-SCORE: %07u",savegame[curr_level-1].highscore);
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_black_font, 2+(320-BTextWidthGet(&big_font,(char*)stroka))/2, 160+2, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_font, (320-BTextWidthGet(&big_font,(char*)stroka))/2, 160, (char*)stroka);
				
				for ( i=0; i<maxmodel; ++i ) {
					if ( models[i].typ>=0 ) {
						models[i].on=models[i].on0;
						models[i].x=models[i].x1;
						models[i].y=models[i].y1;
					} else {
						models[i].on=0;
					}
				}
				//иниц в начале уровня
				for ( i=0; i<maxobj; ++i ) {
					//из массива достаем монстрят
					if ( kobj[i].n>=0 )
						makemonstr(i,kobj[i].n,kobj[i].x,kobj[i].y);
					else
						obj[i].on=0;
				}
				boss=keys=0; //ключи

				Delay(1500);
			}

			waitboom=1;
			GpSrand(34); //иниц ДСЧ
			waitmess=scrsdvig=fire=jump=down=left=right=up=0;
			key_f=key_b=t_fire=t_jump=t_down=t_left=t_right=t_up=0;
			beams=max(5,beams); //ШИШКИ

			man.x=leveldef.nx; //инициализация героя
			man.y=leveldef.ny;
			//направление лица героя
			man.napr = (leveldef.nx < leveldef.lx / 2);

			//центровка по X
			if ( man.x-(320+man.lx)/2 <= 0) r_x=0;
			else if ( man.x-(320+man.lx)/2 > siz_xlev*16-319 ) r_x=siz_xlev*16-319;
			else r_x=man.x-(320+man.lx)/2;
			//центровка по Y
			if ( man.y-(240+man.ly)/2 <= 0 ) r_y=0;
			else if ( man.y-(240+man.ly)/2 > siz_ylev*16-239 ) r_y=siz_ylev*16-239;
			else r_y=man.y-(240+man.ly)/2;

			x=y=sx=sy=0;

			for ( i=0; i<maxbullet; ++i ) {
				bullets[i].typ=-1;
			}
			for ( i=0; i<maxboom; ++i ) {
				booms[i].typ=-1;
			}

			man.on=7; //5 медок энергия
			man.buf=man.stspr;
			man.myrg=wmyrg; //неуязвимость
			//можно прыгать
			man_can_jump = man.sy=1;
			man.on--;
			man.s=s_down;
			times=9998;
			keydata = end = power=man.f=man.f0=man.fw=0;
			times++;
			man.on--;
						
			//инициализируем задержку по таймеру
			tic=gettic()+wclock;
			pdemo=0; //инициализируем демонстрацию

			//!stop_sound();
			modsetup((char *)(boss?"boss.fms":leveldef.musicname), 4, 0 ,0, 0, 0 );
			//!start_sound();
			
			PaletteOff(palette);
			
			//__printf("f_game: cycle start\n");

			// выводить 
			transition = 1;
				
			while ( end==0 ) {
			
				mm(1);
				ingame_menu = end;	//контроль входа в меню

				//центровка по X
				if (man.x-320/2+man.lx/2<=0) x=0;
				else if (man.x-320/2+man.lx/2 > siz_xlev*16-319) x=siz_xlev*16-319;
				else x=man.x-320/2+man.lx/2;

				//центровка по Y
				if (man.y-240/2-32<=0) y=0;
				else if (man.y-240/2-32 > siz_ylev*16-240) y=siz_ylev*16-239;
				else y=man.y-240/2-32;

				left=t_left; right=t_right;
				up=t_up; down=t_down;
				jump=t_jump; fire=t_fire;

				if ( demo ) {
					//если демонстрация, то расшифруем
					i=demobuf[pdemo++];
					left=i&1;	right=i&2;
					up=i&4;		down=i&8;
					jump=i&16;	fire=i&32;
					end=i&64;
					if ( pdemo>=maxdemo || keydata) { 	//прервем дмонстрацию по любой кнопке
						end=1;
					}
				} else {
					//иначе пишем демонстрацию
					if ( pdemo<maxdemo ) {
						demobuf[pdemo++]=left|(right<<1)|(up<<2)|(down<<3)|(jump<<4)|(fire<<5)|(end<<6);
					}
				}
#ifndef RELEASE
				Profile("rislab",rislab(x,y);)
				Profile("domodel",domodel();)
				Profile("doobj",doobj();)
				Profile("doman",doman();)
				Profile("dobullet",dobullet();)
				Profile("putup",putup();)
				Profile("doboom",doboom();)
#else
				rislab(x,y);
				domodel();
				doobj();
				doman();
				dobullet(); putup(); doboom();
#endif
				if (demo && (times & 16) )
				{
					//выводим мигающую надпись DEMO
					GpTransBlt(NULL,&gpDraw[nflip],
					230,200,88,24,(unsigned char*)menus,
					232,120,
					320,200,
					0);
				}

				if( waitmess>0 ) {
					waitmess--; MaxY=240;
					text_font.color[0] = 8;
					BTextOut(NULL, &gpDraw[nflip], &text_font, 24+1, messy+1, (char*)mess);
					text_font.color[0] = 12+(waitboom&3);
					BTextOut(NULL, &gpDraw[nflip], &text_font, 24, messy, (char*)mess);
					if( messsy<1 || waitmess<6) {
						messy+=(++messsy);
					}
				}

				if (times<=0) {
					man.s=s_crash;
					times=1000;
					//addmess("Time Is Over",60);
			     	addmess(textindex[12],60);

				} else {
					--times;
				}
				if ( waitfire>0 ) {
					--waitfire;
				}
				
				put_score(); put_additems();
				FlipAndShow();

				if ( transition ) {
					PaletteOn(palette);
					transition = 0;
				}

				if ( left ) { //кнопка влево
					if (man.sx>=-maxspeed) {
						//в воде
						if( leveldef.typ!=t_water || (waitboom&3)==0 ) {
							--man.sx;
						}
					}
					if ( man.sx<0 )
						man.napr=0;
				} else if (right==0 && man.sx<0) {
					//скольжение
					if( leveldef.typ!=t_ice || (waitboom&3)==0 ) {
						++man.sx;
					}
				}
				if ( right ) { //кнопка вправо
					if (man.sx<=maxspeed) {
						//в воде
						if( leveldef.typ!=t_water || (waitboom&3)==0 ) {
							++man.sx;
						}
					}
					if ( man.sx>0 )
						man.napr=1;
				} else if (left==0 && man.sx>0) {
					//скольжение
					if( leveldef.typ!=t_ice || (waitboom&3)==0 ) {
						--man.sx;
					}
				}
				if ( jump == 0 && man.s==s_go )
					man_can_jump = 1;
				if ( jump ) { //прыжок
					if ( man.s==s_go && man_can_jump ) {
						man_can_jump = 0;
						SE(sfx_jump);
						man.sy=maxjump;
						man.s=s_jump;
					} else
					if( (jetpack && power && man.s==s_down) && ( up || (keydata & GPC_VK_FR) ) ) {
						//в полет
#ifndef DEMO
						man.s=s_fly; man.dsx1=man.dsx2=0;
#endif
					} else
					if ( man.s==s_fly && power) {
						//в добавим газку
						if( man.sy>-3 )
							--man.sy;
					}
				}
				else if ( man.s==s_jump ) {
					man.sy=man.sy/2;
					man.s=s_down;
				}

				if ( fire && man.s!=s_fly ) { //стрельба

					if ( waitfire==0 ) {
						waitfire=8;

						if (beams>0) {
							if ( addbullet(man.x+16,man.y-20,man.sx+(man.napr!=0?4:-4),up?-2:(down?2:0),1,items+256*4*8) ) {
								//фаза броска
								man.buf=man.stspr+4*man.lx*man.ly;
								man.fw=3;
								--beams;
								SE(sfx_throw);
							}
						}
					}
				}
				//УПРОЩЕНИЕ ИГРЫ ДЛЯ BOSSа
				if (boss && beams==0 && waitfire==0 && (waitboom&63)==0 ) {
					beams = 1;
					SE(sfx_cone);
				}


				if ( up  && man.s==s_go && man.sx==0 ) { //сдвиг для просмотра вверх экрана
					actmodel(); //использ вещь

					if ( /*(man.y - scrsdvig)>36 &&*/ scrsdvig < (200-32) ) {
						//scrsdvig < 200 ) {
						scrsdvig+=4;
						scrsdvig_=0;
					}
					//центровка по Y
					if ( man.y-scrsdvig-(240+man.ly)/2 <= 0 ) r_y=0;
					else if ( man.y-scrsdvig-(240+man.ly)/2 > siz_ylev*16-239 ) r_y=siz_ylev*16-239;
					else r_y=man.y-scrsdvig-(240+man.ly)/2;
					//if (man.y-scrsdvig-240/2-32 <= 0 ) r_y=0;
					//else if ( man.y-scrsdvig-240/2-32 > siz_ylev*16-240 ) r_y=siz_ylev*16-240;
					//else r_y=man.y-scrsdvig-240/2-32;
				} else {
					scrsdvig=0;
				}

				if ( down && man.s==s_go && man.sx==0 ) { //сдв вниз
					if ( /*(r_y + scrsdvig_ < (siz_ylev*16-32) ) &&*/ scrsdvig_ < (200-32) ) {
					//if ( scrsdvig_ < 200) {
						scrsdvig_+=4;
						scrsdvig=0;
					}
					//центровка по Y
					if ( man.y+scrsdvig_-(240+man.ly)/2 <= 0 ) r_y=0;
					else if ( man.y+scrsdvig_-(240+man.ly)/2 > siz_ylev*16-239 ) r_y=siz_ylev*16-239;
					else r_y=man.y+scrsdvig_-(240+man.ly)/2;
					//if (man.y+scrsdvig_-240/2-32<=0) r_y=0;
					//else if (man.y+scrsdvig_-240/2-32 > siz_ylev*16-240) r_y=siz_ylev*16-240;
					//else r_y=man.y+scrsdvig_-240/2-32;
				} else {
					scrsdvig_=0;
				}

				if ( ingame_menu && demo == 0) {//выход из игры в меню.
					SE(sfx_OK);
					ingame_menu = 0;
					//START
					while ( end ) { end = 0; mm(1); }

					j = i = 0;
					while ( 1 ) {
	//-- exit menu---------------------
						//--рисуем фон меню - скроллируем
						do_strip(3);
						show_strip(0);
						//-- конец фона скролла
		
						//меню
						//cursor
						GpRectFill(NULL, &gpDraw[nflip], 44, i*32 -48 + nmeny , 232, 24, 136); //синий
						GpTransBlt(NULL,&gpDraw[nflip], //верх
							44, i*32 -48 -8 + nmeny, 232,8,(unsigned char*)logomenu,
							0,184,
							320,200,
							0);
						GpTransBlt(NULL,&gpDraw[nflip], //низ
							44, i*32 -48 + 24 + nmeny, 232,8,(unsigned char*)logomenu,
							0,192,
							320,200,
							0);
						GpTransBlt(NULL,&gpDraw[nflip], //лево
							44 -8, i*32 -48 + nmeny, 8,24,(unsigned char*)logomenu,
							304,0,
							320,200,
							0);
						GpTransBlt(NULL,&gpDraw[nflip], //право
							44+232, i*32 -48 + nmeny, 8,24,(unsigned char*)logomenu,
							312,0,
							320,200,
							0);

						//пункты меню
						//return to game
						GpTransBlt(NULL,&gpDraw[nflip],
							44,-1*32 + nmeny - 16,232,24,(unsigned char*)menus,
							0,120,320,200,0);
						if (continues)
						{
							//save game & exit
							GpTransBlt(NULL,&gpDraw[nflip],
							44,0*32 + nmeny - 16,232,24,(unsigned char*)menus,
							0,144,320,200,0);
						}
						//exit to menu
						GpTransBlt(NULL,&gpDraw[nflip],
							44,1*32 + nmeny - 16,232,24,(unsigned char*)menus,
							0,168,320,200,0);
		
						//цикл меню
						FlipAndShow();
						GpKeyGetEx(&keydata);
					
						Delay(50);
						j++;
						if ( keydata & GPC_VK_FR ) //press FR
						{
							i = 2;
							j = 0;
							//SE(sfx_cursor);
						} else
						if ( keydata & GPC_VK_FL ) //press FL
						{
							j = i = 0;
							//SE(sfx_cursor);
						} else
						if ( keydata & GPC_VK_UP ) {
							if ( j>menu_cycles_wait ) {
								if (--i<0 ) {
									i=2;
								}
								if (continues == 0 && i == 1)
									i=0;
								j=0;
								SE(sfx_cursor);
							}
						} else
						if ( keydata & GPC_VK_DOWN ) {
							if ( j>menu_cycles_wait ) {
								down=right=0;
								if (++i>2 ) {
									i=0;
								}
								if (continues == 0 && i == 1)
									i=2;
								j=0;
								SE(sfx_cursor);
							}
						} else
						if ( keydata & GPC_VK_FA ) {	//OK
							SE(sfx_OK);
							break;
						} else //cancel
						if ( (keydata & GPC_VK_FB) || (keydata & GPC_VK_START) ) {
							SE(sfx_cancel);
							//защита от залипания клавиши
							while (keydata)
							{
								GpKeyGetEx(&keydata);
								Delay(50);
							}
							i=0;
							break;
						}

					}
					switch ( i ) {
					case 0:
						//return to game
						end=0;
						break;
					case 1:
						// save and exit
						i = select_savegame(1);
						if ( i>=0 )
						{
							continues--;	//для сохранения вычитаем 1 continue
							savegame[i].level = curr_level;
							savegame[i].money = min(coins+(continues+glass+kolun+shboots*2+jetpack*2)*100+beams*5+shields*50,9999);
							//!stop_sound();
							while ( is_sfx_playing() );
							modstop();
							save_savegame();
						}
						else
						{
							//нажали отмену
							end=0;
							continue;
						}
						ingame_menu = end = 1;
						man.s=s_over;
						continues=hearts=0;
						break;

					case 2:	// exit to menu
						//Delay(500);
						ingame_menu = end = 1;
						man.s=s_over;
						continues=hearts=0;
						//!stop_sound();
						modstop();
						break;
					}
				}

				if ( pause && demo == 0) {//пауза в игре
					SE(sfx_OK);
					Saturate(0,0,320,240,142);
					//выводим в центре PAUSE
					GpTransBlt(NULL,&gpDraw[nflip],
						88,108,144,24,(unsigned char*)menus,
						176,24,320,200,0);
					FlipAndShow();
					//while ( is_sfx_playing() );

					while ( pause ) { /*pause = 0;*/ mm(1); }
					while ( keydata == 0)
					{ 
						Delay(50);
						mm(1);
					}
					while ( keydata ) { /*pause = 0;*/ mm(1); }
					end = 0;
					SE(sfx_cancel);
					while ( is_sfx_playing() );
				}
#ifndef RELEASE
#ifndef DEMO
/*				if ( key_f ) {//BOSS-key
					keys +=1;
					power = 600;		

					//Профилер
					Cls(0);
					PROFILEDisplayTimers();
					FlipAndShow();
					while ( keydata ) { mm(1); }
					while ( keydata == 0)
					{ 
						Delay(50);
						mm(1);
					}
					while ( keydata ) { mm(1); }
					end = 0;
				}*/
			
				if ( key_b ) {
					PROFILEDisplayTimers();
					FlipAndShow();
					//задержка по таймеру
					tic=gettic()+2000; //0...640
					while (gettic()<tic);

					//на от времени дохн
					if ( t_down ) {
						hearts=man.on=1;
						times=100;
					} //Жизни+$
					if ( t_right ) {
						coins=9000;
						score += 10010;
						hearts=man.on=5;

						keys +=1;
						power = 600;		
					}
					//на след. урвнь
					if ( t_up ) {
						man.s=s_end;
						end=1;
					}
					//добавим супер вещи
					if ( t_left ) {
						shields=glass=kolun=continues=jetpack=shboots=2;
						maxjump = -10;
					}
				}
#endif
#endif
				if ( man.s==s_go && man.sy<1 ) {
					//для отсутствия остановов при движении с горы
					man.sy=2;
				}

				//задержка по таймеру
				while (gettic()<tic);
				tic=gettic()+wclock; //0...640
			}

			//loading();
			//PaletteOff(palette);
			transition = 0;

			//!stop_sound();
			modstop();

			//если мужика пришили... то
			if ( demo ) {
				//прерываение игры
				end=hearts=0;
			} else if ( man.s==s_over && ingame_menu==0 ) {
				//подох
				end=0;

				//убавляем жизнь
				if (--hearts<=0) {
					//OVER?
					PaletteOff(palette);

					SetLib("mult");
	
					if ( continues ) {
						//tic=gettic()+18*8;
						do_mult("continue.scr",0);
						//vprintB(tfnt+144*3,"Continue %1u",continues);
					} else {
						//tic=gettic()+18*3;
						do_mult("gameover.scr",0);
					}

					//PaletteOff(palette);
					end=0;
				}

			} else if ( man.s==s_end && ingame_menu==0 ) {
//на конец уровня ------------------------
				//SetLib("graph");
				//TileBar(0,0,320,200,64,64,(block)tiles+random(4)*64*64);
				SE(sfx_OK);
				Saturate(0,0,320,240,142);

				gp_str_func.sprintf(stroka,"LEVEL %u SUMMARY",curr_level);
				BTextOut(NULL, &gpDraw[nflip], &big_black_font, 20+2, 20+2, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip], &big_green_font, 20, 20, (char*)stroka);

				FlipAndShow();

				while ( is_sfx_playing() );
				Delay(200);
				SE(sfx_item);

				//подсчет кол-ва вещей
				x=0;
				for ( i=0; i<siz_level; ++i ) {
					if ( blevel[i]&224 ) ++x;
				}
				gp_str_func.sprintf(stroka, "ITEMS: %u%%",(s16)((nitems-x)*100/nitems));
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_black_font, 20+2, 70+2, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_font, 20, 70, (char*)stroka);

				while ( is_sfx_playing() );
				Delay(200);
				SE(sfx_magic);
				//считаем оставш число секретов
				x=0;
				for ( i=0; i<maxmodel; ++i ) {
					if ( models[i].typ>=m_copy ) ++x;
				}
				gp_str_func.sprintf(stroka, "SECRETS: %u%%",(s16)((nsecret-x)*100/nsecret));
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_black_font, 20+2, 110+2, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_green_font, 20, 110, (char*)stroka);

				while ( is_sfx_playing() );
				Delay(200);
				SE(sfx_coin);
				gp_str_func.sprintf(stroka, "SCORE: %u",score);
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_black_font, 20+2, 150+2, (char*)stroka);
				BTextOut(NULL, &gpDraw[nflip?0:1], &big_font, 20, 150, (char*)stroka);
				if ( score > savegame[curr_level-1].highscore)
				{
					gp_str_func.sprintf(stroka, "HI-SCORE: %07u",savegame[curr_level-1].highscore);
					BTextOut(NULL, &gpDraw[nflip?0:1], &big_black_font, 20+2, 190+2, (char*)stroka);
					BTextOut(NULL, &gpDraw[nflip?0:1], &big_red_font, 20, 190, (char*)stroka);
				}


				if ( score > savegame[curr_level-1].highscore)
				{
					savegame[curr_level-1].highscore = score;
					while ( is_sfx_playing() );
					save_savegame();
				}

				//PaletteOn(palette);

				Delay(14*200);
				tic=gettic()+18*200;
				end=0;
				while (gettic()<tic && end==0) md(1);
				PaletteOff(palette);

//-----------------------------
				//выкл муз
				//!stop_sound();
				modstop();
	
				transition = 1;

				//на следующь уровень
				
				//проверка... на последний уровень обычный или из бонусов
				if ( leveldef.end &&
					( (curr_level == 7 && installed_bonus==0) || (curr_level > 7 && installed_bonus) )
					) {
					//конец игры
					if (installed_bonus)
						SetLib("bonus\\mult");
					else
						SetLib("mult");
					do_mult("theend.scr",0);
					PaletteOff(palette);
					do_mult("staff.scr",0);

					continues=end=hearts=0;

					//проверка на конец
				} else {
					//инкремент уровня
					++curr_level;
					end=shields=0; //убираем щит, если есть
#ifdef DEMO
					if( curr_level>=2 ) {
						SetLib("mult");
						do_mult("02level.scr",0);
						Saturate(0,0,320,240,205);
						BTextOut(NULL, &gpDraw[nflip], &big_black_font, 21+2, 110+2, "THE END OF DEMO");
						BTextOut(NULL, &gpDraw[nflip], &big_font, 21, 110, "THE END OF DEMO");
						FlipAndShow();
						Delay(2500);
						fatalerror(textindex[11]);
						//GpAppExit();
					}
#endif


				}
			} else {
				//прерываение игры
				continues=end=hearts=0;
			}
		} // для hearts
	} //это для contin

	PaletteOff(palette);
}