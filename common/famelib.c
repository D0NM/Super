
#include "gpdef.h"
#include "gpstdlib.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpstdio.h"

#include "famegraf.h"
#include "all.h"

#include "gp32\gp32.h"
#include "gp32\debug.h"
#include "gp32\24x.h"
#include "gp32\option.h"

void fatalerror(char *);

#define MaxBlocks 256

struct zapblk {
	u32 posblk;
	u32 lenblk;
	u32 lenpak;
	char comp;
	char typblk;
	char name[13];
} flib[MaxBlocks];
//одна запись = 27 байт

//static s16 fdz_lib=-1;
F_HANDLE fdz_lib=-1;
unsigned long tmp_size;
unsigned long old_offset;

s16 numblk=0;	//кол-во файлов в библ-ке
u32 posmap;	//позиция MAP в файле

u32 posblk;
u32 lenblk;
u32 lenpak;

block pakdest; //буфер для паковки/распаковки
char lockunpak=0;	//запрет на разархивацию при взятии из бибки

char name_lib[300];
char curr_lib_name[300];
extern char stroka[300];

s16 SetLib(char *name) { //начальная инициализация
	static s16 i,j=0;
	struct zapblk blk[1];

#ifndef RELEASE
	__printf("SetLib %s\n",name);
#endif
	if ( gp_str_func.compare(name, curr_lib_name)==0 ) {
		//такая библиотека уже открыта - не будем ее открывать по-новому
		return 0;
	}

	gp_str_func.strcpy(curr_lib_name,name);

	if( gp_str_func.gpstrlen(name)==0 && fdz_lib!=-1) {
		//__printf("SetLib Closing previous lib\n");
		GpFileClose(fdz_lib);
		fdz_lib=-1;
		return 1;
	}
	if( fdz_lib!=-1) {
		//__printf("SetLib Closing previous lib 2!\n");	
		GpFileClose(fdz_lib);
	}

	gp_str_func.strcpy(name_lib,name);
	for (i=1; i<gp_str_func.gpstrlen(name_lib); i++)
	{
		if (name_lib[i] == '.')
		{
			//есть точка и расширение в бибке
			j=1;
			break;
			//__printf("est' point!\n");
		}
	}
	//strstr(name_lib,".")==NULL
	if( j == 0 ) {
		//есть точка и расширение в бибке
		//добавим расширение
		gp_str_func.strcat(name_lib,".fml");

	}
	//__printf("SetLib poluchili name: %s\n",name_lib);
	
	gp_str_func.memset(flib,0,sizeof(flib));

	if ( GpFileOpen (name_lib, OPEN_R | OPEN_W , &fdz_lib) != SM_OK ) {
		return 0;	//!!! ничегон ельзя писать!!
		
		//такой нет бибки
		if ( GpFileCreate(name_lib, NOT_IF_EXIST, &fdz_lib) !=SM_OK ) {
			//не создали новую
			fdz_lib=-1;
			return 0;
		} else {
			//__printf("New Lib Created\n");
			//новая
			numblk=0;
			posmap=4;
			if( GpFileWrite (fdz_lib, &posmap, 4) != SM_OK
				|| GpFileSeek(fdz_lib, FROM_BEGIN, posmap , NULL) != SM_OK
				|| GpFileWrite (fdz_lib, &numblk, 2) != SM_OK ) {

					GpFileClose(fdz_lib);
					fdz_lib=-1;
					return 0;
				}
		}
	}

	//считаем ссылку на MAP кусок
	if ( GpFileSeek(fdz_lib, FROM_BEGIN, 0 , NULL) != SM_OK
		|| GpFileRead (fdz_lib, &posmap, 4, NULL) != SM_OK
		|| GpFileSeek(fdz_lib, FROM_BEGIN, posmap , NULL) != SM_OK
		//считаем кол-во блоков в файле
		|| GpFileRead (fdz_lib, &numblk, 2, NULL) != SM_OK ) {
		//не могу считать заголовок
		GpFileClose(fdz_lib);
		fdz_lib=-1;
		return 0;
	}

	if (numblk>MaxBlocks) numblk=MaxBlocks;
	for (i=0; i<numblk; ++i) {
		//__printf("SetLib: Read Block N %i\n",i);
		//читаем одну запись MAP-файла
		if (GpFileRead (fdz_lib, &blk[0], 27, (unsigned long*)&tmp_size) != SM_OK ) {
			//не читается запись
			GpFileClose(fdz_lib);
			return (0);
		}
		gp_str_func.memcpy(&flib[i], &blk[0], 27);
		//уменьшаем регистр имени
		gp_str_func.lowercase((char*)&flib[i].name, 13);
	}
	return 1;
}

