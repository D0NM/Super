//файл с глобальными путями файлов и т.д.

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
#include "modplayer\modplayer.h"

#include "famegraf.h"
#include "all.h"

#ifdef DEMO
	char path_to_data_file[]="gp:\\gpmm\\sp_demo\\";
	char path_to_lng_file[]="gp:\\gpmm\\sp_demo\\language\\";
	char lang_ini_file[]="sp_lang.ini";
	char savegame_ini_file[]="sp_save.ini";
#else
	char path_to_data_file[]="gp:\\game\\plusha\\";
	char path_to_lng_file[]="gp:\\game\\plusha\\language\\";
//	char path_to_data_file[]="gp:\\gpmm\\plusha\\";
//	char path_to_lng_file[]="gp:\\gpmm\\plusha\\language\\";
	char lang_ini_file[]="language.ini";
	char savegame_ini_file[]="gp:\\gpsys\\plusha.sav";
#endif