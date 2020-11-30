#ifndef __gpstdio_internal_h__
#define __gpstdio_internal_h__

#include "gpstdio.h"
ERR_CODE GpSmcIDGet(unsigned char* data);
ERR_CODE GpE2PROMRead(int pos, int count, unsigned char * dest);
ERR_CODE GpE2PROMWrite(int pos, int count, unsigned char * src);
ERR_CODE GpDeviceIDGet(unsigned char * data);
ERR_CODE GpDevUserInfoGet(int mode, unsigned char * data);
ERR_CODE GpDevUserInfoSet(int mode,unsigned char * data);

#define SMC_FAC_SAMSUNG		0xec
#define SMC_FAC_TOSHIBA		0x98

#define SMC_DEV_1M		0x6e
#define SMC_DEV_2M		0xea
#define SMC_DEV_4M		0xe3
#define SMC_DEV_8M		0xe6
#define SMC_DEV_16M		0x73
#define SMC_DEV_32M		0x75
#define SMC_DEV_64M		0x76
#define SMC_DEV_128M	0x79	

#endif