#ifndef __OPTION_H__
#define __OPTION_H__


#define FCLK 135428571
#define HCLK (135428571/2)
#define PCLK (135428571/4)


//BUSWIDTH; 16,32
#define BUSWIDTH    (32)


#define _RAM_STARTADDRESS 	0xc000000
#define _ISR_STARTADDRESS 	0xcffff00     //GCS6:128M SDRAM x 2
#define _MMUTT_STARTADDRESS	0xcff8000     //KIW ADD
#define HEAPEND		  	0xcff0000

//If you use ADS1.x, please define ADS10
#define ADS10 TRUE

// note: makefile,option.a should be changed

#endif /*__OPTION_H__*/

