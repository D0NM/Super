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
extern block menus;	//�������... �����������.
extern block logomenu;	//��� ��������-�������

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

struct st_svgame savegame[20];	//20 ������ ����. ��� ����� ������� ����...

u16 savegames;	//���� �� ����������� ����

void check_savegame(void);
void save_savegame(void);

// - �������� ���� ��� ������
int select_savegame(int t);
// ����� ������ ����� ��� ����
int show_highscore(void);


//-- ��� ����������� ����������� 
void init_savegame(void);
void encode_savegame(void);
void decode_savegame(void);
//- �� famelib
//void _fmeminv(block s, u32 n);

void check_savegame(void)
{
// ��������� ����������� ���� ��� ��������������
	ERR_CODE err_code;
	F_HANDLE h_rfile;
	unsigned long n_dummy;

	//������ language.ini ����

	err_code = GpFileOpen(savegame_ini_file, OPEN_R, &h_rfile);
	if (err_code != SM_OK)
	{
		//���� ��� INI ����� ��� ��� ������ �� 
		init_savegame();
		save_savegame();
		return;
	}

	//ini ���� ������, ������� ����
	err_code = GpFileRead(h_rfile, savegame, sizeof(savegame), (ulong*)&n_dummy);
	if (err_code != SM_OK)
	{
		//���� ������ ���� ��
		init_savegame();
		save_savegame();
		return;
	}

	_fmeminv((block)savegame,sizeof(savegame));

	//������� INI ���� 
	GpFileClose(h_rfile);
}

void init_savegame(void)
{
//�������������� ������ savegame �������.
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
// �������� ������ ����������� ������... ����������� �� ���
// arg t - 0=load, 1= continue
// A, START = accept, B = cancel
	int i,j,l,keydata,op=0;

	//������ �� ��������� �������
	GpKeyGetEx(&keydata);
	while (keydata)
	{
		GpKeyGetEx(&keydata);
		Delay(50);
	}


	//���� ������
	l = i = 0;

	keydata = 0;
	while(1)
	{
		//���
		//GpRectFill(NULL, &gpDraw[nflip], 0, 0, 320, 240, 8);
		do_strip(0);
		show_strip(0);

		//�������� �������� - ���� ���� ��� �������
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

		//������� ������ ������
		for (j=0; j<6; j++)
		{
			//cursor
			if ( j == i )
			{
				GpRectFill(NULL, &gpDraw[nflip], 0, j*32 + ssg_py +2, 320, 24, 136); //�����
				GpTransBlt(NULL,&gpDraw[nflip], //����
					op, j*32 + ssg_py -6, 320,8,(unsigned char*)logomenu,
					0,184,
					320,200,
					0);
				GpTransBlt(NULL,&gpDraw[nflip], //����
					op-320, j*32 + ssg_py -6, 320,8,(unsigned char*)logomenu,
					0,184,
					320,200,
					0);

				GpTransBlt(NULL,&gpDraw[nflip], //���
					op, j*32 + ssg_py + 24 +2, 320,8,(unsigned char*)logomenu,
					0,192,
					320,200,
					0);
				GpTransBlt(NULL,&gpDraw[nflip], //���
					op-320, j*32 + ssg_py + 24 +2, 320,8,(unsigned char*)logomenu,
					0,192,
					320,200,
					0);
				op--;	//������ ������� - �������
				if (op < 0)
					op = 320;
			}

			//���� ���������
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

		//���� ������� �������
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
			//������...
			SE(sfx_OK);
			return i;
		} else
		if ( keydata & GPC_VK_FB ) //press B = cancel
		{
			//�� ������...
			SE(sfx_cancel);
			return -1;
		}
		l++;
		Delay(45);
	}
}

void encode_savegame(void)
{
	//���������� ��������
	int i,p;

	//��������� ��� �����
	GpUserInfoGet(stroka, stroka);
	p = (stroka[0]+stroka[1]+stroka[2]+stroka[3])&255;
	//�������� �������� geepee 8) � ��������

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
	//����������� ��������
	int i,p;

	savegames = 0;

	//��������� ��� �����
	GpUserInfoGet(stroka, stroka);
	p = (stroka[0]+stroka[1]+stroka[2]+stroka[3])&255;

	for (i=0; i<20; i++)
	{
		//�������� CRC
		if ( savegame[i].CRC == savegame[i].highscore + savegame[i].level + savegame[i].money + savegame[i].random + 123*(i+1) + p && savegame[i].level <= 20 && savegame[i].money <= 9999 )
		{
			//��� ��.
			savegame[i].highscore = (savegame[i].highscore - 7) / 3;
			savegame[i].level /= 901;
			savegame[i].money /= 3;
			savegames++;
			savegame[i].random = random(64000);
			savegame[i].CRC = random(64000);
		}
		else
		{
			//��� �������� ��� ����
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
	//������� ��� ����� � ��� ����
	ERR_CODE err_code;
	F_HANDLE h_rfile;

	_fmeminv((block)savegame,sizeof(savegame));

	err_code = GpFileCreate(savegame_ini_file, ALWAYS_CREATE, &h_rfile);
	if (err_code != SM_OK)
	{
		//���� ��� INI ����� ��� ��� ������ ��
		fatalerror("Error: Can't create gpsys\\plusha.sav...");
	}

	//ini ���� ������
	err_code = GpFileWrite(h_rfile, savegame, sizeof(savegame));
	if (err_code != SM_OK)
	{
		fatalerror("Error: Can't save the game...");
	}

	_fmeminv((block)savegame,sizeof(savegame));

	//������� INI ���� 
	GpFileClose(h_rfile);
}

int show_highscore(void)
{
	//��������� �� ���������� ����� ���� ������ ��������
	//� �������� ������� 
	int i,j=0,keydata;

	//������� �������� ����� (�� ���������� �)
	if ( savegame[0].highscore == 10000 )
		return 1;

	keydata = 0;
	while( keydata == 0)
	{
		//���
		do_strip(0);
		show_strip(0);

		//�����
		BTextOut(NULL, &gpDraw[nflip], &big_black_font, 88+2, 8+2, (char*)"HI-SCORE");
		BTextOut(NULL, &gpDraw[nflip], &big_green_font, 88, 8, (char*)"HI-SCORE");

		//������� ������ ������
		for (i=0; i<7; i++)
		{
			//�� ���������� ������� ������ �������, �� ������� �� ����
			if ( savegame[i].highscore == 10000 * (i+1) )
				break;
			gp_str_func.sprintf(stroka, "LEVEL %1u : %07u",i+1,savegame[i].highscore);
			BTextOut(NULL, &gpDraw[nflip], &big_black_font, 20+1, i*29 + ssg_py+1-4, (char*)stroka);
			BTextOut(NULL, &gpDraw[nflip], &big_font, 20, i*29 + ssg_py-4, (char*)stroka);
		}

		FlipAndShow();

		//���� ������� �������
		GpKeyGetEx(&keydata);
		Delay(50);
		if (++j > 150)
			return 1;	//����� �� �������
	}
	return 0;	//����� �� �������
}