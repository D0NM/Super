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
extern block textdata;	//��������� ���-� � �������
extern block textindex[200]; //������ �� ������ ������ 0,1...
char txt_dumb[]="^_^";

extern struct sshop shop[12];

extern struct BFont big_font;

//char path_to_lng_file[]="gp:\\gpmm\\gpooh\\language\\";
extern char path_to_lng_file[];
//char lang_ini_file[]="language.ini";
extern char lang_ini_file[];
char current_language[16];
extern char path_to_data_file[];

int is_language_loaded = 1;	//1 = ���� �� �������� 0 - ��������

void check_language(void)
{
	ERR_CODE err_code;
	F_HANDLE h_rfile;
	unsigned long n_dummy;

	//������ language.ini ����
	GpRelativePathSet(path_to_lng_file);

	err_code = GpFileOpen(lang_ini_file, OPEN_R, &h_rfile);
	if (err_code != SM_OK)
	{
		//���� ��� INI ����� ��� ��� ������ �� �������� ����
		select_language();
		return;
	}

	//ini ���� ������, ������� ����
	err_code = GpFileRead(h_rfile, current_language, 16, (ulong*)&n_dummy);
	if (err_code != SM_OK || n_dummy < 16)
	{
		//���� INI ����� �� �������� ����� �� �������� ����
		GpFileClose(h_rfile);
		select_language();
		return;
	}

	//������� INI ���� 
	GpFileClose(h_rfile);
	//������� ����� ��������
	load_language();
}

void select_language(void)
{
	ERR_CODE err_code;

	GPDIRENTRY *p_list;
	typedef struct {           
		char name[16];
		char *g;	//�������.. ��� ���� ����-� ����� �����
	}MYDIRENTRY;
	MYDIRENTRY *pi_list;
	GPFILEATTR mAttr;
	int i,j,keydata;
	unsigned long list_count = 0, read_count = 0, final_count = 0;

	init_loading();
	loading();

	//����� ��� - �������� ������ ������
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
		//����� 1 ���� ������. �� ������� ��� � ��������. � ������� ini.
		gp_str_func.strcpy(current_language,pi_list[0].name);
		//����� ini
		save_language();
		//������ �������� ������
		load_language();
		return;
	}

	//������ �� ������� ���������� ����� ���� � ���.
	for (i=0; i<final_count; i++)
	{
		loading();

		SetLib(pi_list[i].name);
		pi_list[i].g = (block)famemalloc(SizeLib("flag.bin"));
		GetLib("flag.bin",pi_list[i].g);
		TurnIt(pi_list[i].g,320,200);
	}
	//��������� �������� ����
	//SetLib("");
	

	//!start_sound();

	//���� ������
	i=0;
	
	keydata = 0;
	while(1)
	{
		GpRectFill(NULL, &gpDraw[nflip], 0, 0, 320, 240, 227); //������-�������

		//������� ������ ������
		for (j=0; j<final_count; j++)
		{
			//cursor
			if (i == j)
			{
				GpRectFill(NULL, &gpDraw[nflip], 0, (j-i)*70 + 85-3, 320, 64+6, 144); //�����
				GpRectFill(NULL, &gpDraw[nflip], 10+2, (j-i)*70 + 85+2, 96, 64, 255); //������
			}

			//����
			GpTransBlt(NULL,&gpDraw[nflip],
				10,(j-i)*70 + 85,96,64,(unsigned char*)pi_list[j].g,
				0,0,
				320,200,
				0);
			//�������� �����
			GpTransBlt(NULL,&gpDraw[nflip],
				20+96,(j-i)*70 + 85 + 16,192,32,(unsigned char*)pi_list[j].g,
				96,0,
				320,200,
				0);
		}

		FlipAndShow();

 		//���� ������� �������
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
			//���� ������... ������ ����
			gp_str_func.strcpy(current_language,pi_list[i].name);
			SE(sfx_OK);
			break;
		}
	}

	//������� ��������� ����
	GpRectFill(NULL, &gpDraw[nflip], 0, 0, 320, 240, 150); //�����
	//����
	GpTransBlt(NULL,&gpDraw[nflip],
		112,64,96,64,(unsigned char*)pi_list[i].g,
		0,0,320,200,0);
	//�������� �����
	GpTransBlt(NULL,&gpDraw[nflip],
		64, 144,192,32,(unsigned char*)pi_list[i].g,
		96,0,320,200,0);
	FlipAndShow();

	//������� - ��������� ��� ������.
	for (i= final_count - 1; i>=0; i--)
	{
		gp_mem_func.free(pi_list[i].g);
	}
	gp_mem_func.free(pi_list);
	gp_mem_func.free(p_list);

	Delay(600);
	//!stop_sound();

	//������� ��������� ��� � ini-����
	save_language();
	
	//������� �������� ����������
	load_language();
}

