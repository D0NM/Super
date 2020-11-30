#include "gpdef.h"
#include "gpstdlib.h"

#include "gpos_internal.h"

#define HARD_RT_STACK_SIZE		(2<<10)
#define IDLE_STACK_SIZE			(256)

GP_THREAD _gp_thread_list[8];
GP_THREAD * _gp_cur_thread = NULL;
GP_THREAD * _gp_highest_thread = NULL;
GP_THREAD * _gp_ret_from_rt = NULL;
PROGTIMER gProgTimer[GPC_MAXPROGTIMER];

GP_THREAD * _gp_pend_list[32];
GP_THREAD * _gp_t_lock_thread[4];
int _gp_pend_rw_idx[4][2];

int _gp_t_count = 0;
int _gp_os_status = 0;

/*
********************************************************************************************************
_os_init_thread_stack
	context image pc - r0, cpsr : zero initialize
	return value : the current stack value of the thread
********************************************************************************************************
*/
void * _os_init_thread_stack(void (*func)(void *), unsigned int * ptr)
{
	ptr = (unsigned int*)((unsigned)ptr & ~0x7);
	*(--ptr) = (unsigned int)func;	//pc
	*(--ptr) = 0;		//r14
	*(--ptr) = 0;		//r12
	*(--ptr) = 0;		//r11
	*(--ptr) = 0;		//r10
	*(--ptr) = 0;		//r9
	*(--ptr) = 0;		//r8
	*(--ptr) = 0;		//r7
	*(--ptr) = 0;		//r6
	*(--ptr) = 0;		//r5
	*(--ptr) = 0;		//r4
	*(--ptr) = 0;		//r3
	*(--ptr) = 0;		//r2
	*(--ptr) = 0;		//r1
	*(--ptr) = 0;		//r0
	*(--ptr) = 0x40 | 0x13;		//cpsr = iF_SVC
	//*(--ptr) = 0xc0 | 0x13;		//spsr = IF_SVC
	return (void *)ptr;
}

/*
********************************************************************************************************
_os_exit
********************************************************************************************************
*/
void _os_exit(void)
{
}

/*
********************************************************************************************************
_os_idle_thread
********************************************************************************************************
*/
void _os_idle_thread(void * arg)
{
	while ( 1 )
	;
}

/*
********************************************************************************************************
_gp_thread_create
********************************************************************************************************
*/
void _gp_thread_create(void (*func)(void *), int priority, int stk_size)
{
	GP_THREAD * tmp_thread;
	unsigned char * s_ptr;
	
	if ( stk_size)
	{
		stk_size += 0x7f;
		stk_size &= (~0x7f);
		s_ptr = (unsigned char *)gp_mem_func.zimalloc(stk_size);
	}
	else
	{
		stk_size = 0;
		s_ptr = NULL;
	}
	
	tmp_thread = &_gp_thread_list[_gp_t_count];
	if ( s_ptr )
	{
		tmp_thread->init_stack_ptr = (void*)s_ptr;
		s_ptr += stk_size;	//8byte align
		tmp_thread->stack_ptr = _os_init_thread_stack(func, (unsigned int*)s_ptr);
	}
	else
	{
		tmp_thread->init_stack_ptr = NULL;
		tmp_thread->stack_ptr = NULL;
	}
	tmp_thread->stk_size = stk_size;
	tmp_thread->t_id = _gp_t_count;
	tmp_thread->t_state = GPOS_STAT_NO_OPER;
	tmp_thread->t_priority = priority;
	tmp_thread->t_stop_tick = 0;
	tmp_thread->t_delay_tick = 0;
	tmp_thread->t_run_tick = 0;
	tmp_thread->t_max_run = GPOS_DFT_RUNNING_TIME;
	tmp_thread->t_flag = GPOS_FLAG_NORMAL;
	tmp_thread->entry_point = func;
	_gp_t_count++;
}

