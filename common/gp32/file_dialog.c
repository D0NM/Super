#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpstdio.h"

#include "debug.h"
#include "file_dialog.h"
#include "bfont.h"
#include "../gpmain.h"
#include "../famegraf.h"
#include "../all.h"


extern GPDRAWSURFACE gpDraw[2];
extern int nflip;
char *tmp_string[MAX_PATH_NAME_LEN];
extern char g_string[MAX_PATH_NAME_LEN];
//кол-во строк на экране... в столбце
#define n_fstr 17

extern ERR_CODE _make_final_path(const char * p_name);
extern char _relative_path[MAX_PATH_NAME_LEN];
extern char _final_path[MAX_PATH_NAME_LEN];

GPDIRENTRY *p_list;
typedef struct {           
	//char name[16];
	int type;
	unsigned int size;
}MYDIRENTRY;
MYDIRENTRY	*pi_list;

unsigned long list_count = 0, read_count = 0;

void FlipAndShow(void)
{
	//flip and show hidden surface		
	GpSurfaceFlip(&gpDraw[nflip++]);
	nflip %= 2;
}

void Delay(int ms) {

	unsigned int i, ticks;
	
	ticks=GpTickCountGet()+ms;
	if (ticks <= ms) return;
	while( GpTickCountGet() < ticks )
	{
		for (i = 0; i< 10; i++)
		{
		}
	};
}

void Clrs(int color ,int nflip)
{
	//очистка экрана
	GpRectFill(NULL, &gpDraw[nflip], 0,0,GPC_LCD_WIDTH,GPC_LCD_HEIGHT, color);
#ifndef RELEASE
	//свободн память
	gp_str_func.sprintf(g_string, "Memory Available:%d", gp_mem_func.availablemem());
	GpTextOut(NULL, &gpDraw[nflip], 14, 213, g_string, 15);
#endif
}

/*
//показать окошко и сообщение в нем по центру
void DrawMessage(char *s, int nflip)
{
	int tw,th;
	GPRECT rect;	
	rect.left = 2;
	rect.top = 2;
	rect.right = GPC_LCD_WIDTH;
	rect.bottom = GPC_LCD_HEIGHT;

 	//вывод окошка красивого
	tw=GpTextWidthGet(s);
	th=GpTextHeightGet(s);
	DrawWindow( (GPC_LCD_WIDTH - (tw+20) )/2, (GPC_LCD_HEIGHT - (th+20) )/2, tw+20, th+20, nflip,
	90,94,152,86,81);

	GpTextDraw(&gpDraw[nflip], &rect, GPC_GT_HCENTER | GPC_GT_VCENTER, 
		    	   (char*)s, 0, gp_str_func.gpstrlen(s), 0x0);
	rect.left = 0;
	rect.top = 0;
	GpTextDraw(&gpDraw[nflip], &rect, GPC_GT_HCENTER | GPC_GT_VCENTER, 
		    	   (char*)s, 0, gp_str_func.gpstrlen(s), 254);
}
//показать окошко и сообщение в нем по центру
void DrawError(char *s, int nflip)
{
	int tw,th;
	GPRECT rect;	
	rect.left = 2;
	rect.top = 2;
	rect.right = GPC_LCD_WIDTH;
	rect.bottom = GPC_LCD_HEIGHT;

 	//вывод окошка c ошибкой!
	tw=GpTextWidthGet(s);
	th=GpTextHeightGet(s);
	DrawWindow( (GPC_LCD_WIDTH - (tw+20) )/2, (GPC_LCD_HEIGHT - (th+20) )/2, tw+20, th+20, nflip,
	224,240,241,192,160);

	GpTextDraw(&gpDraw[nflip], &rect, GPC_GT_HCENTER | GPC_GT_VCENTER, 
		    	   (char*)s, 0, gp_str_func.gpstrlen(s), 8);
	rect.left = 0;
	rect.top = 0;
	GpTextDraw(&gpDraw[nflip], &rect, GPC_GT_HCENTER | GPC_GT_VCENTER, 
		    	   (char*)s, 0, gp_str_func.gpstrlen(s), 15);
}

// green 90,94,152,86,81
// red 224,240,241,192,160

void DrawWindow(int x, int y, int lx, int ly, int nflip, int c1, int c2, int c3, int c4, int c5)
{
	//тень под окном
	GpRectFill(NULL, &gpDraw[nflip], x+5,y+5,lx,ly, 0);
	GpRectFill(NULL, &gpDraw[nflip], x-1,y-1,lx+2,ly+2, 0);
	//CurrentColor=wfon;
	GpRectFill(NULL, &gpDraw[nflip], x,y,lx,ly, c1);
	//CurrentColor=wlight;
	GpRectFill(NULL, &gpDraw[nflip], x,y,lx,1, c2);
	GpRectFill(NULL, &gpDraw[nflip], x,y,1,ly, c2);
	//CurrentColor++;
	GpRectFill(NULL, &gpDraw[nflip], x,y,1,1, c3);
	//CurrentColor=wshadow;
	GpRectFill(NULL, &gpDraw[nflip], x,y+ly-1,lx,1, c4);
	GpRectFill(NULL, &gpDraw[nflip], x+lx-1,y+1,1,ly-1, c4);
	//CurrentColor++;
	GpRectFill(NULL, &gpDraw[nflip], x+lx-1,y+ly-1,1,1, c5);
}
*/