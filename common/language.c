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
extern block textdata;	//текстовая инф-я в строках
extern block textindex[200]; //ссылки на строки текста 0,1...
char txt_dumb[]="^_^";

extern struct sshop shop[12];

extern struct BFont big_font;

//char path_to_lng_file[]="gp:\\gpmm\\gpooh\\language\\";
extern char path_to_lng_file[];
//char lang_ini_file[]="language.ini";
extern char lang_ini_file[];
char current_language[16];
extern char path_to_data_file[];

int is_language_loaded = 1;	//1 = язык не загружен 0 - загружен

void check_language(void)
{
	ERR_CODE err_code;
	F_HANDLE h_rfile;
	unsigned long n_dummy;

	//грузим language.ini файл
	GpRelativePathSet(path_to_lng_file);

	err_code = GpFileOpen(lang_ini_file, OPEN_R, &h_rfile);
	if (err_code != SM_OK)
	{
		//если нет INI файла или там ошибка то выбираем язык
		select_language();
		return;
	}

	//ini файл открыт, считаем язык
	err_code = GpFileRead(h_rfile, current_language, 16, (ulong*)&n_dummy);
	if (err_code != SM_OK || n_dummy < 16)
	{
		//если INI файла не содержит имени то выбираем язык
		GpFileClose(h_rfile);
		select_language();
		return;
	}

	//закроем INI файл 
	GpFileClose(h_rfile);
	//считаем файлы языковые
	load_language();
}

