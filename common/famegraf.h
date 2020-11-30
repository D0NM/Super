#ifndef H_FAMEGRAPH
#define H_FAMEGRAPH

#include "gp32\gp32.h"

#define min(x,y) (x>y?y:x)
#define max(x,y) (x<y?y:x)
//#undef abs
//#define abs(x) (x<0?(-x):x)

typedef /*unsigned*/ char  * block;
typedef void  * screen;

extern void  moddevice( s16 *device );
extern void  modvolume( s16 vol1, s16 vol2,s16 vol3,s16 vol4);
extern void  modsetup( char *filenm, s16 looping, s16 prot,s16 mixspeed,
									 s16 device, s16 *status);
extern void  modstop(void);
extern void  modinit(void);

extern void  CopyBlock(s16,s16,s16,s16,screen dest,s16,s16,screen src);

/*
#define k_up	18432
#define k_right	19712
#define k_left	19200
#define k_down	20480
#define k_esc	283
#define k_enter	7181
#define k_f10	17408
#define	k_c	11875
#define k_f	8550
#define k_l	9836
#define k_s	8051
#define k_d	8292
#define k_v	12150
*/

#define wfon (s16)134 /*фон окна*/
#define wlight (s16)128 /*блик окна*/
#define wshadow (s16)146 /*тень окна*/



// Графические переменные
extern s16 	MaxX;
extern s16 	MaxY;
extern s16 	MinX;
extern s16 	MinY;
extern screen	CurrentScreen;
extern block	CurrentAdr;
extern block	CurrentAdr1;
extern screen	SpritesScreen;
extern screen	VScreen;
extern unsigned char  CurrentColor;
extern block fonts;
extern s16 G_x, G_y;
extern s16 G_lx, G_ly, G_interval;
extern char char_fgd, char_bkgd;
extern block palette;
extern block palette1;

// Графические функции
void  Vga256(void);
void  SetScreen(screen);
void  NormalScreen(void);
void  ScreenCopy(screen dest,screen src);
void  CopyToScreen(screen src);
void  Cls(u8);
void  WVR(void);
void  SetColor(s8);
void  PutRGB(s8,s8,s8,s8);
void  PutPalette(block);
//void  GetPalette(block);
//void  PutFont(block);
//s16   GetCharWidth(s16);
//void  DisplayChar(s16 c, s16 x, s16 y, char fgd, char bkgd);
void  Clip(s16, s16, s16, s16);
 void  PutPixel(s16, s16);
s16   GetPixel(s16, s16);
 void  Bar(s16, s16, s16, s16);
void  Rectangle(s16, s16, s16, s16);
void  Line(s16, s16, s16, s16);
 void  PutImg(s16, s16, s16, s16, block);
 void  GetImg(s16, s16, s16, s16, block);
 void  PutMas(s16, s16, s16, s16, block);
 void  PutMasr(s16, s16, s16, s16, block);
 void  PutBlink(s16, s16, s16, s16, block);
 void  PutBlinkr(s16, s16, s16, s16, block);
 void  PutImg16(s16, s16, block);
 void  PutMas16(s16, s16, block);
void  CalcAdr(s16, s16);
 void  SPutImg16(void);
 void  SPutMas16(void);
void  PutCImg(s16, s16, s16, s16, block);
void  PutCMas(s16, s16, s16, s16, block);
void  PutCMasr(s16, s16, s16, s16, block);
void  PutCBlink(s16, s16, s16, s16, block);
void  PutCBlinkr(s16, s16, s16, s16, block);
void  PutSMas(s16, s16, s16, s16, s16, s16, s16, s16, block);
void  PutSMasr(s16, s16, s16, s16, s16, s16, s16, s16, block);
void  PutSImg(s16, s16, s16, s16, s16, s16, s16, s16, block);
void  PutSBlink(s16, s16, s16, s16, s16, s16, s16, s16, block);
void  PutSBlinkr(s16, s16, s16, s16, s16, s16, s16, s16, block);
//void CopyBlock(s16,s16,s16,s16,screen dest,s16,s16,screen src);
void  CopyBlock0(screen src);
u32  gettic(void);
void CBar(s16, s16, s16, s16);
void TileBar(s16, s16, s16, s16, s16,s16,block);
void CopyCBlock(s16, s16, s16, s16, screen dest, s16, s16, screen src);
//--
void Saturate(s16 x, s16 y, s16 lx, s16 ly, int start_c);
//для Sпрайт
void PutCSatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c);
void PutCSaturr(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c);
//для спрайта с абрисом
void PutCASatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c);
void PutCASaturr(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c);
//для фона под спр
void PutCESatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c);
void PutCESaturr(s16 x, s16 y, s16 lx, s16 ly, block buf, int start_c);
// как в СДК для куска больш картинки
void BltSatur(s16 x, s16 y, s16 lx, s16 ly, block buf, int sx, int sy, int bx, int by,int start_c);


