#include "gpmm.h"

extern unsigned int my_tick_counter;
/*-----------------------------------------------------------------------------
GP32_Sound.c   v0.02, 17.04.2002                     (w) 2002 by Groepaz/Hitmen
-----------------------------------------------------------------------------*/
/*
WARNING: this is work in progress.... a lot of the information might be in-
         complete, inaccurate or even plain wrong - please always refer to the
         mentioned references aswell. (and let me know if you find any obvious
         errors or have something to add to the comments if you care about
         improving this document.)

                                                                groepaz@gmx.net

todo: (or call it know-bugs ;=))

        - write intro
        - extend overview
        - more comments in all other sections

*/

/*
Contents:
---------

Introduction
References
Overview

Table- I/O Port Register
Part #1 - using the L3 bus

Table - IIS Register
Part #2 - using the IIS bus

Table  - DMA Controller Registers
Part #3 - using DMAs

Table  - Interupt Controller Registers
Part #4 - using Interupts

Part #5 - finally playing a sound
*/

/*
Introduction:
-------------

-todo-

This Document describes in detail only what isn't mentioned in the references
below (and thus is specific to the GP32).

This was written as actual C-source so it can be easily checked that the
provided information is correct. It can currently be compiled with GCC and the
ARM-SDT (ADS not checked but should work) - please define the compiler you
are using below.

*/

//#define __GCC2x__
//#define __GCC3x__
#define __ARMSDT__
//#define __ARMADS__

/* these save some typing later ;) */
#ifdef __GCC2x__
#define __GCC__
#endif
#ifdef __GCC3x__
#define __GCC__
#endif
#ifdef __ARMSDT__
#define __ARM__
#endif
#ifdef __ARMADS__
#define __ARM__
#endif

/* define so DM2_Done irq will be used to play chunks of the samples
   rather than playing the entire sample at once. you must use this
   method to play samples larger than 0x7ffff bytes. */
#define USEDMA2IRQ

/*
References:
-----------

um_s3c2400_rev30.pdf (or um_s3c2400x_rev10.pdf) - available at Samsung site
(this includes the ARM cpu-manual)

UDA1330A.pdf                                    - available at Philips site
(this includes info about the soundchip and iis/l3 bus)

I2SBUS.pdf                                      - available at Philips site
(further general information about the iis bus)

test_24x_146u.zip                               - available at Samsung site
(invalueable sourcecode with a lot of lowlevel information inside)

remember the comments in this file won't cover anything very detailed that
is already contained in these files, so you might want to read these aswell.

Overview:
---------

+----------------------+                        +--------------------+
|                      |       (L3 Bus)         |                    |
| Samsung  PORT E b  9 |--------Commands------->| L3 Clock  Philips  |
| s3c2400         b 10 |                        |    Mode   uda1330a |
|                 b 11 |                        |    Data            |
|                      |                        |                    |
|           DMA 2      |-------Audio-Data------>| DAC                |
|                      |<------Audio-Data-------| ADC                |
|                      |       (IIS Bus)        |                    |
+----------------------+                        +--------------------+

The general process of playing sound on the Gp32 would be like:

1) stop any previously going sound-dma
2) setup the L3 bus
3) send commands to setup the uda1330a over L3 bus
4) setup DMA2
5) use DMA2 to send audio data to the uda1330a over IIS bus

you could probably play sound by direct cpu-writes aswell but i really
don't have a clue why you would want to do that ;)

*/

void _swi_install_irq(unsigned long nr,void *ptr);
void _swi_uninstall_irq(unsigned long nr);

void _swi_install_irq(unsigned long nr,void *ptr) {
        __asm {
                mov      r0,nr
                mov      r1,ptr
                swi      0x9, {r0-r1}, {r14}
        }
}

void _swi_uninstall_irq(unsigned long nr) {
        __asm {
                mov      r0,nr
                swi      0xa, {r0}, {r14}
        }
}

/*-----------------------------------------------------------------------------
Table- I/O Port Register
-----------------------------------------------------------------------------*/

#define rPECON		(*(volatile unsigned *)0x1560002c)
#define rPEDAT		(*(volatile unsigned *)0x15600030)
#define rPEUP		(*(volatile unsigned *)0x15600034)

/* for other i/o registers refer to CPU-Manual */

/*-----------------------------------------------------------------------------
Part #1 - using the L3 bus
-----------------------------------------------------------------------------*/

