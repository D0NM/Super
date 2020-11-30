/*
скриптовая система показа мультов

sprite <name> <lx> <ly> <ox> <oy> <pic name> <pic width> <pic height>
 - создать спрайт
picture <pic name> <pic width> <pic height>
 - создать и загрузить картинку
animate sprite <n> <sx> <sy> <delay>
 - анимировать спрайт n фаз, sx sy - сдвиг в картинке, Delay - пауза между сменой картинок
strip <name.pic> <name.scr> <number>
 - загрузить стрип и сделать его фоном с шагом number
background <number>
 - задать задний фон - заливка цветом номер
background <pic name> <step x> <step y>
 - задать задний фон - скроллирующуюся картинку
background stripe <number>
 - задать задний фон - загруженный ранее stripe + скорость
palette <name>
 - загрузить палитру (не работает.. не тестировал)
on
 - палитру вкл
off
 - затушить экран
text <number>
 - текст.. из языковых... гмм
render
 - вкл/выкл отрисовку
fx <sprite> <flow|shake|stop|walk> <sx> <sy> <wait>
 - задать спец эффект спрайту (и границы и задержку срабатывания)
set <sprite> <x> <y>
 - переместить спрайт Х У
move <sprite> <x> <y> <step>
 - отправить спрайт по линии в Х У с шагом прироста step
wait <number renders>
 - ждать number отрисовок
while <sprite>
 - ждать пока спрайт движется
swap <sprite> <sprite>
 - поменять спрайты местами
hide <sprite>
 - спрятать спрайт
show <sprite>
 - показать спрайт
;
 - комментарий
q
 - выход

*/

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

extern screen hidscr; //скрытая страничка
extern s16 end;
extern GPDRAWSURFACE gpDraw[2];
extern int nflip;
extern block bckg;
extern block textindex[200]; //ссылки на строки текста 0,1...100
extern struct BFont text_font;

void show_strip(s16 y);
void do_strip(s16 sx);
int fatoi(char *s);
int gettok(char **s, char *d); //выбираем текстовый токен... строку или число. если конец строки - возвр -1
int getNtok(char **s);

char script_quit[]="q";	//конец скрипта
char *script_index[300]; //ссылки на строки скрипта
block mult_script;
int background_type;	//тип фона 0 = картина, -1 = stripe, остальное 1..255 = цвет
block background_image;	//ссылка кртинку фона 320х200 !!
int background_sx = 0, background_sy = 0; //скорость смещения подложки но Х и У
int background_x = 0;
int background_y = 0;

int background_stripe;	//скорость скролла страйпа

int text_colour;	//цвет выводимого текста 0 - нет текста
int text_n;		//текущий текст субтитров -1 - нету

int ready_to_render;	//загрузилось ли все что надо... можно ли показывать мульт

int pictures_n = 0; //кол-во картинок текущее
struct ss_pictures { //описание картинок
	char name[14];
	int lx,ly; //размер картинки
	block buf; //ссылка на текущую фазу
};
struct ss_pictures s_pictures[20];

int sprites_n = 0;	//кол-во спрайтов текущее
struct ss_sprites { //описание спрайтов
	char name[14];
	int ox,oy;	//смещение спрайта внутри его картинки
	int lx,ly;	//размер спрайта
	int x,y;	//коорд-ы спрайта на экране
	int x1,y1;	//смещение для спрайта
	int x2,y2;	//КУДА НАДО попасть смещению спрайта
	int xt,yt;	//x,y target - куда перемещаем x,y
	int typ;	//тип спрайта (0 - спрайт, 1 - текст, 2 - ???)
	int m;		//тип движения
	int glue;	//ссылка на спрайт к которому приклеен
	int gx,gy;	//смещение приклеенного спрайта
//-- для линейного перемещения 
	int dl_x,dl_y;
	int mdl_x,mdl_y;
	int ms;	//скорость
	
//-- для анимации
	int sx,sy;	//смещение для след фазы спрайта
	int n;		//кол-во фаз спрайта 0 - нету
	int f;		//номер тек фазы
	int wa;	//тек задержка смены фазы /дриблинга
	int da;	//МАКС задержка в циклах до смены фазы

//-- для спец эффектов и перемены движения
	int wait;	//тек задержка смены фазы /дриблинга
	int delay;	//МАКС задержка в циклах до смены фазы
	int dx,dy;	//границы дрожания - ездиния
	int fx;		//тип FX спрайта
	struct ss_pictures *pic; //ссылка на картинку

	int on;		//показывать спрайт или нет (-1 - спрайт пуст) 2- прозрачный
};
struct ss_sprites s_sprites[50];


