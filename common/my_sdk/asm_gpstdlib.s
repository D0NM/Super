		GET asm_def.a

;OS define =======================================================================================
GOS_RES_MEM				EQU	0
GOS_RES_NET				EQU	1
GOS_RES_FIO				EQU	2
GOS_RES_GDI				EQU 3
		
		AREA   ASM_STDLIB,CODE,READONLY
		
		EXPORT	GpMClkGet
		EXPORT	GpHClkGet
		EXPORT	GpPClkGet
		EXPORT	GpLcdInfoGet
		EXPORT	GpGraphicModeSet
		EXPORT	GpLcdSurfaceGet
		EXPORT	GpLcdEnable
		EXPORT	GpLcdDisable
		EXPORT	GpLcdStatusGet
		EXPORT	GpLcdLock
		EXPORT	GpLcdUnlock
		EXPORT	GpFlipModeSet
		EXPORT	ACHI_P_DIVIDE
		EXPORT	GpNetTpsSet
		EXPORT	_gp_timer0_int
		
		EXPORT	swi_port_config_reset
		EXPORT	swi_set_sndbuffer
		EXPORT	GpAppPathSet
		EXPORT	GpAppPathGet
		EXPORT	GpArgSet
		EXPORT 	GpAppExecute
		EXPORT	GpAppExit
		EXPORT	GpUserInfoGet
		EXPORT	swi_install_irq
		EXPORT	swi_uninstall_irq
		EXPORT	swi_pal_addr_get
		EXPORT	swi_io_oper_set
		EXPORT	swi_smc_in
		EXPORT	swi_usb_sof_get
		EXPORT	swi_usb_line_reset
		EXPORT	GpClockSpeedChange
		EXPORT	swi_timer_operate
		EXPORT	swi_iis_operate
		EXPORT 	swi_mmu_change
		EXPORT	swi_pal_oper
		EXPORT	swi_chan_pal_fade
	;&<[ internal technic	
		EXPORT	_gp_smc_id_get
		EXPORT	_gp_e2prom_read
		EXPORT	_gp_e2prom_write	
		EXPORT	_gp_dev_id_get
	;]>&
		EXPORT	GpControlVolume
		IMPORT	_gp_os_res_lock
		IMPORT	_gp_os_res_unlock
		
		EXPORT	ARMDisableInterrupt
		EXPORT	ARMEnableInterrupt
		EXPORT	GpTickCountGet
		IMPORT	_timepassed
		
GpTickCountGet
		stmdb	sp!,{lr}
		ldr		r0,=_timepassed
		ldr		r0,[r0]
		ldr		r0,[r0]
		ldmia	sp!,{pc}
				
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

ARMDisableInterrupt
		stmdb	sp!,{r0}
		mrs		r0,cpsr
		orr		r0,r0,#PSR_INTMASK
		msr		cpsr_cxsf,r0
		ldmia	sp!,{r0}
		mov		pc,lr
ARMEnableInterrupt
		stmdb	sp!,{r0}
		mrs		r0,cpsr
		bic		r0,r0,#PSR_IBIT
		msr		cpsr_cxsf,r0
		ldmia	sp!,{r0}
		mov		pc,lr
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

;************************************************************************
;unsigned int GpMClkGet(void)
;************************************************************************
GpMClkGet
		stmdb	sp!,{lr}
		mov		r0,#4
		swi		_SCMD_GET_BASE
		ldr		r0,[r0]
		ldmia	sp!,{pc}
