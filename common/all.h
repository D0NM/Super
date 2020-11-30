#ifndef H_ALL
#define H_ALL

#include "gp32.h"

//#define RELEASE 1
//#define NOEMU 1
//#define DEMO 1
#define FASTDRAW 1

#define wclock 50

#define wmyrg 18*1 //кол во мырганий мужика

#define f_def files[0]
#define f_level files[1]
#define f_blevel files[2]
#define f_bmonstr files[3]
#define f_dmonstr files[4]
#define f_models files[5]
#define f_demo files[6]
#define f_fon files[7]
#define f_bckg files[8]
#define f_dstrip files[9]

//положение меню
#define xsp 296
#define ysp 3

#define maxsiz_level 100*500

#define maxdemo 1000

//режимы
enum {s_go,s_down,s_jump,s_fly,s_lift,s_ouch,s_crash,s_over,s_end};

//модели
enum {m_lift0,m_lift,m_door,m_button,m_switch,m_keyhole,m_hole,m_brick,
m_warp,m_shop,m_touch,m_flag,m_copy,m_item,m_mult,m_delete,m_timer, m_monster};

//типы уровней (разгон и инерция и т.п.)
enum {t_normal,t_ice,t_water,t_air,t_cave};

//--опис прогр
//void splitcopy0(screen, s16, s16, screen);
void putup(void);
s16 yesno(char *);
void put_curr(void); //вывод тьекущих объектов редактирования
void f_quit(void); //выход в дос
void f_disk(void); //работа с диском
void f_size(void); //изменить размеры уровня
void f_clear(void); //очистка уровня
void f_game(void); //проба игры
void f_gamedemo(void);
void f_poohxy(void);
void paramlev(void);
void readlev(void);
void savelev(void);
void makepass(s16);
s16 testpass(void);
void mm(u8); //обработчик прерываний от клавиатуры
void mz(u8); //обработчик для заставки
void md(u8); //обработчик для демонстр

void get_joy(void); //обработчик джойстика

void f_map(void); //вывод карты уровня

void but_up(void); //сдвиг при редактировании на 1 клетку экрана
void but_down(void);
void but_left(void);
void but_right(void);

void choose_fon(void); //выбор фона
void choose_trase(void);
void choose_items(void);
void choose_monstr(void);
void choose_model(void); //установка лифтов и др. механизмов
void switch_fon(void);
void switch_trase(void);
void switch_items(void);
void switch_monstr(void);
void switch_model(void);
void show_model(s16 n);
void edmodel(s16 n);
void domodel(void);
void actmodel(void);

void ris_lab(s16,s16); //вывод экрана для редакт
void rislab(u16,u16); //вывод экрана в игр
s16 check(s16,s16); //проверка на пустоту
s16 checkdw(s16,s16); //проверка на пустоту
void getitem(s16, s16); //взятие вещи по координатам

void addboom(s16 x, s16 y, s16 lx, s16 ly, signed char sx, signed char sy, signed char typ, block buf);
#define b_boom 0	//взрыв на месте
#define b_buh 1		//нет смены фаз
#define b_crash 2	//разлет кусков
#define b_fall 3	//опадение кусков

void addmess(char *m,unsigned char w/*=32*/);
void doboom(void); //обработка взрыврв
s16 addbullet(s16 x, s16 y, s16 sx, s16 sy, char typ, block buf);
void doshop(s16);
void dobullet(void);
void put_score(void);
void put_additems(void);
void open_exit(void);
void doman(void); //обработка героя
void readmonstr(void); //считать описание монстров
int makemonstr(s16 i, s16 n, u16 x,u16 y);
s16 addmonstr(s16 n, u16 x,u16 y);
void doobj(void); //обработка объектов

void do_mult(char * name, int allow_interrupt); //показ мульта
void readstrip(void);

void loading(void);

//-- savegame
void check_savegame(void);
void continue_savegame(void);
void save_savegame(void);
// - показать слот для выбора
int select_savegame(int t);


//------- структуры

struct levdef {
	char name[16];
	u16 lx,ly; //размер уровня
	u16 nx,ny;	//начальн коорд героя
	unsigned char fonR,fonG,fonB; //окраска заднего фона
	signed char end; //последний ли уровень?
	char bckgname[13],fonname[13]; //назв задн план, фон
	char mname0[26]/*,mname1[13]*/; //назв м-ф в нач и конце уровня
	signed char typ; //тип уровня
	char musicname[13]; //назв музыки на уровень
};

//враги
#define maxobj 128


