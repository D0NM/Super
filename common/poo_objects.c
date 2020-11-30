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
//extern screen hidscr; //скрытая страничка

extern block textindex[100]; //ссылки на строки текста 0,1...
int fatoi(char*s);

/*signed char*/int poc[32][16]={
	{16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16}, //пустота 0
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //цел прозрачный блок

	{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0}, //скат /| 2

	{15,15,14,14,13,13,12,12,11,11,10,10,9,9,8,8}, //в друг стор 5
	{7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0}, //6


	{15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12}, //самый слаб скат
	{11,11,11,11,10,10,10,10,9,9,9,9,8,8,8,8},
	{07,07,07,07,06,06,06,06,05,05,05,05,04,04,04,04},
	{03,03,03,03,02,02,02,02,01,01,01,01,00,00,00,00},

	{00,00,00,00,01,01,01,01,02,02,02,02,03,03,03,03}, //самый слаб скат
	{04,04,04,04,05,05,05,05,06,06,06,06,07,07,07,07},
	{8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11},
	{12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15},

	{0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7}, //скат слаб 3
	{8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15}, //продолжение 4

	{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}, //скат |\ 1


	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //цел блок
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //кирпичная
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, // ?
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //исчезающая

	{02,01,00,00,00,00,01,01,01,01,00,00,00,00,01,02}, //конвеер
	{02,01,00,00,00,00,01,01,01,01,00,00,00,00,01,02}, //конвеер

	{-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10}, //подпорка

	{16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16}, //вывод сверху
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //вывод сверху . но как блок на котором стоят
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //резерв 3
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //резерв 4
	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00}, //резерв 5

	{11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11}, //еще выше
	{07,07,07,07,07,07,07,07,07,07,07,07,07,07,07,07}, //выше
	{03,03,03,03,03,03,03,03,03,03,03,03,03,03,03,03}, //низк

	{00,01,02,03,02,01,00,10,01,00,01,02,03,02,01,00} // шипы
	//{00,01,02,03,02,01,00,10,01,00,01,02,03,02,01,00} // были шипы
};

signed char nappoc[32]={0,0,-3,-1,-1,0,0,0,0,0,0,0,0,1,1,3, 0,0,0,0,-3,3,0,0,0,0,0,0,0,0,0,0}; //папр и смещен
//было в плюше signed char nappoc[32]={0,0,-3,-2,-2,-1,-1,-1,-1,1,1,1,1,2,2,3, 0,0,0,0,-3,3,0,0,0,0,0,0,0,0,0,0}; //папр и смещен
//char napypoc[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

extern s16 x,y,sx,sy; //тек положение точки
extern unsigned char *poslab; //тек поз точки в массиве трассы
extern unsigned char *poslal; //тек поз точки в массиве фона
extern unsigned char tekblock;
extern block blevel;
extern block level;
extern struct strbullets bullets[maxbullet];
extern block items,obloka,fon;
//герой
extern struct hero man;
extern struct levdef leveldef;
extern struct strmodels models[maxmodel];
extern s16 coins;	//монетки
extern s16 hearts;	//сердечки
extern s16 keys;	//ключи
extern s16 beams;	//шишки
extern s16 shields;	//защита
extern s16 power;	//энергия
extern s16 times;	//время
extern u32 score,hiscore; //очки
extern s16 continues;	//продолжения
extern char maxjump;	//высота прыжка
extern char maxspeed;	//максимальная скорость
extern char glass;	//очки
extern char shboots;	//шиповки
extern char spboots;	//скоростные башмаки
extern char jetpack;	//ранец
extern char kolun;	//колун
extern u16 siz_xlev;
extern u16 siz_ylev;
extern u32 siz_level; //=siz_xlev*siz_ylev;
extern u32 svel,cvel;
extern struct boom booms[maxboom];
extern struct monstr monsters[maxmonstr];
extern int nmonstr; //текущ кол-во видов монстров
extern struct object obj[maxobj];
extern struct strkobj kobj[maxobj];
extern block enbull; //под вражеские снаряды
extern u16 r_x,r_y; //прежние координаты лев. верхн угла окна
extern char waitboom;
extern char boss;	//признак активности БОССА
extern s16 end,up,pause,key_f,key_b;
extern block modelspr, additems;
extern block logomenu;

//конечн автоматы для босса 22
enum {mm_none, mm_think, mm_aim, mm_flow, mm_fire};
static int mm_stat = mm_none;
static int mm_delay = 1;

s16 check(s16 x_, s16 y_) { //выод кода блока по координатам
	register s16 sdv;
	//разделим координаты
	sx=(x_&0x0f); x=x_>>4; sy=(y_&0x0f); y=y_>>4;
	//позицию первую вычислим
	tekblock=*(poslab=(unsigned char *)blevel+(sdv=x+y*siz_xlev));
	poslal=(unsigned char *)level+sdv;

	return (tekblock&31);
}

s16 checkdw(s16 x_, s16 y_) {
	//вывод кода блока по координатам с коррекцией на пол
	register s16 sdv;
	//разделим координаты
	sx=(x_&0x0f); x=x_>>4; sy=(y_&0x0f); y=y_>>4;
	//позицию первую вычислим
	tekblock=*(poslab=(unsigned char *)blevel+(sdv=x+y*siz_xlev));
	poslal=(unsigned char *)level+sdv;

	if ( sy>=poc[(tekblock&31)][sx] )
		return (tekblock&31);
	return 0;
}


s16 addbullet(s16 x, s16 y, s16 sx, s16 sy, char typ, block buf) {
	//добавление пули в очередь
	register int i;
	for ( i=0; i<maxbullet; ++i ) {
		if ( bullets[i].typ<0 ) {
			bullets[i].x=x;
			bullets[i].y=y;
			bullets[i].sx=sx;
			bullets[i].sy=sy;
			bullets[i].typ=typ;
			bullets[i].buf=buf;
			bullets[i].f=0;
			return 1;
		}

	}
	return 0;
}

