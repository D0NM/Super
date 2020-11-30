#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpstdio.h"

#include "gp32\debug.h"
//#include "bfont.h"
#include "gp32\file_dialog.h"
//#include "print_text.h"
#include "gpmain.h"
#include "famegraf.h"
#include "all.h"

#include "language.h"
#include "modplayer\modplayer.h"

#include "profiler\sdsfonts.h"


//#include "afm.h"
//#include "res.h"

GPDRAWSURFACE gpDraw[2];
int nflip;
char g_string[100];

extern void main_p(void);
extern char path_to_data_file[];

void GpMain(void *arg)
{
	int i;
	//GpClockSpeedChange(49000000,0x5a042, 0);
	GpFatInit();

	//initialize surfaces (get all info from FW)
	for ( i = 0 ; i < 2 ; i++)
	{
		GpLcdSurfaceGet(&gpDraw[i], i);
	}
	GpSurfaceSet(&gpDraw[0]);	//gpDraw[0]  primary surface 0 setting
	nflip = 1; //1	//gpDraw[1] back surface

#ifndef RELEASE	
	//InitDebug();
	PROFILEResetTimers(NULL);
#endif
	//инициализация звука 8)
	modinit();
	start_sound();
	//stop_sound();

	//идем на главный игровой модуль блин
	main_p();
}