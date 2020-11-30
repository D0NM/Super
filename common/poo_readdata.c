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

extern block level;
extern block blevel;
extern char levnum[4];
extern s16 curr_level;
extern block fon;
extern u16 siz_xlev;
extern u16 siz_ylev;
extern u32 siz_level; //=siz_xlev*siz_ylev;
extern u32 svel,cvel;
extern screen hidscr; //скрытая страничка
extern screen bckg;	 //задний фон
extern struct strkobj kobj[maxobj];
extern struct strmodels models[maxmodel];
extern char files[10][13]; //файлы под уровни
extern struct levdef leveldef;
extern block demobuf; //буфер под демонстр нажатия клавишь
extern u16 nsecret;	//# секретов
extern u16 nitems;	//# вещей
extern struct hero man;
extern int nmonstr; //текущ кол-во видов монстров
extern u32 pmnspr; //последний свободн байт
extern block manspr; //образы героя
extern struct monstr monsters[maxmonstr];
extern char stroka[255];
extern block mnstrspr; //образы гадов

extern int installed_bonus;
extern int demo;


u16 TranspTile[256];

int gettok(char **s, char *d);
int getNtok(char **s);
//моя функция преобр-я СТРОКИ в число
int fatoi(char*s);

void readlev(void) {
	u16 i;
	char *t1,*t2;
	//__printf("ReadLev: ");
	if ( installed_bonus && !demo && curr_level>7) {
		SetLib("bonus\\levels");
	} else {
		SetLib("levels");
	}
	sprintf(levnum,"%02d",curr_level);
	for (i=0; i<10; ++i) {
		files[i][0]=levnum[0];
		files[i][1]=levnum[1];
	}
	//__printf("leveldef, ");
	if (SizeLib(f_def) <=0 )
		fatalerror("Error: Level not found!");

	GetLib(f_def,(block)hidscr);
	gp_str_func.memcpy(&leveldef,(block)hidscr,sizeof(leveldef));
	//из общих парам уровня узнаем имена фона и т.п.
	gp_str_func.strcpy(f_fon,leveldef.fonname);
	gp_str_func.strcpy(f_bckg,leveldef.bckgname);
	siz_xlev=leveldef.lx;
	siz_ylev=leveldef.ly;
	siz_level=(u32)siz_xlev*siz_ylev;

	//__printf("kobj, ");
	//®зЁбвЄ  ­г¦­  Ё§-§  гўҐ«ЁзҐ­Ёп Є®«-ў  ®ЎкҐЄв®ў
	gp_str_func.memset(hidscr,0,sizeof(kobj));
	GetLib(f_bmonstr,(block)hidscr);
	//gp_str_func.memcpy(&kobj,(block)hidscr,sizeof(kobj));
	for (i=0; i<maxobj; ++i) {
		gp_str_func.memcpy(&kobj[i],(block)((long)hidscr+i*5),5);
		kobj[i].n = *(signed char*)((long)hidscr+i*5+4);
		if ( kobj[i].x == 0 && kobj[i].y == 0)
			kobj[i].n = -1;
		//__printf("%d,", kobj[i] );
	}	

	//__printf("models, ");	
	//®зЁбвЄ  ­г¦­  Ё§-§  гўҐ«ЁзҐ­Ёп Є®«-ў  ®ЎкҐЄв®ў
	gp_str_func.memset(hidscr,0,sizeof(models));
	GetLib(f_models,(block)hidscr);
	//gp_str_func.memcpy(&models+i*,(block)hidscr,sizeof(models));	
	for (i=0; i<maxmodel; ++i) {
//		for (j=0; j<49; ++j) {
//			__printf("%x,", *(char*)( (long)hidscr+i*39+j ) );
//		}
		
		gp_str_func.memcpy((block)&models[i],(block)((long)hidscr+i*39),39);
		//models[i].on = 1;
		//models[i].on0 = 1;		
		//(char*)(&models+i*sizeof(models[0])+sizeof(models[0])-2) = (char)(char*)(hidscr+i*39+37);
		//(char*)(&models+i*sizeof(models[0])+sizeof(models[0])-1) = (char)(char*)(hidscr+i*39+38);
		gp_str_func.lowercase((char*)&models[i].name,sizeof(models[0].name));
		gp_str_func.lowercase((char*)&models[i].name0,sizeof(models[0].name0));		
		t1 = (char*)((long)hidscr+i*39+37);
		t2 = (char*)((long)hidscr+i*39+38);
		models[i].on = *t1;
		models[i].on0 = *t2;		
		
		//__printf("\nML:(%s,%s)%i,%i,%i,%i\n",models[i].name,models[i].name0,models[i].on,models[i].on0, *t1, *t2);
	}

	GetLib(f_level,(block)level);
	GetLib(f_blevel,(block)blevel);
	GetLib(f_demo,(block)demobuf);

	GetLib(f_bckg,(block)bckg);
	TurnIt((block)bckg, 320, 200);
	
	GetLib(f_fon,(block)fon);
	for (i=0; i<256; i++) { //добавил!! до 256
		//проверим естьли прозрачные точки
		TranspTile[i]=TurnItT(fon+i*16*16,16,16);
	}
	
	GetLib(f_dstrip,(block)hidscr);
	readstrip();
	GetLib(f_dmonstr,(block)hidscr);
	readmonstr();
		
	//подсчет кол-ва секретов
	nsecret=0;
	for ( i=0; i<maxmodel; ++i ) {
		if ( models[i].typ>=m_copy ) ++nsecret;
	}
	//подсчет кол-ва вещей
	nitems=0;
	for ( i=0; i<siz_level; ++i ) {
		if ( blevel[i]&224 ) ++nitems;
	}
	//__printf("ReadLev: Finish\n");
}