s16 GetLib(char *name, block dest) {
	static s16 i;
#ifndef RELEASE
	__printf("GetLib '%s'\n",name);
#endif

	if ( fdz_lib<0 ) {	//не выбр. биб-ка
		return 0;
	}
	for (i=0; i<MaxBlocks; ++i) {
		if ( gp_str_func.compare(name, flib[i].name)==0 ) {
			//__printf("SetLib: loading block N %i (name: %s)\n",i,name);
			if ( GpFileSeek(fdz_lib, FROM_BEGIN, flib[i].posblk, (long*)&old_offset) != SM_OK
				//файл мал или бибка не открыта
				|| GpFileRead(fdz_lib, dest, flib[i].lenpak, (unsigned long*)&tmp_size) != SM_OK ) {
				//не читается бибка
				return (0);
			}
			_fmeminv(dest,flib[i].lenpak);
			//распаковываем, если разрешено lockunpak!
			if ( lockunpak==0 ) {
				//распаковываем, если запаковано
				if ( flib[i].comp==1 ) {
					if ( (lenblk=UnpackBlock(dest, flib[i].lenpak, flib[i].lenblk))==flib[i].lenblk ) {
						gp_str_func.memcpy(dest,pakdest,lenblk);
						gp_mem_func.free(pakdest); //если упаковывается блок...
					} else {
						gp_mem_func.free(pakdest); //если упаковывается блок...
						break;
					}
				}
			}
			return 1;
		}
	}
	return 0;
}

int SizeLib(char *name) {
	static s16 i;
	//__printf("SizeLib %s\n",name);
	for (i=0; i<MaxBlocks; ++i) {
		if ( gp_str_func.compare(name, flib[i].name)==0 ) {
				//__printf("SizeLib: block %i (name %s) = %i\n",i,name,flib[i].lenblk);
				return (flib[i].lenblk);
		}
	}
	//__printf("SizeLib: name %s - not found!!!!!~!!!\n",name);
//#ifndef RELEASE
//	gp_str_func.sprintf(stroka, "SizeLib of '%s'\n'%s' - not found\n",curr_lib_name,name);
//	fatalerror(stroka);
//#endif
	//fatalerror("Error: SizeLib:file not found");
	return -1;
}

