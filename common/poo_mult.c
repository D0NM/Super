/*
���������� ������� ������ �������

sprite <name> <lx> <ly> <ox> <oy> <pic name> <pic width> <pic height>
 - ������� ������
picture <pic name> <pic width> <pic height>
 - ������� � ��������� ��������
animate sprite <n> <sx> <sy> <delay>
 - ����������� ������ n ���, sx sy - ����� � ��������, Delay - ����� ����� ������ ��������
strip <name.pic> <name.scr> <number>
 - ��������� ����� � ������� ��� ����� � ����� number
background <number>
 - ������ ������ ��� - ������� ������ �����
background <pic name> <step x> <step y>
 - ������ ������ ��� - ��������������� ��������
background stripe <number>
 - ������ ������ ��� - ����������� ����� stripe + ��������
palette <name>
 - ��������� ������� (�� ��������.. �� ����������)
on
 - ������� ���
off
 - �������� �����
text <number>
 - �����.. �� ��������... ���
render
 - ���/���� ���������
fx <sprite> <flow|shake|stop|walk> <sx> <sy> <wait>
 - ������ ���� ������ ������� (� ������� � �������� ������������)
set <sprite> <x> <y>
 - ����������� ������ � �
move <sprite> <x> <y> <step>
 - ��������� ������ �� ����� � � � � ����� �������� step
wait <number renders>
 - ����� number ���������
while <sprite>
 - ����� ���� ������ ��������
swap <sprite> <sprite>
 - �������� ������� �������
hide <sprite>
 - �������� ������
show <sprite>
 - �������� ������
;
 - �����������
q
 - �����

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

extern screen hidscr; //������� ���������
extern s16 end;
extern GPDRAWSURFACE gpDraw[2];
extern int nflip;
extern block bckg;
extern block textindex[200]; //������ �� ������ ������ 0,1...100
extern struct BFont text_font;

void show_strip(s16 y);
void do_strip(s16 sx);
int fatoi(char *s);
int gettok(char **s, char *d); //�������� ��������� �����... ������ ��� �����. ���� ����� ������ - ����� -1
int getNtok(char **s);

char script_quit[]="q";	//����� �������
char *script_index[300]; //������ �� ������ �������
block mult_script;
int background_type;	//��� ���� 0 = �������, -1 = stripe, ��������� 1..255 = ����
block background_image;	//������ ������� ���� 320�200 !!
int background_sx = 0, background_sy = 0; //�������� �������� �������� �� � � �
int background_x = 0;
int background_y = 0;

int background_stripe;	//�������� ������� �������

int text_colour;	//���� ���������� ������ 0 - ��� ������
int text_n;		//������� ����� ��������� -1 - ����

int ready_to_render;	//����������� �� ��� ��� ����... ����� �� ���������� �����

int pictures_n = 0; //���-�� �������� �������
struct ss_pictures { //�������� ��������
	char name[14];
	int lx,ly; //������ ��������
	block buf; //������ �� ������� ����
};
struct ss_pictures s_pictures[20];

int sprites_n = 0;	//���-�� �������� �������
struct ss_sprites { //�������� ��������
	char name[14];
	int ox,oy;	//�������� ������� ������ ��� ��������
	int lx,ly;	//������ �������
	int x,y;	//�����-� ������� �� ������
	int x1,y1;	//�������� ��� �������
	int x2,y2;	//���� ���� ������� �������� �������
	int xt,yt;	//x,y target - ���� ���������� x,y
	int typ;	//��� ������� (0 - ������, 1 - �����, 2 - ???)
	int m;		//��� ��������
	int glue;	//������ �� ������ � �������� ��������
	int gx,gy;	//�������� ������������ �������
//-- ��� ��������� ����������� 
	int dl_x,dl_y;
	int mdl_x,mdl_y;
	int ms;	//��������
	
//-- ��� ��������
	int sx,sy;	//�������� ��� ���� ���� �������
	int n;		//���-�� ��� ������� 0 - ����
	int f;		//����� ��� ����
	int wa;	//��� �������� ����� ���� /���������
	int da;	//���� �������� � ������ �� ����� ����

//-- ��� ���� �������� � �������� ��������
	int wait;	//��� �������� ����� ���� /���������
	int delay;	//���� �������� � ������ �� ����� ����
	int dx,dy;	//������� �������� - �������
	int fx;		//��� FX �������
	struct ss_pictures *pic; //������ �� ��������

	int on;		//���������� ������ ��� ��� (-1 - ������ ����) 2- ����������
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

	//������ �������
	ts = SizeLib(script_name);
	mult_script = (block)famemalloc(ts+2);

	//�������� �������
	GetLib(script_name,(block)mult_script);
	//����������� ������
	p = mult_script;

	for (i=0; i<300; i++)
	{	//�������������� ��� ������ ���������
		script_index[i] = (block)script_quit;
	}
	i=0;
	while ( i<300 && p < (mult_script + ts) )	//���-�� ����� ���� � ����� ������
	{
		//������� ��������� ������
		script_index[i++] = p++;
		//���� ����� �����
		if (*p == 0) {
			break;
		}
		while ( *p !='\n' && *p != '\r') {
			//���������� ���� ������
			p++;
		}
		*p++ = 0;

		//__printf("script_data: str %d ='%s'\n",i-1,script_index[i-1]);

		//���� �� ��� ��
		while ( (*p == '\n' || *p == '\r') && *p ) {
			p++;
		}	
	}
}


block	p;	//��� ������� � ������
char s_command[50];
char s_name[50];
char str1[50];
char str2[50];


GPDRAWTAG m_clip= {0,0,20,320,200};
//	GpRectFill(NULL,&gpDraw[nflip], 0, 0, 320, 18, 148);
//	GpRectFill(NULL,&gpDraw[nflip], 0, 18, 320, 2, 152);
//	GpRectFill(NULL,&gpDraw[nflip], 0, 220, 320, 2, 152);
//	GpRectFill(NULL,&gpDraw[nflip], 0, 222, 320, 18, 148);

//���������� �������� ������ � �����
//1� ��� - ��� ������� � ��� �����, 2� - 1= ����� ��������� �� ������
void do_mult(char * name, int allow_interrupt) {
	int i,n,t;

	//�������� ����
	//!stop_sound();
	modstop();

	//loading();

	init_script();

	load_script(name);

	n=-1; end=0;
	do {
		render_script();
		n++;
		//���� ����� �������� �� ������..� � ������ ����-� ������
		if (allow_interrupt) md(1);
#ifndef RELEASE
		__printf("script_data: N%d ='%s'\n",n,script_index[n]);
#endif
		//���������� ������� ������ �������
		p = script_index[n];
		gettok(&p,(char*)&s_command);

		//������������� �����������
		if (s_command[0] == ';') {
			continue;
		}
		// ����� ������� "q" - �����
		if (gp_str_func.compare(s_command,"q")==0) {
			break;
		}
		//�������� ������� � ���� ������ (+�������� ��������)
		if (gp_str_func.compare(s_command,"sprite")==0) {
			gettok(&p,(char*)&str1);
			make_sprite(str1);
			continue;
		} else
		//�������� ������� � ���� ������ (+�������� ��������)
		if (gp_str_func.compare(s_command,"picture")==0) {
			gettok(&p,(char*)&str1);
			load_picture(str1);
			continue;
		} else
		//����������� ����������� � ���������� X Y �������
		if (gp_str_func.compare(s_command,"set")==0) {
			gettok(&p,(char*)&str1);
			set_sprite(str1,0);
		} else
		//������� ����������� � ���������� X Y �������
		if (gp_str_func.compare(s_command,"move")==0) {
			gettok(&p,(char*)&str1);
			set_sprite(str1,1);
		} else
		//�������� ������
		if (gp_str_func.compare(s_command,"show")==0) {
			gettok(&p,(char*)&str1);
			show_sprite(str1,1);
			continue;
		} else
		//c������� ������
		if (gp_str_func.compare(s_command,"hide")==0) {
			gettok(&p,(char*)&str1);
			show_sprite(str1,0);
			continue;
		} else
		if (gp_str_func.compare(s_command,"showhide")==0) {
			//�������� ������
			gettok(&p,(char*)&str1);
			show_sprite(str1,1);
			//c������� ������
			gettok(&p,(char*)&str1);
			show_sprite(str1,0);
			continue;
		} else
		//������ ������� FX
		if (gp_str_func.compare(s_command,"fx")==0) {
			gettok(&p,(char*)&str1);
			set_fx(str1);
			continue;
		} else
		//���������� ����� ������, ������� �� ����� �������
		if (gp_str_func.compare(s_command,"swap")==0) {
			gettok(&p,(char*)&str1);
			gettok(&p,(char*)&str2);
			swap_sprite(str1,str2);
			continue;
		} else
		//��������� 2� ������ � 1��
		if (gp_str_func.compare(s_command,"glue")==0) {
			gettok(&p,(char*)&str1);
			gettok(&p,(char*)&str2);
			glue_sprite(str1,str2,1);
			continue;
		} else
		//�������� ������
		if (gp_str_func.compare(s_command,"unglue")==0) {
			gettok(&p,(char*)&str1);
			glue_sprite(str1,str2,0);
			continue;
		} else
		//�������� �������
		if (gp_str_func.compare(s_command,"animate")==0) {
			gettok(&p,(char*)&str1);
			animate_sprite(str1);
			continue;
		} else
		//��������� ���������� ��������� �������� �������
		if (gp_str_func.compare(s_command,"allow")==0) {
			allow_interrupt = 1;
			continue;
		} else
		//��������� ������� �����
		if (gp_str_func.compare(s_command,"palette")==0) {
			gettok(&p,(char*)&str1);	//���� ������� ������ ���������
			GetLib(str1,palette);
			gettok(&p,(char*)&str2);
			if (gp_str_func.compare(str2,"on")==0) {
				PutPalette(palette);
			}
			continue;
		} else
		//��������� �����
		if (gp_str_func.compare(s_command,"on")==0) {
			PaletteOn(palette);
			continue;
		} else
		//�������� �����
		if (gp_str_func.compare(s_command,"off")==0) {
			PaletteOff(palette);
			continue;
		} else
		//�������� ������ (�� ���� ������ �� ����)
		if (gp_str_func.compare(s_command,"text")==0) {
			//���� ���������� ������ 0 - ��� ������
			text_colour = getNtok(&p);
			//������� ����� ��������� 0 - ����
			text_n = getNtok(&p);
#ifndef RELEASE
			__printf("text -> col %d Nt %d\n",text_colour,text_n);
#endif
			continue;
		} else
		//�������� ���������� - Stripe
		if (gp_str_func.compare(s_command,"strip")==0) {
			gettok(&p,(char*)&str1);
			GetLib(str1,(block)bckg);
			TurnIt((block)bckg, 320, 200);
			gettok(&p,(char*)&str2);
			GetLib(str2,(block)hidscr);
			readstrip();
			background_type = -1;
			//�������� ���������� ���� ������
			background_stripe = getNtok(&p);
			continue;
		} else
		//���������-��������� �������� ���������� ���������� - Stripe
		if (gp_str_func.compare(s_command,"strip_speed")==0) {
			//�������� ����������
			background_stripe = getNtok(&p);
			continue;
		} else
		//���������-��������� ������� ����
		if (gp_str_func.compare(s_command,"background")==0) {
			gettok(&p,(char*)&str1);
			//��� ��������?
			t = 1; //���� - ��� ��������
			for (i = 0; i< pictures_n; i++)
			{
				if ( gp_str_func.compare(s_pictures[i].name,str1)==0) {
					background_type = 0; //��������
					//�������� ���� ������ �������� - �� �������� ��������
					if (background_image != s_pictures[i].buf)
					{
						background_x = background_y = 0;
						background_image = s_pictures[i].buf;
					}
					background_sx = getNtok(&p);
					background_sy = getNtok(&p);
					//���� reset �� ������� ��������
					gettok(&p,(char*)&str1);
					if ( gp_str_func.compare("reset",str1)==0) {
						background_x = background_y = 0;
					}
					t=0;	//����� ��������
					break;
				}
			}
			if (t) {
				//����
				background_type = fatoi(str1);
				if (gp_str_func.compare(s_command,"strip")==0) {
					background_type = -1; //�����
					background_stripe = getNtok(&p);
				}
			}
			continue;
		} else
		//�������� ��� ������� � ����������� ��
		if (gp_str_func.compare(s_command,"clear")==0) {
			for (i = 0; i< sprites_n; i++)
				s_sprites[i].on = -1;
			//������ ��������
			text_colour = text_n = 0;
			continue;
		} else
		//�����
		if (gp_str_func.compare(s_command,"wait")==0) {
			//���� � ������
			t = getNtok(&p);
			for (i=0; i< t; i++)
			{
				render_script();
				//���� ����� �������� �� ������..� � ������ ����-� ������
				if (allow_interrupt) md(1);
				Delay(10);
				if (end) break;	//����� �� ������� �����
			}
			continue;
		} else
		//����� ���� ������ �� ������� ���� ����
		if (gp_str_func.compare(s_command,"while")==0) {
			//���� � ������
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
				//���� ����� �������� �� ������..� � ������ ����-� ������
				if (allow_interrupt) md(1);
				Delay(10);
				if (end) break;	//����� �� ������� �����
			}
			continue;
		} else
		if (gp_str_func.compare(s_command,"transp")==0) {
			//������� ����������
			gettok(&p,(char*)&str1);
			for (i = 0; i< sprites_n; i++)
			{
				if ( gp_str_func.compare(s_sprites[i].name,str1)==0) {
					break;
				}
			}
			s_sprites[i].on = getNtok(&p); //���� ����� ������ �� 8� ��� ������������
			continue;
		} else
		//������ �������� ������
		if (gp_str_func.compare(s_command,"se")==0) {
			SE(getNtok(&p));
			continue;
		} else
		//������ MOD ������
		if (gp_str_func.compare(s_command,"mod")==0) {
			gettok(&p,(char*)&str1);	//���� ������� ������ ���������
			if (gp_str_func.compare(str1,"stop")==0) {
				modstop();
			} else {
				//!stop_sound();
				modsetup(str1, 4, 0 ,0, 0, 0 );
				//!start_sound();
			}
			continue;
		} else
		//����� �� ���������
		if (gp_str_func.compare(s_command,"render")==0) {
			//����������� ��� - ����
			ready_to_render = ready_to_render?0:1;
			//!start_sound();
		}
	
	} while ( n<300 && end==0 );

	//������� ��������
	text_colour = text_n = 0;

	render_script();
	render_script();	//2� ��� ��� ������� ���

	//������� ������ � �.�.
	free_script();
	//�������� ���� ���� ����������
	while ( is_sfx_playing() );
	modstop();
	//stop_sound();
	//start_sound();
}

#define spr_y_off 20
//��������� ����������� ���������
void render_script(void) {
	int i;

	if ( ready_to_render == 0 ) return; //��������� ������
#ifndef RELEASE
	__printf("R.");
#endif
	//������ ���
	if (background_type < 0)
	{	//���������
		do_strip(background_stripe);
		show_strip(0);
	} else
	if (background_type == 0 && background_image != NULL )
	{	//������� ��������
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
	{	//��� - ����� ������
		//Cls(background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20, 320, 50, background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20+50, 320, 50, background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20+50+50, 320, 50, background_type);
		GpRectFill(&m_clip,&gpDraw[nflip], 0, 20+50+50+50, 320, 50, background_type);
	}

	//������ ������� ��� ����
	for (i = 0; i< sprites_n; i++)
	{
		if (s_sprites[i].on > 0 ) //���������� ���������� ���-��
		if ( s_sprites[i].on < 8 ) {
			//���� ������ �� ����������
			GpTransBlt(&m_clip,&gpDraw[nflip],
			s_sprites[i].x+s_sprites[i].x1,s_sprites[i].y+s_sprites[i].y1+spr_y_off,s_sprites[i].lx,s_sprites[i].ly,(unsigned char*)s_sprites[i].pic->buf,
			s_sprites[i].ox+s_sprites[i].sx*s_sprites[i].f,s_sprites[i].oy+s_sprites[i].sy*s_sprites[i].f,
			s_sprites[i].pic->lx,s_sprites[i].pic->ly,0);
		} else {
			//���� ����������
			BltSatur(s_sprites[i].x+s_sprites[i].x1,s_sprites[i].y+s_sprites[i].y1+spr_y_off,s_sprites[i].lx,s_sprites[i].ly,(block)s_sprites[i].pic->buf,
			s_sprites[i].ox+s_sprites[i].sx*s_sprites[i].f,s_sprites[i].oy+s_sprites[i].sy*s_sprites[i].f,
			s_sprites[i].pic->lx,s_sprites[i].pic->ly,s_sprites[i].on);
		}
	}
	
	//������� ������� ��� ����
	for (i = 0; i< sprites_n; i++)
	{
		if ( s_sprites[i].on > 0 ) {
			do_sprite(i);
		}
	}

	//������� ����-����� � �������
	GpRectFill(NULL,&gpDraw[nflip], 0, 0, 320, 18, 148);
	GpRectFill(NULL,&gpDraw[nflip], 0, 18, 320, 2, 152);
	GpRectFill(NULL,&gpDraw[nflip], 0, 220, 320, 2, 152);
	GpRectFill(NULL,&gpDraw[nflip], 0, 222, 320, 18, 148);

	//���� ���� �������� - ������� ��
	if ( text_colour > 0 && text_n > 0 ) {
		i=( 320-BTextWidthGet(&text_font,textindex[text_n]) )/2;
		text_font.color[0] = 8;
		BTextOut(NULL, &gpDraw[nflip], &text_font, i+1, 220+1, textindex[text_n]);
		//������� ����� ��������� -1 - ����
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
	background_stripe = 1;	//�������� ������� �� ���������
	ready_to_render = 0;	//��������� ������
	//Clrs(8,nflip);	//����
	TileBar(0,0,320,200,64,64,(block)tiles+random(4)*64*64);

	//�������� �������� �������� �������� �� � � �
	background_sx = background_sy = background_x = background_y = 0;
	background_image = NULL;
	background_type = 15; //����� ����

	//������� �������� ����������
	/*for (i=0; i<20; i++)
	{
		s_pictures[i].name[0] = 0;
	}*/
	gp_str_func.memset(s_pictures,0,sizeof(s_pictures));
	pictures_n = 0;
	//������� ��������
	gp_str_func.memset(s_sprites,0,sizeof(s_sprites));
	for (i=0; i<50; i++)
	{
		s_sprites[i].name[0] = 0;
		s_sprites[i].on = -1;
	}
	sprites_n=0;

	//���� ���������� ������ 0 - ��� ������
	text_colour = text_n = 0;
	
}

