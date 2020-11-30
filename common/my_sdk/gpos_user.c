#include "gpdef.h"
#include "gpos_internal.h"
#include "gpstdlib.h"

int GpTimerPause(int idx)
{
	GP_THREAD * t_thread;
	if ( idx < 0 || idx >= GPC_MAXPROGTIMER )
	{
		return GPOS_ERR_INVALIDARG;
	}
	t_thread = &_gp_thread_list[4+idx];
	ARMDisableInterrupt();
	if ( t_thread->t_flag )
	{
		ARMEnableInterrupt();
		return GPOS_ERR_PENDED_THREAD;
	}
	t_thread->t_state = GPOS_STAT_PAUSED;
	_gp_os_schedule();
	GpTaskSW();
	ARMEnableInterrupt();
	return GPOS_ERR_OK;
}

int GpTimerResume(int idx)
{
	GP_THREAD * t_thread;
	if ( idx < 0 || idx >= GPC_MAXPROGTIMER )
	{
		return GPOS_ERR_INVALIDARG;
	}
	t_thread = &_gp_thread_list[4+idx];
	ARMDisableInterrupt();
	if ( t_thread->t_state == GPOS_STAT_RUN )
	{
		t_thread->t_delay_tick = 0;
		ARMEnableInterrupt();
		return GPOS_ERR_OK;
	}
	if ( t_thread->t_state != GPOS_STAT_PAUSED )
	{
		if ( t_thread->t_state != GPOS_STAT_SUSPEND )
		{
			ARMEnableInterrupt();
			return GPOS_ERR_NOEFFECT;
		}
	}
	t_thread->t_state = GPOS_STAT_SUSPEND;
	t_thread->t_delay_tick = 0;
	_gp_os_schedule();
	GpTaskSW();
	ARMEnableInterrupt();
	return GPOS_ERR_OK;
}

void GpTimerKill(int idx)
{
	GP_THREAD * t_thread;
	
	if ( idx < 0 || idx >= GPC_MAXPROGTIMER )
	{
		/*
		ARMDisableInterrupt();
		if ( _gp_cur_thread->t_id >= H_THREAD_TMR0 )
		{
			_gp_cur_thread->t_state = GPOS_STAT_NO_OPER;
			_gp_os_schedule();
			GpTaskSW();
		}
		ARMEnableInterrupt();
		*/
		return;
	}
	t_thread = &_gp_thread_list[4+idx];
	ARMDisableInterrupt();
	if ( t_thread->t_state == GPOS_STAT_NO_OPER )
	{
		ARMEnableInterrupt();
		return;
	}
	_gp_os_status |= (GPOS_CONT_SCHED_LOCKED);
	if ( t_thread->init_stack_ptr )
	{
		gp_mem_func.free(t_thread->init_stack_ptr);
		t_thread->init_stack_ptr = NULL;
	}
	
	t_thread->t_state = GPOS_STAT_NO_OPER;
	gProgTimer[idx].enableflag = 0;
	_gp_os_schedule();
	_gp_os_status &= (~GPOS_CONT_SCHED_LOCKED);
	if ( _gp_highest_thread->t_id != _gp_cur_thread->t_id )
		GpTaskSW();	
	ARMEnableInterrupt();
}

int GpTimerSet(int idx)
{
	GP_THREAD * tmp_thread;
	int d;
	void (*p_func)(void * arg);
	unsigned char * s_ptr;
	
	if (idx < 0 || idx >= GPC_MAXPROGTIMER)
		return GPOS_ERR_INVALIDARG;
	
	d = 1000 / gProgTimer[idx].tmr_tps;
	d >>= 1;
	if ( d < 1 )
	{
		return GPOS_ERR_NOEFFECT;
	}
	
	_gp_os_sched_lock();
	if (!gProgTimer[idx].irq_tmrfunc)
	{
		_gp_os_sched_unlock();
		return GPOS_ERR_NOEFFECT;
	}
	else if ( gProgTimer[idx].enableflag )
	{
		_gp_os_sched_unlock();
		return GPOS_ERR_ALREADY_USED;
	}
	switch ( idx )
	{
	case 0:
		p_func = GpProcTmr2;
		break;
	case 1:
		p_func = GpProcTmr3;
		break;
	case 2:
		p_func = GpProcTmr4;
		break;
	case 3:
		p_func = GpProcTmr5;
		break;
	}
	
	tmp_thread = &_gp_thread_list[4 + idx];
	if ( tmp_thread->stk_size < 256 )
	{
		tmp_thread->stk_size = GpPredefinedStackGet((H_THREAD)(H_THREAD_TMR0 + idx));
	}
	if ( tmp_thread->stk_size < 256 )
	{
		_gp_os_sched_unlock();
		return GPOS_ERR_STACK;
	}
	
	if ( tmp_thread->init_stack_ptr )
	{
		tmp_thread->stack_ptr = _os_init_thread_stack(p_func, (unsigned int*)((char*)tmp_thread->init_stack_ptr + tmp_thread->stk_size));
	}
	else
	{
		s_ptr = (unsigned char*)gp_mem_func.zimalloc(tmp_thread->stk_size);
		if ( !s_ptr )
		{
			_gp_os_sched_unlock();
			return GPOS_ERR_OUTOFMEM;
		}
		tmp_thread->init_stack_ptr = (void*)s_ptr;
		s_ptr += tmp_thread->stk_size;
		tmp_thread->stack_ptr = _os_init_thread_stack(p_func, (unsigned int*)s_ptr);
	}
	
	tmp_thread->t_delay_tick = (d>>1);
	tmp_thread->t_max_delay = d;
	tmp_thread->t_run_tick = 0;
	tmp_thread->t_stop_tick = 0;
	tmp_thread->t_max_run = gProgTimer[idx].max_exec_tick;
	tmp_thread->t_flag = GPOS_FLAG_NORMAL;
	
	ARMDisableInterrupt();
	tmp_thread->t_state = GPOS_STAT_SUSPEND;
	gProgTimer[idx].enableflag = 1;
	//ARMEnableInterrupt();
	_gp_os_sched_unlock();
	return GPOS_ERR_OK;
}

