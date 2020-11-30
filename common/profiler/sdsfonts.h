#ifndef _SDSFONTS_H
#define _SDSFONTS_H

//void TextOut(char *screen,int x,int y,char* text,unsigned short int col1,unsigned short int col2);
void PSTA(char *tt);
void PSTO(char *tt);
void PROFILEResetTimers(char*);
void PROFILEDisplayTimers(); 
#define Profile(yy,xx) PSTA(yy); xx PSTO(yy); 
//#define Profile(yy,xx) xx 

#endif