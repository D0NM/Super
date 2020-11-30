     
     AREA      ASM_FONTLIB,CODE,READONLY
        EXPORT  GpFontOut
;***********************************************************************************************************************************
;GpFontOut
;    info.[0] = target address				info.[1] = target position x (real axis)
;    info.[2] = target position y (real axis)	info.[3] = target drawing width (real axis)
;    info.[4] = target drawing height (real axis)	info.[5] = source address
;    info.[6] = source position x (real axis)	info.[7] = source position y (real axis)
;    info.[10] = fore ground color				info.[8] = srcpitch
;	info.[9]= tgpitch
;************************************************************************************************************************************
GpFontOut
		stmdb		sp!,{r0-r12,lr}
	;target positionn adjustment
		ldr			r1,[r0]
		ldr			r2,[r0,#12]			;r2 = tg_w
		ldr			r3,[r0,#16]			;r3 = tg_h
		cmp			r2,#0
		ldmleia		sp!,{r0-r12,pc}
		cmp			r3,#0
		ldmleia		sp!,{r0-r12,pc}
		ldr			r12,[r0,#36]		;r12 = tgpitch
		ldr			r4,[r0,#4]			;r4 = tg_x
		ldr			r5,[r0,#8]			;r5 = tg_y
		mul			r6,r5,r12			;r6 = tg_y * tgpitch
		add			r1,r1,r6
		add			r1,r1,r4			;r1 = tg_addr
		sub			r1,r1,r12			;r1 = tg_addr
		;==> (r1 tg_addr), (r2 tg_w), (r3 tg_h), (r12 tgpitch)
	;source start position adjustment
		ldr			r5,[r0,#28]			;r5 = src_y
		ldr			r11,[r0,#32]		;r11 = srcpitch
		add			r11,r11,#0x7
		mov			r11,r11,lsr #3		;r11 = (srcpitch+7) / 8
		mul			r6,r5,r11			;r6 = src_y * ((srcpitch+7) / 8)
		ldr			r4,[r0,#20]			;r4 = src_addr
		add			r4,r4,r6			;r4 = src_addr
		sub			r4,r4,r11			;r4 = src_addr
		ldr			r5,[r0,#24]			;r5 = src_x
		mov			r6,r5,lsr #3		;r6 = src_x / 8
		cmp			r6,#0
		beq			_FNT_GETCOLOR
		add			r4,r4,r6
		mov			r6,r6,lsl #3
		sub			r5,r5,r6
		;==> (r4 src_addr), (r5 src_x), (r11 srcpitch)
_FNT_GETCOLOR	
		ldr			r10,[r0,#40]		;r10 = color
_FNT_NEWLINE
		cmp			r3,#0
		ldmeqia		sp!,{r0-r12,pc}
		add			r1,r1,r12			;new tg_addr
		add			r4,r4,r11			;new src_addr
		mov			r6,r1			;working tg_addr
		mov			r7,r4			;working src_addr
		mov			r14,r2			;working tg_w
		
		mov			r8,#0			;check 8bit align
		ldrb			r9,[r7],#1
_FNT_SRCSTART		
		cmp			r8,r5
		beq			_FNT_COPYSTART
		mov			r9,r9,lsr #1
		add			r8,r8,#1
		tst			r8,#7
		ldreqb		r9,[r7],#1
		b			_FNT_SRCSTART
_FNT_COPYSTART
		tst			r9,#0x1		
		beq			_FNT_DOTSKIP
		strb		r10,[r6]
		;stmdb		sp!,{r6}
		;and			r0,r6,#0x3
		;rsb			r0,r0,#0x3
		;bic			r6,r6,#0x3
		;add			r0,r0,r6
		;ldmia		sp!,{r6}
		;strb		r10,[r0]
_FNT_DOTSKIP		
		sub			r14,r14,#1
		cmp			r14,#0
		beq			_FNT_DECHEIGHT
		add			r6,r6,#1
		add			r8,r8,#1
		mov			r9,r9,lsr #1
		tst			r8,#7
		ldreqb		r9,[r7],#1
		b			_FNT_COPYSTART
_FNT_DECHEIGHT
		sub			r3,r3,#1
		b			_FNT_NEWLINE	

		END