void dobullet(void) {
	//вывод и обработка патронов
	register int i,t;
	static struct strbullets *bul;
	for ( i=0; i<maxbullet; ++i ) {
		if ( bullets[i].typ>=0 ) {
			bul=&bullets[i];
			//сторкнулся ли патрон с чем?
			t=check((bul->x+=bul->sx),(bul->y+=bul->sy));
			if ( (t!=0 && t!=23) || bul->y>=siz_ylev*16) {
				check((bul->x-bul->sx),(bul->y-bul->sy));
				//если шишка упала, то пусть лежит
				if ( *poslab==0 && bul->typ==1) {
					if ( bul->sx ) {
						bul->typ=-1;
						addbullet(bul->x-bul->sx,bul->y-bul->sy,0,bul->sy+1,1,items+256*4*8);
					} else {
						*poslab=5<<5;
						SE(sfx_fall);
					}
				} else {
					if( bul->typ<8 || bul->sy>=0 ) {
						bul->typ=-1;
					}
				}
				if ( bul->typ==-1 ) {
					addboom(bul->x-8,bul->y-8,16,16,0,0,b_boom,obloka+256*15);
				} else {
					addboom(bul->x-8,bul->y-8,16,16,0,0,b_boom,obloka);
				}
			}
			if( bul->typ>=8 ) {	//снаряды врага
				//PutCImg(bul->x-r_x+8,bul->y-r_y+8,16,16,bul->buf);
				GpTransBlt(NULL,&gpDraw[nflip], bul->x-r_x-8,bul->y-r_y-8,
					16,16,
					(unsigned char*)bul->buf,
					0,0,
					16,16,
					0);
				if (waitboom&1) {
					if ( ++bul->sy>16 || bul->y>siz_ylev*16 ) {
						bul->typ=-1;
					}
        	                }
			} else {	//призы или шишки
				GpTransBlt(NULL,&gpDraw[nflip], bul->x-r_x-8,bul->y-r_y-8,
					16,16,
					(unsigned char*)bul->buf+256*bul->f,
					0,0,
					16,16,
					0);
				//PutCMas(bul->x-r_x+8,bul->y-r_y+8,16,16,bul->buf+256*bul->f);
				if ( ++bul->f>7 ) {
					bul->f=0;

					if ( bul->sx>0 )
						--bul->sx;
					else if ( bul->sx<0 )
						++bul->sx;

					if (++bul->sy>6) {
						bul->sy=6;
					}
				}
			}

		}

	}
}