void render_script(void);
void init_script(void);
void free_script(void);
void load_script(char *script_name);
block load_picture(char *name);
void make_sprite(char *name);
void set_sprite(char *name, int mode);
void show_sprite(char *name, int on);
void glue_sprite(char *name, char *name2, int mode);
void swap_sprite(char *name,char *name2);
void set_fx(char *name);
void animate_sprite(char *name);
void do_sprite(int i);

void load_script(char *script_name)
{
	int i=0, ts;
	block p;

	//размер скрипта
	ts = SizeLib(script_name);
	mult_script = (block)famemalloc(ts+2);

	//загрузка скрипта
	GetLib(script_name,(block)mult_script);
	//индексируем строки
	p = mult_script;

	for (i=0; i<300; i++)
	{	//инициализируем все строки заглушкой
		script_index[i] = (block)script_quit;
	}
	i=0;
	while ( i<300 && p < (mult_script + ts) )	//кол-во строк макс и объем текста
	{
		//заносим очередную строку
		script_index[i++] = p++;
		//если конец файла
		if (*p == 0) {
			break;
		}
		while ( *p !='\n' && *p != '\r') {
			//пропускаем тело строки
			p++;
		}
		*p++ = 0;

		//__printf("script_data: str %d ='%s'\n",i-1,script_index[i-1]);

		//если ПК или ВК
		while ( (*p == '\n' || *p == '\r') && *p ) {
			p++;
		}	
	}
}


block	p;	//тек позиция в строке
char s_command[50];
char s_name[50];
char str1[50];
char str2[50];


GPDRAWTAG m_clip= {0,0,20,320,200};
//	GpRectFill(NULL,&gpDraw[nflip], 0, 0, 320, 18, 148);
//	GpRectFill(NULL,&gpDraw[nflip], 0, 18, 320, 2, 152);
//	GpRectFill(NULL,&gpDraw[nflip], 0, 220, 320, 2, 152);
//	GpRectFill(NULL,&gpDraw[nflip], 0, 222, 320, 18, 148);