void select_language(void)
{
	ERR_CODE err_code;

	GPDIRENTRY *p_list;
	typedef struct {           
		char name[16];
		char *g;	//графика.. для врем хран-я флага станы
	}MYDIRENTRY;
	MYDIRENTRY *pi_list;
	GPFILEATTR mAttr;
	int i,j,keydata;
	unsigned long list_count = 0, read_count = 0, final_count = 0;

	init_loading();
	loading();

	//файла нет - получаем список языков
	GpRelativePathSet(path_to_lng_file);

	err_code = GpDirEnumNum("\\", &list_count);
	if (err_code != SM_OK || list_count==0)
	{
		fatalerror("Error: Can't find any files\nin the LANGUAGE folder...");
	}

	p_list = (GPDIRENTRY*)gp_mem_func.calloc((int)list_count, sizeof(GPDIRENTRY));
	pi_list = (MYDIRENTRY*)gp_mem_func.calloc((int)list_count, sizeof(MYDIRENTRY));
	GpDirEnumList("\\", 0, list_count, p_list, &read_count);

	loading();

	for (i=0; i<read_count; i++)
	{
		err_code = GpFileAttr(p_list[i].name, &mAttr);
		if (err_code != SM_OK)
		{
			// none
		}
		if (gp_str_func.compare((const char*)p_list[i].name,".")==0)
		{	// .
			//root
		}
		else if (gp_str_func.compare((const char*)p_list[i].name,"..")==0)
		{	// ..
			//previous folder
		} else if (err_code == SM_OK && ((mAttr.attr & 0x10) == 0))	/* file */
		{
			for (j=0; j<13; j++)
			{
				if ( p_list[i].name[j] == '.')
				{
					if ( p_list[i].name[j+3] == 'L' || p_list[i].name[j+3] == 'l' )
					{
						p_list[i].name[j] = 0;
						gp_str_func.strcpy(pi_list[final_count++].name,p_list[i].name);
						break;
					}
				}
			}
			//gp_str_func.strcpy(pi_list[final_count++].name,p_list[i].name);
		}
	}

	if (final_count==0)
	{
		fatalerror("Error: Can't find any *.LNG files...");
	}
	if (final_count==1)
	{
		gp_mem_func.free(pi_list);
		gp_mem_func.free(p_list);
		//всего 1 язык найден. не покажем его и загрузим. и запишим ini.
		gp_str_func.strcpy(current_language,pi_list[0].name);
		//пишем ini
		save_language();
		//грузим языковые данные
		load_language();
		return;
	}

	//грузим из каждого найденного файла ФЛАГ и имя.
	for (i=0; i<final_count; i++)
	{
		loading();

		SetLib(pi_list[i].name);
		pi_list[i].g = (block)famemalloc(SizeLib("flag.bin"));
		GetLib("flag.bin",pi_list[i].g);
		TurnIt(pi_list[i].g,320,200);
	}
	//закрываем открытую базу
	//SetLib("");
	

	//!start_sound();

	//цикл выбора
	i=0;
	
	keydata = 0;
	while(1)
	{
		GpRectFill(NULL, &gpDraw[nflip], 0, 0, 320, 240, 227); //светло-голубой

		//выводим строки файлов
		for (j=0; j<final_count; j++)
		{
			//cursor
			if (i == j)
			{
				GpRectFill(NULL, &gpDraw[nflip], 0, (j-i)*70 + 85-3, 320, 64+6, 144); //синий
				GpRectFill(NULL, &gpDraw[nflip], 10+2, (j-i)*70 + 85+2, 96, 64, 255); //черный
			}

			//флаг
			GpTransBlt(NULL,&gpDraw[nflip],
				10,(j-i)*70 + 85,96,64,(unsigned char*)pi_list[j].g,
				0,0,
				320,200,
				0);
			//название языка
			GpTransBlt(NULL,&gpDraw[nflip],
				20+96,(j-i)*70 + 85 + 16,192,32,(unsigned char*)pi_list[j].g,
				96,0,
				320,200,
				0);
		}

		FlipAndShow();

 		//ждем нажатия клавиши
		Delay(300);
		GpKeyGetEx(&keydata);
		while (keydata == 0 )
		{
			GpKeyGetEx(&keydata);
			Delay(50);
		}

		//keyboard
		if ( keydata & GPC_VK_UP ) //press UP
		{
			i--;
			if (i<0)
			{
				i = 0;
			}
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_DOWN ) //press DOWN
		{
			i++;
			if (i >= final_count)
			{
				i = (final_count-1);
			}
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_FR ) //press FR
		{
			i = (final_count-1);
			SE(sfx_cursor);
		}
		if ( keydata & GPC_VK_FL ) //press FL
		{
			i = 0;
			SE(sfx_cursor);
		}
		if ( (keydata & GPC_VK_FA) || (keydata & GPC_VK_FB) || (keydata & GPC_VK_START) ) //press A, B or START
		{
			//язык выбран... дальше идем
			gp_str_func.strcpy(current_language,pi_list[i].name);
			SE(sfx_OK);
			break;
		}
	}

	//выведем выбранный язык
	GpRectFill(NULL, &gpDraw[nflip], 0, 0, 320, 240, 150); //синий
	//флаг
	GpTransBlt(NULL,&gpDraw[nflip],
		112,64,96,64,(unsigned char*)pi_list[i].g,
		0,0,320,200,0);
	//название языка
	GpTransBlt(NULL,&gpDraw[nflip],
		64, 144,192,32,(unsigned char*)pi_list[i].g,
		96,0,320,200,0);
	FlipAndShow();

	//выбрали - освободим всю память.
	for (i= final_count - 1; i>=0; i--)
	{
		gp_mem_func.free(pi_list[i].g);
	}
	gp_mem_func.free(pi_list);
	gp_mem_func.free(p_list);

	Delay(600);
	//!stop_sound();

	//запишим выбранное имя в ini-файл
	save_language();
	
	//откроем языковую библиотеку
	load_language();
}

void save_language(void)
{
	//запишим имя языка в ини файл
	ERR_CODE err_code;
	F_HANDLE h_rfile;

	//пишем текущий язык в language.ini файл
	GpRelativePathSet(path_to_lng_file);

	err_code = GpFileCreate(lang_ini_file, ALWAYS_CREATE, &h_rfile);
	if (err_code != SM_OK)
	{
		//если нет INI файла или там ошибка то выбираем язык
		fatalerror("Error: Can't create LANGUAGE.INI...");
	}

	//ini файл открыт, считаем язык
	err_code = GpFileWrite(h_rfile, current_language, 16);
	if (err_code != SM_OK)
	{
		fatalerror("Error: Can't write LANGUAGE.INI...");
	}

	//закроем INI файл 
	GpFileClose(h_rfile);
}