void domodel(void) {
	//вывод и обработка патронов
	register s16 i,j,t,t0,x1,y1;
	static struct strmodels *md;
	static u8* mult;
	for ( i=0; i<maxmodel; ++i ) {
		if ( models[i].typ>=0) {
			//если модель включена
			md=&models[i];
			switch ( md->typ ) {
				case m_lift0:
				case m_lift:
					md->sx=md->sy=0;
					if ( md->on ) {
						if ( md->x<md->x2 ) {
							md->sx=1;
						} else if ( md->x>md->x2 ) {
							md->sx=-1;
						} else {
							md->sx=0;
						}
						if ( md->y<md->y2 ) {
							md->sy=1;
						} else if ( md->y>md->y2 ) {
							md->sy=-1;
						} else {
							md->sy=0;
						}
						md->x+=md->sx;
						md->y+=md->sy;
						if ( md->sy==0 && md->sx==0) {
							if ( md->typ==m_lift ) {
								Swap(&md->x2,&md->x1);
								Swap(&md->y2,&md->y1);
							} else {
								md->on=0; //выключим однопроходный лифт
								//md->x = md->x1;
								//md->y = md->y1;
							}
						}
					}
					//Bar(md->x-r_x-1,md->y-r_y-1,md->lx+2,md->ly+2);
					if ( md->ly < 8 ) {
						//мелкий лифт < 8 в высоту
						//левая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x -3, md->y-r_y,
						md->lx - 9 + 3 + 3, 5,
						(unsigned char*)logomenu,
						151,48,
						320,200,0);
						//правая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x + (md->lx - 9 +3), md->y-r_y,
						9, 5,
						(unsigned char*)logomenu,
						247,48,
						320,200,0);
					} else
					if (md->ly == 8) {
						//обычный лифт =8
						//левая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x -3, md->y-r_y,
						md->lx - 9 + 3+3, 8,
						(unsigned char*)logomenu,
						151,48,
						320,200,0);
						//правая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x + (md->lx - 9+3), md->y-r_y,
						9, 8,
						(unsigned char*)logomenu,
						247,48,
						320,200,0);
					} else {
						//большой лифт >8
						//низ
						//левая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x -3, md->y-r_y +6,
						md->lx - 9 + 3+3, md->ly -6,
						(unsigned char*)logomenu,
						151,89- md->ly +7,
						320,200,0);

						//правая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x + (md->lx - 9+3), md->y-r_y +6,
						9, md->ly - 6,
						(unsigned char*)logomenu,
						247,89- md->ly +7,
						320,200,0);

						//верх
						//левая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x -3, md->y-r_y,
						md->lx - 9 + 3+3, 8,
						(unsigned char*)logomenu,
						151,56,
						320,200,0);
						//правая часть
						GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x + (md->lx - 9+3), md->y-r_y,
						9, 8,
						(unsigned char*)logomenu,
						247,56,
						320,200,0);
					}
					/*
					GpBitBlt(NULL,&gpDraw[nflip],
						md->x-r_x,md->y-r_y,md->lx,md->ly-1,(unsigned char*)lifts,
						0,0,
						md->lx,md->ly);
					GpBitBlt(NULL,&gpDraw[nflip],
						md->x-r_x+1,md->y-r_y+md->ly-1,md->lx-2,1,(unsigned char*)lifts,
						0,0,
						md->lx,md->ly);
					*/
					break;

				case m_door:
					//PutCImg(md->x-r_x+16,md->y-r_y+16,32,32,(block)modelspr+256*6+32*32*md->on);
					GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x,md->y-r_y,32,32,(unsigned char*)modelspr+256*6+32*32*md->on,
						0,0,
						32,32,0);
					break;

				case m_warp:
					if( glass && (waitboom&31)==3 ) {
						addboom(md->x+16,md->y+16,8,8,0,-5,b_buh,obloka+256*10+32*32*10+128*5+64*2);
					}
					break;

				case m_shop:
					//PutCMas(md->x-r_x+16,md->y-r_y+16,32,32,(block)modelspr+256*6+32*32*2);
					GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x,md->y-r_y,32,32,(unsigned char*)modelspr+256*6+32*32*2,
						0,0,
						32,32,
						0);
					break;

				case m_flag:	//если не умерли и проходим флажок, то
        				if ( man.s != s_crash && man.s != s_over && man.x+(man.dwx1+man.dwx2)/2>=md->x && man.x+(man.dwx1+man.dwx2)/2<=md->x+16
						&&  man.y<=md->y+48 && man.y>=md->y ) {
						if ( md->on ) {
								md->on=0;
								leveldef.nx=man.x; leveldef.ny=man.y-8;
								addboom(md->x,md->y,16,16,0,0,b_boom,obloka+256*5*4);
								//addmess("Check Point...",40);
							     	addmess(textindex[20],40);
								SE(sfx_checkpoint);
						}
					}
					//PutCMas(md->x-r_x+16,md->y-r_y+16,16,32,(block)modelspr+256*6+32*32*3+32*16*md->on);
					GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x,md->y-r_y,16,32,(unsigned char*)modelspr+256*6+32*32*3+32*16*md->on,
						0,0,
						16,32,
						0);
					break;

				case m_touch:
        				if ( man.x+(man.dwx1+man.dwx2)/2>=md->x && man.x+(man.dwx1+man.dwx2)/2<=md->x+md->lx
		        			&&  man.y<=md->y+md->ly && man.y>=md->y ) {
						if ( md->on ) {
							for ( j=0; j<maxmodel; ++j ) {
								if ( gp_str_func.compare(md->name0,models[j].name)==0 ) {
									models[j].on=1;
									md->on=0;
                                                                }
							}
							if ( gp_str_func.compare("exit",md->name0)==0 ) {
								//конец уровня!!!!
								man.s=s_end;
								end=1;
								//т.к. выход то пока!
								break;
							} else
							if ( gp_str_func.compare("boss",md->name0)==0 ) {
								//BOSS уровня!!!!
								boss=1;
								modsetup("boss.fms", 4, 0 ,0, 0, 0 );
								//addmess("The BOSS of the Level",40);
							     	addmess(textindex[16],40);
								man.myrg=wmyrg;
							}
						}
					}
					break;

				case m_button:
					//PutCMas(md->x-r_x+16,md->y-r_y+16,16,16,(block)modelspr+256*2+256*md->on);
					GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x,md->y-r_y,16,16,(unsigned char*)modelspr+256*2+256*md->on,
						0,0,16,16,0);
					break;

				case m_switch:
					//PutCMas(md->x-r_x+16,md->y-r_y+16,16,16,(block)modelspr+256*4+256*md->on);
					GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x,md->y-r_y,16,16,(unsigned char*)modelspr+256*4+256*md->on,
						0,0,16,16,0);
					break;

				case m_keyhole:
					//PutCImg(md->x-r_x+16,md->y-r_y+16,16,16,(block)modelspr+256*md->on);
					GpTransBlt(NULL,&gpDraw[nflip],
						md->x-r_x,md->y-r_y,16,16,(unsigned char*)modelspr+256*md->on,
						0,0,16,16,0);
					break;

				case m_hole:
				case m_brick:
				case m_timer:
					if ( md->on ) {
						if ( md->x<md->x2 ) {
							md->sx=1;
						} else if ( md->x>md->x2 ) {
							md->sx=-1;
						} else {
							md->sx=0;
						}
						if ( md->y<md->y2 ) {
							md->sy=1;
						} else if ( md->y>md->y2 ) {
							md->sy=-1;
						} else {
							md->sy=0;
						}
						md->x+=md->sx;md->y+=md->sy;
						for( y1=0; y1<=md->ly;y1+=16 )
						for( x1=0; x1<=md->lx;x1+=16 ) {
							t=check(md->x+x1,md->y+y1);
							if ( md->typ==m_hole ) {
								//взрываем
								if (t) {
									addboom(x*16/*+x1*/,y*16/*+y1*/,16,16,5-random(11),1+random(2),b_fall,fon+(*poslal)*256);
									addboom(x*16/*+x1*/,y*16/*+y1*/,16,16,0,0,b_boom,obloka+256*5*4);
									if ( *poslab || *poslal )
										SE(sfx_magic);
									*poslab=*poslal=0;
								}
							} else if ( md->typ==m_brick ) {
								//ставим стенку
								if (t!=16) {
									addboom(x*16/*+x1*/,y*16/*+y1*/,16,16,0,0,b_boom,obloka+256*5*4);
									if ( *poslab==0 || *poslal==0 )
										SE(sfx_magic);
									*poslab=*poslal=16;
								}
							} else {
								//просто считаем время в таймере 8)
							}
						}

						if ( md->sy==0 && md->sx==0) {
							//конец работы
							md->on=0; //выключим себя.. но можем и включить сами себя
							md->x = md->x1;
							md->y = md->y1;


							for ( j=0; j<maxmodel; ++j ) {
								if ( gp_str_func.compare(md->name0,models[j].name)==0 ) {
									if ( md->typ==m_timer ) {
										switch ( md->lx ) {
										case 0: 
											//выключаем
											models[j].on = 0;
											break;
										case 1: 
											//включаем
											models[j].on = 1;
											break;
										default: //удаляем совсем
											models[j].typ = -1;
											break;						
										}
									} else {
										models[j].on=1;
									}
                                                                }
							}
							//md->typ=-1; //я тут изменил 8)
						}
                                        }
					break;

				case m_copy:
					if ( md->on ) {
						//md->typ=-1;
						md->on=0;
						for( y1=0; y1<=md->ly;y1+=16 )
						for( x1=0; x1<=md->lx;x1+=16 ) {
								check(md->x+x1,md->y+y1);
								t=*poslab; t0=*poslal;
								check(md->x2+x1,md->y2+y1);
								*poslab=t; *poslal=t0;

						}
						addboom((md->x2&0xfff0)+((md->lx+16)&0xfff0)/2-16,(md->y2&0xfff0)+((md->ly+16)&0xfff0)/2-16,
						32,32,0,1,b_boom,obloka+(10*256+32*32*5));
						SE(sfx_magic);
			                }
					break;

				case m_item:
					if ( md->on ) {
						GpTransBlt(NULL,&gpDraw[nflip],
							md->x-r_x,md->y-r_y,16,16,(unsigned char*)additems+512*md->lx+((waitboom&2)?256:0),
							0,0,
							16,16,
							0);
						//PutCMas(md->x-r_x+16,md->y-r_y+16,16,16,(block)additems+512*md->lx+((waitboom&2)?256:0));
        					if ( man.x+(man.dwx1+man.dwx2)/2>=md->x && man.x+(man.dwx1+man.dwx2)/2<=md->x+16
		        				&&  man.y<=md->y+32 && man.y>=md->y ) {
							switch ( md->lx ) {
								case 0:
									++continues;
									break;
								case 6:
									shields=1;
									break;
#ifndef DEMO
								case 1:
									--maxjump;
									break;
								case 2:
									shboots=1;
									break;
								case 3:
									jetpack=1;
									break;
								case 4:
									glass=1;
									break;
								case 5:
									kolun=1;
									break;
#endif
							}
							addboom(md->x,md->y+4,16,8,2,-2,b_buh,obloka+256*10+32*32*10+128*1);
							addboom(md->x,md->y+4,16,8,0,-3,b_buh,obloka+256*10+32*32*10+128*1);
							addboom(md->x,md->y+4,16,8,-2,-2,b_buh,obloka+256*10+32*32*10+128*1);
							score+=600;
							man.myrg=wmyrg;
							//md->typ=-1;
							md->on = 0;
							SE(sfx_item);
							put_additems();
						}
					}
					break;
				case m_mult:
					//покажем мультик
					if ( md->on ) {
						//md->typ=-1; //выключим этот объект навсегда
						md->on=0; //выключим эт объект
						if ( SizeLib(md->name0) != 320*200 )
							break;
						mult = (u8*)famemalloc(320*200+8);
						nflip = nflip?0:1;
						//закрасим фон
						Saturate(0,0,320,240, md->ly);
						while ( is_sfx_playing() ); //ждем чтобы звуки замолкли
						//modstop(); //!!

						GetLib(md->name0,(block)mult);
						//TurnIt((block)hidscr, 320, 200);

						Delay(md->lx*50 + 1);

						//1 кадр
						GpTransBlt(NULL,&gpDraw[nflip],
							20, 20, 160,100,mult,
							0,0,
							320,200,
							0);
						//Saturate(20,20,160,100, md->ly/2);
						//1 звук
						if ( md->x1 )
							SE(md->x1);
						//пауза
						Delay(md->lx*100 + 1);
						//2 кадр
						GpTransBlt(NULL,&gpDraw[nflip],
							140, 20, 160,100,mult,
							160,0,
							320,200,
							0);
						//Saturate(140,20,160,100, md->ly/3);
						//2 звук
						if ( md->y1 )
							SE(md->y1);
						//пауза
						Delay(md->lx*100 + 1);
						//3 кадр
						GpTransBlt(NULL,&gpDraw[nflip],
							20, 120, 160,100,mult,
							0,100,
							320,200,
							0);
						//Saturate(20,120,160,100, md->ly/4);
						//3 звук
						if ( md->x2 )
							SE(md->x2);
						//пауза
						Delay(md->lx*100 + 1);
						//4 кадр
						GpTransBlt(NULL,&gpDraw[nflip],
							140, 120, 160,100,mult,
							160,100,
							320,200,
							0);
						//Saturate(140,120,160,100, md->ly/5);
						//4 звук
						if ( md->y2 )
						SE(md->y2);
						//пауза
						Delay(md->lx*100 + 1);

						gp_mem_func.free(mult);
						Saturate(0,0,320,240, md->ly);
						while ( is_sfx_playing() ); //ждем чтобы звуки замолкли
						nflip = nflip?0:1;
						//пауза перед возвратом в игру
						Delay(md->lx*50+1);
					}
					break;
				case m_delete:
					if ( md->on ) {
						md->on=0; //выключим себя.. но можем и включить сами себя
						for ( j=0; j<maxmodel; ++j ) {
							if ( gp_str_func.compare(md->name0,models[j].name)==0 ) {
							//включим или выключим или удалим совсем!
								switch ( md->lx ) {
								case 0: 
									//выключаем
									models[j].on = 0;
									break;
								case 1: 
									//включаем
									models[j].on = 1;
									break;
								default: //удаляем совсем
									models[j].typ = -1;
									break;						
								}
							}
						}
					}
					break;
				case m_monster:
					if ( md->on ) {
						md->on=0; //выключим себя					
						if ( md->lx < nmonstr ) {
							//addboom(ob->x,ob->y-ob->ly,32,32,0,0,b_boom,obloka+(10*256+32*32*5));
							//addboom(kobj[i].x,kobj[i].y-ob->ly,32,32,0,0,b_boom,obloka+(10*256+32*32*5));
							for ( j=0; j<maxobj; ++j ) {
								if ( obj[j].on == 0 ) {
									makemonstr( j , md->lx , md->x, md->y);
									break;
                                                                }
							}

						}
					}
					break;

			}
		}
	}
}

