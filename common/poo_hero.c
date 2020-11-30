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

//герой
extern struct hero man;
//враги
extern struct object obj[maxobj];
//extern struct strkobj kobj[maxobj];
//размер уровня
extern u16 siz_xlev;
extern u16 siz_ylev;
extern u32 siz_level; //=siz_xlev*siz_ylev;
extern u32 svel,cvel;
extern int /*signed char*/ poc[32][16];
extern signed char nappoc[32]; //папр и смещен
extern unsigned char tekblock;
extern s16 x,y,sx,sy; //тек положение точки
extern char waitboom;
extern s16 end,pause,key_f,key_b;
extern s16 left,right,up,down,jump,fire;
extern s16 t_left,t_right,t_up,t_down,t_jump,t_fire;
extern block obloka; //под взрывы
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
extern struct strbullets bullets[maxbullet];
extern struct strmodels models[maxmodel];
extern struct levdef leveldef;
extern unsigned char *poslab; //тек поз точки в массиве трассы
extern unsigned char *poslal; //тек поз точки в массиве фона
extern unsigned char tekblock;
extern block fon;
extern block items;
extern block additems;
extern u16 r_x,r_y; //прежние координаты лев. верхн угла окна

extern int keydata;
extern block textindex[200]; //ссылки на строки текста 0,1...