//считывание описания мульта и показ
//1й арг - имя скрипта в тек бибке, 2й - 1= можно прерывать по кнопке
void do_mult(char * name, int allow_interrupt) {
	int i,n,t;

	//выключим звук
	//!stop_sound();
	modstop();

	//loading();

	init_script();

	load_script(name);

	n=-1; end=0;
	do {
		render_script();
		n++;
		//если можно прервать по кнопке..т о читаем сост-е кнопок
		if (allow_interrupt) md(1);
#ifndef RELEASE
		__printf("script_data: N%d ='%s'\n",n,script_index[n]);
#endif
		//обработаем очередн строку скрипта
		p = script_index[n];
		gettok(&p,(char*)&s_command);

		//перескакиваем комментарий
		if (s_command[0] == ';') {
			continue;
		}
		// Конец скрипта "q" - выход
		if (gp_str_func.compare(s_command,"q")==0) {
			break;
		}
		//создание спрайта с этим именем (+загрузка картинки)
		if (gp_str_func.compare(s_command,"sprite")==0) {
			gettok(&p,(char*)&str1);
			make_sprite(str1);
			continue;
		} else
		//создание спрайта с этим именем (+загрузка картинки)
		if (gp_str_func.compare(s_command,"picture")==0) {
			gettok(&p,(char*)&str1);
			load_picture(str1);
			continue;
		} else
		//мнгновенное перемещение в координаты X Y спрайта
		if (gp_str_func.compare(s_command,"set")==0) {
			gettok(&p,(char*)&str1);
			set_sprite(str1,0);
		} else
		//плавное перемещение в координаты X Y спрайта
		if (gp_str_func.compare(s_command,"move")==0) {
			gettok(&p,(char*)&str1);
			set_sprite(str1,1);
		} else
		//показать спрайт
		if (gp_str_func.compare(s_command,"show")==0) {
			gettok(&p,(char*)&str1);
			show_sprite(str1,1);
			continue;
		} else
		//cпрятать спрайт
		if (gp_str_func.compare(s_command,"hide")==0) {
			gettok(&p,(char*)&str1);
			show_sprite(str1,0);
			continue;
		} else
		if (gp_str_func.compare(s_command,"showhide")==0) {
			//показать спрайт
			gettok(&p,(char*)&str1);
			show_sprite(str1,1);
			//cпрятать спрайт
			gettok(&p,(char*)&str1);
			show_sprite(str1,0);
			continue;
		} else
		//задать спрайту FX
		if (gp_str_func.compare(s_command,"fx")==0) {
			gettok(&p,(char*)&str1);
			set_fx(str1);
			continue;
		} else
		//подставить новый спрайт, скрытый на место старого
		if (gp_str_func.compare(s_command,"swap")==0) {
			gettok(&p,(char*)&str1);
			gettok(&p,(char*)&str2);
			swap_sprite(str1,str2);
			continue;
		} else
		//приклеить 2й спрайт к 1му
		if (gp_str_func.compare(s_command,"glue")==0) {
			gettok(&p,(char*)&str1);
			gettok(&p,(char*)&str2);
			glue_sprite(str1,str2,1);
			continue;
		} else
		//отклеить спрайт
		if (gp_str_func.compare(s_command,"unglue")==0) {
			gettok(&p,(char*)&str1);
			glue_sprite(str1,str2,0);
			continue;
		} else
		//анимация спрайта
		if (gp_str_func.compare(s_command,"animate")==0) {
			gettok(&p,(char*)&str1);
			animate_sprite(str1);
			continue;
		} else
		//разрешить прерывание просмотра мультика кнопкой
		if (gp_str_func.compare(s_command,"allow")==0) {
			allow_interrupt = 1;
			continue;
		} else
		//загрузить палитру новую
		if (gp_str_func.compare(s_command,"palette")==0) {
			gettok(&p,(char*)&str1);	//надо палитру старую сохранять
			GetLib(str1,palette);
			gettok(&p,(char*)&str2);
			if (gp_str_func.compare(str2,"on")==0) {
				PutPalette(palette);
			}
			continue;
		} else
		//осветлить экран
		if (gp_str_func.compare(s_command,"on")==0) {
			PaletteOn(palette);
			continue;
		} else
		//затушить экран
		if (gp_str_func.compare(s_command,"off")==0) {
			PaletteOff(palette);
			continue;
		} else
		//загрузка текста (то есть ссылка на него)
		if (gp_str_func.compare(s_command,"text")==0) {
			//цвет выводимого текста 0 - нет текста
			text_colour = getNtok(&p);
			//текущий текст субтитров 0 - нету
			text_n = getNtok(&p);
#ifndef RELEASE
			__printf("text -> col %d Nt %d\n",text_colour,text_n);
#endif
			continue;
		} else
		//загрузка параллакса - Stripe
		if (gp_str_func.compare(s_command,"strip")==0) {
			gettok(&p,(char*)&str1);
			GetLib(str1,(block)bckg);
			TurnIt((block)bckg, 320, 200);
			gettok(&p,(char*)&str2);
			GetLib(str2,(block)hidscr);
			readstrip();
			background_type = -1;
			//скорость скроллинга если задали
			background_stripe = getNtok(&p);
			continue;
		} else
		//установка-изменение скорости скроллинга параллакса - Stripe
		if (gp_str_func.compare(s_command,"strip_speed")==0) {
			//скорость скроллинга
			background_stripe = getNtok(&p);
			continue;
		} else
		//установка-изменение заднего фона
		if (gp_str_func.compare(s_command,"background")==0) {
			gettok(&p,(char*)&str1);
			//это картинка?
			t = 1; //флаг - нет картинки
			for (i = 0; i< pictures_n; i++)
			{
				if ( gp_str_func.compare(s_pictures[i].name,str1)==0) {
					background_type = 0; //картинка
					//проверим если другая картинка - то смещения обнуляем
					if (background_image != s_pictures[i].buf)
					{
						background_x = background_y = 0;
						background_image = s_pictures[i].buf;
					}
					background_sx = getNtok(&p);
					background_sy = getNtok(&p);
					//если reset то сбросим смещение
					gettok(&p,(char*)&str1);
					if ( gp_str_func.compare("reset",str1)==0) {
						background_x = background_y = 0;
					}
					t=0;	//нашли картинку
					break;
				}
			}
			if (t) {
				//цвет
				background_type = fatoi(str1);
				if (gp_str_func.compare(s_command,"strip")==0) {
					background_type = -1; //стрип
					background_stripe = getNtok(&p);
				}
			}
			continue;
		} else
		//спрятать все спрайты и обездвижить их
		if (gp_str_func.compare(s_command,"clear")==0) {
			for (i = 0; i< sprites_n; i++)
				s_sprites[i].on = -1;
			//уберем субтитры
			text_colour = text_n = 0;
			continue;
		} else
		//пауза
		if (gp_str_func.compare(s_command,"wait")==0) {
			//цикл с паузой
			t = getNtok(&p);
			for (i=0; i< t; i++)
			{
				render_script();
				//если можно прервать по кнопке..т о читаем сост-е кнопок
				if (allow_interrupt) md(1);
				Delay(10);
				if (end) break;	//выход по нажатию кнопы
			}
			continue;
		} else
		//ждать пока спрайт не долетит куда надо
		if (gp_str_func.compare(s_command,"while")==0) {
			//цикл с паузой
			gettok(&p,(char*)&str1);
			for (i = 0; i< sprites_n; i++)
			{
				if ( gp_str_func.compare(s_sprites[i].name,str1)==0) {
					break;
				}
			}
			while ( s_sprites[i].m == 1 && s_sprites[i].m != 2)
			{
#ifndef RELEASE
				__printf("W.");
#endif
				render_script();
				//если можно прервать по кнопке..т о читаем сост-е кнопок
				if (allow_interrupt) md(1);
				Delay(10);
				if (end) break;	//выход по нажатию кнопы
			}
			continue;
		} else
		if (gp_str_func.compare(s_command,"transp")==0) {
			//сделать прозрачным
			gettok(&p,(char*)&str1);
			for (i = 0; i< sprites_n; i++)
			{
				if ( gp_str_func.compare(s_sprites[i].name,str1)==0) {
					break;
				}
			}
			s_sprites[i].on = getNtok(&p); //цвет самый темный из 8и для прозрачности
			continue;
		} else
		//играть звуковой эффект
		if (gp_str_func.compare(s_command,"se")==0) {
			SE(getNtok(&p));
			continue;
		} else
		//играть MOD музыку
		if (gp_str_func.compare(s_command,"mod")==0) {
			gettok(&p,(char*)&str1);	//надо палитру старую сохранять
			if (gp_str_func.compare(str1,"stop")==0) {
				modstop();
			} else {
				//!stop_sound();
				modsetup(str1, 4, 0 ,0, 0, 0 );
				//!start_sound();
			}
			continue;
		} else
		//можно ли рендерить
		if (gp_str_func.compare(s_command,"render")==0) {
			//переключаем ВКЛ - ВЫКЛ
			ready_to_render = ready_to_render?0:1;
			//!start_sound();
		}
	
	} while ( n<300 && end==0 );

	//убираем субтитры
	text_colour = text_n = 0;

	render_script();
	render_script();	//2й раз для скрытой стр

	//очистим память и т.д.
	free_script();
	//подождем пока звук закончится
	while ( is_sfx_playing() );
	modstop();
	//stop_sound();
	//start_sound();
}