void addboom(s16 x, s16 y, s16 lx, s16 ly, signed char sx, signed char sy, signed char typ, block buf) {
	//добавление объекта взрыва в очередь
	register s16 i;
	for ( i=0; i<maxboom; ++i ) {
		if ( booms[i].typ<0 ) {
			booms[i].x=x;
			booms[i].y=y;
			booms[i].lx=lx;
			booms[i].ly=ly;
			booms[i].sx=sx;
			booms[i].sy=sy;
			booms[i].typ=typ;
			booms[i].buf=buf;
			switch ( typ ) {
				case b_boom:
					booms[i].f=4;
					break;
				case b_buh:
					booms[i].f=8;
					break;
				default:
					booms[i].f=1;

			}
			return;
		}

	}
}

void doboom(void) {
	//вывод взрывов и других краткосрочн процессов
	register int i;
	static struct boom *bm;
	for ( i=0; i<maxboom; ++i ) {
		bm=&booms[i];
		switch ( bm->typ ) {
			case b_boom:	//взрыв на месте
				GpTransBlt(NULL,&gpDraw[nflip],
					bm->x-r_x,bm->y-r_y,bm->lx,bm->ly,(unsigned char*)bm->buf+bm->f*bm->lx*bm->ly,
					0,0,
					bm->lx,bm->ly,
					0);					
				//PutCMas(bm->x-r_x+16,bm->y-r_y+16,bm->lx,bm->ly,bm->buf+bm->f*bm->lx*bm->ly);
				if (waitboom&1) {
					if (--bm->f<0)
						bm->typ=-1;
				}
				bm->x+=bm->sx;
				bm->y+=bm->sy;
				break;
			case b_buh:	//нет смены фаз
				GpTransBlt(NULL,&gpDraw[nflip],
					bm->x-r_x,bm->y-r_y,bm->lx,bm->ly,(unsigned char*)bm->buf,
					0,0,
					bm->lx,bm->ly,
					0);					
				//PutCMas(bm->x-r_x+16,bm->y-r_y+16,bm->lx,bm->ly,bm->buf);
				if (waitboom&1) {
					if (--bm->f<0)
						bm->typ=-1;
				}
				bm->x+=bm->sx;
				bm->y+=bm->sy;
				break;
			case b_crash:	//разлет кусков
			case b_fall:	//опадение кусков
				GpTransBlt(NULL,&gpDraw[nflip],
					bm->x-r_x,bm->y-r_y,bm->lx,bm->ly,(unsigned char*)bm->buf,
					0,0,
					bm->lx,bm->ly,
					0);								
				//PutCMas(bm->x-r_x+16,bm->y-r_y+16,bm->lx,bm->ly,bm->buf);
				if (waitboom&1) {
					if ( ++bm->sy>16 || bm->y>siz_ylev*16 ) {
						bm->typ=-1;
					}
				}
				bm->x+=bm->sx;
				bm->y+=bm->sy;
				break;

		}

	}
	waitboom++;
}

int makemonstr(s16 i, s16 n, u16 x,u16 y) {
	struct object *ob;
	//номер монстра, тип монстра, нач коорд монстра
	if ( i<maxobj && n<maxmonstr && n<nmonstr) {
		ob=&obj[i];
		ob->x=x;
		ob->y=y;
		ob->typ=monsters[n].typ;
		ob->n=n;

		ob->dsx=ob->dsy=
		ob->myrg=ob->napr=ob->f=ob->sy=ob->sx=0;

		ob->lx=monsters[n].lx;
		ob->ly=monsters[n].ly;
		ob->lfx=monsters[n].lfx; ob->rtx=monsters[n].rtx;
		ob->lfy=monsters[n].lfy; ob->rty=monsters[n].rty;
		ob->upx=monsters[n].upx; ob->upy=monsters[n].upy;
		ob->dwx=monsters[n].dwx; ob->dwy=monsters[n].dwy;
		ob->on=monsters[n].lives;
		ob->s=s_down;

		if ( ob->typ>=16 ) { //босса инициализируем
			mm_stat = mm_think;
			mm_delay = 20;
		}

		return i;
	}
	return 0;
}


