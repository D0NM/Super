#include "famegraf.h"
#include "all.h"

#include "gp32\file_dialog.h"
#include "gp32\debug.h"
#include "modplayer\modplayer.h"

unsigned char *mod_source;
//есть ли звук и т.д. (больше для отладки надо)
extern s16 sound_dev,sound_mix,sound_vol,sound_state;
extern s16 keys;

static int mod_status = -1;

void  modvolume( s16 vol1, s16 vol2,s16 vol3,s16 vol4)
{
}

void  modsetup( char *filenm, s16 looping, s16 prot,s16 mixspeed, s16 device, s16 *status)
{
	F_HANDLE h_rfile;
	unsigned long n_dummy;
	ERR_CODE err_code;
	unsigned long mod_len = 0;
	//если не инициализировали, то выход
#ifndef RELEASE
	__printf("BMod: Memory Available:%d\n", gp_mem_func.availablemem());
#endif
	if ( sound_dev == 0 || mod_status == -1 )
		return;
	//если играет то остановим и освободим память
	if ( mod_status == 1 )
	{
		modstop();
		//gp_mem_func.free(mod_source);
	}
	GpFileGetSize(filenm, &mod_len);
	err_code = GpFileOpen(filenm, OPEN_R, &h_rfile);
	if (err_code != SM_OK)
	{
#ifdef NOEMU
		fatalerror("Error: Can't open BGM");
#endif
		return;
	}
	//__printf("modsetup: load file %s, size: %i",filenm, mod_len);
	/*mod_source = gp_mem_func.zimalloc((unsigned int)mod_len+1);
	if (mod_source == NULL)
	{
		//gp_mem_func.free(mod_source);
		fatalerror("Error: can't alloc for BGM");
		return;
	}*/
	err_code = GpFileRead(h_rfile, mod_source, mod_len, (ulong*)&n_dummy);
	if (err_code != SM_OK)
	{
		//gp_mem_func.free(mod_source);
#ifdef NOEMU
		fatalerror("Error: Can't read BGM");
#endif
		return;
	}
	GpFileClose(h_rfile);

	//__puts("module_load(my_module)\n");
	module_load((u8 *)mod_source);
	//__puts("module_play()\n");
	//module_play(1,0);
	module_play();
	mod_status = 1;
#ifndef RELEASE
	__printf("EMod: Memory Available:%d\n", gp_mem_func.availablemem());
#endif
	//sound_dev = 0; //для отладки типа тока раз грузим звук
}

void  modstop(void)
{
#ifndef RELEASE
	__printf("ModSB: Memory Available:%d\n", gp_mem_func.availablemem());
#endif
	if ( sound_dev == 0 || mod_status != 1)
		return;

	module_stop();
	//gp_mem_func.free(mod_source);
	mod_status = 0;
#ifndef RELEASE
	__printf("ModSE: Memory Available:%d\n", gp_mem_func.availablemem());
#endif
}
void  modinit(void)
{
	//if (  sound_dev == 0 )
	//	return;
	sound_init(1,0);
	//start_sound();
	mod_status = 0;
}

//--- функции работы со звуковыми эффектами
struct s_SE_index {
	block	buf;
	int	size;
};

struct s_SE_index SE_index[50];
int SE_n = 0;
extern char str1[25];

//--загрузим из текст файла SE.cfg все звуки по порядку
void load_SE(void)
{
	int ts,i,ws;
	block p, s, se_t;

	//размер скрипта
	ts = SizeLib("se.cfg");
	se_t = (block)famemalloc(ts);

	//загрузка скрипта
	GetLib("se.cfg",se_t);
	//индексируем строки
	p = se_t;
	SE_n = 0; //0 звуков загружено

	while ( p < (se_t + ts) )	//кол-во строк макс и объем текста
	{
		loading();
		//заносим очередную строку
		gettok(&p,(char*)&str1);
		SE_index[SE_n].size = SizeLib(str1);
		if (SE_index[SE_n].size <= 0)
			fatalerror("Error: Can't load Sound");
		SE_index[SE_n].buf = (block)famemalloc(ws = SE_index[SE_n].size);
		GetLib(str1,SE_index[SE_n].buf);
		//делаем Signed +0x80
		/*s = SE_index[SE_n].buf;
		for (i=0; i<ws; i++)
		{
			*(s+i) += 0x80;
		}*/
		
#ifndef RELEASE
		__printf("SEL: %d '%s' = %d\n",SE_n,str1,SE_index[SE_n].size);
#endif
		SE_n++;
	}
	gp_mem_func.free(se_t);
}