void doman() { //управление Героем
	register s16 oldx,oldy,i; //сохр стар коорд
	register s16 dw1,dw2,lf1,lf2,rt1,rt2,up1,up2; //коды трассы
	register s16 dwi1,dwi2,lfi1,lfi2,rti1,rti2,upi1,upi2; //коды вещей
	register s16 sx1,sx2;

	oldx=man.x; oldy=man.y;

	//прибавим ускорения по осям
	man.x+=(man.sx+(man.dsx2+man.dsx1)/2); //max!!!!
	man.y+=(man.sy+man.dsy);

	//пр-ка на выход за границу лабиринта
	if (man.x<0) man.x=0;
	if (man.x>siz_xlev*16-man.lx) man.x=siz_xlev*16-man.lx-1;
	if (man.y<man.ly) man.y=man.ly;

	//что справа, слева и вверху?
	lf1=check(man.x+man.lfx,man.y-man.lfy1);
	lfi1=tekblock;
	lf2=check(man.x+man.lfx,man.y-man.lfy2);
	lfi2=tekblock;
	rt1=check(man.x+man.rtx,man.y-man.rty1);
	rti1=tekblock;
	rt2=check(man.x+man.rtx,man.y-man.rty2);
	rti2=tekblock;
	up1=check(man.x+man.upx1,man.y-man.upy);
	upi1=tekblock;
	up2=check(man.x+man.upx2,man.y-man.upy);
	upi2=tekblock;
	//узнаем что под нами
	dw1=checkdw(man.x+man.dwx1,man.y-man.dwy);
	dwi1=tekblock; sx1=sx;
	dw2=checkdw(man.x+man.dwx2,man.y-man.dwy);
	dwi2=tekblock; sx2=sx;


	//для корр. столкновений
	if ( /*(lf1>16 && lf1<=18) ||*/ lf1 == 24 ) { //!!! для столкн. со стенами
		lf1=16;
	}
	if ( /*(rt1>16 && rt1<=18) ||*/ rt1 == 24 ) { //!!! для столкн. со стенами
		rt1=16;
	}
	//для корр. столкновений
	if ( /*(lf2>16 && lf2<=18) ||*/ lf2 == 24 ) { //!!! для столкн. со стенами
		lf2=16;
	}
	if ( /*(rt2>16 && rt2<=18) ||*/ rt2 == 24 ) { //!!! для столкн. со стенами
		rt2=16;
	}

	if ( up1==24 ) { //!!! для стены сверху. но непроходимой
		up1=16;
	}
	if ( up2==24 ) { //!!! для стены сверху. но непроходимой
		up2=16;
	}


	if (++man.f0>6-abs(man.sx+max(man.dsx1,man.dsx2))) {
		man.f0=0; man.f++;
	}

	if ( man.fw>0 )	{
		//если есть задержка в смене фаз
		--man.fw;
	} else
	switch ( man.s ) {
		case s_go:
			//идет
			if ( abs(man.sx+(man.dsx1+man.dsx2)/2)>1 ) {
				if ( man.f>=man.maxgo ) man.f=0;
				man.buf=man.gospr+man.f*man.lx*man.ly;
				man.fw=1;
			} else {
				//трясется на краю
				//!!!
				if ( dw1==0 && sx1<8 && ((dw2>15 && dw2<20) || dw2==1) ) {
					//дыра слева
					man.napr=0;
					man.buf=man.specspr+man.lx*man.ly*(1+(waitboom&1));
					man.fw=2;
					if( (waitboom&15)==0 ) {
						addboom(man.x+12,man.y-man.ly,8,8,0,-1,b_buh,obloka+256*10+32*32*10+128*5+64*2);
						SE(sfx_cursor);
					}

				} else if ( dw2==0 && sx2>8 && ((dw1>15 && dw1<20) || dw1==1) ) {
					//дыра справа
					man.napr=1;
					man.buf=man.specspr+man.lx*man.ly*(1+(waitboom&1));
					man.fw=2;
					if( (waitboom&15)==0 ) {
						addboom(man.x+12,man.y-man.ly,8,8,0,-1,b_buh,obloka+256*10+32*32*10+128*5+64*2);
						SE(sfx_cursor);
					}
				} else
				//стоит
				if ( up ) {
				//смотрит вверх
					man.buf=man.stspr+man.lx*man.ly;
				} else if ( down ) {
				//вниз
					man.buf=man.stspr+2*man.lx*man.ly;
				} else {
					//стоит
					if ((waitboom&31)!=0)
						man.buf=man.stspr;
					else {
						//моргание Плюши
						man.buf=man.specspr;
						man.fw=2;
					}
				}
				man.f=0;
			}
			break;

		case s_jump:
		case s_down:
			//прыжок
			if ( man.sy<-2 ) {
				man.f=0;
			} else if ( man.sy<2 ) {
				man.f=1; man.fw=4;
			} else {
				man.f=2;
			}
			if ( man.sy>7 ) {	//при сильном падении
				man.buf=man.stspr+3*man.lx*man.ly;
				man.fw=2;
			} else {
				//фазы для прыжка
				man.buf=man.jmspr+man.f*man.lx*man.ly;
			}
			break;

		case s_ouch:
		case s_crash:
			//кто-то укусил
			man.buf=man.stspr+3*man.lx*man.ly;
			man.fw=10;
			//man.napr=2-random(2);
			break;

		case s_fly:
			if ( abs(man.sx)<=1 ) {
				man.buf=man.specspr+man.lx*man.ly*3;
				man.fw=1;
			} else {
				man.buf=man.specspr+man.lx*man.ly*(4+(waitboom&1));
				man.fw=2;
			}
			if ( power==0 && man.sy<=0) {	//если кончилось горючее
				++man.sy;
			}
			if( man.sy<0 ) {
				//дым из топки
				addboom(man.napr?(man.x):(man.x+16),man.y-8,16,16,-man.sx,1,b_boom,obloka+256*10);
				SE(sfx_honey);
			}
			break;

		default:
			//????
			man.buf=man.stspr;
			break;
	}

	if ( man.s!=s_crash ) { //для мертвеца не надо таких проверок!

		//втискивание куда не надо
		if ( (lf1==16 || rt1==16) && dw2==16 && dw1==16 && up1==16 && up2==16 ) {
			man.x=oldx;
			man.sx=man.dsx1=man.dsx2=0;
			man.y=oldy;
			man.sy/=2;
		}

		//столкновение со стенами
		if ( ((lf1==16 || lf2==16) && man.sx<0 ) || ((rt1==16 || rt2==16) && man.sx>0)) {
			man.x=oldx;
			man.sx=man.dsx1=man.dsx2=0;
		}

		//проверка на соприкосновение с монстрами
		if ( shields && power ) {
		} else
		if (man.myrg==0 ) {//если мужик не мыргает
			for ( i=0; i<maxobj; ++i ) {
				if ( obj[i].on!=0 && obj[i].myrg==0) {
					//-- было if ( man.x+man.lx>obj[i].x && man.x<obj[i].x+obj[i].lx
					//-- &&  man.y-man.ly<obj[i].y && man.y>obj[i].y-obj[i].ly ) {
					//если монстр жив и не моргает
					if ( man.x+man.dwx2>=obj[i].x && man.x+man.dwx1<=obj[i].x+obj[i].lx
						&&  man.y-man.ly<obj[i].y && man.y>obj[i].y-obj[i].ly ) {
						//если есть шипы-убийцы
						if( shboots && man.s==s_down && man.y<obj[i].y) {
							//если падаем сверху на гада
							man.sy=-8;
							man.s=s_jump;
							SE(sfx_pain);
							//то убъем его
							if ( obj[i].typ<16 && --obj[i].on<=0 ) {
								obj[i].on=0;
								addboom(obj[i].x,obj[i].y-obj[i].ly,obj[i].lx,obj[i].ly,man.sx,random(5),b_fall,obj[i].buf);
								addboom(obj[i].x+obj[i].lx/2-16,obj[i].y-obj[i].ly/2-16,32,32,0,0,b_boom,obloka+(10*256+32*32*5));
								addboom(obj[i].x,obj[i].y-obj[i].ly,16,8,0,-1,b_buh,obloka+256*10+32*32*10);
								score+=100;
							} else {
								obj[i].myrg=8;
							}
							break;
						} else {
							//если мужик не в состоянии аффекта,
							//то в шок его!
							obj[i].myrg=9;

							man.sy=-8; man.s=s_ouch;
							man.myrg=wmyrg;
							//addmess("Ouch!",30);
							if ( --man.on<=0 ) {
								//мед кончился - капут
								man.s=s_crash;
			     					//addmess("Honey is over",60);
							     	addmess(textindex[13],60);
								SE(sfx_death);
							} else
							{
							     	addmess(textindex[14],30);
								SE(sfx_pain);
							}
							break;
						}
					}
				}
			}

			//проверка на столкновение с патронами врага
			//if (man.myrg==0 && shields==0) //если мужик не мыргает
			for ( i=0; i<maxbullet; ++i ) {
				if ( bullets[i].x>=man.x && bullets[i].x<=man.x+man.lx &&
					bullets[i].y<=man.y && bullets[i].y>=man.y-man.ly ) {
						switch ( bullets[i].typ ) {
						case 1:	//шишка
							break;
						case 2:
							bullets[i].typ=-1;
							addboom(bullets[i].x-8,bullets[i].y-8,16,16,0,0,b_boom,obloka);
							obj[i].myrg=9;

							man.sy=-4;
							man.s=s_ouch;
							man.myrg=wmyrg;
							//addmess("oY!",30);
							if ( --man.on<=0 ) {
								//мед кончился - капут
								man.s=s_crash;
								//addmess("There is no Honey Left",60);
							     	addmess(textindex[13],60);
								SE(sfx_death);
							}
							else
							{
							     	addmess(textindex[14],30);
								SE(sfx_pain);
							}
							break;
						case 3:	//монетка
							bullets[i].typ=-1;
							addboom(bullets[i].x-8,bullets[i].y-8,16,16,0,0,b_boom,obloka);
							++score;
							if ( coins<9999 ) {
								++coins;
							}
							SE(sfx_coin);
							break;
						case 4:	//бочка
							bullets[i].typ=-1;
							addboom(bullets[i].x-8,bullets[i].y-8,16,16,0,0,b_boom,obloka);
							score+=200;
							if ( man.on<5 ) {
								++man.on;
								SE(sfx_honey);
							} else {
								coins+=5;
								SE(sfx_coin);
							}
							break;
						}

				}
			}
		}
		//просмотр лифтов и пр.
		for ( i=0; i<maxmodel; ++i ) {
			if ( models[i].typ>=0 )	{
				if ( man.x+(man.dwx1+man.dwx2)/2>=models[i].x && man.x+(man.dwx1+man.dwx2)/2<=models[i].x+models[i].lx
				&&  man.y<=models[i].y+models[i].ly && man.y>=models[i].y ) {
					if ( models[i].typ==m_lift || models[i].typ==m_lift0 ) {
						//лифты
						man.y=models[i].y;
						man.dsx1=man.dsx2=models[i].sx;
						man.dsy=models[i].sy;
						if (man.s<s_lift)
							man.s=s_lift;
						break;
					}
				}
			}
		}

	}

	if ( man.myrg>0 ) { //прекращаем моргание
		--man.myrg;
	}
	if ( power>0 ) { //защита-то кончается
		--power;
	}
//enum {s_go,s_down,s_jump,s_fly,s_lift,s_ouch,s_crash,s_over,s_end};
	if (	( 
		( (dw1==31 || dw2==31) && man.s != s_jump )
		 || 
		( (up1==31 || up2==31) && man.s != s_down ) 
		)
		
		&& man.s!=s_crash ) {

		man.sy=-8;	//шипы - в шок его!
		man.s=s_crash;
		//addmess("Live -1",60);
	     	addmess(textindex[19],60);
		SE(sfx_death);
	}

	//проверка на пропасть
	//падение
	if ( man.s==s_down ) {
		if( man.sy<8 && (leveldef.typ!=t_water || (waitboom&1)==0) ) {
			++man.sy;
		}
		if (dw1!=0 || dw2!=0) {
			//земля под ногами
			man.s=s_go;

			man.sy=0;
			if ( min(poc[dw1][sx1],poc[dw2][sx2])<0 ) {
				//выравнивает на скатах
				man.y-=16;
				dw1=check(man.x+man.dwx1,man.y-man.dwy);
				dwi1=tekblock; sx1=sx;
				dw2=check(man.x+man.dwx2,man.y-man.dwy);
				dwi2=tekblock; sx2=sx;
			}
			if ( dw1 && dw2 ) {
				man.y=(man.y&0xfff0)+min(poc[dw1][sx1],poc[dw2][sx2]);
				man.dsx1=nappoc[dw1];
				man.dsx2=nappoc[dw2];
				man.sy=0;
			} else if ( dw1 ) {
				man.y=(man.y&0xfff0)+poc[dw1][sx1];
				man.dsx1=nappoc[dw1];
				man.dsx2=man.sy=0;
			} else if ( dw2 ) {
				man.y=(man.y&0xfff0)+poc[dw2][sx2];
				man.dsx2=nappoc[dw2];
				man.dsx1=man.sy=0;
			}
			if( leveldef.typ==t_ice ) {
				//скользко
				man.dsx1*=2;
				man.dsx2*=2;
			}
			man.buf=man.jmspr+3*man.lx*man.ly;
			man.fw=2;
			addboom(man.x+8,man.y-4,16,16,0,0,b_boom,obloka+16*16*5);
			SE(sfx_fall);

			if ( kolun && (down || (keydata & GPC_VK_FL) ) ) { //если есть колун
				//рисуем колун по центру чувака снизу
				//PutMas16(px_ai,20+20*4,additems+256*10 + ((times+44 & 32)?256:0) );
				if ( dw1>16 && dw1<=19 ) { //долбим колуном
					check(man.x+man.dwx1,man.y-man.dwy);
					if ( (--*poslal)<=15 ) {
						addbullet(x*16+8,y*16+8,-random(2),0,8,fon+(*poslal+1)*256);
						*poslab=*poslal=0;
					} else {
						addboom(x*16+2-random(4),y*16-16,16,16,0,1,b_crash,additems+256*10);
					}
					SE(sfx_cancel);
				} else
				if ( dw2>16 && dw2<=19 ) { //долбим колуном
					check(man.x+man.dwx2,man.y-man.dwy);
					if ( (--*poslal)<=15 ) {
						addbullet(x*16+8,y*16+8,random(2),0,8,fon+(*poslal+1)*256);
						*poslab=*poslal=0;
					} else {
						addboom(x*16+2-random(4),y*16-16,16,16,0,1,b_crash,additems+256*10);
					}
					SE(sfx_cancel);
				}
			}
		}
	} else
	//прыжок
	if ( man.s==s_jump || man.s==s_ouch ) {
		if ( dw1 || dw2 ) {
		   //прыжок в сторону наклона
		   man.dsx1=nappoc[dw1];
		   man.dsx2=nappoc[dw2];
		} //для up1
		if ( up1>=16 && up1<=19 ) { //удар о кирпичный или исчезающий потолок
				man.s=s_down;
				man.sy=1;
				check(man.x+man.upx1,man.y-man.upy);
				SE(sfx_splat);
				if ( up1!=16 && (--*poslal)<=15 ) { //если [?] то-приз
					addbullet(x*16+8,y*16+8,-1,-1,8,fon+(*poslal+1)*256);
					*poslal=*poslab=0;
					if ( up1==18 ) {
						if ( random(2) ) {
							addbullet(x*16+8,y*16,0,-1,3,items);
						} else {
							addbullet(x*16+8,y*16,0,-1,4,items+256*8);
						}
					}
					if( (waitboom&1)==0 ) {
						addboom(man.x+12,man.y-man.ly,8,8,0,-1,b_buh,obloka+256*10+32*32*10+128*5+64*3);
					}
				} else {
					addboom(x*16,y*16,16,16,0,0,b_boom,obloka+16*16*5);
				}
			// для фикса бага
			up2=check(man.x+man.upx2,man.y-man.upy);
			//upi2=tekblock;
		} /*else*/ //для up2
		if ( up2>=16 && up2<=19 ) { //удар о кирпичный или исчезающий потолок
				man.s=s_down;
				man.sy=1;
				check(man.x+man.upx2,man.y-man.upy);
				SE(sfx_splat);
				if ( up2!=16 && (--*poslal)<=15 ) { //если [?] то-приз
					addbullet(x*16+8,y*16+8,1,-1,8,fon+(*poslal+1)*256);
					*poslal=*poslab=0;
					if ( up2==18 ) {
						if ( random(2) ) {
							addbullet(x*16+8,y*16,0,-1,3,items);
						} else {
							addbullet(x*16+8,y*16,0,-1,4,items+256*8);
						}
					}
					if( (waitboom&1)==0 ) {
						addboom(man.x+12,man.y-man.ly,8,8,0,-1,b_buh,obloka+256*10+32*32*10+128*5+64*3);
					}
				} else {
					addboom(x*16,y*16,16,16,0,0,b_boom,obloka+16*16*5);
				}
		} else
		if ( man.y<=man.ly ) { //выход за экран
			man.s=s_down;
			man.y=man.ly;
			man.sy=2;
			upi1=upi2=0;
		} else
		if (++man.sy>=0) {
			man.s=s_down;
			man.sy=man.dsx1=man.dsx2=0;
		}
	} else
	if ( man.s==s_fly ) {
		//полет
		if( jump==0 && man.sy<5 ) {
			++man.sy;
		}
		if ( dw1 || dw2 ) {
			//опустился
			man.s=down;
			man.y=oldy;
		}
		if ( up1>1 || up2>1 ) { //удар о потолок
			SE(sfx_fall);
			man.s=s_down;
			man.sy=1;
		}
	} else
	if ( man.s==s_crash ) {
		//это совсем капут
		if(man.sy<32) ++man.sy;
		man.sx=man.dsx1=man.dsx2=0;
		if (man.y>siz_ylev*16) {
			//выпадение ниже лабиринта
			man.s=s_over;
			end=1;
		}
	} else //норма
	if ( man.s==s_go ) {
		man.dsy=0;
		if ( dw1==0 && dw2==0 ) {
				man.s=s_down;
				man.sy=1; //2
		} else
		if ( dw1==19 ) { //исчезающая стенка
			check(man.x+man.dwx1,man.y-man.dwy);
			if ( man.x==oldx && (times&7)==1 ) {
				if ( (--*poslal)<=15 ) {
					addbullet(x*16+8,y*16+8,-random(2),0,8,fon+(*poslal+1)*256);
					*poslab=*poslal=0;
				} else {
					addboom(x*16+2-random(4),y*16,16,16,0,0,b_boom,obloka+16*16*5);
				}
				SE(sfx_fall);
			}
			dw2=checkdw(man.x+man.dwx2,man.y-man.dwy);
			//dwi2=tekblock; sx2=sx;
		} /*else*/
		if ( dw2==19 ) { //исчезающая стенка
			check(man.x+man.dwx2,man.y-man.dwy);
			if ( man.x==oldx && (times&7)==1 ) {
				if ( (--*poslal)<=15 ) {
					addbullet(x*16+8,y*16+8,random(2),0,8,fon+(*poslal+1)*256);
					*poslab=*poslal=0;
				} else {
					addboom(x*16+2-random(4),y*16,16,16,0,0,b_boom,obloka+16*16*5);
				}
				SE(sfx_fall);
			}
		}

		if ( min(poc[dw1][sx1],poc[dw2][sx2])<0 ) {
			//если подпорка ... наверх!
			man.y-=16;
			dw1=check(man.x+man.dwx1,man.y-man.dwy);
			dwi1=tekblock; sx1=sx;
			dw2=check(man.x+man.dwx2,man.y-man.dwy);
			dwi2=tekblock; sx2=sx;
		}
		//если что-то под ногами
		if ( dw1 && dw2 ) {
			man.y=(man.y&0xfff0)+min(poc[dw1][sx1],poc[dw2][sx2]);
			man.dsx1=nappoc[dw1];
			man.dsx2=nappoc[dw2];
			man.sy=0;
		} else if ( dw1 ) {
			man.y=(man.y&0xfff0)+poc[dw1][sx1];
			man.dsx1=nappoc[dw1];
			man.dsx2=man.sy=0;
		} else if ( dw2 ) {
			man.y=(man.y&0xfff0)+poc[dw2][sx2];
			man.dsx2=nappoc[dw2];
			man.dsx1=man.sy=0;
		}
		if( leveldef.typ==t_ice ) {
			man.dsx1*=2;
			man.dsx2*=2;
		}

	} else
	if ( man.s==s_lift ) {
		man.s=s_go;
	}

	if( man.s!=s_crash ) {
		//проверка на сбор вещей
		if ( lfi1 )
			getitem(man.x+man.lfx,man.y-man.lfy1);
		else if ( rti1 )
			getitem(man.x+man.rtx,man.y-man.rty1);
		else if ( upi1 )
			getitem(man.x+man.upx1,man.y-man.upy);
		else if ( upi2 )
			getitem(man.x+man.upx2,man.y-man.upy);
		else if ( dwi1 )
			getitem(man.x+man.dwx1,man.y-man.dwy);
		else if ( lfi2 )
			getitem(man.x+man.lfx,man.y-man.lfy2);
		else if ( rti2 )
			getitem(man.x+man.rtx,man.y-man.rty2);
		else if ( dwi2 )
			getitem(man.x+man.dwx2,man.y-man.dwy);
	}

	if ( (power>32 || power&1) ) {
		//CurrentColor=20;
		//полоса энергии
		//Bar(24,20, max((s16)power>>2,1) ,8);
		Saturate(4,24-16, 8, max(power/3/*>>2*/,1) , 142);
		PutMas16(0,24-16+max(power/3/*>>2*/,1),items+256*8*6+((waitboom&7)*256));
	}
	if ( (power>32 || power&1) && shields ) {
			//вывод тени за ним
			if ( man.napr ) {
				PutCESaturr((man.sx?0:1)+man.x-r_x-man.sx-man.sx,man.y-r_y-man.ly-man.sy-man.sy,man.lx,man.ly,man.buf, 142);
				//CurrentColor--;
				PutCESaturr((man.sx?0:-1)+man.x-r_x-man.sx,man.y-r_y-man.ly-man.sy,man.lx,man.ly,man.buf,142);
				//CurrentColor--;
				PutCASaturr(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf,142);
				//PutCBlinkr(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf);
			} else {
				PutCESatur((man.sx?0:1)+man.x-r_x-man.sx-man.sx,man.y-r_y-man.ly-man.sy-man.sy,man.lx,man.ly,man.buf,142);
				//CurrentColor--;
				PutCESatur((man.sx?0:-1)+man.x-r_x-man.sx,man.y-r_y-man.ly-man.sy,man.lx,man.ly,man.buf,142);
				//CurrentColor--;
				PutCASatur(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf,142);
				//PutCBlink(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf);
			}
	} else {
		if ((man.myrg&2)==0) {
			if ( man.napr ) { //если мужик повернут, то выводим задом на перед
				PutCMasr(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf);
			} else {
				PutCMas(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf);
			}
		} else {
			CurrentColor=15; //он ранен и мигает
			if ( man.napr ) { //если мужик повернут, то выводим задом на перед
				PutCASaturr(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf,206);
			} else {
				PutCASatur(man.x-r_x,man.y-r_y-man.ly,man.lx,man.ly,man.buf,206);
			}
		}
	}
}