#define spr_y_off 20
//отрисовка спрайтового окружения
void render_script(void) {
	int i;

	if ( ready_to_render == 0 ) return; //рендерить нельзя
#ifndef RELEASE
	__printf("R.");
#endif
	//рисуем фон
	if (background_type < 0)
	{	//скроллинг
		do_strip(background_stripe);
		show_strip(0);
	} else
	if (background_type == 0 && background_image != NULL )
	{	//фоновая картинка
		GpBitBlt(&m_clip,&gpDraw[nflip],
			0+background_x,spr_y_off+background_y,320,200,(unsigned char*)background_image,
			0,0,
			320,200);
		GpBitBlt(&m_clip,&gpDraw[nflip],
			-320+background_x,spr_y_off+background_y,320,200,(unsigned char*)background_image,
			0,0,
			320,200);
		GpBitBlt(&m_clip,&gpDraw[nflip],
			-320+background_x,-200+spr_y_off+background_y,320,200,(unsigned char*)background_image,
			0,0,
			320,200);
		GpBitBlt(&m_clip,&gpDraw[nflip],
			0+background_x,-200+spr_y_off+background_y,320,200,(unsigned char*)background_image,
			0,0,
			320,200);

		background_x += background_sx;
		background_y += background_sy;
		if (background_x<0)
			background_x += 320;
		if (background_x>319)
			background_x -= 320;
		if (background_y<0)
			background_y += 200;
		if (background_y>199)
			background_y -= 200;
	} else
	{	//фон - одним цветом
		//Cls(background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20, 320, 50, background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20+50, 320, 50, background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20+50+50, 320, 50, background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20+50+50+50, 320, 50, background_type);
	}

	//рисуем объекты как надо
	for (i = 0; i< sprites_n; i++)
	{
		if (s_sprites[i].on > 0 ) //показываем включенные спр-ты
		if ( s_sprites[i].on < 8 ) {
			//если спрайт не прозрачный
			GpTransBlt(&m_clip,&gpDraw[nflip],
			s_sprites[i].x+s_sprites[i].x1,s_sprites[i].y+s_sprites[i].y1+spr_y_off,s_sprites[i].lx,s_sprites[i].ly,(unsigned char*)s_sprites[i].pic->buf,
			s_sprites[i].ox+s_sprites[i].sx*s_sprites[i].f,s_sprites[i].oy+s_sprites[i].sy*s_sprites[i].f,
			s_sprites[i].pic->lx,s_sprites[i].pic->ly,0);
		} else {
			//если прозрачный
			BltSatur(s_sprites[i].x+s_sprites[i].x1,s_sprites[i].y+s_sprites[i].y1+spr_y_off,s_sprites[i].lx,s_sprites[i].ly,(block)s_sprites[i].pic->buf,
			s_sprites[i].ox+s_sprites[i].sx*s_sprites[i].f,s_sprites[i].oy+s_sprites[i].sy*s_sprites[i].f,
			s_sprites[i].pic->lx,s_sprites[i].pic->ly,s_sprites[i].on);
		}
	}
	
	//двигаем объекты как надо
	for (i = 0; i< sprites_n; i++)
	{
		if ( s_sprites[i].on > 0 ) {
			do_sprite(i);
		}
	}

	//обрезка низа-верха в роликах
	GpRectFill(NULL,&gpDraw[nflip], 0, 0, 320, 18, 148);
	GpRectFill(NULL,&gpDraw[nflip], 0, 18, 320, 2, 152);
	GpRectFill(NULL,&gpDraw[nflip], 0, 220, 320, 2, 152);
	GpRectFill(NULL,&gpDraw[nflip], 0, 222, 320, 18, 148);

	//если есть субтитры - выводим их
	if ( text_colour > 0 && text_n > 0 ) {
		i=( 320-BTextWidthGet(&text_font,textindex[text_n]) )/2;
		text_font.color[0] = 8;
		BTextOut(NULL, &gpDraw[nflip], &text_font, i+1, 220+1, textindex[text_n]);
		//текущий текст субтитров -1 - нету
		text_font.color[0] = text_colour;
		BTextOut(NULL, &gpDraw[nflip], &text_font, i, 220, textindex[text_n]);
	}

	FlipAndShow();
	Delay(10);
}