/*
********************************************************************************************************
GpInitializeOS
********************************************************************************************************
*/
void GpKernelInitialize(void)
{
	int i;
	
	_gp_t_count = 0;
	_gp_cur_thread = NULL;
	_gp_highest_thread = NULL;
	
	_gp_os_status = GPOS_CONT_SCHED_LOCKED;
	
	_gp_thread_create(GpProcSound, GPOS_PRIO_HARD_RT, HARD_RT_STACK_SIZE);				//2KB stack
	_gp_thread_create(_os_idle_thread, GPOS_PRIO_IDLE_LEVEL, IDLE_STACK_SIZE);			//100B
	_gp_thread_create(GpMain, GPOS_PRIO_NORMAL, GpPredefinedStackGet(H_THREAD_GPMAIN));	//100KB stack
	_gp_thread_create(GpProcTCPIP, GPOS_PRIO_SOFT_RT, 0);								//64KB stack
	_gp_thread_create(GpProcTmr2, GPOS_PRIO_ABOVE_NORMAL, 0);			//4KB stack
	_gp_thread_create(GpProcTmr3, GPOS_PRIO_ABOVE_NORMAL, 0);			//4KB stack
	_gp_thread_create(GpProcTmr4, GPOS_PRIO_ABOVE_NORMAL, 0);			//4KB stack
	_gp_thread_create(GpProcTmr5, GPOS_PRIO_ABOVE_NORMAL, 0);			//4KB stack
	
	_gp_thread_list[1].t_state = GPOS_STAT_READY;
	_gp_thread_list[2].t_state = GPOS_STAT_READY;
	_gp_cur_thread = NULL;
	_gp_highest_thread = &_gp_thread_list[2];
	
	for ( i=0; i<24; i++ )
	{
		_gp_pend_list[i] = NULL;
	}
	for ( i=0; i<4; i++ )
	{
		_gp_t_lock_thread[i] = NULL;
		_gp_pend_rw_idx[i][0] = 0;
		_gp_pend_rw_idx[i][1] = 0;
	}
	for ( i=0; i<GPC_MAXPROGTIMER; i++ )
	{
		gProgTimer[i].enableflag = 0;
		gProgTimer[i].max_exec_tick = 0;
		gProgTimer[i].tmr_tps = 2;
		gProgTimer[i].irq_tmrfunc = NULL;
	}
}

/*
********************************************************************************************************
_gp_os_schedule
********************************************************************************************************
*/
void _gp_os_schedule(void)
{
	GP_THREAD * tmp_thread;
	int i;
	
	_gp_highest_thread = &_gp_thread_list[1];
	for ( i=2; i<8; i++ )
	{
		tmp_thread = &_gp_thread_list[i];
		if ( tmp_thread->t_state == GPOS_STAT_PEND )
		{
			tmp_thread->t_stop_tick++;
		}
		if ( tmp_thread->t_state == GPOS_STAT_READY )
		{
			if ( tmp_thread->t_priority < _gp_highest_thread->t_priority )
			{
				_gp_highest_thread = tmp_thread;
			}
			else if ( tmp_thread->t_priority == _gp_highest_thread->t_priority )
			{
				if ( tmp_thread->t_stop_tick >= _gp_highest_thread->t_stop_tick )
				{
					_gp_highest_thread = tmp_thread;
				}
			}
		}
	}
}

void _gp_os_res_lock(int idx)
{
	int n_off;
	int t_val;
#if 1	
	ARMDisableInterrupt();
	t_val = _gp_os_status & (GPOS_CONT_SCHED_LOCKED | GPOS_CONT_STARTED);
	if ( t_val == GPOS_CONT_STARTED )
	{
		if ( _gp_t_lock_thread[idx] )
		{
			n_off = idx * PEND_BUF_COUNT;
			n_off += _gp_pend_rw_idx[idx][1];
			_gp_pend_list[n_off] = _gp_cur_thread;
			_gp_pend_rw_idx[idx][1]++;
			if (_gp_pend_rw_idx[idx][1] >= PEND_BUF_COUNT )
				_gp_pend_rw_idx[idx][1] = 0;
			_gp_cur_thread->t_state = GPOS_STAT_PEND;
			_gp_cur_thread->t_stop_tick++;
			_gp_highest_thread = _gp_t_lock_thread[idx];
			GpTaskSW();
		}
		else
		{
			_gp_t_lock_thread[idx] = _gp_cur_thread;
			_gp_cur_thread->t_flag |= (1 << idx);
		}
	}
	else if ( t_val & GPOS_CONT_STARTED )
	{
		if ( _gp_t_lock_thread[idx] )
		{
			n_off = idx * PEND_BUF_COUNT;
			t_val = _gp_pend_rw_idx[idx][0];
			t_val += (PEND_BUF_COUNT-1);
			if ( t_val >= PEND_BUF_COUNT )
				t_val -= PEND_BUF_COUNT;
			n_off += t_val;
			_gp_pend_rw_idx[idx][0] = t_val;
			_gp_pend_list[n_off] = _gp_cur_thread;
			_gp_cur_thread->t_state = GPOS_STAT_PEND;
			_gp_cur_thread->t_stop_tick++;
			_gp_highest_thread = _gp_t_lock_thread[idx];
			GpTaskSW();
		}
		else
		{
			_gp_t_lock_thread[idx] = _gp_cur_thread;
			_gp_cur_thread->t_flag |= (1 << idx);
		}
	}
	ARMEnableInterrupt();
#endif	
}