void free_script(void)
{
	int i;
	//������� �������� ����������
	for (i= pictures_n-1; i>=0; i--)
	{
		gp_mem_func.free(s_pictures[i].buf);
		s_pictures[i].name[0] = 0;
	}
	//������� ��������
	for (i= sprites_n-1; i>=0; i--)
	{
		s_sprites[i].name[0] = 0;
		s_sprites[i].on = -1;
	}

	//��������� ������
	gp_mem_func.free(mult_script);

}

//��������� ��������
block load_picture(char *name)
{
	int i;
	//���� ������� ���� ��� ���� ����� �������� � ������, ����� ������ �� �������
	for (i = 0; i< pictures_n; i++)
	{
		if ( gp_str_func.compare(s_pictures[i].name,name)==0) {
			return (block)&s_pictures[i];
		}
	}
	//����� �������� ��� - ��������
	for (i = 0; i<20; i++)
	{
		if ( s_pictures[i].name[0]==0 ) {
			//������� ������� ��������
			s_pictures[i].lx = getNtok(&p);
			s_pictures[i].ly = getNtok(&p);
			gp_str_func.strcpy(s_pictures[i].name,name);
			//������
			s_pictures[i].buf = (block)famemalloc(SizeLib(name));
			//�������� ��������
			GetLib(name,(block)s_pictures[i].buf);
			//������� ��������
			TurnIt(s_pictures[i].buf,s_pictures[i].lx,s_pictures[i].ly);
			//�������� ����-���-�� ����������� ��������
			pictures_n++;
#ifndef RELEASE
			__printf("picture %s [%d x %d]\n",s_pictures[i].name,s_pictures[i].lx,s_pictures[i].ly);
#endif
			return (block)&s_pictures[i];
		}
	}
	//������-�� ��� ������ ���� ����� ��..
	return (block)&s_pictures[0];
}