void load_language(void)
{
	int i=0,j=0, ts;
	block p;
	//здесь грузим всю языковую инфу из выбранного язык
	SetLib(current_language);

	//это уже сделано в главном модуле:  menus=(block)famemalloc(320*200);
	//а теперь загрузим менюшки и т.д.
	GetLib("menus.bin",(block)menus);
	TurnIt(menus,320,200);

	is_language_loaded = 0;	//язык типа загружен - показываем LOADING на этом языке
	//loading();

	//загрузка текстовой базы сообщений мультиязычной и занесение ссылок на стоки в массив
	ts = SizeLib("textdata.txt")+2;
	GetLib("textdata.txt",(block)textdata);
	//индексируем строки
	p = textdata;

	for (i=0; i<200; i++)
	{	//инициализируем все строки заглушкой
		textindex[i] = (block)txt_dumb;
	}
	i=0;
	while ( i<200 && p < (textdata + ts) )	//кол-во строк макс и объем текста
	{
		//заносим очередную строку
		textindex[i++] = p++;
		//если конец файла
		if (*p == 0) {
			break;
		}
		while ( *p !='\n' && *p != '\r') {
			//пропускаем тело строки
			if (*p == ';') //обрубаем комментарии
				*p++ = 0;
			if (*p == '#') //делаем из # перевод строки
				*p++ = '\n';
			p++;
		}
		*p++ = 0;
#ifndef RELEASE
		__printf("textdata: str %d ='%s'\n",i-1,textindex[i-1]);
#endif
		//если ПК или ВК
		while (*p == '\n' || *p == '\r') {
			p++;
		}	
	}
	//лечим буг... 8(
	*p++ = 0;
	textindex[i-1] = (block)txt_dumb;
#ifndef RELEASE
	__printf("F textdata: str %d ='%s'\n",i-1,textindex[i-1]);
	__printf("FIN textdata: str %d ='%s'\n",i,textindex[i]);
	//__printf("textdata: strings loaded %d\n",i);
#endif

	//заносим названия вещей в магазин
	j = 21;
	for (i=0; i<12; i++)
	{
		shop[i].name = textindex[j++];
		shop[i].description = textindex[j++];
	}

	GpRelativePathSet(path_to_data_file);	//!!! избавиться от этого
}

int loading_bar = 0;	//позиция в загрузке игры

#define ldb_x 60
#define ldb_y 140

void loading(void)
{
//показать надпись LOADING при загрузке чего-либо
	Clrs(139, nflip);	//черный

	if ( is_language_loaded ) 
	{
		BTextNOut(NULL, &gpDraw[nflip], &big_font, 84, 108, "LOADING...", 0, 10);
	}
	else
	{
		//если уже есть загруженные менюшки.. то... их и покажем
		GpTransBlt(NULL,&gpDraw[nflip],
			64,108,192,24,(unsigned char*)menus,
			128,96,
			320,200,
			0);
	}

	GpRectFill(NULL, &gpDraw[nflip], ldb_x, ldb_y, 200, 20, 142);
	GpRectFill(NULL, &gpDraw[nflip], ldb_x+1, ldb_y+1, 200-2, 20-2, 225);
	GpRectFill(NULL, &gpDraw[nflip], ldb_x+4, ldb_y+4, 200-8, 20-8, 228);
	GpRectFill(NULL, &gpDraw[nflip], ldb_x+5, ldb_y+5, 200-10, 20-10, 150);

	GpRectFill(NULL, &gpDraw[nflip], ldb_x+6, ldb_y+6, max(1,loading_bar)/*200-12*/, 20-12, 14);	//красн

	if ( loading_bar <= 200-12-5) loading_bar += 5; else loading_bar = 200-12;
	
	FlipAndShow();
}

void init_loading(void) {
	loading_bar = 0;
}
