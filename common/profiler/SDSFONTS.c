#include "gpstdlib.h"
#include "gpmem.h"
#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
//#include "gpfont.h"
#include "../gp32/bfont.h"
#include "gpstdio.h"

#define GP32

extern struct BFont text_font;
extern GPDRAWSURFACE gpDraw[2];
extern int nflip;


#define QGUID(x) unsigned char G=0;int zz=0; while (x[zz]!=0) {G+=x[zz]*zz++;}
int   PROFILETimes   [256]; 
int   PROFILETimesACC[256]; 
int   PROFILEST      [256]; 
int   PROFILECOUNT   [256]; 
char *PROFILEName    [256]; 
//char *intmap;
int COLS[6]={31<<6,31<<1,31<<11,31<<6,31<<1,31<<11};
void PROFILEResetTimers(char *map) 
        {
	int a; 
	//intmap=map;
        for (a=0;a<256;a++)
                { 
                PROFILETimes[a]=0; 
                PROFILEST[a]=0;
		PROFILETimesACC[a]=0;
                PROFILECOUNT[a]=0; 
                } 
         }
#ifdef GP32
void PSTA(char *tt) {QGUID(tt);PROFILEName[G]=tt; PROFILECOUNT[G]++; PROFILEST[G]=GpTickCountGet();}
void PSTO(char *tt) {QGUID(tt);PROFILETimes[G]=GpTickCountGet()-PROFILEST[G]; PROFILEName[G]=tt; PROFILETimesACC[G]+=PROFILETimes[G];}
#else
void PSTA(char *tt) {}
void PSTO(char *tt) {}
#endif

void PROFILEDisplayTimers() 
{ 
int CTN=0; 
int a,b,c;
int ct;
char XTEMP[512];
//unsigned short *zz=(unsigned short *)intmap;
Clrs(2,nflip);
for (a=0;a<256;a++) 
        { 
        if (PROFILEName[a]!=0) 
                { 

			if (PROFILETimes[a]!=PROFILETimesACC[a])
			{
			ct=PROFILETimesACC[a];
			if (ct<0) ct=0;
			if (ct>319) ct=319;
			//for (c=0;c<ct;c++)
			//for (b=0;b<6;b++)
			//	{
			//	//zz[(CTN*12)+b+(c*240)]=COLS[CTN % 6];
			//	}

			}

			ct=PROFILETimes[a];
			if (ct<0) ct=0;
			if (ct>319) ct=319;
			//for (c=0;c<ct;c++)
			//for (b=0;b<BMH;b++)
			//	{
			//	zz[(CTN*BMH)+b+(c*240)]=COLS[CTN % 6];
			//	}
                       
			gp_str_func.sprintf(XTEMP,"%16s %4d,%8d,%4d",PROFILEName[a],PROFILETimes[a],PROFILETimesACC[a],PROFILECOUNT[a]);
			//TextOut(intmap,0,CTN*BMH,XTEMP,31<<6,0xffff);
			text_font.color[0] = 8; //цвет
			BTextOut(NULL, &gpDraw[nflip], &text_font, 1, CTN*12+1, XTEMP);
			text_font.color[0] = 15; //цвет
			BTextOut(NULL, &gpDraw[nflip], &text_font, 0, CTN*12, XTEMP);


                CTN++; 
                } 
        } 
} 