//������� ������
//������: sprite <sprname> <lx> <ly> <ox> <oy> <pic name> <lx> <ly>
void make_sprite(char *name)
{
	int i;
	//���� ������� ���� ��� ���� ����� � ������, ����� ������ �� �������
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			goto change_existing_sprite;
		}
	}
	//������ ������� ��� - ��������
	for (i = 0; i<50; i++)
	{
		if ( s_sprites[i].on < 0 ) {
			s_sprites[i].x = s_sprites[i].y = s_sprites[i].xt = s_sprites[i].yt = s_sprites[i].on = s_sprites[i].m = 0;
			sprites_n++;
			break;
		}
	}
	//���� ����� �������� �� �������������� ��������� ��...

change_existing_sprite:
	//������� ������� ��������
	s_sprites[i].lx = getNtok(&p);
	s_sprites[i].ly = getNtok(&p);
	s_sprites[i].ox = getNtok(&p);
	s_sprites[i].oy = getNtok(&p);
	gp_str_func.strcpy(s_sprites[i].name,name);
#ifndef RELEASE
	__printf("sprite %s [%d x %d]\n",s_sprites[i].name,s_sprites[i].lx,s_sprites[i].ly);
#endif
	//��������� ��������� ��������
	s_sprites[i].fx = s_sprites[i].wait = s_sprites[i].delay =
	s_sprites[i].n = s_sprites[i].f = s_sprites[i].m =
	s_sprites[i].wa = s_sprites[i].da = 0;

	//������ �������� ������� ���� ����
	gettok(&p,(char*)&str1);
	s_sprites[i].pic = (struct ss_pictures *)load_picture(str1);

	return;
}

