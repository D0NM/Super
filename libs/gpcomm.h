#ifndef	__GPCOMM_H__
#define	__GPCOMM_H__

#include "gpstdlib.h"

//ERROR LIST===========================================
#define	GPC_COMM_OK			0x0
#define	GPC_COMM_ERR_PARAM	0x1
#define GPC_COMM_USED		0x2
#define GPC_COMM_ERR_BUF	0x3
#define GPC_COMM_UNKNOWN	0xff

#define COMM_PORT_0		0
#define COMM_PORT_1		1
#define COMM_USB_D		2
#define COMM_USB_H		3
#define COMM_APP_RF		4
#define COMM_APP_NET	5
#define COMM_APP_TV		6

typedef struct tagGPN_DESC{
	int port_kind;
	int tr_buf_size;
	int tr_rate;
	int tr_mode;
	int sz_pkt;
	void (*isr_comm_ram)(void);
	//int isr_comm_ram;
	int reserved1;
	int reserved2;
}GPN_DESC;

typedef struct tagGPN_COMM{
	int (*comm_open)(GPN_DESC * p_desc);			//return 0 
	void (*comm_hw_reset)(void);
	void (*comm_sw_reset)(void);
	void (*comm_close)(void);
	int (*comm_send_ready)(void);					//return 1
	int (*comm_send)(unsigned char * p_data, int n_sz);	//return 1
	int (*comm_send_one)(unsigned char c_data);			//return 1
	int (*comm_recv)(unsigned char * p_data, int n_max_sz);	//return size
	int (*comm_recv_one)(unsigned char * p_data);	//return 1
	unsigned char (*comm_recv_sync)(void);
	unsigned char * p_comm_buf;
	int * p_buf_size;
	int * p_comm_var[4];
	void (*force_env_clear)(void);
	int (*comm_port_ready)(void);			//return 1
	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
}GPN_COMM;
int GpCommCreate(GPN_DESC * p_desc, GPN_COMM * p_play);
void GpCommDelete(GPN_DESC * p_desc);
/*
p_play->comm_open(p_desc);
*/

#endif