extern block tiles;
void init_script(void)
{
	int i;
	background_stripe = 1;	//скорость скролла по умолчанию
	ready_to_render = 0;	//рендерить нельзя
	//Clrs(8,nflip);	//черн
	TileBar(0,0,320,200,64,64,(block)tiles+random(4)*64*64);

	//ПОДЛОЖКА скорость смещения подложки но Х и У
	background_sx = background_sy = background_x = background_y = 0;
	background_image = NULL;
	background_type = 15; //белый цвет

	//очистка картинок загружнных
	/*for (i=0; i<20; i++)
	{
		s_pictures[i].name[0] = 0;
	}*/
	gp_str_func.memset(s_pictures,0,sizeof(s_pictures));
	pictures_n = 0;
	//очистка спрайтов
	gp_str_func.memset(s_sprites,0,sizeof(s_sprites));
	for (i=0; i<50; i++)
	{
		s_sprites[i].name[0] = 0;
		s_sprites[i].on = -1;
	}
	sprites_n=0;

	//цвет выводимого текста 0 - нет текста
	text_colour = text_n = 0;
	
}

void free_script(void)
{
	int i;
	//очистка картинок загружнных
	for (i= pictures_n-1; i>=0; i--)
	{
		gp_mem_func.free(s_pictures[i].buf);
		s_pictures[i].name[0] = 0;
	}
	//очистка спрайтов
	for (i= sprites_n-1; i>=0; i--)
	{
		s_sprites[i].name[0] = 0;
		s_sprites[i].on = -1;
	}

	//освободим память
	gp_mem_func.free(mult_script);

}

//загружаем картинку
block load_picture(char *name)
{
	int i;
	//ищем сначала если уже есть такие картинки в памяти, чтобы больше не грузить
	for (i = 0; i< pictures_n; i++)
	{
		if ( gp_str_func.compare(s_pictures[i].name,name)==0) {
			return (block)&s_pictures[i];
		}
	}
	//такой картинки нет - загрузим
	for (i = 0; i<20; i++)
	{
		if ( s_pictures[i].name[0]==0 ) {
			//занесем размеры картинки
			s_pictures[i].lx = getNtok(&p);
			s_pictures[i].ly = getNtok(&p);
			gp_str_func.strcpy(s_pictures[i].name,name);
			//размер
			s_pictures[i].buf = (block)famemalloc(SizeLib(name));
			//загрузка картинки
			GetLib(name,(block)s_pictures[i].buf);
			//поворот картинки
			TurnIt(s_pictures[i].buf,s_pictures[i].lx,s_pictures[i].ly);
			//увеличим макс-кол-во загруженных картинок
			pictures_n++;
#ifndef RELEASE
			__printf("picture %s [%d x %d]\n",s_pictures[i].name,s_pictures[i].lx,s_pictures[i].ly);
#endif
			return (block)&s_pictures[i];
		}
	}
	//вообще-то тут должен быть еррор но..
	return (block)&s_pictures[0];
}

