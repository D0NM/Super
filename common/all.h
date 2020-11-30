#ifndef H_ALL
#define H_ALL

#include "gp32.h"

//#define RELEASE 1
//#define NOEMU 1
//#define DEMO 1
#define FASTDRAW 1

#define wclock 50

#define wmyrg 18*1 //��� �� �������� ������

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

//��������� ����
#define xsp 296
#define ysp 3

#define maxsiz_level 100*500

#define maxdemo 1000

//������
enum {s_go,s_down,s_jump,s_fly,s_lift,s_ouch,s_crash,s_over,s_end};

//������
enum {m_lift0,m_lift,m_door,m_button,m_switch,m_keyhole,m_hole,m_brick,
m_warp,m_shop,m_touch,m_flag,m_copy,m_item,m_mult,m_delete,m_timer, m_monster};

//���� ������� (������ � ������� � �.�.)
enum {t_normal,t_ice,t_water,t_air,t_cave};

//--���� �����
//void splitcopy0(screen, s16, s16, screen);
void putup(void);
s16 yesno(char *);
void put_curr(void); //����� �������� �������� ��������������
void f_quit(void); //����� � ���
void f_disk(void); //������ � ������
void f_size(void); //�������� ������� ������
void f_clear(void); //������� ������
void f_game(void); //����� ����
void f_gamedemo(void);
void f_poohxy(void);
void paramlev(void);
void readlev(void);
void savelev(void);
void makepass(s16);
s16 testpass(void);
void mm(u8); //���������� ���������� �� ����������
void mz(u8); //���������� ��� ��������
void md(u8); //���������� ��� ��������

void get_joy(void); //���������� ���������

void f_map(void); //����� ����� ������

void but_up(void); //����� ��� �������������� �� 1 ������ ������
void but_down(void);
void but_left(void);
void but_right(void);

void choose_fon(void); //����� ����
void choose_trase(void);
void choose_items(void);
void choose_monstr(void);
void choose_model(void); //��������� ������ � ��. ����������
void switch_fon(void);
void switch_trase(void);
void switch_items(void);
void switch_monstr(void);
void switch_model(void);
void show_model(s16 n);
void edmodel(s16 n);
void domodel(void);
void actmodel(void);

void ris_lab(s16,s16); //����� ������ ��� ������
void rislab(u16,u16); //����� ������ � ���
s16 check(s16,s16); //�������� �� �������
s16 checkdw(s16,s16); //�������� �� �������
void getitem(s16, s16); //������ ���� �� �����������

void addboom(s16 x, s16 y, s16 lx, s16 ly, signed char sx, signed char sy, signed char typ, block buf);
#define b_boom 0	//����� �� �����
#define b_buh 1		//��� ����� ���
#define b_crash 2	//������ ������
#define b_fall 3	//�������� ������

void addmess(char *m,unsigned char w/*=32*/);
void doboom(void); //��������� �������
s16 addbullet(s16 x, s16 y, s16 sx, s16 sy, char typ, block buf);
void doshop(s16);
void dobullet(void);
void put_score(void);
void put_additems(void);
void open_exit(void);
void doman(void); //��������� �����
void readmonstr(void); //������� �������� ��������
int makemonstr(s16 i, s16 n, u16 x,u16 y);
s16 addmonstr(s16 n, u16 x,u16 y);
void doobj(void); //��������� ��������

void do_mult(char * name, int allow_interrupt); //����� ������
void readstrip(void);

void loading(void);

//-- savegame
void check_savegame(void);
void continue_savegame(void);
void save_savegame(void);
// - �������� ���� ��� ������
int select_savegame(int t);


//------- ���������

struct levdef {
	char name[16];
	u16 lx,ly; //������ ������
	u16 nx,ny;	//������� ����� �����
	unsigned char fonR,fonG,fonB; //������� ������� ����
	signed char end; //��������� �� �������?
	char bckgname[13],fonname[13]; //���� ���� ����, ���
	char mname0[26]/*,mname1[13]*/; //���� �-� � ��� � ����� ������
	signed char typ; //��� ������
	char musicname[13]; //���� ������ �� �������
};

//�����
#define maxobj 128