//���������� ���������� ������
//������: set <sprname> <x> <y>
void set_sprite(char *name, int mode)
{
	int i,dx,dy;
	//���� ������� ���� ��� ���� ����� � ������, ����� ������ �� �������
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//������ ������� ��� - ������
	if (mode)
	{	//���� MOVE
		s_sprites[i].xt = getNtok(&p);
		s_sprites[i].yt = getNtok(&p);
		s_sprites[i].ms = getNtok(&p);	//���! ��������

		s_sprites[i].m = 1; //�������� ��������
		//���������� ����������, ����������� ��� ��-� �� �����

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
	{	//���� SET
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

//���������� ��� ������ ������
//������: show/hide <sprname>
void show_sprite(char *name, int on)
{
	int i;
	//���� ������� ���� ��� ���� ����� � ������, ����� ������ �� �������
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//������ ������� ��� - ������
#ifndef RELEASE
	__printf("show N %d = %d\n",i,on);
#endif
	s_sprites[i].on = on;

	return;
}

//��������� ������ 2� � 1�� ��� ��������
void glue_sprite(char *name, char *name2, int mode)
{
	int i,j;
	//���� ������� 1�
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	if ( mode == 0)
	{
		//��������
		s_sprites[i].m = 0;
#ifndef RELEASE
		__printf("unglue N %d",i);
#endif
		return;
	}

	//���� 2�
	for (j = 0; j< sprites_n; j++)
	{
		if ( gp_str_func.compare(s_sprites[j].name,name2)==0) {
			break;
		}
	}
	//������ ������� ��� - ������

	s_sprites[j].glue = i;
	s_sprites[j].gx = s_sprites[i].x - s_sprites[j].x;
	s_sprites[j].gy = s_sprites[i].y - s_sprites[j].y;

	s_sprites[j].m = 2;	//��� ����������� glue
#ifndef RELEASE
	__printf("glue N %d <- N %d\n",i,j);
#endif
	return;
}

//������: animate <sprname> <n> <sx> <sy> <delay>
void animate_sprite(char *name)
{
	int i;
	//���� ������� ���� ��� ���� ����� � ������, ����� ������ �� �������
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//������ ������� ��� - ������

	s_sprites[i].n = getNtok(&p)-1;
	s_sprites[i].wa = s_sprites[i].f = 0;
	s_sprites[i].sx = getNtok(&p);//���! ��������
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
	//���� ������� 1�
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//���� 2�
	for (j = 0; j< sprites_n; j++)
	{
		if ( gp_str_func.compare(s_sprites[j].name,name2)==0) {
			break;
		}
	}
	//������ ������� ��� - ������
#ifndef RELEASE
	__printf("swap N %d <- N %d\n",i,j);
#endif
	swap(&s_sprites[i].x,&s_sprites[j].x);
	swap(&s_sprites[i].y,&s_sprites[j].y);
	swap(&s_sprites[i].xt,&s_sprites[j].xt);
	swap(&s_sprites[i].yt,&s_sprites[j].yt);
	//������������ ���������� ���� ������� ������
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

	//���������� ����-�
	swap(&s_sprites[i].ms,&s_sprites[j].ms);
	swap(&s_sprites[i].dl_x,&s_sprites[j].dl_x);
	swap(&s_sprites[i].dl_y,&s_sprites[j].dl_y);
	swap(&s_sprites[i].mdl_x,&s_sprites[j].mdl_x);
	swap(&s_sprites[i].mdl_y,&s_sprites[j].mdl_y);

	//glue
	swap(&s_sprites[i].glue,&s_sprites[j].glue);
	swap(&s_sprites[i].gx,&s_sprites[j].gx);
	swap(&s_sprites[i].gy,&s_sprites[j].gy);
	//���������� ������ �� ������������ GLUE object � ��������
	for (k = 0; k< sprites_n; k++)
	{
		if ( s_sprites[k].glue == i ) {
			s_sprites[k].glue = j;
		}
	}
	return;
}

char n_fx[10][6]={"stop","shake","flow","walk","na","na","na","na","na10"};

//��������� ������ ��� ������ ������� ������������ ��� ���������
//������: fx <sprname> <fx name> <sx> <sy> <delay>
void set_fx(char *name)
{
	int i,j;
	//���� �������
	for (i = 0; i< sprites_n; i++)
	{
		if ( gp_str_func.compare(s_sprites[i].name,name)==0) {
			break;
		}
	}
	//������ ������� ��� - ������

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
	if (j)	//���� �� fx "stop"
	{
		s_sprites[i].dx = getNtok(&p);
		s_sprites[i].dy = getNtok(&p);
	} else {
		return;
	}
	//� ������������ ��� ���������� ������� ����� ���� ��������!
	s_sprites[i].delay = getNtok(&p);
	return;
}

#define sign(n) n>0?1:-1

//Fx ��� ������� (�������� � �.�.)
void do_sprite(int i)
{
	int j;
	//��������� �������� ��������
	if ( --s_sprites[i].wa < 0 )
	{
		s_sprites[i].wa = s_sprites[i].da;
		//��������� ���� �������
		if ( --s_sprites[i].f < 0 )
		{
			s_sprites[i].f = s_sprites[i].n;
		}
	}
	//�� ���� FX ������ �� ��� ����
	switch (s_sprites[i].fx)
	{
		case 1: //shake
			//��������� �������� ��� FX � ����� ��������
			if ( --s_sprites[i].wait < 0 )
			{
				s_sprites[i].wait = s_sprites[i].delay;

				s_sprites[i].x2 = s_sprites[i].x1 = random(s_sprites[i].dx*2) - s_sprites[i].dx;
				s_sprites[i].y2 = s_sprites[i].y1 = random(s_sprites[i].dy*2) - s_sprites[i].dy;
			}
			break;
		case 2: //flow   x:
			//��������� �������� ��� FX � ����� ��������
			if ( --s_sprites[i].wait < 0 )
			{
				s_sprites[i].wait = s_sprites[i].delay;

				if ( s_sprites[i].x1 == s_sprites[i].x2 && s_sprites[i].y1 == s_sprites[i].y2 )
				{
					s_sprites[i].x2 = random(s_sprites[i].dx*2) - s_sprites[i].dx;
					s_sprites[i].y2 = random(s_sprites[i].dy*2) - s_sprites[i].dy;
				}
			}
			//������ ������ � ����
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
			//��������� �������� ��� ����� ��������
			if ( --s_sprites[i].wait < 0 )
			{
				s_sprites[i].wait = s_sprites[i].delay;

				s_sprites[i].x2 = -sign(s_sprites[i].x2)*s_sprites[i].dx;
				s_sprites[i].y2 = -sign(s_sprites[i].y2)*s_sprites[i].dy;
			}
			//������ ������ � ����
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

	//���������� ����-� �������
	switch (s_sprites[i].m)
	{
		case 0: //stop
			break;
		case 2: //glue
			s_sprites[i].x = s_sprites[s_sprites[i].glue].x+s_sprites[s_sprites[i].glue].x1 - s_sprites[i].gx;
			s_sprites[i].y = s_sprites[s_sprites[i].glue].y+s_sprites[s_sprites[i].glue].y1 - s_sprites[i].gy;
			break;
		default:
			//���� ���� - ��� ���� ��� �������
			for (j = 0; j< s_sprites[i].ms; j++) {
				//����� �� �
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
				//����� �� �
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
			//���� �������� ���� - ����. ����-�
			if ( s_sprites[i].x == s_sprites[i].xt && s_sprites[i].y == s_sprites[i].yt ) {
				s_sprites[i].m = 0;
			}
			break;
	}
}