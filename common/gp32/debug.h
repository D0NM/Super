//////////////////////////////////////////////////////////////////////////////
// debug.h                                                                  //
//////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG_H
#define _DEBUG_H

/*
	Debug library
	Note: include debug.h after stdio.h and conio.h!!!
*/

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
#include "GP32.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
// public
#define printf			__printf
#undef	getchar
#define getchar			__getchar
#undef	putchar
#define putchar(c)		__putchar(c)
#define gets(s)			__gets(s)
#define puts(s)			__puts(s)
#define kbhit()			__kbhit()

// function -> constructor & class instance
//#define INIT(fn) class fn##_Init { public: fn##_Init() { fn(); } } fn##_init
//#define INIT1(fn,param1) class fn##_Init { public: fn##_Init() { fn(param1); } } fn##_init

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
// Prototypes                                                               //
//////////////////////////////////////////////////////////////////////////////
void InitDebug(void);		// auto-initialized
int /*bool*/ __kbhit(void);
void __putchar(int c);
int __getchar(void);		// non-blocking
int __printf(char *fmt, ...);
int __puts(const char *s);
char * __gets(char *s);

#ifdef __cplusplus
}
#endif


#endif // _DEBUG_H
