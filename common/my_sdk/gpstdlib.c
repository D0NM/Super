#include "gpdef.h"
#include "gpcomm.h"
#include "gpstdlib.h"
#include "gpos_def.h"

extern void _GPOSTickISR(void);
extern void _gp_timer0_int(void);
GPMEMFUNC gp_mem_func;
GPSTRFUNC gp_str_func;

unsigned int * _timepassed;
int _before_out_keydata;
int _gpKEYCHANGED;
int _gpKEYPOLLING;
unsigned int * _reg_io_key_a, * _reg_io_key_b;

unsigned int __gpver_for_hw;
unsigned int __gpver_for_fw;

//===============================================
#define TMR01_FREQ		20000	//10khz ==> 10000 = PCLK/(TMR01_DIV*(TMR01_PRESCALE+1))
#define TMR0_TPS		1000	//1khz = 1msec

volatile unsigned long gpnet_cticks = 0L;
volatile int gpnet_MSPT = 1000;
volatile int gpnet_cur_ms = 0;

struct tag_timer_reg{
	int cmd;		//0 - all stop/disable, (1 - setting | 2 - start), 4 - pause, 8 - stop
	int val_timer;
	unsigned int evt_handler;
	int val_freq;
	int val_tps;
};
extern void swi_install_irq(int /*int_idx*/, unsigned int /*func*/); 
extern void swi_uninstall_irq(int /*int_idx*/);
extern int swi_timer_operate(struct tag_timer_reg *);
extern unsigned int swi_usb_sof_get(void);
extern void swi_usb_line_reset(void);

/*************************** Key Input Processing *****************/
extern int _VirtualKeyMap(int loop_cnt);
int GpKeyGet(void)
{   
	int data = _VirtualKeyMap(_gpKEYPOLLING);
	_gpKEYCHANGED = _before_out_keydata ^ data;
	_before_out_keydata = data;
	return data;
}
int GpKeyChanged(void)
{
	return _gpKEYCHANGED;
}

int GpKeyGetEx(int * data)
{
	*data = _VirtualKeyMap(_gpKEYPOLLING);
	_gpKEYCHANGED = _before_out_keydata ^ *data;
	_before_out_keydata = *data;
	return _gpKEYCHANGED;
}

void GpKeyInit(void)
{
	_before_out_keydata = GPC_VK_NONE;
	_gpKEYCHANGED = GPC_VK_NONE;	
}

void GpKeyPollingTimeSet(int loop_cnt)
{
	if (loop_cnt < 1) 
		_gpKEYPOLLING = 1;
	else
		_gpKEYPOLLING = loop_cnt;
}

/****************************BINGGO SDK INTERFACE***********************/
//Timer=================================================================
void _system_timer_init(void)
{
	struct tag_timer_reg _timer;
	//timer 0 and 1
	_timer.cmd = 0;
	//rTCON = 0;
	swi_timer_operate(&_timer);
	//timer 0
	_timer.cmd = 1;
	_timer.evt_handler = (unsigned)_gp_timer0_int;
	_timer.val_timer = 0;
	_timer.val_freq = TMR01_FREQ;
	_timer.val_tps = TMR0_TPS;
	swi_timer_operate(&_timer);
	//timer 1
	_timer.cmd = 1;
	_timer.evt_handler = (unsigned)_GPOSTickISR;
	_timer.val_timer = 1;
	_timer.val_freq = TMR01_FREQ * 2;
	_timer.val_tps = TMR0_TPS;
	swi_timer_operate(&_timer);
	
	_timer.cmd = 2;
	_timer.val_timer = 0;
	swi_timer_operate(&_timer);
}

//SDK INIT=====================================================================
extern void swi_port_config_reset(void);
void _gp_sdk_init(void)
{
	GpKeyInit();
	_system_timer_init();
}

int GpUSBLineCheck(void)
{
	unsigned int n_tick;
	unsigned int sof, sof1;
		
	sof=sof1=0;	
	
	n_tick = GpTickCountGet();
	sof = swi_usb_sof_get();
	while ( GpTickCountGet() - n_tick < 30 )
	;
	sof1 = swi_usb_sof_get();
	if ( sof == sof1 )
	{
		swi_usb_line_reset();
		return 0;
	}
	return 1;
}

void __decode_user_info(char * p_id, char * p_pwd)
{
	int i,j;
	char _decode_stream[9] = {"SANGHYUK\0"};
	for ( i=0; i<16; i++ )
	{
		j = i & 0x7;
		*p_id ^= _decode_stream[j];
		*p_pwd ^= _decode_stream[j];
		p_id++;
		p_pwd++;
	}
}