int GpTimerOptSet(int idx, int tmr_tps, int max_exec_tick, void (*irq_tmrfunc)(void))
{
	if ( tmr_tps < 1 )
		return GPOS_ERR_INVALIDARG;
	if ( irq_tmrfunc == NULL )
		return GPOS_ERR_NOEFFECT;
	if ( idx < 0 || idx >= GPC_MAXPROGTIMER )
		return GPOS_ERR_INVALIDARG;
	
	ARMDisableInterrupt();
	if ( gProgTimer[idx].enableflag )
	{
		ARMEnableInterrupt();
		return GPOS_ERR_ALREADY_USED;
	}
	gProgTimer[idx].max_exec_tick = max_exec_tick;
	gProgTimer[idx].tmr_tps = tmr_tps;
	gProgTimer[idx].irq_tmrfunc = irq_tmrfunc;
	
	ARMEnableInterrupt();
	return GPOS_ERR_OK;
}

int _is_gpnet_available(void)
{
	_gp_os_sched_lock();
	if ( _gp_thread_list[H_THREAD_NET].t_state != GPOS_STAT_NO_OPER )
	{
		_gp_os_sched_unlock();
		return 1;
	}
	_gp_os_sched_unlock();
	return 0;
}

void (*net_tk_yield)(void);
void GpNetThreadAct(void (*t_func)(void))
{
	GP_THREAD * t_thread;
	int stk_size;
	
	_gp_os_sched_lock();

	stk_size = GpPredefinedStackGet(H_THREAD_NET) + 0x7f;
	stk_size &= (~0x7f);
	t_thread = &_gp_thread_list[H_THREAD_NET];
	if ( t_thread->init_stack_ptr )
	{
		gp_mem_func.free(t_thread->init_stack_ptr);
		t_thread->init_stack_ptr = NULL;
	}
	if ( !_gp_os_opt_assign(t_thread, GPOS_PRIO_SOFT_RT, stk_size) )
	{
	}
	
	t_thread->t_state = GPOS_STAT_READY;
	t_thread->t_max_delay = 50;
	t_thread->t_delay_tick = 0;
	t_thread->t_stop_tick = 0;
	t_thread->t_run_tick = 0;
	t_thread->t_max_run = 0;
	net_tk_yield = t_func;
	_gp_os_sched_unlock();
}

void GpNetThreadDelete(void)
{
	GP_THREAD * t_thread;
	
	t_thread = &_gp_thread_list[H_THREAD_NET];
	ARMDisableInterrupt();
#ifdef __ACHI_PPP_DEBUG
	_achi_ppp_debug(2);
#endif	
	net_tk_yield = NULL;
	if ( t_thread->t_state == GPOS_STAT_NO_OPER )
	{
#ifdef __ACHI_PPP_DEBUG
		_achi_ppp_debug(101);
#endif				
		ARMEnableInterrupt();
		return;
	}
	
	//_gp_os_sched_lock();	//achi 2001.12.17
	if ( t_thread->init_stack_ptr )
	{
		gp_mem_func.free(t_thread->init_stack_ptr);
		t_thread->init_stack_ptr = NULL;
	}
	t_thread->t_state = GPOS_STAT_NO_OPER;
#ifdef __ACHI_PPP_DEBUG
	_achi_ppp_debug(3);
#endif	
	_gp_os_schedule();
	_gp_os_status &= (~GPOS_CONT_SCHED_LOCKED);
#ifdef __ACHI_PPP_DEBUG
	_achi_ppp_debug(4);
#endif	
	GpTaskSW();
	ARMEnableInterrupt();
}

void GpProcTCPIP(void * arg)
{
	while ( 1 )
	{
		_gp_os_res_lock(GPOS_RESID_NET);
		if ( net_tk_yield )		net_tk_yield();
		else
		{
			_gp_os_res_unlock(GPOS_RESID_NET);
			GpNetThreadDelete();
		}
		_gp_os_res_unlock(GPOS_RESID_NET);
		GpThreadSleep(50);
	}
}