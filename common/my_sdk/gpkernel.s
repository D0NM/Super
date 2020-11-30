SIZEOF_THREAD_STR	EQU	(13 << 2)
	
		AREA _GP_KERNEL, CODE, READONLY
	
		EXPORT	GpKernelStart
		EXPORT 	GpKernelOptSet
		EXPORT	GpThreadSleep
		EXPORT	_GPOSTickISR
		EXPORT 	GpTaskSW
		
		EXPORT	_GpIsrSound
		EXPORT	_GpIsrSpKeyA	;start
		EXPORT	_GpIsrSpKeyB	;select
		EXPORT	GpKernelLock
		EXPORT	GpKernelUnlock
		EXPORT	_gp_os_sched_lock
		EXPORT	_gp_os_sched_unlock
		EXPORT	GpThreadHandleGet
		EXPORT	_os_stack_copy
		EXPORT	GpProcTmr2
		EXPORT	GpProcTmr3
		EXPORT	GpProcTmr4
		EXPORT	GpProcTmr5
		
		IMPORT	_gp_os_schedule
		IMPORT	_gp_cur_thread
		IMPORT	_gp_highest_thread
		IMPORT	_gp_os_status
		IMPORT	_gp_thread_list		
		IMPORT 	_gp_ret_from_rt
		IMPORT	_gp_os_tick_sched
		IMPORT 	gProgTimer
		IMPORT	GpTimerKill
		IMPORT	gpnet_cticks  				;/* clock ticks since startup */
;	[ OS_DEBUG
;		IMPORT	_bef_achi_t
;		IMPORT	_aft_achi_t
;	]	
		MACRO
$ibit_disable	IBIT_DISABLE	$reg
$ibit_disable
		mrs		$reg,cpsr
		orr		$reg,$reg,#0xc0
		msr		cpsr_cxsf,$reg
		MEND
	
		MACRO	
$ibit_enable 	IBIT_ENABLE		$reg
$ibit_enable
		mrs		$reg,cpsr
		bic		$reg,$reg,#0x80
		msr		cpsr_cxsf,$reg
		MEND
		
ACHI_P_DIVIDE
		;a = b / c, remain
		;return r0 = a, r1 = remain
		stmdb	sp!,{r2,r3,lr}
		mov		r2,r0
		mov		r3,r1
		mov		r0,#0
0
		cmp		r2,r3
		blt		%F1
		add		r0,r0,#1		
		sub		r2,r2,r3
		b		%B0		
1
		mov		r1,r2
		ldmia	sp!,{r2,r3,pc}
			
