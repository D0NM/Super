#ifndef __gpos_internal_h__
#define __gpos_internal_h__

//#include "24x.h"
#include "gpos_def.h"

#define GPOS_STAT_RUN			0x1
#define GPOS_STAT_READY			0x2
#define GPOS_STAT_SUSPEND		0x4
#define GPOS_STAT_NO_OPER		0x8
#define GPOS_STAT_PEND			0x10
#define GPOS_STAT_BLOCKED		0x20
#define GPOS_STAT_RT_PEND		0x40
#define GPOS_STAT_PAUSED		0x80

#define GPOS_PRIO_HARD_RT		0
#define GPOS_PRIO_SOFT_RT		1
#define GPOS_PRIO_IDLE_LEVEL	6

#define GPOS_FLAG_NORMAL		0
#define GPOS_FLAG_PEND_MEM		0x1
#define GPOS_FLAG_PEND_NET		0x2
#define GPOS_FLAG_PEND_FIO		0x4
#define GPOS_FLAG_PEND_GDI		0x8
#define GPOS_FLAG_PEND_SPKEY_A	0x10
#define GPOS_FLAG_PEND_SPKEY_B	0x20

#define GPOS_CONT_STARTED		0x1
#define GPOS_CONT_SCHED_LOCKED	0x2
#define GPOS_CONT_PRIO_REV		0x4

//CAUTION : IF YOU CHANGE THE FOLLOWINGS, THE GPKERNEL.S MUST BE CHECKED!!
typedef struct tagGP_THREAD GP_THREAD;
struct tagGP_THREAD
{
	void * stack_ptr;
	void * init_stack_ptr;
	int stk_size;
	int t_id;
	int t_priority;
	int t_state;
	int t_stop_tick;
	int t_run_tick;
	int t_delay_tick;
	int t_max_delay;
	int t_max_run;
	int t_flag;
	void (*entry_point)(void *);
};
extern GP_THREAD _gp_thread_list[8];
extern GP_THREAD * _gp_cur_thread;
extern GP_THREAD * _gp_highest_thread;
extern GP_THREAD * _gp_ret_from_rt;

#define PEND_BUF_COUNT	8
#define PEND_OFF_MEM	0
#define PEND_OFF_NET	8
#define PEND_OFF_FIO	16
#define PEND_OFF_GDI	24

#define GPOS_RESID_MEM	0
#define GPOS_RESID_NET	1
#define GPOS_RESID_FIO	2
#define GPOS_RESID_GDI	3
	
extern GP_THREAD * _gp_pend_list[32];
extern GP_THREAD * _gp_t_lock_thread[4];
extern int _gp_pend_rw_idx[4][2];

extern int _gp_t_count;
extern int _gp_os_status;

void GpTaskSW(void);
void _GpIsrSound(void);

void GpProcSound(void * arg);
void GpProcTCPIP(void * arg);
void GpProcTmr2(void * arg);
void GpProcTmr3(void * arg);
void GpProcTmr4(void * arg);
void GpProcTmr5(void * arg);

void _gp_os_schedule(void);
void * _os_init_thread_stack(void (*func)(void *), unsigned int * ptr);
void _gp_os_sched_lock(void);
void _gp_os_sched_unlock(void);
void _os_stack_copy(void * o_stk, void * n_stk);
void _gp_os_res_lock(int idx);
void _gp_os_res_unlock(int idx);
int _is_gpnet_available(void);
int _gp_os_opt_assign(GP_THREAD * t_thread, int priority, int stk_size);

typedef struct tagPROGTIMER{
	int enableflag;
	int max_exec_tick;		//execution time = 2msec * max_exec_tick
	int tmr_tps;			//tick per 2msec
	void (*irq_tmrfunc)(void);
}PROGTIMER;
extern PROGTIMER gProgTimer[GPC_MAXPROGTIMER];

extern void ARMDisableInterrupt(void);
extern void ARMEnableInterrupt(void);

#endif