void save_language(void)
{
	//������� ��� ����� � ��� ����
	ERR_CODE err_code;
	F_HANDLE h_rfile;

	//����� ������� ���� � language.ini ����
	GpRelativePathSet(path_to_lng_file);

	err_code = GpFileCreate(lang_ini_file, ALWAYS_CREATE, &h_rfile);
	if (err_code != SM_OK)
	{
		//���� ��� INI ����� ��� ��� ������ �� �������� ����
		fatalerror("Error: Can't create LANGUAGE.INI...");
	}

	//ini ���� ������, ������� ����
	err_code = GpFileWrite(h_rfile, current_language, 16);
	if (err_code != SM_OK)
	{
		fatalerror("Error: Can't write LANGUAGE.INI...");
	}

	//������� INI ���� 
	GpFileClose(h_rfile);
}

void load_language(void)
{
	int i=0,j=0, ts;
	block p;
	//����� ������ ��� �������� ���� �� ���������� ����
	SetLib(current_language);

	//��� ��� ������� � ������� ������:  menus=(block)famemalloc(320*200);
	//� ������ �������� ������� � �.�.
	GetLib("menus.bin",(block)menus);
	TurnIt(menus,320,200);

	is_language_loaded = 0;	//���� ���� �������� - ���������� LOADING �� ���� �����
	//loading();

	//�������� ��������� ���� ��������� ������������� � ��������� ������ �� ����� � ������
	ts = SizeLib("textdata.txt")+2;
	GetLib("textdata.txt",(block)textdata);
	//����������� ������
	p = textdata;

	for (i=0; i<200; i++)
	{	//�������������� ��� ������ ���������
		textindex[i] = (block)txt_dumb;
	}
	i=0;
	while ( i<200 && p < (textdata + ts) )	//���-�� ����� ���� � ����� ������
	{
		//������� ��������� ������
		textindex[i++] = p++;
		//���� ����� �����
		if (*p == 0) {
			break;
		}
		while ( *p !='\n' && *p != '\r') {
			//���������� ���� ������
			if (*p == ';') //�������� �����������
				*p++ = 0;
			if (*p == '#') //������ �� # ������� ������
				*p++ = '\n';
			p++;
		}
		*p++ = 0;
#ifndef RELEASE
		__printf("textdata: str %d ='%s'\n",i-1,textindex[i-1]);
#endif
		//���� �� ��� ��
		while (*p == '\n' || *p == '\r') {
			p++;
		}	
	}
	//����� ���... 8(
	*p++ = 0;
	textindex[i-1] = (block)txt_dumb;
#ifndef RELEASE
	__printf("F textdata: str %d ='%s'\n",i-1,textindex[i-1]);
	__printf("FIN textdata: str %d ='%s'\n",i,textindex[i]);
	//__printf("textdata: strings loaded %d\n",i);
#endif

	//������� �������� ����� � �������
	j = 21;
	for (i=0; i<12; i++)
	{
		shop[i].name = textindex[j++];
		shop[i].description = textindex[j++];
	}

	GpRelativePathSet(path_to_data_file);	//!!! ���������� �� �����
}

int loading_bar = 0;	//������� � �������� ����

#define ldb_x 60
#define ldb_y 140

void loading(void)
{
//�������� ������� LOADING ��� �������� ����-����
	Clrs(139, nflip);	//������

	if ( is_language_loaded ) 
	{
		BTextNOut(NULL, &gpDraw[nflip], &big_font, 84, 108, "LOADING...", 0, 10);
	}
	else
	{
		//���� ��� ���� ����������� �������.. ��... �� � �������
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

	GpRectFill(NULL, &gpDraw[nflip], ldb_x+6, ldb_y+6, max(1,loading_bar)/*200-12*/, 20-12, 14);	//�����

	if ( loading_bar <= 200-12-5) loading_bar += 5; else loading_bar = 200-12;
	
	FlipAndShow();
}

void init_loading(void) {
	loading_bar = 0;
}
