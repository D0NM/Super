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

extern s16 end,pause,key_f,key_b;
extern s16 left,right,up,down,jump,fire;
extern s16 t_left,t_right,t_up,t_down,t_jump,t_fire;

/*int	g_key_pressed;		//list of buttons pressed in curent loop
int	g_key_pressed_old;	//list of buttons pressed in previous loop
int	keydata;			//list of buttons with key-down event
int	g_key_up;			//list of buttons with key-up buttons
*/
int keydata;	//клавиши

extern struct hero man;


void sdKeys(void);

void mm(unsigned char butt) {
	register char c;
	pause = key_f=key_b=t_left=t_right=t_up=t_down=t_jump=t_fire=0;
	//обработка прерывания по клавиатуре

	GpKeyGetEx(&keydata);
	c=1;
/*	if ( butt&128 ) {
		c=0;
		butt=butt&127;
	} else {
		c=1;
	}*/
	if ( keydata & GPC_VK_START ) //Esc (start)
		end=c;
	if ( keydata & GPC_VK_SELECT ) //PAUSE
		pause=c;		
	if ( keydata & GPC_VK_RIGHT )
		t_right=c;
	if ( keydata & GPC_VK_LEFT )
		t_left=c;
	if ( keydata & GPC_VK_UP )
		t_up=c;
	if ( keydata & GPC_VK_DOWN )
		t_down=c;
	if ( keydata & GPC_VK_FB ) //alt spc entr
		t_fire=c;
	if ( keydata & GPC_VK_FA ) //ctrl
		t_jump=c;
	if ( keydata & GPC_VK_FL ) //секретная кнопка
		key_b=1;		
	if ( keydata & GPC_VK_FR ) //секретная кнопка след уровень
		key_f=1;
//	sdKeys();
}

void mz(unsigned char butt) {
	//обработка прерывания по клавиатуре
	pause = key_f=key_b=end=left=right=up=down=jump=fire=0;
	GpKeyGetEx(&keydata);

	if ( keydata & GPC_VK_START ) //Esc (start)
		end=1;
	if ( keydata & GPC_VK_RIGHT )
		right=1;
	if ( keydata & GPC_VK_LEFT )
		left=1;
	if ( keydata & GPC_VK_UP )
		up=1;
	if ( keydata & GPC_VK_DOWN )
		down=1;
	if ( keydata & GPC_VK_FB ) //alt spc entr
		fire=1;
	if ( keydata & GPC_VK_FA ) //ctrl
		jump=1;
	if ( keydata & GPC_VK_FL ) //секретная кнопка
		key_b=1;
	if ( keydata & GPC_VK_FR ) //секретная кнопка след уровень
		key_f=1;
	//sdKeys();
}

void md(unsigned char butt) {
	//обработка прерывания по клавиатуре
	//для демонстрации
	GpKeyGetEx(&keydata);
	if (keydata)
		end=1;
	//sdKeys();
}

/*
void sdKeys(void)
{
	//очистка экрана
	//GpRectFill(NULL, &gpDraw[nflip], 0,220,GPC_LCD_WIDTH,20, 0);

	if ( keydata & GPC_VK_START )
		GpTextOut(NULL, &gpDraw[nflip], 0, 222, "Start", 255);
	if ( keydata & GPC_VK_RIGHT )
		GpTextOut(NULL, &gpDraw[nflip], 42, 222, "->", 255);
	if ( keydata & GPC_VK_LEFT )
		GpTextOut(NULL, &gpDraw[nflip], 60, 222, "<-", 255);
	if ( keydata & GPC_VK_UP )
		GpTextOut(NULL, &gpDraw[nflip], 70, 222, "^", 255);
	if ( keydata & GPC_VK_DOWN )
		GpTextOut(NULL, &gpDraw[nflip], 80, 222, "V", 255);
	if ( keydata & GPC_VK_FB )
		GpTextOut(NULL, &gpDraw[nflip], 90, 222, "[A]", 255);
	if ( keydata & GPC_VK_FA )
		GpTextOut(NULL, &gpDraw[nflip], 114, 222, "[B]", 255);
	if ( keydata & GPC_VK_SELECT )
		GpTextOut(NULL, &gpDraw[nflip], 138, 222, "Select", 255);
	if ( keydata & GPC_VK_FL )
		GpTextOut(NULL, &gpDraw[nflip], 190, 222, "LS", 255);
	if ( keydata & GPC_VK_FR )
		GpTextOut(NULL, &gpDraw[nflip], 210, 222, "RS", 255);		
}
*/