struct object { //описание объекта
	//char name[8];
	s16 x,y; //координаты тек кв-та
	signed char sx,sy; //ускорение
	signed char dsx,dsy; //торможение по осям
	unsigned char lx,ly; //высота ширина
	signed char lfx,lfy; //коорд точек опоры
	signed char rtx,rty; //коорд точек опоры
	signed char upx,upy; //коорд точек опоры
	signed char dwx,dwy; //коорд точек опоры и центра
	signed char typ;	//тип объекта
	signed char n;	 //номер в массиве номеров монстров
	signed char s;	//состояние

	block buf; //ссылка на текущую фазу

	signed char f,f0,napr,myrg; //фаза скорость смены фазы и направление

	signed char on; //вкл/выкл 1..x, 0 жизненн сила
};

struct strkobj {
	s16 x,y; //координаты тек
	s16 n;	//номер типа монстра-объекта
	//signed char
};

struct hero {	//описание героя (глобальное)
	char name[8];
	s16 x,y; //координаты тек кв-та
	signed char sx,sy; //ускорение
	signed char dsx1,dsx2,dsy; //торможение по осям
	unsigned char lx,ly; //высота ширина
	signed char lfx,lfy1,lfy2; //коорд точек опоры
	signed char rtx,rty1,rty2; //коорд точек опоры
	signed char upx1,upx2,upy; //коорд точек опоры
	signed char dwx1,dwx2,dwy; //коорд точек опоры и центра
	signed char typ;	//тип объекта
	char namespr[12]; //название спрайта
	block gospr; //ссылка на начало массива с фазами
	block jmspr;
	block stspr;
	block specspr;
	signed char maxgo; //мах кол-во фаз движения фаза
	signed char maxjm;
	signed char maxst;
	signed char maxspec;
	signed char s;	//состояние

	block buf; //ссылка на текущую фазу

	signed char f,f0,fw,napr,myrg; //фаза скорость смены фазы и направление

	signed char on; //вкл/выкл 1..x, 0 жизненн сила
};

//типы монстров
#define maxmonstr 16

struct monstr { //описание монстров
	char name[9];
	unsigned char lx,ly; //высота, ширина
	signed char lfx,lfy;	//коорд точек опоры
	signed char rtx,rty;	//коорд точек опоры
	signed char upx,upy;	//коорд точек опоры
	signed char dwx,dwy;	//коорд точек опоры и центра
	signed char typ;	//тип поведения объекта
	signed char speed;
	signed char lives;
	char namespr[13]; //название файла спрайта
	block gospr; //ссылка на начало массива с фазами
	block jmspr;
	block stspr;
	block specspr;
	signed char maxgo; //мах кол-во фаз движения фаза
	signed char maxjm;
	signed char maxst;
	signed char maxspec;
	//только для монстра
	signed char _t_;
};

//макс кол-во лифтов и прочих моделей
#define maxmodel 128
struct strmodels {
	char name[8]; //свое имя
	char name0[8]; //что включить
	s16 x,y; //координаты тек кв-та
	signed char sx,sy; //ускорение
	unsigned char lx,ly; //высота ширина
	s16 x1,y1; //коорд начального положения
	s16 x2,y2; //коорд конечного положения
	signed char typ;	//тип модели
	block buf; //ссылка на текущую фазу	
	//s16 tmp1,tmp2; //ЗАТЫЧКА
	signed char on,on0; //вкл/выкл 1..x, 0 жизненн сила
	
};

//магазин
struct sshop {	//описание предмета из магазина
	s16 price; //цена
	char *name; //название англ буквами
	char *description; //описание
	unsigned char lx,ly; //высота ширина
	block *buf; //ссылка на фазу
	s16 sm;
	signed char mf;	//кол-во фаз
};

//макс кол-во взрывов
#define maxboom 32
struct boom { //координаты взрывов
	s16 x,y; //координаты тек кв-та
	unsigned char lx,ly; //высота ширина
	signed char sx,sy; //высота ширина
	signed char typ;	//тип объекта
	block buf; //ссылка на фазу
	signed char f; //фаза
};

//пули - шишки
#define maxbullet 20
struct strbullets {
	s16 x,y; //координаты тек кв-та
	signed char sx,sy; //ускорение
	signed char typ;	//тип объекта
	signed char f;		//фаза
	block buf; //ссылка на текущую фазу
};

//номера звуков
enum {sfx_cursor, sfx_OK, sfx_cancel, sfx_error, 
	sfx_jump, sfx_throw, sfx_fall, sfx_pain, sfx_death, 
	sfx_teleport, sfx_on, sfx_off, sfx_lock, sfx_checkpoint,
	sfx_cone, sfx_coin, sfx_key, sfx_honey, sfx_live, sfx_clock, sfx_item, sfx_power,
	sfx_magic,
	sfx_splat, sfx_crack, sfx_crash,
	sfx_alien1, sfx_alien2, sfx_alien3,
	sfx_kill
};

#endif