/*
s16 PutLib(char *name, block dest, u32 lenblk) {
	static s16 i,j,k;
	
	return 0;	//!!! ничегон ельзя писать!!
	
	//__printf("PutLib %s\n",name);
	if ( fdz_lib<0 ) {	//|| lenblk>(u32)65000
		return 0;
	}

	k=0;	//Флаг, что не добавл в MAP
	j=-1; ++numblk;
	for (i=0; i<MaxBlocks; ++i) {
		if ( gp_str_func.compare(name, flib[i].name)==0 ) {
			--numblk;
			j=i; break;
		}
	}
	//Если такого блока нет, то ищем пустой
	if (j==-1) {
		//__printf("PutLib : Look for an emplty block\n");
		for (i=0; i<MaxBlocks; ++i) {
			if ( flib[i].name[0] == 0 ) {	//gp_str_func.gpstrlen(flib[i].name)==0
				k=1; j=i; break;
			}
		}
	}

	if (j==-1 || numblk>MaxBlocks) { //Если и пустого блока нет, то выход
		--numblk;
		//Нет места в MAP
		return (0);
	} else {
	   	if ( (lenpak=PackBlock(dest,lenblk))<lenblk ) {
			//если упаковывается блок...
	  		flib[j].comp=1;
			if (lenpak<=flib[j].lenpak && k==0) {	
				GpFileSeek(fdz_lib, FROM_BEGIN ,flib[j].posblk , NULL);
				//flib[j].posblk = old_offset;
			} else {
				GpFileSeek(fdz_lib, FROM_END , 0 , (long*)&old_offset);
				//__printf("PutLib : Add to the end of file %i\n",old_offset);
				flib[j].posblk = old_offset; //lseek (fdz_lib, 0L , SEEK_END);
				k=1;
			}
			_fmeminv(pakdest,lenpak);
			//__printf("PutLib : Write to the file %i\n",lenpak);
			if ( GpFileWrite (fdz_lib, pakdest, lenpak) != SM_OK ) {
				//не пишется в бибку
				--numblk;
				gp_mem_func.free(pakdest);
				return 0;
			}
			gp_mem_func.free(pakdest);

	  	} else {
			//не упаковался
			lenpak=lenblk;
			flib[j].comp=0;
			if (lenpak<=flib[j].lenpak && k==0) {
				GpFileSeek(fdz_lib, FROM_BEGIN, flib[j].posblk, (long*)&old_offset);
				//flib[j].posblk = lseek (fdz_lib, flib[j].posblk , SEEK_SET);
			} else {
				GpFileSeek(fdz_lib, FROM_END, 0, (long*)&old_offset);
				//__printf("PutLib : Add to the end of file %i (2)\n",old_offset);
				flib[j].posblk = old_offset; //lseek (fdz_lib, 0L , SEEK_END);
				k=1;
			}

			_fmeminv(dest,lenpak);
			//__printf("PutLib : Write to the file %i\n",lenpak);
			if ( GpFileWrite (fdz_lib, dest, lenpak) != SM_OK ) {
				//не пишется в бибку
				--numblk;
				_fmeminv(dest,lenpak);
				return 0;
			}
			_fmeminv(dest,lenpak);
	  	}

		if( k ) {
			//добавлен новая запись в MAP
			GpFileSeek(fdz_lib, FROM_END, 0L, (long*)&old_offset);
			posmap = old_offset; //lseek (fdz_lib, 0L , SEEK_END);
			//__printf("PutLib : MAP -> eof: %i\n",posmap);
		}

		gp_str_func.strcpy(flib[j].name, name);
		flib[j].lenblk=lenblk;
		flib[j].lenpak=lenpak;

		return ( SaveMap() );
	}
	//return (1);
}

s16 DelLib(char *name) {
	static s16 i;
	//__printf("DelLib %s\n",name);
	if ( fdz_lib<0 ) {
		return 0;
	}

	for (i=0; i<MaxBlocks; ++i) {
		if ( gp_str_func.compare(name, flib[i].name)==0 ) {
			--numblk;
			flib[i].name[0]=0;
		}
	}
	return ( SaveMap() );
}

s16 SaveMap(void) {
	static s16 i;
	
	return 0;	//!!! ничегон ельзя писать!!
	
	//__printf("SaveMap: Begin\n");
	if ( fdz_lib<0 ) {
		return 0;
	}
	
	//__printf("SaveMap: Write: Posmap %i -> Begin, # blok %i\n",posmap,numblk);
	
	if ( GpFileSeek(fdz_lib, FROM_BEGIN, 0L, NULL) != SM_OK
		|| GpFileWrite (fdz_lib, &posmap, 4) != SM_OK
		|| GpFileSeek(fdz_lib, FROM_BEGIN, posmap, NULL) != SM_OK
		|| GpFileWrite (fdz_lib, &numblk, 2) != SM_OK ) {
		//не пишется в бибку
		return 0;
	}
	for (i=0; i<MaxBlocks; ++i) {
		if ( flib[i].name[0] ) { //gp_str_func.gpstrlen(flib[i].name)!=0
			if (GpFileWrite (fdz_lib, &flib[i], 27) != SM_OK ) { //sizeof(flib[0])
				//не пишется в бибку
				return 0;
			}
		}
	}
	return 1;
}
*/