//использовать МОДЕЛЬ, при наж кнопке вверх
void actmodel(void) {
	register s16 i,j;
	if (man.myrg==0) //если мужик не в шоке, то
	for ( i=0; i<maxmodel; ++i ) {
		//проверка на соприкосновение с моделью
		if ( models[i].typ>=0 ) {
			if ( man.x+(man.upx1+man.upx2)/2>=models[i].x && man.x+(man.upx1+man.upx2)/2<=models[i].x+models[i].lx
				&&  man.y-man.upy<models[i].y+models[i].ly && man.y-man.upy>models[i].y ) {
				if( models[i].typ>=0 && models[i].on ) {
					if ( gp_str_func.compare("exit",models[i].name0)==0 ) {
						//конец уровня!!!!
						man.s=s_end;
						end=1;
						//т.к. выод то пока!
						break;
					} else
					if ( gp_str_func.compare("boss",models[i].name0)==0 ) {
						//BOSS уровня!!!!
						boss=1;
						modsetup("boss.fms", 4, 0 , 0, 0, 0 );
						//addmess("The BOSS of the Level",40);
					     	addmess(textindex[16],40);

						man.myrg=wmyrg;
					}
				}
				switch ( models[i].typ ) {
				case m_door:
				case m_warp:
					//ДВЕРЬ
					if ( models[i].on ) {
						//телепортация
						man.x=models[i].x2;
						man.y=models[i].y2;
						man.sx=man.sy=man.dsx1=man.dsx2=0;
						man.s=s_ouch;
						SE(sfx_teleport);
						addboom(man.x,man.y-man.ly,32,32,0,0,b_boom,obloka+(10*256+32*32*5));
						man.myrg=wmyrg;
					}
					break;

				case m_button:
					//КНОПКА
					if ( models[i].on ) {
						for ( j=0; j<maxmodel; ++j ) {
							if ( gp_str_func.compare(models[i].name0,models[j].name)==0 ) {
								models[j].on=1;
								models[i].on=0;
								man.myrg=wmyrg;
								//addmess("Pop",30);
								SE(sfx_on);
							     	addmess(textindex[17],30);
							}
						}
					}
					break;

				case m_switch:
					//ПЕРЕКЛ
					if ( models[i].on ) {
						models[i].on=0;
						SE(sfx_off);
					} else {
						models[i].on=1;
						SE(sfx_on);
					}
					for ( j=0; j<maxmodel; ++j ) {
						if ( gp_str_func.compare(models[i].name0,models[j].name)==0 ) {
							models[j].on=models[i].on;
							man.myrg=wmyrg;
							//addmess("Click",30);
						     	addmess(textindex[17],30);
						}
					}
					break;

				case m_keyhole:
					//ЗАМОК
					if ( models[i].on && keys>0 ) {
						--keys;
						for ( j=0; j<maxmodel; ++j ) {
							if ( gp_str_func.compare(models[i].name0,models[j].name)==0 ) {
								models[j].on=1;
								models[i].on=0;
								man.myrg=wmyrg;
								//addmess("Crack",30);
							     	addmess(textindex[18],30);
								SE(sfx_lock);
							}
						}
					}
					break;

				case m_shop:
					//магазин
					doshop(fatoi(models[i].name0));
					man.myrg=wmyrg;
					break;
				}
			}
		}
	}
}

void open_exit(void) {
	register s16 j;
	for ( j=0; j<maxmodel; ++j ) {
		if ( gp_str_func.compare("exit",models[j].name)==0 ) {
			models[j].on=1;
			man.myrg=wmyrg;
			//addmess("The Exit is Open",60);
		     	addmess(textindex[15],60);
			SE(sfx_magic);
		}
	}
}

