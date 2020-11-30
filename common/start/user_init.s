	[ GP_DBG
START_SP_AT_DBG	EQU	0x0c700000
	]
	
		IMPORT	_fw_init_for_dbg
	[ GP_DBG
		IMPORT 	GpAppPathSet
	]
		EXPORT	asm_user_entry
		EXPORT	asm_user_entry_path
		
        AREA    asm_user_init,CODE,READONLY
asm_user_entry
	[ 	GP_DBG
		ldr		sp,=START_SP_AT_DBG
		stmdb	sp!,{lr}
		bl		_fw_init_for_dbg
		ldmia	sp!,{lr}
	]
		mov		pc,lr
asm_user_entry_path
	[ 	GP_DBG
		stmdb	sp!,{r0-r3,lr}
		
		sub		sp,sp,#0xff
		sub		sp,sp,#1
		
		mov		r0,sp
		ldr		r1,=0x5c3a7067
		ldr		r2,=0x656d6167
		mov		r3,#0
		stmia	r0,{r1-r3}
		mov		r1,#9
		bl		GpAppPathSet
		
		add		sp,sp,#0xff
		add		sp,sp,#1
		ldmia	sp!,{r0-r3,lr}
	]
		mov		pc,lr		
		END
                      