void _gp_os_res_unlock(int idx)
{
	GP_THREAD * tmp_thread;
	int n_off;
	
	int t_val;
#if 1	
	ARMDisableInterrupt();
	t_val = _gp_os_status & (GPOS_CONT_SCHED_LOCKED | GPOS_CONT_STARTED);
	
	if ( t_val & GPOS_CONT_STARTED )
	{
_re_check_pended_t:		
		if ( _gp_pend_rw_idx[idx][0] == _gp_pend_rw_idx[idx][1] )
		{
			_gp_t_lock_thread[idx] = NULL;
			_gp_cur_thread->t_flag &= (~(1<<idx));
		}
		else
		{
			_gp_cur_thread->t_state = GPOS_STAT_READY;
			
			n_off = idx * PEND_BUF_COUNT;
			n_off += _gp_pend_rw_idx[idx][0];
			tmp_thread = _gp_pend_list[n_off];
			_gp_pend_rw_idx[idx][0]++;
			if ( _gp_pend_rw_idx[idx][0] >= PEND_BUF_COUNT )
				_gp_pend_rw_idx[idx][0] = 0;
			
			switch ( tmp_thread->t_state )
			{
			case GPOS_STAT_NO_OPER:
				goto _re_check_pended_t;
				break;
			default:
				break;
			}
			_gp_highest_thread = tmp_thread;
			_gp_t_lock_thread[idx] = tmp_thread;
			GpTaskSW();
/***********************************************************************
			if ( tmp_thread->t_priority < _gp_cur_thread->t_priority )
			{
				_gp_cur_thread->t_state = GPOS_STAT_READY;
				_gp_highest_thread = tmp_thread;
				_gp_t_lock_thread[idx] = tmp_thread;
				GpTaskSW();
			}
			else
			{
				_gp_t_lock_thread[idx] = NULL;
				tmp_thread->t_state = GPOS_STAT_READY;
				
			}
**********************************************************************/
		}
	}
	ARMEnableInterrupt();
#endif	
}

/*
********************************************************************************************************
GpTickOS
********************************************************************************************************
*/
void _gp_os_tick_sched(void)
{
	GP_THREAD * tmp_thread;
	int i,j;
	
	_gp_highest_thread = &_gp_thread_list[1];
	for ( i=2; i<8; i++ )
	{
		tmp_thread = &_gp_thread_list[i];
		if ( tmp_thread->t_state == GPOS_STAT_SUSPEND )
		{
			tmp_thread->t_delay_tick++;
			if ( tmp_thread->t_delay_tick >= tmp_thread->t_max_delay )
			{
				tmp_thread->t_delay_tick = 0;
				tmp_thread->t_stop_tick = 0;
				tmp_thread->t_state = GPOS_STAT_READY;
			}
		}
		else if ( tmp_thread->t_state == GPOS_STAT_PEND )
		{
			tmp_thread->t_stop_tick++;
		}
		else if ( tmp_thread->t_state == GPOS_STAT_RUN )
		{
			tmp_thread->t_run_tick++;
			if ( tmp_thread->t_max_run && tmp_thread->t_run_tick > tmp_thread->t_max_run )
			{
				tmp_thread->t_state = GPOS_STAT_READY;
				tmp_thread->t_stop_tick = 0;
			}			
		}
		if ( tmp_thread->t_state == GPOS_STAT_READY )
		{
			tmp_thread->t_stop_tick++;
			if ( tmp_thread->t_priority < _gp_highest_thread->t_priority )
			{
				_gp_highest_thread = tmp_thread;
			}
			else if ( tmp_thread->t_priority == _gp_highest_thread->t_priority )
			{
				//at equal priority, ROUND ROBIN
				if ( tmp_thread->t_stop_tick > _gp_highest_thread->t_stop_tick )
				{
					_gp_highest_thread = tmp_thread;
				}
			}
			else if ( _gp_os_status & GPOS_CONT_PRIO_REV ) 
			{
				j = tmp_thread->t_priority - _gp_highest_thread->t_priority;
				j <<= 3;
				if ( tmp_thread->t_stop_tick > ( j + _gp_highest_thread->t_stop_tick) )
					_gp_highest_thread = tmp_thread;
			}
		}
	}
	if ( _gp_os_status & GPOS_CONT_SCHED_LOCKED )
	{
		_gp_highest_thread = _gp_cur_thread;
		_gp_cur_thread->t_state = GPOS_STAT_RUN;	
	}
}