//создаем спрайт
//формат: sprite <sprname> <lx> <ly> <ox> <oy> <pic name> <lx> <ly>
void make_sprite(char *name)
{
	int i;
	//ищем сначала если уже есть такие в памяти, чтобы больше не грузить
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			goto change_existing_sprite;
		}
	}
	//такого спрайта нет - загрузим
	for (i = 0; i<50; i++)
	{
		if ( s_sprites[i].on < 0 ) {
			s_sprites[i].x = s_sprites[i].y = s_sprites[i].xt = s_sprites[i].yt = s_sprites[i].on = s_sprites[i].m = 0;
			sprites_n++;
			break;
		}
	}
	//типа можно контроль за переполненеием поставить но...

change_existing_sprite:
	//занесем размеры картинки
	s_sprites[i].lx = getNtok(&p);
	s_sprites[i].ly = getNtok(&p);
	s_sprites[i].ox = getNtok(&p);
	s_sprites[i].oy = getNtok(&p);
	gp_str_func.strcpy(s_sprites[i].name,name);
#ifndef RELEASE
	__printf("sprite %s [%d x %d]\n",s_sprites[i].name,s_sprites[i].lx,s_sprites[i].ly);
#endif
	//начальные установки спрайтов
	s_sprites[i].fx = s_sprites[i].wait = s_sprites[i].delay =
	s_sprites[i].n = s_sprites[i].f = s_sprites[i].m =
	s_sprites[i].wa = s_sprites[i].da = 0;

	//грузим картинку спрайта если надо
	gettok(&p,(char*)&str1);
	s_sprites[i].pic = (struct ss_pictures *)load_picture(str1);

	return;
}

//мнгновенно перемещаем спрайт
//формат: set <sprname> <x> <y>
void set_sprite(char *name, int mode)
{
	int i,dx,dy;
	//ищем сначала если уже есть такие в памяти, чтобы больше не грузить
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//такого спрайта нет - ошибка
	if (mode)
	{	//если MOVE
		s_sprites[i].xt = getNtok(&p);
		s_sprites[i].yt = getNtok(&p);
		s_sprites[i].ms = getNtok(&p);	//шаг! прироста

		s_sprites[i].m = 1; //включить движение
		//рассчитаем переменные, необходимые для дв-я по линии

		dx = abs(s_sprites[i].xt-s_sprites[i].x);
		dy = abs(s_sprites[i].yt-s_sprites[i].y);

		if ( dx < dy ) {
			s_sprites[i].mdl_y=0;
			s_sprites[i].mdl_x=dy / max(1,dx);
		} else {
			s_sprites[i].mdl_x=0;
			s_sprites[i].mdl_y=dx / max(1,dy);
		}
		s_sprites[i].dl_x = s_sprites[i].dl_y = 0;
#ifndef RELEASE
		__printf("move N %d = %d,%d\n",i,s_sprites[i].xt,s_sprites[i].yt);
#endif
	}
	else
	{	//есди SET
		s_sprites[i].x = s_sprites[i].xt = getNtok(&p);
		s_sprites[i].y = s_sprites[i].yt = getNtok(&p);
		s_sprites[i].x1 = s_sprites[i].x2 = s_sprites[i].y1 = s_sprites[i].y2 = 0;
		s_sprites[i].m = 0;
#ifndef RELEASE
		__printf("set N %d = %d,%d\n",i,s_sprites[i].x,s_sprites[i].y);
#endif
	}
	s_sprites[i].on = 1;
	return;
}

//показываем или прячем спрайт
//формат: show/hide <sprname>
void show_sprite(char *name, int on)
{
	int i;
	//ищем сначала если уже есть такие в памяти, чтобы больше не грузить
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//такого спрайта нет - ошибка
#ifndef RELEASE
	__printf("show N %d = %d\n",i,on);
#endif
	s_sprites[i].on = on;

	return;
}

//приклеить спрайт 2й к 1му или отклеить
void glue_sprite(char *name, char *name2, int mode)
{
	int i,j;
	//ищем сначала 1й
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	if ( mode == 0)
	{
		//отклеить
		s_sprites[i].m = 0;
#ifndef RELEASE
		__printf("unglue N %d",i);
#endif
		return;
	}

	//ищем 2й
	for (j = 0; j< sprites_n; j++)
	{
		if ( gp_str_func.compare(s_sprites[j].name,name2)==0) {
			break;
		}
	}
	//такого спрайта нет - ошибка

	s_sprites[j].glue = i;
	s_sprites[j].gx = s_sprites[i].x - s_sprites[j].x;
	s_sprites[j].gy = s_sprites[i].y - s_sprites[j].y;

	s_sprites[j].m = 2;	//тип перемещения glue
#ifndef RELEASE
	__printf("glue N %d <- N %d\n",i,j);
#endif
	return;
}