// Другие Графические функции
void InitGraph(void);
void CloseGraph(void);
void PaletteOn(block);
void PaletteOff(block);
void PaletteWhite(block);
void MoveXY(s16, s16);
void PutCh(u8);
u16 famestrlen(unsigned char *);
void vputs(char * f);
void vputsc(char * f);
s16  vprint( char *fmt, ... );
char CharUp(u8);
//вывод большого размера
void putBch(s16 x, s16 y, unsigned char c, block buf);
void vputBs(char * f,block buf);
s16 vprintB( block buf, char *fmt, ... );
void PutMtb(s16 x,s16 y,s16 lx,s16 ly,s16 mx,s16 my,block buffer);
s16  GetString(char *str,s16 lstr);
void WPut(s16 x, s16 y, s16 lx, s16 ly);
void WPut1(s16 x, s16 y, s16 lx, s16 ly);
void _WOpen(s16 x, s16 y, s16 lx, s16 ly); //без отрисовки
void WOpen(s16 x, s16 y, s16 lx, s16 ly);
void WClose(void);

void Swap (s16 *,s16 *); //обменять местами переменные

//функции для Trident и поддержки нескольк страниц!
#define Mode_D0 0
#define Mode_D4 2
#define Mode_S0 0
#define Mode_S2 1
#define Mode_S4 2
#define Mode_R0 0
#define Mode_R4 2
extern char Mode_256;

void SetSinchronReg(char New_v, char Allow, char Number);
void SetEltControl(char New_v, char Allow, char Number);
void SetGraphReg(char New_v, char Allow, char Number);
void XOR_17(char Allow);
void StartAddress(u16 Address);

void MapMask(char Mask);
void SelectPlane(char Number);
void MemoryMap(char Map);
void Chain_4(char Allow);
void DubleWordMode(char Allow);
void ByteOrWordMode(char Allow);
void AddressConvolution(char Allow);
void SetDMode(char Mode_Number);
void SetSMode(char Mode_Number);
void Set_Mode_256(char Mode_Number);
void SetActivePage(char Page);
void SetVisualPage(char Page);

void ReadFont(char *font_name);
void ReadPalette(char *palette_name,block dest);
block ReadSpr(char *data_name,u32 n);
void ReadData(char *data_name,block dest, u32 n);

// Спрайты (c) BMV

struct sprites {
  s16 x,y;          //текущие координаты
  s16 ox,oy;        //старые координаты
  s16 lx,ly;        //ширина и высота тек. спр.
  block map;        //ссылка на граф.образ
  unsigned char on; //1-вкл, 0-выкл
};

void InitSpr(void); //начальная инициализация
s16 CreateSpr(s16 lx,s16 ly, block map); //создать спрайт
void ClearSpr(s16 num); //удалитьть спрайт
void SprOn(s16); //показывать спрайт
void SprOff(s16); //прятать спрайт
void MoveSpr(s16 num, s16 x, s16 y); //движение спрайта
void SetSpr(s16 num, block map); //движение спрайта

void ShowSprites(void); //вывести все активные спрайты на экран
void EraseSprites(void); //восстановить прежний фон под спрайтами

// функции для работы с Библиотекой
s16 SetLib(char *);
s16 PutLib(char *, block, u32);
s16 GetLib(char *, block);
s16 GetLibs(char *, block);	//тоже но не грузит, если было это имя
int SizeLib(char *);
s16 DelLib(char *);
s16 SaveMap(void);
s16 PackLib(void);
u32 PackBlock(block src, u32 lenblk);
u32 UnpackBlock(block src, u32 lenpak, u32 lenblk);
extern block pakdest;
void *famemalloc(int); //типа farmalloc но с диагностикой
void _fmeminv(block s, u32 n); //инвертирует кусок памяти
void fatalerror(char *t);


//--
int random(int r);
//char getch(void);
void TurnIt(block s, u16 w, u16 h);
//поворот с подсчетом прозр точек!!
int TurnItT(block s, u16 w, u16 h);

int gettok(char **s, char *d);
int getNtok(char **s);
void show_strip(s16 y);
void do_strip(s16 sx);
int fatoi(char *s);
int show_highscore(void);

//звук
void SE(int n);
void load_SE(void);
void free_SE(void);

#endif