;************************************************************************		
;unsigned int GpHClkGet(void)
;************************************************************************
GpHClkGet
		stmdb	sp!,{lr}
		mov		r0,#4
		swi		_SCMD_GET_BASE
		ldr		r0,[r0,#4]
		ldmia	sp!,{pc}
;************************************************************************		
;unsigned int GpPClkGet(void)
;************************************************************************		
GpPClkGet
		stmdb	sp!,{lr}
		mov		r0,#4
		swi		_SCMD_GET_BASE
		ldr		r0,[r0,#8]
		ldmia	sp!,{pc}
;************************************************************************
;void GpLcdInfoGet(GPLCDINFO * p_info)		
;************************************************************************
GpLcdInfoGet
		stmdb	sp!,{r2,lr}
		mov		r2,#7
		swi		_SCMD_LCD_STATUS 
		ldr		r2,[sp],#4
		ldr		lr,[sp],#4
		mov		pc,lr
;************************************************************************
;int GpGraphicModeSet(int gd_bpp, int * gp_pal)		
;************************************************************************
	IMPORT	__gpver_for_hw
	IMPORT	__gpver_for_fw
GpGraphicModeSet
		stmdb	sp!,{r12,lr}
		stmdb	sp!,{r0-r2}
		mov		r0,#GOS_RES_GDI
		bl		_gp_os_res_lock
		ldr		r0,[sp],#4
		ldr		r1,[sp],#4
		mov		r2,#0
	;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	;by achi 2002.1.8
	;register backup
		stmdb	sp!,{r0}			;sp-->r0,r2,r12,lr
	;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
		swi		_SCMD_LCD_STATUS
		stmdb	sp!,{r0,r1}			;sp-->r0,r1,org r0,r2,r12,lr
		mov		r0,#GOS_RES_GDI
		bl		_gp_os_res_unlock
	;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	;by achi 2002.1.8
	;bug fixing code for 16bit graphic mode set
	;	call swi_get_base. argument is 8
	;	if fw version is less than 1.5, r0 & r1 should be NULL
	;add
		ldmia	sp!,{r0-r2}			;load r0, r1, original r0 --> so r2 is original r0. i.e. gd_bpp
		stmdb	sp!,{r0,r1}			;sp-->r0,r1,r2,r12,lr
		cmp		r2,#0x10
		bne		%F0					;if not 16bpp then, exit
		
		mov		r0,#8
		mov		r1,#0
		swi		_SCMD_GET_BASE
		add		r0,r0,r1
		cmp		r0,#0x10
		bge		%F0					;if version base get swi exists, then fw is normal !!!
		ldr		r0,=0x14a00010		;else you can access hardware address (LCDCON5)
		ldr		r1,[r0]
		bic		r1,r1,#0x3
		orr		r1,r1,#0x1			;bswap disable, hwswap enable
		str		r1,[r0]
0
	;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		ldmia	sp!,{r0-r2,r12,pc}
		
;************************************************************************		
;int GpLcdSurfaceGet(GPDRAWSURFACE * ptgpds, int idx)
;************************************************************************		
GpLcdSurfaceGet
		stmdb	sp!,{r2,lr}
		mov		r2,#1
		swi		_SCMD_LCD_STATUS
		ldmia	sp!,{r2,pc}
GpLcdEnable
;************************************************************************		
;void GpLcdEnable(void)
;************************************************************************		
		stmdb	sp!,{r2,lr}
		mov		r2,#3
		swi		_SCMD_LCD_STATUS
		ldr		r2,[sp],#4
		ldr		lr,[sp],#4
		mov		pc,lr
GpLcdDisable
;************************************************************************		
;void GpLcdDisable(void)
;************************************************************************		
		stmdb	sp!,{r2,lr}
		mov		r2,#4
		swi		_SCMD_LCD_STATUS
		ldr		r2,[sp],#4
		ldr		lr,[sp],#4
		mov		pc,lr
GpLcdStatusGet
;************************************************************************		
;int GpLcdStatusGet(void)
;************************************************************************		
		stmdb	sp!,{r2,lr}
		mov		r2,#5
		swi		_SCMD_LCD_STATUS
		ldr		r2,[sp],#4
		ldr		lr,[sp],#4
		mov		pc,lr
GpLcdLock
;************************************************************************		
;unsigned char * GpLcdLock(void);
;************************************************************************		
		stmdb	sp!,{r2,lr}
		mov		r0,#GOS_RES_GDI
		bl		_gp_os_res_lock
		mov		r2,#6
		swi		_SCMD_LCD_STATUS
		ldr		r2,[sp],#4
		ldr		lr,[sp],#4
		mov		pc,lr
GpLcdUnlock
;************************************************************************		
;void GpLcdUnlock(void);
;************************************************************************		
		stmdb	sp!,{r0,r12,lr}
		mov		r0,#GOS_RES_GDI
		bl		_gp_os_res_unlock
		ldmia	sp!,{r0,r12,pc}	
GpFlipModeSet
;************************************************************************		
;void GpFlipModeSet(int mode);
;************************************************************************		
		stmdb	sp!,{r1,r2,lr}
		mov		r2,#2
		swi		_SCMD_LCD_STATUS
		ldmia	sp!,{r1,r2,pc}
;************************************************************************		
;void GpNetTPSSet(int tps)
;************************************************************************		
		IMPORT	gpnet_MSPT
		IMPORT	gpnet_cur_ms
		IMPORT	gpnet_cticks
_gp_timer0_int
		stmdb	sp!,{r0-r4,lr}
	;gpnet_cur_ms++;
		ldr		r0,=gpnet_cur_ms
		ldr		r3,[r0]
		add		r3,r3,#1
	;if ( gpnet_cur_ms >= gpnet_MSPT )
		ldr		r1,=gpnet_MSPT
		ldr		r1,[r1]
		cmp		r1,r3,lsr #1
		bgt		%F0
	;gpnet_cticks++;
	;gpnet_cur_ms = 0;
		ldr		r1,=gpnet_cticks
		ldr		r2,[r1]
		add		r2,r2,#1
		str		r2,[r1]
		mov		r3,#0
0
		str		r3,[r0]		
		ldmia	sp!,{r0-r4,lr}
		mov		pc,lr
GpNetTpsSet
		stmdb	sp!,{r0,r1,lr}
		
		cmp		r0,#1
		ldmltia	sp!,{r0,r1,pc}
		
		IBIT_DISABLE	r1
		
		mov		r1,r0
		mov		r0,#1
		mov		r0,r0,lsl #10
		sub		r0,r0,#24
		bl		ACHI_P_DIVIDE
		ldr		r1,=gpnet_MSPT
		str		r0,[r1]
		ldr		r1,=gpnet_cur_ms
		mov		r0,#0
		str		r0,[r1]
		ldr		r1,=gpnet_cticks
		str		r0,[r1]
		
		IBIT_ENABLE	r1
		
		ldmia	sp!,{r0,r1,pc}
;************************************************************************		
swi_port_config_reset
;************************************************************************		
		stmdb	sp!,{lr}
		swi		_SCMD_PORT_CONFIG
		ldmia	sp!,{pc}		
;************************************************************************		
swi_set_sndbuffer
;************************************************************************		
		;r0 = unsigned short * _sndmixedbuf[2]
		;r1 = size
		stmdb	sp!,{lr}
		swi		_SCMD_SET_SNDBUFFER
		ldmia	sp!,{pc}
;************************************************************************		
;int GpAppPathSet(const char * p_name, int n_len);
;************************************************************************		
GpAppPathSet
		stmdb	sp!,{r2-r4,lr}
		mov		r4,#0				;access to app var for app path set
		swi		_SCMD_APPVAR_ACCESS
		ldmia	sp!,{r2-r4,pc}
;************************************************************************		
;char * GpAppPathGet(int * n_len);
;************************************************************************		
GpAppPathGet
		stmdb	sp!,{r1-r4,lr}
		mov		r4,#1				;access to app var for app path get
		swi		_SCMD_APPVAR_ACCESS
		ldmia	sp!,{r1-r4,pc}
;************************************************************************		
;*int GpArgSet(int len, char * p_arg);
;************************************************************************		
GpArgSet
		stmdb	sp!,{r2-r4,lr}
		mov		r4,#2				;access to app var for arg set
		swi		_SCMD_APPVAR_ACCESS
		ldmia	sp!,{r2-r4,pc}
;************************************************************************		
;*int GpUserInfoGet(char * p_id, char * p_pwd)
;************************************************************************		
	IMPORT	__decode_user_info
GpUserInfoGet
		stmdb	sp!,{r2-r6,lr}
		
		mov		r5,r0
		mov		r6,r1
		stmdb	sp!,{r5,r6}
		
		mov		r4,#3
		swi		_SCMD_APPVAR_ACCESS
		
		ldmia	sp!,{r5,r6}
		stmdb	sp!,{r0,r12}
		mov		r0,r5
		mov		r1,r6
		bl		__decode_user_info		;void __decode_user_info(char * p_id, char * p_pwd);
		ldmia	sp!,{r0,r12}
		
		ldmia	sp!,{r2-r6,pc}
;************************************************************************		
;int GpAppExecute(unsigned char * p_code_ptr, const char * s_path);
;************************************************************************		
GpAppExecute
		stmdb	sp!,{r1-r12,lr}
		swi		_SCMD_EXE_PROGRAM
		ldmia	sp!,{r1-r12,pc}
GpAppExit
		stmdb	sp!,{r1-r12,lr}
		swi		_SCMD_SW_RESET
		ldmia	sp!,{r1-r12,pc}
;************************************************************************		
;int _VirtualKeymap(void);
;************************************************************************		
		EXPORT	_VirtualKeyMap
		IMPORT	_reg_io_key_a
		IMPORT	_reg_io_key_b
_VirtualKeyMap
		stmdb	sp!,{r1-r5,lr}
		mov		r2,r0
		ldr		r0,=_reg_io_key_a
		ldr		r0,[r0]
		ldr		r1,=_reg_io_key_b
		ldr		r1,[r1]
		mov		r3,#0
		mvn		r3,r3
		mov		r4,r3
0
		cmp		r2,#0
		ble		%F1
		ldr		r5,[r0]
		and		r3,r3,r5
		ldr		r5,[r1]
		and		r4,r4,r5
		sub		r2,r2,#1
		b		%B0
1		
		mov		r0,#1
		mov		r1,r3
		mov		r2,r4
		swi		_SCMD_KEY_SCAN		;r0=raw_keydata (if pressed, bit cleared)
		mvn		r0,r0
		ldmia	sp!,{r1-r5,pc}
;*****************************************************************************
;void swi_install_irq(int /*int_idx*/, unsigned int /*func*/); 
swi_install_irq
		stmdb	sp!,{lr}
		swi		_SCMD_INSTALL_IRQ
		ldmia	sp!,{pc}
;*****************************************************************************
;void swi_uninstall_irq(int /*int_idx*/);
swi_uninstall_irq
		stmdb	sp!,{lr}
		swi		_SCMD_UNINSTALL_IRQ
		ldmia	sp!,{pc}
;*****************************************************************************
;void swi_pal_addr_get(&ptr, &reg)
swi_pal_addr_get
		stmdb	sp!,{r2-r3,lr}
		mov		r2,r1
		mov		r1,r0
		mov		r0,#2
		swi		_SCMD_GET_BASE
		ldmia	sp!,{r2-r3,pc}
;*****************************************************************************
;void swi_io_oper_set(unsigned int** ptr)
swi_io_oper_set
		stmdb	sp!,{r0,r1,lr}
		mov		r1,r0
		mov		r0,#3
		swi		_SCMD_GET_BASE
		ldmia	sp!,{r0,r1,pc}
;*****************************************************************************
;int swi_smc_in(void)
swi_smc_in
		stmdb	sp!,{lr}
		swi		_SCMD_CHK_SMC
		ldmia	sp!,{pc}			
;*****************************************************************************
;int swi_usb_sof_get
swi_usb_sof_get
		stmdb	sp!,{lr}
		mov		r0,#0			;get sof command
		swi		_SCMD_USBD_FUNC
		ldmia	sp!,{pc}
;*****************************************************************************
;void swi_usb_sof_get
swi_usb_line_reset		
		stmdb	sp!,{lr}
		mov		r0,#1			;reset command
		swi		_SCMD_USBD_FUNC
		ldmia	sp!,{pc}
;*****************************************************************************
GpClockSpeedChange
	;int GpClockSpeedChange(int master_speed, int div_factor, int clk_mode);
		stmdb	sp!,{r0-r2,lr}
		mov		r0,sp
		swi		_SCMD_CLOCK_OPER
		add		sp,sp,#12
		ldmia	sp!,{pc}
;*****************************************************************************
swi_timer_operate
	;int swi_timer_operate(struct tag_tmr_reg * p_tmr)
		stmdb	sp!,{lr}
		add		r0,sp,#4
		swi		_SCMD_TMR_OPER
		ldmia	sp!,{pc}
;*****************************************************************************
swi_iis_operate
	;int swi_iis_operate(struct tag_iis_info * p_iis)
		stmdb	sp!,{lr}
		add		r0,sp,#4
		swi		_SCMD_IIS_OPER
		ldmia	sp!,{pc}
;******************************************************************************
swi_mmu_change
	;void swi_mmu_change(r0, r1, r2)
		stmdb	sp!,{lr}
		swi		_SCMD_MMU_SET
		ldmia	sp!,{pc}
;******************************************************************************
_gp_smc_id_get
		stmdb	sp!,{lr}
		swi		(0x100:OR:_SCMD_SMC_ID_GET)
		ldmia	sp!,{pc}
_gp_e2prom_read
		stmdb	sp!,{r1-r3,lr}
		mov		r3,#0
		swi		(0x100:OR:_SCMD_E2PROM_ACCESS)
		ldmia	sp!,{r1-r3,pc}
_gp_e2prom_write
		stmdb	sp!,{r1-r3,lr}
		mov		r3,#1
		swi		(0x100:OR:_SCMD_E2PROM_ACCESS)
		ldmia	sp!,{r1-r3,pc}
_gp_dev_id_get
		stmdb	sp!,{lr}
		swi		(0x100:OR:_SCMD_DEV_ID_GET)
		ldmia	sp!,{pc}
;*******************************************************************************
GpControlVolume
		stmdb	sp!,{lr}
		swi		_SCMD_VOL_CONTROL
		ldmia	sp!,{lr}
		mov		pc,lr
;*******************************************************************************
swi_pal_oper
		stmdb	sp!,{lr}
		cmp		r0,#1
		bne		%F2
1	;swi_pal_oper_realize(1)
		mov		r0,#1
		swi		_SCMD_PAL_OPER
		b		%F0
2		cmp		r0,#2
		bne		%F3
	;swi_pal_oper_realize(2)
		mov		r0,#2
		swi		_SCMD_PAL_OPER
		b		%F0
3		cmp		r0,#3
		bne		%F4
	;swi_pal_oper_fade_in(3, h_pal)
		mov		r0,#3
		swi		_SCMD_PAL_OPER
		b		%F0
4		cmp		r0,#4
		bne		%F6
	;swi_pal_oper_fade_out
		mov		r0,#4
		swi		_SCMD_PAL_OPER
		b		%F0			
6		cmp		r0,#6
		bne		%F0
	;swi_pal_oper_nomalize
		mov		r0,#6
		swi		_SCMD_PAL_OPER
		;b		%F0
0		ldmia	sp!,{pc}
;*******************************************************************************
swi_chan_pal_fade
	;swi_chan_pal_fade(int step, int chan, h_pal)
		stmdb	sp!,{r1-r3,lr}
		mov		r3,r2
		mov		r2,r1
		mov		r1,r0
		mov		r0,#5
		swi		_SCMD_PAL_OPER
		ldmia	sp!,{r1-r3,pc}
		
		END