//формат: animate <sprname> <n> <sx> <sy> <delay>
void animate_sprite(char *name)
{
	int i;
	//ищем сначала если уже есть такие в памяти, чтобы больше не грузить
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//такого спрайта нет - ошибка

	s_sprites[i].n = getNtok(&p)-1;
	s_sprites[i].wa = s_sprites[i].f = 0;
	s_sprites[i].sx = getNtok(&p);//шаг! прироста
	s_sprites[i].sy = getNtok(&p);
	s_sprites[i].da = getNtok(&p);	
}

void swap (int *pa, int *pb) {
	register int t;
	t=*pa; *pa=*pb; *pb=t;
}

void swap_sprite(char *name,char *name2)
{
	int i,j,k;
	//ищем сначала 1й
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//ищем 2й
	for (j = 0; j< sprites_n; j++)
	{
		if ( gp_str_func.compare(s_sprites[j].name,name2)==0) {
			break;
		}
	}
	//такого спрайта нет - ошибка
#ifndef RELEASE
	__printf("swap N %d <- N %d\n",i,j);
#endif
	swap(&s_sprites[i].x,&s_sprites[j].x);
	swap(&s_sprites[i].y,&s_sprites[j].y);
	swap(&s_sprites[i].xt,&s_sprites[j].xt);
	swap(&s_sprites[i].yt,&s_sprites[j].yt);
	//корректируем координаты если размеры разные
	if (s_sprites[i].x != s_sprites[j].x)
	{
		s_sprites[j].x += (s_sprites[i].lx - s_sprites[j].lx)/2;
		s_sprites[j].xt += (s_sprites[i].lx - s_sprites[j].lx)/2;
	}
	if (s_sprites[i].y != s_sprites[j].y)
	{
		s_sprites[j].y += (s_sprites[i].ly - s_sprites[j].ly)/2;
		s_sprites[j].yt += (s_sprites[i].ly - s_sprites[j].ly)/2;
	}
	//
	swap(&s_sprites[i].dx,&s_sprites[j].dx);
	swap(&s_sprites[i].dy,&s_sprites[j].dy);
	swap(&s_sprites[i].on,&s_sprites[j].on);
	swap(&s_sprites[i].x1,&s_sprites[j].x1);
	swap(&s_sprites[i].x2,&s_sprites[j].x2);
	swap(&s_sprites[i].y1,&s_sprites[j].y1);
	swap(&s_sprites[i].y2,&s_sprites[j].y2);
	swap(&s_sprites[i].m,&s_sprites[j].m);

	//глобальное движ-е
	swap(&s_sprites[i].ms,&s_sprites[j].ms);
	swap(&s_sprites[i].dl_x,&s_sprites[j].dl_x);
	swap(&s_sprites[i].dl_y,&s_sprites[j].dl_y);
	swap(&s_sprites[i].mdl_x,&s_sprites[j].mdl_x);
	swap(&s_sprites[i].mdl_y,&s_sprites[j].mdl_y);

	//glue
	swap(&s_sprites[i].glue,&s_sprites[j].glue);
	swap(&s_sprites[i].gx,&s_sprites[j].gx);
	swap(&s_sprites[i].gy,&s_sprites[j].gy);
	//переправим ссылку на родительский GLUE object у дочерних
	for (k = 0; k< sprites_n; k++)
	{
		if ( s_sprites[k].glue == i ) {
			s_sprites[k].glue = j;
		}
	}
	return;
}

char n_fx[10][6]={"stop","shake","flow","walk","na","na","na","na","na10"};

//установим эффект для вывода спрайта относительно его координат
//формат: fx <sprname> <fx name> <sx> <sy> <delay>
void set_fx(char *name)
{
	int i,j;
	//ищем сначала
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//такого спрайта нет - ошибка

	gettok(&p,(char*)&str2);
	for (j = 0; j< 10; j++)
	{
		if ( gp_str_func.compare(n_fx[j],str2)==0) {
			break;
		}
	}
#ifndef RELEASE
	__printf("fx N %d fx(%s)\n",i,n_fx[j]);
#endif
	s_sprites[i].wait = 0;
	s_sprites[i].fx = j;
	if (j)	//если не fx "stop"
	{
		s_sprites[i].dx = getNtok(&p);
		s_sprites[i].dy = getNtok(&p);
	} else {
		return;
	}
	//у приклеенного или тормозного спрайта может быть задержка!
	s_sprites[i].delay = getNtok(&p);
	return;
}