;*****************************************************************************************************
;GpKernelStart
;*****************************************************************************************************	
GpKernelStart
		IBIT_DISABLE r0
		
		ldr		r0,=_gp_cur_thread
		ldr		r1,=_gp_highest_thread
		ldr		r2,[r1]
		str		r2,[r0]
		mov		r3,#1
		str		r3,[r2,#20]				;store 1 to t_state by GPOS_STAT_RUN
		ldr		sp,[r2]
		
	;_gp_os_status |= GPOS_CONT_STARTED	
	;_gp_os_status &= (~GPOS_CONT_SCHED_LOCKED)
		ldr		r1,=_gp_os_status		
		ldr		r2,[r1]
		orr		r2,r2,#1
		bic		r2,r2,#2
		str		r2,[r1]
	;*context loading
		mrs		r0,cpsr
		orr		r0,r0,#0xc0
		msr		cpsr_cxsf,r0
		
		ldr		r0,[sp],#4				;load cpsr
		msr		spsr_cxsf,r0	
		ldmia	sp!,{r0-r12,lr}
		ldr		lr,[sp],#4
	;*backup r0-r3,lr,spsr
		stmdb	sp!,{r0-r3,lr}
		mrs		r0,spsr
		stmdb	sp!,{r0}
		
	;*ostimer start
		mov		r1,#2					;only timer start
		mov		r2,#1					;timer 1
		stmdb	sp!,{r1,r2}
		mov		r0,sp
		swi		0x13					;_SCMD_TMR_OPER
		add		sp,sp,#8

	;*restore r0-r3,lr,spsr	
		ldmia	sp!,{r0}
		msr		spsr_cxsf,r0
		ldmia	sp!,{r0-r3,lr}
	;jump task
		movs	pc,lr
;*****************************************************************************************************
;void GpKernelOptSet(int flag)
;*****************************************************************************************************	
GpKernelOptSet
		stmdb	sp!,{r1-r3,lr}
	;interrupt disable
		mrs		r3,cpsr
		orr		r3,r3,#0xc0
		msr		cpsr_cxsf,r3
	;_gp_os_status = (_gp_os_status & 0x3) | (flag << 2)	
		ldr		r2,=_gp_os_status
		ldr		r3,[r2]
		and		r3,r3,#3
		orr		r3,r3,r0,lsl #2
		str		r3,[r2]
	;interrupt enable	
		mrs		r3,cpsr
		bic		r3,r3,#0x80
		msr		cpsr_cxsf,r3
		ldmia	sp!,{r1-r3,pc}
;*****************************************************************************************************
;void GpThreadSleep(unsigned int delay)
;*****************************************************************************************************
GpThreadSleep
		stmdb	sp!,{r1-r3,lr}
	
	;if ( delay < 1 ) return;
		cmp		r0,#1
		ldmltia	sp!,{r1-r3,pc}
	
	;ARMDisableInterrupt();	
		IBIT_DISABLE r3
	
	;_gp_cur_thread->t_state = GPOS_STAT_SUSPEND;
	;_gp_cur_thread->t_delay_tick = 0;
	;_gp_cur_thread->t_max_delay = delay;	
		mov		r3,r0					;t_max_delay = delay
		ldr		r0,=_gp_cur_thread
		ldr		r0,[r0]
		mov		r1,#0x4					;t_state = GPOS_STAT_SUSPEND
		mov		r2,#0					;t_delay_tick = 0
		str		r1,[r0,#20]
		add		r0,r0,#32
		stmia	r0,{r2,r3}
	
	;_gp_os_schedule();	
		stmdb	sp!,{r4-r12,lr}
		bl		_gp_os_schedule
		ldmia	sp!,{r4-r12,lr}
	
	;GpTaskSW();	
		bl		GpTaskSW
	
	;ARMEnableInterrupt();	
		IBIT_ENABLE r3
		ldmia	sp!,{r1-r3,pc}
;=====================================================================================================
_isr_no_oper_return
	ldmia	sp!,{r0-r5}
	subs	pc,lr,#4
	
;*****************************************************************************************************
;_GPOSTickISR
;*****************************************************************************************************		
_GPOSTickISR
		stmfd	sp!,{r0-r5}			;irq sp
		
		ldr		r0,=_gp_os_status
		ldr		r0,[r0]
		tst		r0,#1
		beq		_isr_no_oper_return
		
;if ( _gp_cur_thread->t_id == 0 ) i.e. hart_rt_thread
;{
;	//jump to hard rt
;}
		ldr		r0,=_gp_cur_thread
		ldr		r0,[r0]
		ldr		r1,[r0,#12]
		cmp		r1,#0
		beq		_isr_no_oper_return
		
		stmdb	sp!,{r8-r12,lr}
		bl		_gp_os_tick_sched
		ldmia	sp!,{r8-r12,lr}
	
	;if ( _gp_cur_thread->t_state == GPOS_STAT_RUN )
	;{
	;	if ( _gp_highest_thread->t_priority < _gp_cur_thread->t_priority )
	;	{
	;		_gp_highest_thread->t_stop_tick = 0;
	;		//task switch
	;	}
	;}
	;else
	;	//task switch
	
		ldr		r4,=_gp_cur_thread
		ldr		r0,[r4]
		ldr		r2,[r0,#20]			;_gp_cur_thread->t_state
		cmp		r2,#1
		bne		%F1
		ldr		r5,=_gp_highest_thread
		ldr		r1,[r5]
		ldr		r2,[r0,#16]			;_gp_cur_thread->t_priority
		ldr		r3,[r1,#16]			;_gp_highest_thread->t_priority
		cmp		r2,r3
		bgt		%F1
		
		ldr		r4,[r4]
		str		r4,[r5]				;_gp_highest_thread = _gp_cur_thread
		mov		r2,#1
		str		r2,[r0,#20]			;_gp_cur_thread->t_state = run
		
		b		_isr_no_oper_return
1	
		mov		r0,sp
		sub		r1,lr,#4
		mrs		r2,spsr				;spsr_irq
		add		sp,sp,#24			;irq stack restore
		orr		r3,r2,#0xc0			;interrupt disable
		msr		cpsr_cxsf,r3		;mode change to SVC
		
		stmfd	sp!,{r1}
		stmfd	sp!,{r6-r12,lr}
		mov		r6,r0
		mov		r7,r2
		ldmia	r6!,{r0-r5}
		stmfd	sp!,{r0-r5}			;save context
		stmfd	sp!,{r7}
		ldr		r0,=_gp_cur_thread
		ldr		r1,[r0]
		str		sp,[r1]
		mov		r2,#2				;_gp_cur_thread->t_state = GPOS_STAT_READY
		mov		r3,#0				;_gp_cur_thread->t_stop_tick = 0;
		add		r1,r1,#20
		stmia	r1,{r2,r3}			
		
		ldr		r1,=_gp_highest_thread
		ldr		r2,[r1]
		str		r2,[r0]
		add		r1,r2,#20
		mov		r3,#1				;_gp_highest_thread->t_state = GPOS_STAT_RUN
		mov		r4,#0				;_gp_highest_thread->t_stop_tick = 0
		mov		r5,#0				;_gp_highest_thread->t_run_tick = 0
		stmia	r1,{r3-r5}
		
		ldr		sp,[r2]				;new task_sp load
		ldmia	sp!,{r0}
		msr		spsr_cxsf,r0
		ldmia	sp!,{r0-r12,lr,pc}^
;*****************************************************************************************************
;GpTaskSW
;*****************************************************************************************************	
GpTaskSW
		stmfd	sp!,{lr}
		stmfd	sp!,{r0-r12,lr}
		mrs		r1,cpsr
		stmfd	sp!,{r1}
		ldr		r0,=_gp_cur_thread
		ldr		r1,[r0]
		str		sp,[r1]
;	[ OS_DEBUG
;		ldr		r2,=_bef_achi_t
;		str		r1,[r2]
;	]
		
		ldr		r1,=_gp_highest_thread
		ldr		r2,[r1]
		str		r2,[r0]
	
;	[ OS_DEBUG
;		ldr		r1,=_aft_achi_t
;		str		r2,[r1]
;	]
		
		mov		r1,#1
		str		r1,[r2,#20]			;_gp_highest_thread->t_state = GPOS_STAT_RUN
		
		ldr		sp,[r2]				;new task_sp load
		ldmia	sp!,{r0}
		msr		spsr_cxsf,r0
		ldmia	sp!,{r0-r12,lr,pc}^

;*****************************************************************************************************
;_GpIsrSound
;*****************************************************************************************************	
_GpIsrSound
		stmfd	sp!,{r0-r3}			;irq sp
		
		mov		r0,sp
		sub		r1,lr,#4
		mrs		r2,spsr				;spsr_irq
		add		sp,sp,#16
		orr		r3,r2,#0xc0			;interrupt disable
		msr		cpsr_cxsf,r3		;mode change to SVC
		
		stmfd	sp!,{r1}
		stmfd	sp!,{r4-r12,lr}
		mov		r4,r0
		mov		r5,r2
		ldmia	r4!,{r0-r3}
		stmfd	sp!,{r0-r3}			;save context
		stmfd	sp!,{r5}
		ldr		r0,=_gp_cur_thread
		ldr		r1,[r0]
		str		sp,[r1]
		ldr		r2,=_gp_ret_from_rt
		str		r1,[r2]
		
		ldr		r2,=_gp_thread_list
		str		r2,[r0]				;_gp_cur_thread = &_gp_thread_list[0]
		
		ldr		sp,[r2]				;new task_sp load
		ldmia	sp!,{r0}
		msr		spsr_cxsf,r0
		ldmia	sp!,{r0-r12,lr,pc}^
;*****************************************************************************************************
;_GpIsrSpKeyA	;start
;*****************************************************************************************************	
_GpIsrSpKeyA	;start

;*****************************************************************************************************
;_GpIsrSpKeyB	;select
;*****************************************************************************************************	
_GpIsrSpKeyB	;select

;*****************************************************************************************************
;void _gp_os_sched_lock(void)
;*****************************************************************************************************
GpKernelLock
_gp_os_sched_lock	
		stmdb	sp!,{r0,r1,lr}
	;ARMDisableInterrupt()
		IBIT_DISABLE r0
	;_gp_os_status |= GPOS_CONT_SCHED_LOCKED;
		ldr		r0,=_gp_os_status
		ldr		r1,[r0]
		orr		r1,r1,#0x2
		str		r1,[r0]
	;ARMEnableInterrupt();
		IBIT_ENABLE r0
		ldmia	sp!,{r0,r1,pc}
;*****************************************************************************************************
;void _gp_os_sched_unlock(void)
;*****************************************************************************************************
GpKernelUnlock
_gp_os_sched_unlock	
		stmdb	sp!,{r0-r2,lr}
	;ARMDisableInterrupt();
		IBIT_DISABLE r0
	;_gp_os_status &= ~GPOS_CONT_SCHED_LOCKED;
		ldr		r0,=_gp_os_status
		ldr		r1,[r0]
		mov		r2,#0x2
		mvn		r2,r2
		and		r1,r1,r2
		str		r1,[r0]
	;ARMEnableInterrupt();
		IBIT_ENABLE r0
		ldmia	sp!,{r0-r2,pc}
;*****************************************************************************************************
;H_THREAD GpThreadHandleGet(void)
;*****************************************************************************************************
GpThreadHandleGet
		stmdb	sp!,{r1,lr}
		IBIT_DISABLE r1
		ldr		r1,=_gp_cur_thread
		ldr		r1,[r1]
		ldr		r0,[r1,#12]				;_gp_cur_thread->t_id
		IBIT_ENABLE r1
		ldmia	sp!,{r1,pc}
;*****************************************************************************************************
;void _os_stack_copy(void * o_stk, void * n_stk);
;*****************************************************************************************************
_os_stack_copy
		stmdb	sp!,{r0-r10,lr}
		sub		r10,r0,sp
0
		cmp		r10,#32
		blt		%F1
		ldmdb	r0!,{r2-r9}
		stmfd	r1!,{r2-r9}
		sub		r10,r10,#32
		b		%B0
1
		cmp		r10,#16
		blt		%F2
		ldmdb	r0!,{r2-r5}
		stmfd	r1!,{r2-r5}
		sub		r10,r10,#16
2
		cmp		r10,#8
		blt		%F3
	[ {TRUE}
		sub		r0,r0,#8
		ldr		r2,[r0]
		ldr		r3,[r0,#4]
	|
		dmdb	r0!,{r2,r3}
	]
		stmfd	r1!,{r2,r3}
		sub		r10,r10,#8
3
		cmp		r10,#4
		blt		%F4
		ldmdb	r0!,{r2}
		stmfd	r1!,{r2}
		sub		r10,r10,#4
4
		mov		sp,r1
		ldmia	sp!,{r0-r10,pc}
;*****************************************************************************************************
;GpProcTmr
	;d = 1000 / gProgTimer[3].tmr_tps;
	;d >>= 1;
	;while ( 1 )
	;{
	;	if ( gProgTimer[3].irq_tmrfunc )
	;	{
	;		gProgTimer[3].irq_tmrfunc();	
	;		GpThreadSleep(d);
	;	}
	;	else
	;	{
	;		ARMDisableInterrupt();
	;		_gp_thread_list[7].t_state = GPOS_STAT_NO_OPER;
	;		_gp_os_schedule();
	;		GpTaskSW();
	;	}
	;}
;*****************************************************************************************************
GpProcTmr2
		mov		r2,#0
		b		_GpProcTmr
GpProcTmr3
		mov		r2,#1
		b		_GpProcTmr
GpProcTmr4
		mov		r2,#2
		b		_GpProcTmr
GpProcTmr5		
		mov		r2,#3
		b		_GpProcTmr
_GpProcTmr
		ldr		r3,=gProgTimer
		add		r3,r3,r2,lsl #4		;sizeof(PROGTIMER) = 16
		ldr		r1,[r3,#8]			;r1 = tmr_tps
		mov		r0,#125
		mov		r0,r0,lsl #3
		bl		ACHI_P_DIVIDE
		mov		r0,r0,lsr #1
		mov		r1,r3
	;r0 = d, r1 = gProgTimer[idx], r2 = idx
0
		ldr		r3,[r1,#12]
		cmp		r3,#0
		beq		%F1
		stmdb	sp!,{r0-r2,lr}
		mov		lr,pc
		mov		pc,r3
		ldr		r0,[sp]
		bl		GpThreadSleep
		ldmia	sp!,{r0-r2,lr}
		b		%B0		
1
;	[ OS_DEBUG
		b		.
;	]
		IBIT_DISABLE r5
		stmdb	sp!,{r0-r12}
		bl		GpTimerKill
		b		.
		
		
		END