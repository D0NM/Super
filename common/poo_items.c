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

extern unsigned char tekblock;
extern unsigned char *poslab; //��� ��� ����� � ������� ������
extern s16 x,y,sx,sy; //��� ��������� �����
extern block obloka; //��� ������
extern block ind; //��� ������ ������ � ����������
//�����
extern struct hero man;
extern s16 coins;	//�������
extern s16 hearts;	//��������
extern s16 keys;	//�����
extern s16 beams;	//�����
extern s16 shields;	//������
extern s16 power;	//�������
extern s16 times;	//�����
extern u32 score,hiscore; //����
extern s16 continues;	//�����������
extern signed char maxjump;	//������ ������
extern char maxspeed;	//������������ ��������
extern char glass;	//����
extern char shboots;	//�������
extern char spboots;	//���������� �������
extern char jetpack;	//�����
extern char kolun;	//�����
extern block additems;
extern block items;
extern unsigned char waitmess;
extern s16 messy;
extern signed char messsy;
extern char * mess;

extern char stroka[300];
extern struct BFont score_font;
extern struct BFont big_font;

void getitem(s16 x_, s16 y_) { //������ ���� �� �����������

	check(x_,y_); //��������� �� ��, ��� � ��� �� ���� �����.
	switch ( tekblock&224 ) {
		case 1<<5: //������
			*poslab=*poslab&31;
			addboom(x*16+4,y*16+4,8,8,0,-1,b_buh,obloka+256*10+32*32*10+128*5);
			SE(sfx_coin);
			++score;
			if ( coins<9999 ) {
				++coins;
			}
			break;

		case 2<<5: //��� - �������
			*poslab=*poslab&31;
			addboom(x*16,y*16,16,8,0,-1,b_buh,obloka+256*10+32*32*10+128*1);
			score+=200;
			if ( man.on<5 ) {
				++man.on;
				SE(sfx_honey);
			} else {
				coins+=5;
				SE(sfx_coin);
			}
			break;

		case 3<<5: //������ - �����
			*poslab=*poslab&31;
			addboom(x*16,y*16,16,8,0,-1,b_buh,obloka+256*10+32*32*10+128*3);
			score+=1000;
			if ( hearts<9 ) {
				++hearts;
				SE(sfx_live);
				put_additems();
			}
			break;

		case 4<<5: //�����
			*poslab=*poslab&31;
			addboom(x*16,y*16,16,16,0,0,b_boom,obloka+256*5*4);
			addboom(x*16,y*16,16,8,0,-1,b_buh,obloka+256*10+32*32*10+128*1);
			score+=200;
			if ( keys<9 ) {
				++keys;
				SE(sfx_key);
			} else {
				coins+=25;
				SE(sfx_coin);
			}
			break;

		case 5<<5: //�����
			*poslab=*poslab&31;
			addboom(x*16+4,y*16+4,8,8,0,-1,b_buh,obloka+256*10+32*32*10+128*5+64);
			if ( beams<99 ) {
				++beams;
				SE(sfx_cone);
			}
			break;

		case 6<<5: //�����
			*poslab=*poslab&31;
			addboom(x*16,y*16,16,8,0,-1,b_buh,obloka+256*10+32*32*10+128*2);
			score+=500;
			times=9999;
			SE(sfx_clock);
			break;

		case 7<<5: //�������
        		*poslab=*poslab&31;
			addboom(x*16,y*16,16,16,0,0,b_boom,obloka+256*5*4);
			addboom(x*16,y*16,16,8,0,-1,b_buh,obloka+256*10+32*32*10+128*2);
			score+=500;
			power=500;
			SE(sfx_power);
			//break;
		}
}

void put_additems(void) {
     //����� ������������� �����

#define px_ai 300

	//char_bkgd=218;
	if ( shields ) { //���
		PutMas16(px_ai,20+20*0,additems+256*12 + (((times & 32))?256:0) );
	}
	if ( glass ) { //������. ����
		PutMas16(px_ai,20+20*1,additems+256*8 + ((times+11 & 32)?256:0) );
	}
	if ( maxjump < -8 ) { //��������
		PutMas16(px_ai,20+20*2,additems+256*2 + ((times+22 & 32)?256:0) );
		if ( maxjump < -9 ) {
			gp_str_func.sprintf(stroka, "%1d",abs(maxjump+8));
			BTextOut(NULL, &gpDraw[nflip], &score_font, px_ai-9,20+20*2, (char*)stroka);
		}
	}
	if ( shboots ) { //�������
		PutMas16(px_ai,20+20*3,additems+256*4 + ((times+33 & 32)?256:0) );
	}
	if ( kolun ) { //�����
		PutMas16(px_ai,20+20*4,additems+256*10 + ((times+44 & 32)?256:0) );
	}
	if ( jetpack ) { //������. ����
		PutMas16(px_ai,20+20*5,additems+256*6 + ((times+55 & 32)?256:0) );
	}

	if ( continues>0 ) { //�����������
		PutMas16(px_ai,20+20*6,additems + ((times+67 & 32)?256:0) );
		if ( continues>1 ) {
			//MoveXY(px_ai,20+20*6);
			//vprint("%1d",continues);
			gp_str_func.sprintf(stroka, "%1d",continues);
			BTextOut(NULL, &gpDraw[nflip], &score_font, px_ai-9,20+20*6, (char*)stroka);
		}
	}
	//�����
	PutMas16(px_ai,20+20*7,items+256*8*2 + 256*( (times>>3) & 3));
//	MoveXY(px_ai,20+20*7); vprint("%1d",hearts);
	gp_str_func.sprintf(stroka, "%1d",hearts);
	BTextOut(NULL, &gpDraw[nflip], &score_font, px_ai-9,20+20*7, (char*)stroka);



	char_bkgd=0;
}

void put_score(void) {
	//����� �����
	static s16 i;
	//screen old=CurrentScreen;
		
#define score_pos_y 240
//	char_fgd=16;
	for ( i=0; i<5; i++ ) {
		PutMas(8+i*9,score_pos_y-12,8,8,ind+((i<man.on)?64:0));
	}

//	char_bkgd=218;

	PutMas(70,score_pos_y-12,8,8,ind+8*8*2);
	gp_str_func.sprintf(stroka, "%04d", coins);
	BTextOut(NULL, &gpDraw[nflip], &score_font, 80,score_pos_y-16, (char*)stroka);

	PutMas(130,score_pos_y-12,8,8,ind+8*8*5);
	gp_str_func.sprintf(stroka, "%02d",beams);
	BTextOut(NULL, &gpDraw[nflip], &score_font, 140,score_pos_y-16, (char*)stroka);


	PutMas(170,score_pos_y-12,8,8,ind+8*8*4);
	gp_str_func.sprintf(stroka, "%1d",keys);
	BTextOut(NULL, &gpDraw[nflip], &score_font, 180,score_pos_y-16, (char*)stroka);


	PutMas(200,score_pos_y-12,8,8,ind+8*8*6);
	gp_str_func.sprintf(stroka, "%04d",times);
	BTextOut(NULL, &gpDraw[nflip], &score_font, 210,score_pos_y-16, (char*)stroka);

	gp_str_func.sprintf(stroka, "%06lu",score);
	BTextOut(NULL, &gpDraw[nflip], &score_font, 260,score_pos_y-16, (char*)stroka);


//	char_bkgd=0;

}

void addmess(char *m,unsigned char w) {
	waitmess=w; mess=m;
	messy=176+40; messsy=-5;
}