void readmonstr(void) {
	//считывание описания из файла и создание массива
	s16 i,j,k,f;
	int sprite_size;
	block pos=(block)hidscr;

	nmonstr=0;
	pmnspr=0; //последний свободн байт в буфере образов монстров

	if ( installed_bonus && !demo && curr_level>7) {
		SetLib("bonus\\graph");
	} else {
		SetLib("graph");
	}

	//worm 16 16  0 8 ?  16 8 ?  8 16 ?  8 0 ?  0  worm.bin  1 2 3 4 5 6
/*	n=_sscanf(stroka,"%s  %u %u  %d %d %d  %d %d %d  %d %d %d  %d %d %d  %u  %s  %u %u %u %u",
	&man.name,&man.lx,&man.ly,
	&man.lfx,&man.lfy1,&man.lfy2,&man.rtx,&man.rty1,&man.rty2,
	&man.upx1,&man.upx2,&man.upy,&man.dwx1,&man.dwx2,&man.dwy,
	&man.typ,&man.namespr,&man.maxgo,&man.maxjm,&man.maxst,&man.maxspec
	);
*/

	gettok(&pos,(char*)&man.name);
	
	man.lx=getNtok(&pos);	
	man.ly=getNtok(&pos);
	
	man.lfx=getNtok(&pos);	
	man.lfy1=getNtok(&pos);	
	man.lfy2=getNtok(&pos);	
	man.rtx=getNtok(&pos);	
	man.rty1=getNtok(&pos);	
	man.rty2=getNtok(&pos);	
	
	man.upx1=getNtok(&pos);	
	man.upx2=getNtok(&pos);	
	man.upy=getNtok(&pos);	
	man.dwx1=getNtok(&pos);	
	man.dwx2=getNtok(&pos);
	man.dwy=getNtok(&pos);
	
	man.typ=getNtok(&pos);
	gettok(&pos,(char*)&man.namespr);
	man.maxgo=getNtok(&pos);
	man.maxjm=getNtok(&pos);
	man.maxst=getNtok(&pos);
	man.maxspec=getNtok(&pos);	
	
	man.dsx1=man.dsx2=man.dsy=man.f=man.sy=man.sx=0;
	
	//__printf("---\n%s %s\n",man.name,man.namespr);

	//farfree(manspr);
	//manspr=(block)famemalloc(SizeLib(man.namespr));
	GetLib(man.namespr,(block)manspr);
//VINNY		32 32	4 9 22	27 9 22	9 22 30	9 22 0	0	men.bin		6 4 5 3
	for (i=0; i<21-6+man.maxgo; i++) {
		TurnIt(manspr+i*man.lx*man.ly,man.lx,man.ly);
	}	

	#define m_s man.lx*man.ly
	//рассчет смещений спрайтов
	man.gospr=manspr;
	man.jmspr=manspr+m_s*(man.maxgo);
	man.stspr=manspr+m_s*(man.maxgo+man.maxjm);
	man.specspr=manspr+m_s*(man.maxgo+man.maxjm+man.maxst);

	//идем на след строку
	while ( 1 ) {

		for (i=0; i<maxmonstr; ++i ) {
			//считаем очередн строку
//			gp_str_func.memcpy(&stroka,pos,250);

			//worm 16 16  0 8  16 8  8 16  8 0  0  worm.bin  4
			gettok(&pos,(char*)&monsters[i].name);
			
			if ( (monsters[i].name[0] == 'q' && monsters[i].name[1] == 0) || monsters[i].name[0] == 0 ) {
				//когда описание кончилось - выход
				//__printf("Load monsters: Finished\n");
				return;
			}
	
			monsters[i].lx=getNtok(&pos);	
			monsters[i].ly=getNtok(&pos);
	
			monsters[i].lfx=getNtok(&pos);	
			monsters[i].lfy=getNtok(&pos);	
			monsters[i].rtx=getNtok(&pos);		
			monsters[i].rty=getNtok(&pos);	
	
			monsters[i].upx=getNtok(&pos);	
			monsters[i].upy=getNtok(&pos);	
			monsters[i].dwx=getNtok(&pos);
			monsters[i].dwy=getNtok(&pos);
	
			monsters[i].typ=getNtok(&pos);
			monsters[i].speed=getNtok(&pos);			
			monsters[i].lives=getNtok(&pos);
			gettok(&pos,(char*)&monsters[i].namespr);
			monsters[i].maxgo=getNtok(&pos);
			monsters[i].maxjm=getNtok(&pos);
			monsters[i].maxst=getNtok(&pos);
			monsters[i].maxspec=getNtok(&pos);	

			//это монстр
			//вычислим начала спрайтов
			f=0; //флаг, что таких спрайтов нет...
			for ( j=0; j<nmonstr; ++j ) {
				if (j<i && gp_str_func.compare(monsters[i].namespr,monsters[j].namespr)==0) {
					//если уже есть данный спрайт, то
					//сошлемся на него
					monsters[i].gospr=monsters[j].gospr;
					monsters[i].jmspr=monsters[j].jmspr;
					monsters[i].stspr=monsters[j].stspr;
					monsters[i].specspr=monsters[j].specspr;
					f=1; //флаг, что есть такой спрайт
					++nmonstr;
					break;
				}
			}

			//если такого спр не было, то загрузим
			if (f==0 && GetLib(monsters[i].namespr,mnstrspr+pmnspr)) {
				#define m_o monsters[i].lx*monsters[i].ly
				
				sprite_size = SizeLib(monsters[i].namespr);
				//__printf("--\n%s %s\n",monsters[i].name,monsters[i].namespr);
				for (k=0; k < sprite_size / (m_o); k++) {
					TurnIt(mnstrspr+pmnspr+k*(monsters[i].lx*monsters[i].ly),monsters[i].lx,monsters[i].ly);
				}	

				monsters[i].gospr=mnstrspr+pmnspr;
				monsters[i].jmspr=mnstrspr+pmnspr+m_o*(monsters[i].maxgo);
				monsters[i].stspr=mnstrspr+pmnspr+m_o*(monsters[i].maxgo+monsters[i].maxjm);
				monsters[i].specspr=mnstrspr+pmnspr+m_o*(monsters[i].maxgo+monsters[i].maxjm+monsters[i].maxst);

				if (monsters[i].maxst==0)
					monsters[i].stspr=monsters[i].gospr;

				if (monsters[i].maxjm==0)
					monsters[i].jmspr=monsters[i].gospr;

				if (monsters[i].maxspec==0)
					monsters[i].specspr=monsters[i].stspr;


				pmnspr += sprite_size;
				++nmonstr;
			}
		}
	}
}