void _WrL3Addr(unsigned char data);
void _WrL3Data(unsigned char data,int halt);
void Init1330(void);

#define L3C                             0x200   /* bit 9  */
#define L3M                             0x400   /* bit 10 */
#define L3D                             0x800   /* bit 11 */
#define L3_MASK                         0xe00

#define L3DELAY                             8   /* delay for bus-transfer */

/*
   This routine sends an adress plus type of transfer to the L3 bus
*/

void _WrL3Addr(unsigned char data) {
signed long i,j;

    rPEDAT = rPEDAT & ~L3_MASK;       //L3D=L/L3M=L(in address mode)/L3C=L
    rPEDAT |= L3C;                    //L3C=H
    for(j=0;j<L3DELAY;j++);           //tsu(L3) > 190ns

    //PD[8:6]=L3D:L3C:L3M
    for(i=0;i<8;i++)	//LSB first
    {
	if(data&0x1)	//if data's LSB is 'H'
	{
            rPEDAT &= ~L3C;           //L3C=L
            rPEDAT |= L3D;            //L3D=H
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
            rPEDAT |= L3C;            //L3C=H
            rPEDAT |= L3D;            //L3D=H
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
	}
	else		//if data's LSB is 'L'
	{
            rPEDAT &= ~L3C;           //L3C=L
            rPEDAT &= ~L3D;           //L3D=L
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
            rPEDAT |= L3C;            //L3C=H
            rPEDAT &= ~L3D;           //L3D=L
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
	}
	data >>=1;
    }
    //L3M=H,L3C=H
    rPEDAT = (rPEDAT & ~L3_MASK) | (L3C|L3M);
}

/*
   This routine sends a data word to the L3 bus
*/

void _WrL3Data(unsigned char data,int halt) {
signed long i,j;

    if(halt) {

        rPEDAT |= L3C;                //L3C=H(while tstp, L3 interface halt condition)
        for(j=0;j<L3DELAY;j++);       //tstp(L3) > 190ns
        rPEDAT &= (~L3M);
        for (j=0;j<L3DELAY;j++);
        rPEDAT |= L3M;
    }

    rPEDAT = (rPEDAT & ~L3_MASK) | (L3C|L3M);        //L3M=H(in data transfer mode)
    for(j=0;j<L3DELAY;j++);                          //tsu(L3)D > 190ns

    //PD[8:6]=L3D:L3C:L3M
    for(i=0;i<8;i++)
    {
        if(data&0x1)	//if data's LSB is 'H'
        {
            rPEDAT &= ~L3C;           //L3C=L
            rPEDAT |= L3D;            //L3D=H
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
            rPEDAT |= (L3C|L3D);      //L3C=H,L3D=H
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
        }
        else		//if data's LSB is 'L'
        {
            rPEDAT &= ~L3C;           //L3C=L
            rPEDAT &= ~L3D;           //L3D=L
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
            rPEDAT |= L3C;            //L3C=H
            rPEDAT &= ~L3D;           //L3D=L
            for(j=0;j<L3DELAY;j++);   //tcy(L3) > 500ns
        }
        data>>=1;	//for check next bit
    }

    rPEDAT = (rPEDAT & ~L3_MASK) | L3C;
    for (j=0;j<L3DELAY;j++);
    rPEDAT |= L3M;

}
/*
        This routine initializes the i/o port used for L3-bus and
        then configures the UDA1330
*/

void Init1330(void) {

    /****** I/O Port E Initialize ******/

    /* port reconfiguration :
            PORTE 9,10,11 --> output
            pull-up disable
            L3_MODE, L3_CLK High
    */
    rPEDAT = (rPEDAT & ~0xe00) | (L3M|L3C);
    rPEUP |= 0xe00;
    rPECON = (rPECON & (~(0x3f << 18))) | (0x15<<18);

    /****** send commands via L3 Interface
     Data bits 7 to 2 represent a 6-bit device address where bit 7 is the MSB.
     The address of the UDA1330ATS is 000101 (bit 7 to bit 2).
    ******/

    /* status type transfer , data value - clock=512fs, MSB format */
    _WrL3Addr(0x14+2); //STATUS (0) (000101xx+10)
    _WrL3Data(
              (0<<4)+  //  00,  : 512,384,256fs         (SC : System Clock Freq)  //0
              (4<<1)   // 000,  : iis,lsb16,lsb18,lsb20,msb //4
              ,1);

    /* data type transfer , data value - full volume */
    _WrL3Addr(0x14 + 0);     /* DATA0 (000101xx+00) */
    _WrL3Data(0x3f           /* volume */
                 ,1);

    /* data type transfer , data value - de-emphasis, no muting */
    _WrL3Addr(0x14 + 0);     /* DATA0 (000101xx+00) */
    _WrL3Data(0x80+          /* select de-emhasis/mute */
              (0<<3)         /* de-emp 44khz */ //2
              ,1);

}

