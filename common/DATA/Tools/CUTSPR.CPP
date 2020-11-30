#include <alloc.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <string.h>
#include <sys\stat.h>
#include <conio.h>
#include	<ctype.h>
#include	<bios.h>
#include	<assert.h>
#include	<mem.h>

#include "famegraf.h"
#include "mouse.h"

Mouse m;

block sprbuf,sprbufs,tmp;
screen hidscr;

int pack_spr(int lx, int ly, block src, block dest);

void ReadData(char *data_name,block dest,unsigned long n) {
	int fdz;
	if ( (fdz=open (data_name,O_BINARY | O_RDONLY)) < 0) exit (1);
	//if ( lseek(fdz,(unsigned long)0x34e,0)< 0) exit (1);
	if ((int)read (fdz, dest, n) == -1 ) exit (1);
	close (fdz);
}

void SaveData(char *data_name,block dest,unsigned long n) {
	int fdz;
	if ( (fdz=open (data_name,O_BINARY | O_CREAT | O_RDWR)) < 0) fatalerror("cant open wr");
	//if ( lseek(fdz,(unsigned long)0x34e,0)< 0) exit (1);
	write (fdz, dest, n);
	close (fdz);
}

block spadr[16*12+4];

long tic;
long t_fast,t_slow,t_old,t_nont;

void conv_solid_spr(void) {
	int x,y,sizs,i;
	long siza=0;
	long pos_s=0;
  //v sprayty
	//SetLib("tmp");

	//InitGraph();
	//Vga256();

	sprbuf=(block)famemalloc((unsigned long)320*200);
	sprbufs=(block)famemalloc((unsigned long)320*200);
	tmp=(block)famemalloc((unsigned long)320*204);
	//hidscr=famemalloc((unsigned long)320*200);

	ReadData("my.bin",(block)sprbuf,(unsigned long)320*200);
	//CopyToScreen(sprbuf);

	NormalScreen();
	SetScreen(sprbuf);
	//заполняем два массива спрайтами 20*16
	i=x=y=0;
	for ( y=0; y<12; ++y ) {
		for ( x=0; x<16; ++x ) {
			//PutImg16(x*16,y*16,sprbuf+(y*16+x)*256);
			GetImg(x*16,y*16,16,16,tmp+pos_s);
			//sizs=pack_spr(20,16, tmp+pos_s, sprbuf+siza);
			//spadr[i++]=sprbuf+siza;
			pos_s+=(16*16);
			//siza+=sizs;
		}
	}

	//memcpy(tmp,(char*)CurrentScreen,320*200);
	//SetScreen(hidscr);

//	while (kbhit()==0);

	SaveData("f.bin",(char *)tmp,(16*16*192));
	//textmode(C80);
}

void conv_spr_solid(void) {
	int x,y,sizs,i;
	long siza=0;
	long pos_s=0;

	//SetLib("tmp");

	//InitGraph();
	//Vga256();


	sprbuf=(block)famemalloc((unsigned long)320*200);
	sprbufs=(block)famemalloc((unsigned long)320*200);
	tmp=(block)famemalloc((unsigned long)320*204);
	//hidscr=famemalloc((unsigned long)320*200);

	ReadData("02fon.bin",(block)sprbuf,(unsigned long)16*16*192);
	//CopyToScreen(sprbuf);

	NormalScreen();
	SetScreen(sprbufs);
	//заполняем два массива спрайтами 20*16
	i=x=y=0;
	for ( y=0; y<12; ++y ) {
		for ( x=0; x<16; ++x ) {
			PutImg16(x*16,y*16,sprbuf+(y*16+x)*256);
			//GetImg(x*20,y*16,20,16,tmp+pos_s);
			//sizs=pack_spr(20,16, tmp+pos_s, sprbuf+siza);
			//spadr[i++]=sprbuf+siza;
			//pos_s+=(16*20);
			//siza+=sizs;
		}
	}

	memcpy(tmp,(char*)CurrentScreen,320*200);
	//SetScreen(hidscr);

//	while (kbhit()==0);

	SaveData("my.bin",(char *)tmp,320*200);
	//textmode(C80);
}
void fatalerror(char *t) {
	//выход по фатальной ошибке
	//SetLib("");
	//CloseGraph();
	//textmode(C80);
	puts(t);
	//k.off();
	exit(1);
}

void main(void) {
	//conv_solid_spr();
	conv_spr_solid();
}