void doobj(void) {
	//поведение врагов
	register int oldx,oldy,i,j;
	register s16 dw,lf,rt,up_;
	static struct object *ob;

	for ( i=0; i<maxobj; ++i ) {
		if ( obj[i].on!=0 ) {
			//если активен, то изучаем
			ob=&obj[i];
			oldx=ob->x; oldy=ob->y;

			//пр-ка на выход за границу лабиринта
			if (ob->x<0) {
				ob->x=0;
				ob->sx=monsters[ob->n].speed;
			}
			if (ob->x>siz_xlev*16-ob->lx) {
				ob->x=siz_xlev*16-ob->lx-1;
				ob->sx=-monsters[ob->n].speed;
			}
			if (ob->y<=ob->ly) {
				ob->sy=monsters[ob->n].speed;
				ob->y=ob->ly;
			}
			if (ob->y>siz_ylev*16) {
				ob->sy=-monsters[ob->n].speed;
				ob->y=siz_ylev*16;
			}

			//прибавим ускорения по осям
			ob->x+=(ob->sx+ob->dsx);
			ob->y+=(ob->sy+ob->dsy);

			//что справа, слева и вверху?
			lf=check(ob->x+ob->lfx,ob->y-ob->lfy);
			rt=check(ob->x+ob->rtx,ob->y-ob->rty);
			up_=check(ob->x+ob->upx,ob->y-ob->upy);
			//узнаем что под нами
			dw=check(ob->x+ob->dwx,ob->y-ob->dwy);
			if ( dw==23 ) {
				dw=0;
			}
			//для корр. столкновений
			if ( /*(lf>16 && lf<=18) ||*/ lf == 24 ) { //!!! для столкн. со стенами
				lf=16;
			}
			if ( /*(rt>16 && rt<=18) ||*/ rt == 24 ) { //!!! для столкн. со стенами
				rt=16;
			}
			if ( up_==24 ) { //!!! для стены сверху. но непроходимой
				up_=16;
			}
			if ( ob->typ==3 || ob->typ==6 || ob->typ==8 || ob->typ==19 || ob->typ==22 ) //летун
			{
				ob->s=s_fly;
			}

			if (++ob->f0>6-abs(ob->sx+ob->dsx)) {
				ob->f0=0;
				ob->f++;
			}

			if ( (ob->s==s_jump || ob->s==s_down) && monsters[ob->n].maxjm && abs(ob->sy)>2) {
				//прыгает
				ob->buf=monsters[ob->n].jmspr;
			} else
			if ( (ob->sx+ob->dsx!=0 && ob->x!=oldx) || (monsters[ob->n].speed == 0 && monsters[ob->n].maxst <= 0 ) ) {
				//идет
				if ( ob->f>=monsters[ob->n].maxgo ) ob->f=0;
				ob->buf=monsters[ob->n].gospr+ob->f*ob->lx*ob->ly;
			} else {
				//стоит
				if ( ob->f>=monsters[ob->n].maxst ) ob->f=0;
				ob->buf=monsters[ob->n].stspr+ob->f*ob->lx*ob->ly;
				//ob->buf=monsters[ob->n].stspr;
			}
			if (ob->sx>0) { //поворачиваем спрайт в зависимости от направления
				ob->napr=1;
			} else if ( ob->sx<0 ){
				ob->napr=0;
			}

			//столкновение со стенами
			if ( (lf==16 && ob->sx<0 )|| (rt==16 && ob->sx>0)) {
				if ( lf ) {
					ob->sx=0; //monsters[ob->n].speed;
					ob->x=oldx;
				}
				if ( rt ) {
					ob->sx=0; //-monsters[ob->n].speed;
					ob->x=oldx;
				}
			}

			if ( ob->myrg<=0 ) //если не моргает
			for ( j=0; j<maxbullet; ++j ) {	//столкновение с пулями и блоками
				if ( (bullets[j].typ==1 || bullets[j].typ>=8) && bullets[j].x>=ob->x && bullets[j].x<=ob->x+ob->lx &&
					bullets[j].y<=ob->y && bullets[j].y>=ob->y-ob->ly
				) {
					bullets[j].typ=-1;
					addboom(bullets[j].x-8,bullets[j].y-8,16,16,0,0,b_boom,obloka);
					ob->myrg=16;
					SE(sfx_pain);	//!!!
					if ( --ob->on<=0 ) {
						if ( ob->typ>=16 ) {
							//если убит БОСС!!!!!!!
							boss=0;
							//включаем нормальную музыку
							modsetup(leveldef.musicname, 4, 0, 0, 0, 0);
							SE(sfx_crash);
							//!! вроде исправил
							for ( i=0; i<maxbullet; ++i ) {
								bullets[i].typ = -1;
							}
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,-2,-2,3,items);	//монетка
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,2,-2,3,items);	//монетка
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,-1,-3,3,items);	//монетка
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,1,-3,3,items);	//монетка
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,0,-4,3,items);	//монетка
							score+=5000;
							coins+=30;
							open_exit();
							addboom(ob->x,ob->y-ob->ly,ob->lx,ob->ly,bullets[j].sx,-random(5),b_fall,ob->buf);
							addboom(ob->x,ob->y-ob->ly,16,8,0,-2,b_buh,obloka+256*10+32*32*10+128*4);
							for ( i=0; i<maxobj; ++i ) {
								ob=&obj[i];
								ob->on=0;
							}
							return;

						} else {
							SE(sfx_kill);
							addboom(ob->x,ob->y-ob->ly,16,8,0,-1,b_buh,obloka+256*10+32*32*10);
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,0,-3,3,items);	//Монетка
							score+=100;
						}
						ob->on=0;
						addboom(ob->x,ob->y-ob->ly,16,16,0,-8,b_fall,obloka+256*5*5+random(5)*256);
						addboom(ob->x,ob->y-ob->ly,ob->lx,ob->ly,bullets[j].sx,-random(5),b_fall,ob->buf);
						addboom(ob->x+ob->lx/2-16,ob->y-ob->ly/2-16,32,32,0,0,b_boom,obloka+(10*256+32*32*5));
					}
				}
			}
			if ( ob->s==s_fly ) {
				//полет
				if ( up_ && up_!=23) {
					ob->sy=max(monsters[ob->n].speed,1);
				}
				if ( dw && dw!=23) {
					ob->sy=-max(monsters[ob->n].speed,1);
				}
				if ( lf && lf!=23) {
					ob->sx=monsters[ob->n].speed;
				}
				if ( rt && rt!=23 ) {
					ob->sx=-monsters[ob->n].speed;
				}
			} else
			//проверка на пропасть
			//падение
			if ( ob->s==s_down ) {
				if(ob->sy<7) ++ob->sy;
				//if (dw!=0 && (dw1!=0 || dw2!=0)) {
				if (dw!=0) {
					if ( dw==31 ) {
						//на шипах помрем!
						if ( ob->myrg==0 ) {
							--ob->on;
							ob->myrg=16;
						}
					}
					//земля под ногами
					ob->s=s_go;
					ob->sy=0;
					if ( poc[dw][sx]>=0 ) {
						ob->y=(ob->y&0xfff0)+poc[dw][sx];
					} else {
						ob->y-=16;
						dw=check(ob->x+ob->dwx,ob->y-ob->dwy);
						ob->y=(ob->y&0xfff0)+poc[dw][sx];
					}
				} else {
					//падаем - используем сец. спрайт для падения
					if (monsters[ob->n].maxspec)
						ob->buf=monsters[ob->n].specspr;
				}
			} else
			//прыжок
			if ( ob->s==s_jump) {
				if ( up_==16 ) { //удар о потолок
						ob->s=s_down;
						ob->sy=0;
				} else
				if ( ob->y<=ob->ly || ob->y<=0) { //выход за экран
					ob->s=s_down;
					ob->y=ob->ly;
					ob->sy=4;
				} else
				if (++ob->sy>=0) {
					ob->s=s_down;
					ob->sy=0;
					ob->dsx=0;
				}
			} else {
				//норма
				if ( ob->s==s_go ) {
					if ( dw==0 ) {
						ob->s=s_down;
						ob->sy=2;
						//ob->dsx=ob->dsx/2;
						ob->dsy=0;
					} else if ( dw==31 ) {
						//на шипах помрем!
						if ( ob->myrg==0 ) {
							ob->myrg=16;
							if (--ob->on<=0) {
								//монстр возрождается после шипов
								//SE(sfx_throw);
								addboom(ob->x,ob->y-ob->ly,32,32,0,0,b_boom,obloka+(10*256+32*32*5));
								addboom(kobj[i].x,kobj[i].y-ob->ly,32,32,0,0,b_boom,obloka+(10*256+32*32*5));
								makemonstr(i,kobj[i].n,kobj[i].x,kobj[i].y);
								break;
							}
						}
					} else {
						ob->sy=0;
						//прибавим от рельефа скорость движения

						//выравнивание по рельефу
						if ( poc[dw][sx]>=0 ) {
							ob->y=(ob->y&0xfff0)+poc[dw][sx];
						} else {
							ob->y-=16;
							dw=check(ob->x+ob->dwx,ob->y-ob->dwy);
							ob->y=(ob->y&0xfff0)+poc[dw][sx];
						}
						ob->dsx=nappoc[dw]/2;
					}
				}
			}

			if ( ob->typ>=16 && !boss) {
				continue;
			}

			if (ob->s==s_go || ob->s==s_fly) {
				switch ( ob->typ ) {
					case 7: //бродяга от ст до ст - не падает в пропасть
						if (dw == 0) {
							ob->x=oldx - ob->sx - ob->sx;
							ob->y=oldy;
							ob->sx = -ob->sx;
							ob->sy = -ob->sy;
							ob->dsy = ob->dsx = ob->f = 0;
							ob->buf=monsters[ob->n].stspr;
							break;
						}
					case 0: //бродяга от стены до стены
						if( lf==16 && ob->sx<0 ) {
							ob->f=ob->sx=0;
							ob->buf=monsters[ob->n].stspr;
							break;
						}
						if( rt==16 && ob->sx>0 ) {
							ob->f=ob->sx=0;
							ob->buf=monsters[ob->n].stspr;
							break;
						}
						if (ob->sx==0 || (ob->sx+ob->dsx)==0) {
							if (lf==16) {
								ob->sx=monsters[ob->n].speed;
							} else if (rt==16) {
								ob->sx=-monsters[ob->n].speed;
							} else {
								ob->sx=monsters[ob->n].speed-random(monsters[ob->n].speed*2);
							}
						}
						break;

					case 1: //умный преследователь
						if ( dw==0 ) {	//внизу пропасть!
							ob->x=oldx;
							ob->y=oldy;
							ob->f=ob->sy=ob->sx=0;
							ob->buf=monsters[ob->n].stspr;
							break;
						}
						if (ob->x>man.x) {
							if ( ob->sx>-monsters[ob->n].speed )
								--ob->sx;
						}
						else if (ob->x<man.x) {
							if ( ob->sx<monsters[ob->n].speed )
								++ob->sx;
						}
						else ob->sx/=2;
						break;

					case 2: //прыгун
						if ( dw ) {
							ob->sy=-random(10);
							ob->s=s_jump;
							ob->napr=(man.x<ob->x)?0:1;
						}
						break;


					case 8: //летун от стены до стены ( -1 HP за удар)
						if (lf || rt || dw || up_ ) {
							ob->myrg = 4;
							ob->on--;
						}
					case 3: //летун от стены до стены
						if ( ob->sx==0 ) {
							if (lf) ob->sx=monsters[ob->n].speed;
							else if (rt) ob->sx=-monsters[ob->n].speed;
							else ob->sx=monsters[ob->n].speed-random(monsters[ob->n].speed*2);
						}
						break;

					case 4: //падун от потолка до пола СНАРЯД и т.п.
						if ( dw ) {
							ob->on=0;
							addboom(ob->x+ob->lx/2-8,ob->y-ob->ly/2-8,16,16,0,0,b_boom,obloka);
						}
						break;

					case 5: //прыгун от стены до стены
						if ((lf==16 || rt==16) && ob->sx)
							ob->sx=0;
						else if ( ob->sx+ob->dsx==0) {
							if (lf==16) {
								ob->sx=monsters[ob->n].speed;
							} else if (rt==16) {
								ob->sx=-monsters[ob->n].speed;
							} else ob->sx=monsters[ob->n].speed-random(monsters[ob->n].speed*2);
						}
						if ( dw ) {
							ob->sy=-3-random(3);
							ob->s=s_jump;
						}
						break;

					case 6: //летающий умный преследователь
						if (ob->x>man.x) {
							if ( ob->sx>-monsters[ob->n].speed )
								--ob->sx;
						} else if (ob->x<man.x) {
							if ( ob->sx<monsters[ob->n].speed )
								++ob->sx;
						} else ob->sx=0;

						if (ob->y>man.y-20) {
							if ( ob->sy>-max(monsters[ob->n].speed,1) )
								--ob->sy;
						} else if (ob->y<man.y-20) {
							if ( ob->sy<max(monsters[ob->n].speed,1) )
								++ob->sy;
						} else ob->sy=0;
						break;

					case 16: //BOSS N2
						//if( boss==0 )
						//	break;
						if (ob->x+ob->lx/2 > man.x+man.lx/2) {	//-16
							if ( ob->sx>-monsters[ob->n].speed )
								--ob->sx;
						}
						else if (ob->x+ob->lx/2 < man.x+man.lx/2) {
							if ( ob->sx<monsters[ob->n].speed )
								++ob->sx;
						} else {
							ob->sx=0;
						}
                        			if ((waitboom&31)==0) {
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly,(man.x<ob->x)?-5:((man.x>ob->x)?5:0),ob->y>man.y+16?-3:0, 2,enbull);
							SE(sfx_throw);
						}

						if (oldx==ob->x/*ob->y>=man.y*/ && (lf || rt) && dw) {
							ob->sy=-8;
							ob->s=s_jump;
							SE(sfx_jump);
						}
						break;

					case 17: //BOSS N3
						//if( boss==0 )
						//	break;
						if (ob->x > man.x) {
							if ( ob->sx>-monsters[ob->n].speed )
								--ob->sx;
						}
						else if (ob->x < man.x-16) {
							if ( ob->sx<monsters[ob->n].speed )
								++ob->sx;
						} else {
							if ( man.y<=ob->y ) {
								//вражье семя!
								if ((waitboom&7)==0) {	//16*14
									addbullet(man.x+8,ob->y-16*15,man.sx,2+random(2), 2,enbull+256*8);
									//addbullet(man.x+8,ob->y-16*14,man.sx,4, 2,items+256*1*8);
									SE(sfx_death);
									//мини прыжок
									ob->sy=-2;
									ob->s=s_jump;
								}
							}
							ob->sx=0;
						}
                        			if ((waitboom&15)==0) {
							SE(sfx_throw);
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,(man.x<ob->x)?-5:((man.x>ob->x)?5:0),ob->y>man.y+16?-5:0, 2,enbull);
						}
						break;

					case 18: //BOSS N1
						//if( boss==0 )
						//	break;
						if (ob->x > man.x+8) {
							if ( ob->sx>-monsters[ob->n].speed )
								--ob->sx;
						}
						else if (ob->x < man.x-8) {
							if ( ob->sx<monsters[ob->n].speed )
								++ob->sx;
						} else {
							ob->sy=-8;
							ob->s=s_jump;
							ob->sx=0;
							SE(sfx_jump);
						}
						break;

					case 19: //BOSS N4 летающий
						//if( boss==0 )
						//	break;
						if (ob->x+ob->lx/2-2>man.x+man.lx/2) {
							if ( ob->sx>-monsters[ob->n].speed && ob->sy<0)
								--ob->sx;
						}
						else if (ob->x+ob->lx/2+2<man.x+man.lx/2) {
							if ( ob->sx<monsters[ob->n].speed && ob->sy<0)
								++ob->sx;
						} else {
                        				if ((waitboom&3)==0) {
								addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,man.sx,2+random(2), 2,enbull+256*8*2);
								SE(sfx_throw);
							}
							if ( ob->sy>-4 ) {
								ob->sy--;
								ob->sx=0;
							}
						}
						if (ob->y>man.y-man.ly ) {
							ob->sy--;
						}
						if ( up ) {
							ob->sy++;
						}
						break;

					case 21: //boss N6
						//if( boss==0 )
						//	break;
						if (ob->x>man.x) {
							if ( ob->sx>-monsters[ob->n].speed )
								--ob->sx;
							if( lf==16 ) {
								ob->sx=-monsters[ob->n].speed;
								ob->sy=-8;
								ob->s=s_jump;
								SE(sfx_jump);
							}
						}
						else if (ob->x<man.x-21) {
							if ( ob->sx<monsters[ob->n].speed )
								++ob->sx;
							if( rt==16 ) {
								ob->sx=monsters[ob->n].speed;
								ob->sy=-8;
								ob->s=s_jump;
								SE(sfx_jump);
							}
						} else if (man.sx==0) {
								ob->sy=-8;
								ob->s=s_jump;
								SE(sfx_jump);
						}
						//вражье семя!
						if ((waitboom&15)==0) {
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly,man.x>ob->x?5:-5,-2+random(4),2,fon+(176*256));
							SE(sfx_throw);
						}
						break;

					case 20: //босс 7Й - робот большой
						//if( boss==0 )
						//	break;
						if (ob->x > man.x+man.lx) {
							if ( ob->sx>-monsters[ob->n].speed )
								--ob->sx;
						}
						else if (ob->x < man.x - ob->lx) {
							if ( ob->sx<monsters[ob->n].speed )
								++ob->sx;
						} else {
							if ((waitboom&31)==0) {
								ob->sy=-4;
								ob->s=s_jump;
								SE(sfx_jump);
							}
						}
						if( lf==16 ) {
							ob->sx=monsters[ob->n].speed;
							ob->sy=-8;
							ob->s=s_jump;
							SE(sfx_jump);
						} else
						if( rt==16 ) {
							ob->sx=-monsters[ob->n].speed;
							ob->sy=-8;
							ob->s=s_jump;
							SE(sfx_jump);
						}
						//вражье семя!
						if ((waitboom&15)==0) {
							addbullet(ob->x+ob->lx/2,ob->y-ob->ly,man.x>ob->x?11:-11,3+random(3),2,enbull+256*8*3);
							SE(sfx_throw);
						}
						break;

					case 22: //BOSS N8 пират летающий (плюется головами мишек)
						//enum {mm_none, mm_think, mm_aim, mm_flow, mm_fire};
						switch ( mm_stat) {
						case mm_think:
							//if ( ob->sy < 2 ) {
							//	ob->sy++;
							//}
/*							if ( ob->sy>-4 && ob->y<man.y-50) {
								ob->sy--;
								//ob->sx=0;
							} else if ( ob->sy< 4 ) {
								ob->sy++;
							}
							if ( ob->x+ob->lx/2-32 > man.x+man.lx/2) {
								if ( ob->sx>-monsters[ob->n].speed/2 && ob->sy<0)
									--ob->sx;
							}
							else if (ob->x+ob->lx/2+32<man.x+man.lx/2) {
								if ( ob->sx<monsters[ob->n].speed/2 && ob->sy<0)
									++ob->sx;
							}
*/
							break;
						case mm_flow:
							//if ( ob->sy>-4 && ob->y<man.y-150) {
							//	ob->sy--;
								//ob->sx=0;
							//}
							if ( ob->x+ob->lx/2-32 > man.x+man.lx/2 && (waitboom&15)==0 ) {
								if ( ob->sx > -1 )
									ob->sx = -1;
							}
							else if (ob->x+ob->lx/2+32<man.x+man.lx/2 && (waitboom&15)==0) {
								if ( ob->sx < 1 )
									ob->sx = 1;
							}
							break;
						case mm_aim:
							if ( ob->x+ob->lx/2-4 > man.x+man.lx/2) {
								if ( ob->sx>-monsters[ob->n].speed )
									--ob->sx;
							}
							else if (ob->x+ob->lx/2+4<man.x+man.lx/2) {
								if ( ob->sx<monsters[ob->n].speed )
									++ob->sx;
							} else {
								if (ob->sx >0)
									ob->sx--;
								if (ob->sx <0)
									ob->sx++;
							}

							//if ( ob->x == oldx && (lf || rt) ) {
							//	ob->sy--;
							//} else
							if ( ob->sy>-4 && ob->y > man.y-40 /*&& (waitboom&7)==0*/ ) {
								//&& (waitboom&15)==0)
								ob->sy--;
							} else
							if ( ob->sy < 4 && ob->y < man.y-90 /*&& (waitboom&7)==0*/ ) {
								ob->sy++;
							} else {
								if (ob->sy >0)
									ob->sy--;
								if (ob->sy <0)
									ob->sy++;
							}
							break;
						case mm_fire:
							if ( ob->x+ob->lx/2-2 > man.x+man.lx/2) {
								ob->sx = -1;
							}
							else if (ob->x+ob->lx/2+2<man.x+man.lx/2) {
								ob->sx = 1;
							}

							if ( ob->sy>-2 && ob->y > man.y-60 /*&& (waitboom&7)==0*/ ) {
								//&& (waitboom&15)==0)
								ob->sy--;
							} else
							if ( ob->sy < 2 && ob->y < man.y-100 /*&& (waitboom&7)==0*/ ) {
								ob->sy++;
							} else {
								if (ob->sy >0)
									ob->sy--;
								if (ob->sy <0)
									ob->sy++;
							}

							//if ( ob->x+ob->lx/2-132 > man.x+man.lx/2) {
							//	if ( ob->sx>-1 )
							//		ob->sx = -1;
							//}
							//else if (ob->x+ob->lx/2+132<man.x+man.lx/2) {
							//	if ( ob->sx<1 )
							//		ob->sx = 1;
							//}
							ob->buf=monsters[ob->n].specspr;
                        				if ((waitboom&31)==0) {						
								//addbullet(ob->x+ob->lx/2,ob->y-ob->ly/2,man.sx,2+random(2), 2,enbull+256*8*2);
								for ( j=0; j<maxobj; ++j ) {
									if ( obj[j].on == 0 ) {
										// ob->n+1 = след в списке монстр
										makemonstr( j , ob->n+1, ob->x+40, ob->y-ob->ly/3);
										obj[j].sx = ( (ob->x+ob->lx/2-2 > man.x+man.lx/2)?-3:3 );
										obj[j].sy = 5;
										break;
                                	                                }
								}
								SE(sfx_error);
							}
							break;
						case mm_none:
						default:
							break;
						}
						if ( --mm_delay <=0 ) {
							//выберем другое поведение
							mm_delay = (j = random(100))+50;
							switch ( mm_stat) {
							case mm_think:
								if (j <50) {
									mm_stat = mm_aim;
									mm_delay = 100+random(20);
								} else {
									mm_stat = mm_fire;
									mm_delay = 100+random(32);
								}
								break;
							case mm_flow:
								mm_stat = mm_aim;
								break;
							case mm_aim:
								mm_delay = random(50)+50;
								mm_stat = mm_think;
								break;
							case mm_fire:
								mm_stat = mm_flow;
								break;
							case mm_none:
							default:
								mm_stat = mm_think;
								break;
							}
						}
						//конец 22го босса				
						break;


					default:
						break;
				}
			}

			if (ob->myrg>0) --ob->myrg;
			if ((ob->myrg&2)==0) {
				if ( ob->napr ) {
					//PutCMasr(ob->x-r_x+16,ob->y-r_y+16-ob->ly,ob->lx,ob->ly,ob->buf);
					GpTransLRBlt(NULL,&gpDraw[nflip],
						ob->x-r_x,ob->y-r_y-ob->ly,ob->lx,ob->ly,(unsigned char*)ob->buf,
						0,0,
						ob->lx,ob->ly,
						0);		
				} else {
					//PutCMas(ob->x-r_x+16,ob->y-r_y+16-ob->ly,ob->lx,ob->ly,ob->buf);
					GpTransBlt(NULL,&gpDraw[nflip],
						ob->x-r_x,ob->y-r_y-ob->ly,ob->lx,ob->ly,(unsigned char*)ob->buf,
						0,0,
						ob->lx,ob->ly,
						0);								
				}
			} else {
				CurrentColor=33;
				if ( ob->napr ) {
					PutCSaturr(ob->x-r_x,ob->y-r_y-ob->ly,ob->lx,ob->ly,ob->buf,39);
				} else {
					PutCSatur(ob->x-r_x,ob->y-r_y-ob->ly,ob->lx,ob->ly,ob->buf,39);
				}
			}
			if ( boss && ob->typ>=16 ) {
				// если BOSS - то покажем его полоску жизней
				//CurrentColor=(32+(ob->myrg&1));
				//Bar(24,30, max((s16)ob->on*8,1) ,8);
				CurrentColor=(45+(ob->myrg&1));
				Saturate(30-16, 24-16, 8, max((s16)ob->on*6,1) , CurrentColor);
				//PutMas16(24+max((s16)ob->on*8,1),26,items+256*8*2+((waitboom&7)*256));
				GpTransBlt(NULL,&gpDraw[nflip],
					26-16,24-16+max((s16)ob->on*6,1),16,16,(unsigned char*)items+256*8*2+(waitboom&7)*256,
					0,0,
					16,16,
					0);	
			}
		}
	}
}