//--освобождаем память от загруженных звуков
void free_SE(void)
{
	int i;
	for ( i=(SE_n-1); i >= 0; i-- )
	{
		gp_mem_func.free(SE_index[i].buf);
	}
		
}

//added to prevent stop_sound work twice.. or so
extern volatile int st_snd;


void SE(int i)
{
	static	unsigned int sfx1, sfx2, sfx1_t, sfx2_t;
	register unsigned int t = gettic();
	if (i >= SE_n || st_snd == 0) return; //нет такого звука или выключен звук
	//проверка на то, чтобы не включать одинаковые звуки в течение
	//определенного времени 8)
#define sdx_max_ticks 180
	if (i == sfx1) {
		if (t < sfx1_t + sdx_max_ticks)
		{
			//нельзя запускать звук еще
			return;
		} else {
			//можно
			sfx1_t = t;
		}
	}
	else {
		if (i == sfx2) {
			if (t < sfx2_t + sdx_max_ticks) {
				//нельзя запускать звук еще
				return;
			} else {
				//можно
				sfx2_t = t;
			}
		}	
		else {	//удаляем из памяти ссылки на старые звуки - заменяем более старую новой
			if ( sfx1_t < sfx2_t ) {
				sfx1_t = t;
				sfx1 = i;
			} else {
				sfx2_t = t;
				sfx2 = i;
			}
		}
	}

#ifndef RELEASE
//	__printf("PSE: %d [%d b]\n",i,SE_index[i].size);
//	__printf(" size: %d [%u,%u...]\n",SE_index[i].size-44,(s8 *)SE_index[i].buf+44,(s8 *)SE_index[i].buf+45);
#endif
	sfx_play((s8 *)SE_index[i].buf, SE_index[i].size, (float)0.5, 0, 0, 63);
}

/////////////////////////////////////////////////////////////////////////////
//int sfx_play(s8 *sfx_data, int lenght, float rate, s8 paning, float pan_increment_50th, u32 volume);
//  
//	starts playing a sfx
//	
//	sfx_data = pointer to 8bit waveform data
//  lenght = lenght of the waveform data
//  rate = speed to play   1.0 = normal speed  , 0.5 = half speed... 2.0 = double speed... and so on
//  paning = -127 means left, 0 means center, 127 means right
//  pan_increment_50th = increment for the pan variable each 1/50 of second
//  volume = 0..63  the volume of the waveform
//
//  returns: the channel number where the sfx will be playing
//

/*
void SDK_SE(int i)
{
	static	unsigned int sfx1, sfx2, sfx1_t, sfx2_t;
	register unsigned int t = gettic();
	if (i >= SE_n) return; //нет такого звука
	//проверка на то, чтобы не включать одинаковые звуки в течение
	//определенного времени 8)
#define sdx_max_ticks 150
	if (i == sfx1) {
		if (t < sfx1_t + sdx_max_ticks)
		{
			//нельзя запускать звук еще
			return;
		} else {
			//можно
			sfx1_t = t;
		}
	}
	else {
		if (i == sfx2) {
			if (t < sfx2_t + sdx_max_ticks) {
				//нельзя запускать звук еще
				return;
			} else {
				//можно
				sfx2_t = t;
			}
		}	
		else {	//удаляем из памяти ссылки на старые звуки - заменяем более старую новой
			if ( sfx1_t < sfx2_t ) {
				sfx1_t = t;
				sfx1 = i;
			} else {
				sfx2_t = t;
				sfx2 = i;
			}
		}
	}

#ifndef RELEASE
	__printf("PSE: %d [%d b]\n",i,SE_index[i].size);
#endif
	GpPcmPlay((unsigned short*)SE_index[i].buf+8,SE_index[i].size-8, 0);
}
*/