/*-----------------------------------------------------------------------------
Table - IIS Register
-----------------------------------------------------------------------------*/

#define rIISCON		(*(volatile unsigned *)0x15508000)
#define rIISMOD		(*(volatile unsigned *)0x15508004)
#define rIISPSR		(*(volatile unsigned *)0x15508008)
#define rIISFIFCON	(*(volatile unsigned *)0x1550800c)
#define IISFIF		((volatile unsigned short *)0x15508010)

/*-----------------------------------------------------------------------------
Part #2 - using the IIS bus
-----------------------------------------------------------------------------*/

void InitIIS(void);

void InitIIS(void) {

    /****** IIS Initialize ******/

    /* prescaler for 44khz */
    rIISPSR=(2<<5)+2;

    rIISCON=/* 8,7,6 readonly */
            (1<<5)+  /* dma transmit request enable */
            (0<<4)+  /* dma recieve request disable */
            (0<<3)+  /* Transmit LR-Clock idle state */
            (1<<2)+  /* Recieve LR-Clock idle state */
            (1<<1)+  /* prescaler enabled */
            (0<<0);  /* iis disabled */

    rIISMOD=
            (0<<8)+             /* master mode */
            (2<<6)+             /* transmit mode */
            (0<<5)+             /* lowbyte=left channel */
            (1<<4)+             /* iis compatible samples / MSB samples */
            (1<<3)+             /* 16 bit per channel */
            (0<<2)+             /* CDCLK=256fs */
            (1<<0);             /* serial bitclock 32fs */  //*** 0  48fs

    rIISFIFCON=
               (1<<11)+   /* transmit fifo mode = dma */
               (0<<10)+   /* recieve fifo mode = normal */
               (1<<9)+    /* transmit fifo enable */
               (0<<8);    /* recieve fifo disable */
                /* 7-4 Transmit Fifo Data Count (readonly) */
                /* 3-0 Recieve Fifo Data Count (readonly) */

    /****** IIS Tx Start ******/
    rIISCON |=0x1;  /* iis enable */

}

/*-----------------------------------------------------------------------------
Table  - DMA Controller Registers
-----------------------------------------------------------------------------*/

#define rDISRC2         (*(volatile unsigned *)0x14600040)
#define rDIDST2		(*(volatile unsigned *)0x14600044)
#define rDCON2		(*(volatile unsigned *)0x14600048)
#define rDSTAT2		(*(volatile unsigned *)0x1460004c)
#define rDCSRC2		(*(volatile unsigned *)0x14600050)
#define rDCDST2		(*(volatile unsigned *)0x14600054)
#define rDMASKTRIG2	(*(volatile unsigned *)0x14600058)

/* for other DMA registers refer to CPU manual */

/*-----------------------------------------------------------------------------
Part #3 - using DMAs
-----------------------------------------------------------------------------*/

void _GpPCMPlay(unsigned char* src, int size, int rpt);

void _GpPCMPlay(unsigned char* src, int size, int rpt) {

    rDIDST2=(1<<30)+       /* destination on peripherial bus */
            (1<<29)+       /* fixed adress */
            ((int)IISFIF); /* IIS FIFO txd */
    rDISRC2=(0<<30)+       /* source from system bus */
            (0<<29)+       /* auto-increment */
            (int)(src);    /* buffer adress */
    rDCON2 =(1<<30)+       /* handshake mode */
            (0<<29)+       /* DREQ and DACK are synchronized to PCLK (APB clock) */
            (1<<28)+       /* generate irq when transfer is done */
            (0<<27)+       /* unit transfer */
            (0<<26)+       /* single service */
            (0<<24)+       /* dma-req.source=I2SSDO */
            (1<<23)+       /* (H/W request mode) */
            (rpt<<22)+     /* auto reload on/off */
            (2<<20)+       /* data size, byte,hword,word */
            (size/4);      /* transfer size (hwords) */

    rDMASKTRIG2=(0<<2)+(1<<1)+0;  /* no-stop, DMA2 channel on, no-sw trigger */

}