int _gp_os_opt_assign(GP_THREAD * t_thread, int priority, int stk_size)
{
	unsigned char * s_ptr;
	unsigned char * old_s_ptr, * new_s_ptr;
	unsigned int n_val;
	
	if ( t_thread->init_stack_ptr )
	{
		old_s_ptr = (unsigned char*)t_thread->init_stack_ptr + t_thread->stk_size;
		n_val = (unsigned)old_s_ptr;
		__asm
		{
			sub	n_val, n_val, sp
		}
		if ( stk_size <= n_val )
		{
			return 0;
		}
	}
	
	s_ptr = (unsigned char *)gp_mem_func.zimalloc(stk_size);
	if ( t_thread->init_stack_ptr ) 
	{
		new_s_ptr = s_ptr + stk_size;
		
		//ARMDisableInterrupt();
		n_val = (unsigned)old_s_ptr;
		n_val -= (unsigned)t_thread->stack_ptr;
		//ARMEnableInterrupt();
		
		_os_stack_copy(old_s_ptr, new_s_ptr);
		old_s_ptr = t_thread->init_stack_ptr;
		t_thread->init_stack_ptr = (void*)s_ptr;
		t_thread->stack_ptr = (void *)((unsigned)new_s_ptr - n_val);
		gp_mem_func.free(old_s_ptr);
	}
	else
	{
		t_thread->init_stack_ptr = (void*)s_ptr;
		s_ptr += stk_size;
		t_thread->stack_ptr = _os_init_thread_stack(t_thread->entry_point, (unsigned int*)s_ptr);
	}
	t_thread->stk_size = stk_size;
	t_thread->t_priority = priority;
	return 1;
}

int GpThreadOptSet(H_THREAD th, int priority, int stk_size)
{
	GP_THREAD * t_thread;
	
	if ( th < 2 ) return GPOS_ERR_INVALIDARG;
	if ( (priority < GPOS_PRIO_ABOVE_NORMAL) || (priority > GPOS_PRIO_LOW) )
	{
		return GPOS_ERR_INVALIDARG;
	}
	if ( stk_size < 256 )
	{
		return GPOS_ERR_STACK;
	}
	stk_size += 0x7f;
	stk_size &= (~0x7f);
	
	ARMDisableInterrupt();
#if 0	
	if ( _gp_thread_list[th].t_state & GPOS_STAT_RUN )
	{
		ARMEnableInterrupt();
		return GPOS_ERR_PREV_ACCESS;
	}
#endif
	switch ( th )
	{
	case 0:
	case 1:
		ARMEnableInterrupt();
		return GPOS_ERR_INVALID_ACCESS;
	default:
		break;
	}
	ARMEnableInterrupt();
	t_thread = &_gp_thread_list[th];
	
	_gp_os_sched_lock();
	if ( !_gp_os_opt_assign(t_thread, priority, stk_size))
	{
		_gp_os_sched_unlock();
		return GPOS_ERR_STACK;
	}
	_gp_os_sched_unlock();
	
	return GPOS_ERR_OK;
}