/*
s16 PackLib(void) {
	s16 i; //,j;
	block tmp;
	char *tmp1,*tmp2;
	F_HANDLE fdz;
	
	return 0;	//!!! ничегон ельзя писать!!
		
	//__printf("PackLib\n");
	if( fdz_lib<0 ) {
		return 0;
	}
	//Упаковка библиотеки
	//GpFileRemove("tmp.tmp");
	if ( GpFileCreate ("tmp.tmp", ALWAYS_CREATE, &fdz) != SM_OK ) {
		//не открывается TMPшный файл
		GpFileClose(fdz);
		return 0;
	}
	if (GpFileWrite (fdz, &posmap, 4) != SM_OK ) {
		GpFileClose(fdz);
		return 0;
	}
	for (i=0; i<MaxBlocks; ++i) {
		if ( flib[i].name[0] ) { //gp_str_func.gpstrlen(flib[i].name)!=0
			tmp=(block)famemalloc( flib[i].lenpak );
			lockunpak=1; //запрет на разархивацию
			if (GetLib(flib[i].name,tmp)==0) {
				gp_mem_func.free(tmp);
				lockunpak=0; //разрешение на разархивацию
				GpFileClose(fdz);
				return (0);
			}
			_fmeminv(tmp,flib[i].lenpak);
			lockunpak=0; //разрешение на разархивацию
			//lseek (fdz, 0L , SEEK_END);
			GpFileSeek(fdz, FROM_END, 0, (long*)&old_offset);
			flib[i].posblk = old_offset;
			if (GpFileWrite (fdz, tmp, flib[i].lenpak) != SM_OK ) {
				gp_mem_func.free(tmp);
				GpFileClose(fdz);
				return (0);
			}
			gp_mem_func.free(tmp);
		}
	}
	//posmap = lseek (fdz, 0L , SEEK_END);
	GpFileSeek(fdz, FROM_END, 0L, (long*)&old_offset);
	posmap = old_offset;
	GpFileClose(fdz_lib);
	GpFileRemove(name_lib);
	fdz_lib = fdz;
	if( SaveMap() ) {
		GpFileClose(fdz_lib);
		tmp1=(char*)famemalloc( 256 );
		tmp2=(char*)famemalloc( 256 );
		GpRelativePathGet(tmp1);
		//GpRelativePathGet(tmp2);
		gp_str_func.strcpy(tmp2,tmp1);
		gp_str_func.strcat(tmp1,"tmp.tmp");
		//gp_str_func.strcat(tmp2,"");
		gp_str_func.strcat(tmp2,name_lib);
		
		GpFileRename(tmp1,tmp2);
		gp_mem_func.free(tmp2);
		gp_mem_func.free(tmp1);
		//__printf("PackLib: Rename %s -> %s\n",tmp1,tmp2);
	
		if ( GpFileOpen (name_lib, OPEN_R | OPEN_W ,&fdz_lib) != SM_OK ) {
			fdz_lib=-1;
			return 0;
		}
		return 1;
	}
	return 0;
}

u32 PackBlock(block src, u32 lenblk) {
	unsigned char t,l,cl;
	s32 dpos,spos;

	//__printf("PackBlock Len:%i\n",lenblk);

	cl=dpos=spos=0;
	pakdest=(block)famemalloc((u32)(lenblk+3));

	t=src[spos];
	for ( spos=0; spos<lenblk; spos++ ) {
		l=t; //предыдущий символ
		t=src[spos]; //возмем новый симв
		if (l==t) { //новый символ равен предыдущему
			cl++;
			if (cl>63) { //кол-во одинаковых симв превысило 64
				//закодир заголовок 11хххххх & символ
				pakdest[dpos++]= (0xff); //63
				pakdest[dpos++]= l;
				cl=1;
			}
		} else { //новый символ не равен предыдущему
			if (cl>1) { //запись цепочки
				pakdest[dpos++]= (0xc0 | cl);
				pakdest[dpos++]= l;
			} else { //символ без 2х перв бит
				if (l>127) { //запись цепочки
					pakdest[dpos++]= 0xc1; //1
					pakdest[dpos++]= l;
				} else {
					pakdest[dpos++]= l;
				}
			}
			cl=1;
		}
		if (dpos>=lenblk) {
			//ошибка-переполнение при архивации
			return (dpos);
		}
	} //конец осн. цикла
	if (cl >1) {
		pakdest[dpos++]= (0xc0 | cl);
		pakdest[dpos++]= l;
	} else {
		if (t>127) { //запись цепочки
			pakdest[dpos++]= 0xc1; //63
			pakdest[dpos++]= t;
		} else {
			pakdest[dpos++]= t;
		}
	}
	return (dpos);
}
*/

u32 UnpackBlock(block src, u32 lenpak, u32 lenblk) {
	static unsigned char t,cl;
	static u32 dpos,spos;
	//__printf("UnpackBlock %i\n",lenblk);

	dpos=spos=0;
	pakdest=(block)famemalloc((u32)(lenblk+64));

	for ( spos=0; spos<lenpak; ++spos ) {
		t=src[spos]; //возмем новый симв
		if ( t>127 ) {
			cl=t&0x3f; //00111111
			t=src[++spos];
			while (cl--) {
				pakdest[dpos++]= t;
			}
		} else {
			pakdest[dpos++]= t;
		}
		if (dpos>lenblk) {
			break; //ошибка при разархивации
		}
/*		__asm
		{
		nop
		nop
		nop
		nop
		}
*/
	}
	return (dpos);
}

void *famemalloc(int n) {
	void *t;
	n = (n+15)&0xffff0;
#ifndef RELEASE
	__printf("FameMalloc %i\n",n);
#endif
	if (n <= 0)
		fatalerror("Error: Malloc error\ndue to the damaged game data");
	if( (t=gp_mem_func.zimalloc(n))==NULL ) {
		fatalerror("Error: Malloc error\ndue to lack of free memory");
	}
	//*(char*)(t+n-1)=0;
	return t;
}

void _fmeminv(block s, u32 n) {
	u32 i;
	//__printf("_fmeminv %i\n",n);
	//инвертирует строку
	for (i=0; i<n; ++i) {
		*s=(~*s);
		s++;
/*		__asm
		{
		nop
		nop
		nop
		nop
		}
*/


	}
}