#ifdef USEDMA2IRQ

/*-----------------------------------------------------------------------------
Table  - Interupt Controller Registers
-----------------------------------------------------------------------------*/

#define rSRCPND		(*(volatile unsigned *)0x14400000)
#define rINTMOD		(*(volatile unsigned *)0x14400004)
#define rINTMSK		(*(volatile unsigned *)0x14400008)
#define rPRIORITY	(*(volatile unsigned *)0x1440000c)
#define rINTPND		(*(volatile unsigned *)0x14400010)
#define rINTOFFSET	(*(volatile unsigned *)0x14400014)

/*-----------------------------------------------------------------------------
Part #4 - using Interupts
-----------------------------------------------------------------------------*/

/*
Since the Table of Adresses for Interupt-Service-Routines appearently is not
mapped to a fixed memory location, the bios provides two system-calls (aka swi)
for manipulating it.
*/



/*
        this routine plays one chunk of the sample after the other
        (called from DMA2-Interupt)
*/

unsigned char *smpstart;
unsigned char *smpend;
unsigned char *smpptr;
int smpinc;

void playnextchunk(void);

void playnextchunk(void) {
unsigned long size;


        size=(smpend-smpptr);
        
                /* play next chunk */
                _GpPCMPlay(smpptr,smpinc,1);

           		smpptr+=smpinc;
				if (smpptr>smpend) {
                		smpptr=smpstart;
                }
        
}

/*
The Interupt-Service-Routine should be written according to the ARM Manual,
that is preserve any registers and exit by "subs pc,lr,#4". If you want to
write this code in C you have to tell the compiler about this special require-
ment as in these examples:
*/


/* (arm sdt/ads) */
void __irq myDMA2_ISR(void);
void myDMA2_ISR(void) {

        playnextchunk();
        interrupt_mixer();

		my_tick_counter += 20;
}



/*
        Last not least, install and activate your own interupt routine like this:
*/

//#define nISR_DMA2       0x13
//#define BIT_DMA2	(0x1<<19) !!!
#define nISR_DMA2       0x13
#define BIT_DMA2	(0x1<<19)

/* for other IRQ numbers/bits refer to CPU-Manual */

void myDMA2_ISR_Install(void);

void myDMA2_ISR_Install(void) {

    /* disable interupts */
    ARMDisableInterrupt();

    /* mask all interupts

       this will obviously disable any other interupts that were previously
       active. if you dont want that, preserve the value of rINTMSK.
    */
//    rINTMSK=0xffffffff;

    /* set vector for DMA2_DONE interupt

       this systemcall copies the said vector to the table used by the interupt
       handler in the bios.

    */
    _swi_install_irq(nISR_DMA2,(void*)&myDMA2_ISR);

    /* set irq regs for DMA2_DONE interupt */
    rSRCPND     =BIT_DMA2;
    rINTPND     =BIT_DMA2;
    rINTMOD    &=~(BIT_DMA2);
    rINTMSK    &=~(BIT_DMA2);

    /* enable interupts again */
   ARMEnableInterrupt();

}

#endif

struct tag_iis_info{
	int cmd;
	int addr_mode;
	unsigned int mem_addr;
	unsigned int isr_func;
	int src_bytes;
	int sr_rate;
};


void __GpIsrSound(void) {
}
/*-----------------------------------------------------------------------------
Part #5 - finally playing a sound
-----------------------------------------------------------------------------*/

/* these refer to the sample-data */


/*
         this is the main entry point of the example
*/

void gp32sound_init(short int *buffer,int buffer_size,int BUFFER1_SIZE) {
struct tag_iis_info _iis_info;
    /* stop dma2 */
    rDMASKTRIG2=(1<<2)+(0<<1)+0;  /* stop, DMA2 channel off, no-sw trigger */

    /* initialize uda1330a by sending appropriate commands over L3 bus */
    Init1330();
 
 
    /* setup IIS bus */
    InitIIS();

    /* play sample in a loop */



   
        /* install dma2-irq */
        myDMA2_ISR_Install();
        /* setup pointers for playnextchunk() */
        smpstart=(unsigned char *)buffer;
        smpend=(unsigned char *)(((int)buffer)+buffer_size);
        smpptr=smpstart;
		smpinc=BUFFER1_SIZE;        
        /* start playing */
        playnextchunk();


    /* wait forever so sample can be heard */


#ifdef __GCC__
    /* will never be executed but stays here to eliminate warning */
    return(0);
#endif

}