#define sign(n) n>0?1:-1

//Fx для спрайта (дрожание и т.д.)
void do_sprite(int i)
{
	int j;
	//отработка задержек анимации
	if ( --s_sprites[i].wa < 0 )
	{
		s_sprites[i].wa = s_sprites[i].da;
		//СЛЕДУЮЩАЯ ФАЗА спрайта
		if ( --s_sprites[i].f < 0 )
		{
			s_sprites[i].f = s_sprites[i].n;
		}
	}
	//по типу FX делаем то что надо
	switch (s_sprites[i].fx)
	{
		case 1: //shake
			//отработка задержек для FX и смены движения
			if ( --s_sprites[i].wait < 0 )
			{
				s_sprites[i].wait = s_sprites[i].delay;

				s_sprites[i].x2 = s_sprites[i].x1 = random(s_sprites[i].dx*2) - s_sprites[i].dx;
				s_sprites[i].y2 = s_sprites[i].y1 = random(s_sprites[i].dy*2) - s_sprites[i].dy;
			}
			break;
		case 2: //flow   x:
			//отработка задержек для FX и смены движения
			if ( --s_sprites[i].wait < 0 )
			{
				s_sprites[i].wait = s_sprites[i].delay;

				if ( s_sprites[i].x1 == s_sprites[i].x2 && s_sprites[i].y1 == s_sprites[i].y2 )
				{
					s_sprites[i].x2 = random(s_sprites[i].dx*2) - s_sprites[i].dx;
					s_sprites[i].y2 = random(s_sprites[i].dy*2) - s_sprites[i].dy;
				}
			}
			//движем спрайт к цели
			if (s_sprites[i].x1 < s_sprites[i].x2)
				s_sprites[i].x1++;
			else if (s_sprites[i].x1 > s_sprites[i].x2)
				s_sprites[i].x1--;
			if (s_sprites[i].y1 < s_sprites[i].y2)
				s_sprites[i].y1++;
			else if (s_sprites[i].y1 > s_sprites[i].y2)
				s_sprites[i].y1--;
			break;
		case 3: //walk
			//отработка задержку для смены движения
			if ( --s_sprites[i].wait < 0 )
			{
				s_sprites[i].wait = s_sprites[i].delay;

				s_sprites[i].x2 = -sign(s_sprites[i].x2)*s_sprites[i].dx;
				s_sprites[i].y2 = -sign(s_sprites[i].y2)*s_sprites[i].dy;
			}
			//движем спрайт к цели
			if (s_sprites[i].x1 < s_sprites[i].x2)
				s_sprites[i].x1++;
			else if (s_sprites[i].x1 > s_sprites[i].x2)
				s_sprites[i].x1--;
			if (s_sprites[i].y1 < s_sprites[i].y2)
				s_sprites[i].y1++;
			else if (s_sprites[i].y1 > s_sprites[i].y2)
				s_sprites[i].y1--;
			break;
	}

	//глобальное движ-е спрайта
	switch (s_sprites[i].m)
	{
		case 0: //stop
			break;
		case 2: //glue
			s_sprites[i].x = s_sprites[s_sprites[i].glue].x+s_sprites[s_sprites[i].glue].x1 - s_sprites[i].gx;
			s_sprites[i].y = s_sprites[s_sprites[i].glue].y+s_sprites[s_sprites[i].glue].y1 - s_sprites[i].gy;
			break;
		default:
			//этот цикл - для ШАГА при перемещ
			for (j = 0; j< s_sprites[i].ms; j++) {
				//сдвиг по Х
				if ( --s_sprites[i].dl_x <= 0) {
					s_sprites[i].dl_x = s_sprites[i].mdl_x;
					if (s_sprites[i].x < s_sprites[i].xt)
					{
						s_sprites[i].x++;
					}
					else if (s_sprites[i].x > s_sprites[i].xt)
					{
						s_sprites[i].x--;
					}
				}
				//сдвиг по У
				if ( --s_sprites[i].dl_y <= 0) {
					s_sprites[i].dl_y = s_sprites[i].mdl_y;
					if (s_sprites[i].y < s_sprites[i].yt)
					{
						s_sprites[i].y++;
					}
					else if (s_sprites[i].y > s_sprites[i].yt)
					{
						s_sprites[i].y--;
					}
				}
			}
			//если достигли цели - выкл. движ-е
			if ( s_sprites[i].x == s_sprites[i].xt && s_sprites[i].y == s_sprites[i].yt ) {
				s_sprites[i].m = 0;
			}
			break;
	}
}