struct object { //�������� �������
	//char name[8];
	s16 x,y; //���������� ��� ��-��
	signed char sx,sy; //���������
	signed char dsx,dsy; //���������� �� ����
	unsigned char lx,ly; //������ ������
	signed char lfx,lfy; //����� ����� �����
	signed char rtx,rty; //����� ����� �����
	signed char upx,upy; //����� ����� �����
	signed char dwx,dwy; //����� ����� ����� � ������
	signed char typ;	//��� �������
	signed char n;	 //����� � ������� ������� ��������
	signed char s;	//���������

	block buf; //������ �� ������� ����

	signed char f,f0,napr,myrg; //���� �������� ����� ���� � �����������

	signed char on; //���/���� 1..x, 0 ������� ����
};

struct strkobj {
	s16 x,y; //���������� ���
	s16 n;	//����� ���� �������-�������
	//signed char
};

struct hero {	//�������� ����� (����������)
	char name[8];
	s16 x,y; //���������� ��� ��-��
	signed char sx,sy; //���������
	signed char dsx1,dsx2,dsy; //���������� �� ����
	unsigned char lx,ly; //������ ������
	signed char lfx,lfy1,lfy2; //����� ����� �����
	signed char rtx,rty1,rty2; //����� ����� �����
	signed char upx1,upx2,upy; //����� ����� �����
	signed char dwx1,dwx2,dwy; //����� ����� ����� � ������
	signed char typ;	//��� �������
	char namespr[12]; //�������� �������
	block gospr; //������ �� ������ ������� � ������
	block jmspr;
	block stspr;
	block specspr;
	signed char maxgo; //��� ���-�� ��� �������� ����
	signed char maxjm;
	signed char maxst;
	signed char maxspec;
	signed char s;	//���������

	block buf; //������ �� ������� ����

	signed char f,f0,fw,napr,myrg; //���� �������� ����� ���� � �����������

	signed char on; //���/���� 1..x, 0 ������� ����
};

//���� ��������
#define maxmonstr 16

struct monstr { //�������� ��������
	char name[9];
	unsigned char lx,ly; //������, ������
	signed char lfx,lfy;	//����� ����� �����
	signed char rtx,rty;	//����� ����� �����
	signed char upx,upy;	//����� ����� �����
	signed char dwx,dwy;	//����� ����� ����� � ������
	signed char typ;	//��� ��������� �������
	signed char speed;
	signed char lives;
	char namespr[13]; //�������� ����� �������
	block gospr; //������ �� ������ ������� � ������
	block jmspr;
	block stspr;
	block specspr;
	signed char maxgo; //��� ���-�� ��� �������� ����
	signed char maxjm;
	signed char maxst;
	signed char maxspec;
	//������ ��� �������
	signed char _t_;
};

//���� ���-�� ������ � ������ �������
#define maxmodel 128
struct strmodels {
	char name[8]; //���� ���
	char name0[8]; //��� ��������
	s16 x,y; //���������� ��� ��-��
	signed char sx,sy; //���������
	unsigned char lx,ly; //������ ������
	s16 x1,y1; //����� ���������� ���������
	s16 x2,y2; //����� ��������� ���������
	signed char typ;	//��� ������
	block buf; //������ �� ������� ����	
	//s16 tmp1,tmp2; //�������
	signed char on,on0; //���/���� 1..x, 0 ������� ����
	
};

//�������
struct sshop {	//�������� �������� �� ��������
	s16 price; //����
	char *name; //�������� ���� �������
	char *description; //��������
	unsigned char lx,ly; //������ ������
	block *buf; //������ �� ����
	s16 sm;
	signed char mf;	//���-�� ���
};

//���� ���-�� �������
#define maxboom 32
struct boom { //���������� �������
	s16 x,y; //���������� ��� ��-��
	unsigned char lx,ly; //������ ������
	signed char sx,sy; //������ ������
	signed char typ;	//��� �������
	block buf; //������ �� ����
	signed char f; //����
};

//���� - �����
#define maxbullet 20
struct strbullets {
	s16 x,y; //���������� ��� ��-��
	signed char sx,sy; //���������
	signed char typ;	//��� �������
	signed char f;		//����
	block buf; //������ �� ������� ����
};

//������ ������
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