	GET snd_option.a
	
	AREA ASM_PCMMIXER,CODE,READONLY
	
	EXPORT _GpPcmMixingStereo16
	EXPORT _GpPcmMixingMono16
	EXPORT _GpPcmMixingStereo8
	EXPORT _GpPcmMixingMono8
	
;=====================================================================================================
;r0 = _sndmixedbuf
;r1 = _sndmixer (i_ptr, c_ptr, i_length, c_length, repeatflag)
;r2 = SNDMIXING_UNIT
;=====================================================================================================	
_GpPcmMixingStereo16
	stmdb	sp!,{r3-r12,lr}
	stmdb	sp!,{r2,lr}
	
	mov		r12,r0
	mov		r0,#0
	mov		r14,#0
	mov		r9,#0
	mov		r10,#0
;check_smix_src0
	ldr		r3,[r1,#4]			;check _sndmixer[0].c_ptr
	cmp		r3,#0
	beq		check_smix_src1
	ldr		r8,[r1,#12]
check_smix_src1
	ldr		r4,[r1,#24]			;check _sndmixer[1].c_ptr
	cmp		r4,#0
	beq		check_smix_src2
	ldr		r9,[r1,#32]
check_smix_src2
	ldr		r5,[r1,#44]			;check _sndmixer[2].c_ptr
	cmp		r5,#0
	beq		check_smix_src3
	ldr		r10,[r1,#52]
check_smix_src3
	ldr		r6,[r1,#64]			;check _sndmixer[3].c_ptr
	cmp		r6,#0
	beq		start_ssnd_mix
	ldr		r11,[r1,#72]
;=================================================================================
;r3,r4,r5,r6 ==> c_ptr, r9-> c_length of (src0 & src1), r10-> c_length of (src2 & src3)
;r14 ==> repeatflag bitwise var
;=================================================================================
start_ssnd_mix
	mov		r7,#0
	mov		r14,#0
;*get_ssnd_src0		
	cmp		r3,#0
	beq		get_ssnd_src1
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r7,[r3],#2
  |
  	ldrsh	r7,[r3],#2
  ]
	sub		r8,r8,#1
	cmp		r8,#0
	bne		get_ssnd_src1
	ldr		r2,[r1,#16]		;repeat flag
	cmp		r2,#0
	beq		remove_ssnd_src0
	ldr		r3,[r1]			;r3 = _sndmixer[0].c_ptr = _sndmixer[0].i_ptr
	ldr		r8,[r1,#8]		;r8 = _sndmixer[0].c_length = _sndmixer[0].i_length
	b		get_ssnd_src1
remove_ssnd_src0
	mov		r3,#0
	str		r3,[r1]			;_sndmixer[0].i_ptr = _sndmixer[0].c_ptr = NULL
;*get_ssnd_src1	
get_ssnd_src1
	cmp		r4,#0
	beq		get_ssnd_src2
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r2,[r4],#2
  |
  	ldrsh	r2,[r4],#2
  ]
	add		r7,r7,r2
	sub		r9,r9,#1
	cmp		r9,#0
	bne		get_ssnd_src2
	ldr		r2,[r1,#36]	
	cmp		r2,#0
	beq		remove_ssnd_src1
	ldr		r4,[r1,#20]
	ldr		r9,[r1,#28]
	b		get_ssnd_src2
remove_ssnd_src1
	mov		r4,#0
	str		r4,[r1,#20]
	
get_ssnd_src2
	cmp		r5,#0
	beq		get_ssnd_src3
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r2,[r5],#2
  |
  	ldrsh	r2,[r5],#2
  ]
	add		r7,r7,r2
	sub		r10,r10,#1
	cmp		r10,#0
	bne		get_ssnd_src3
	ldr		r2,[r1,#56]
	cmp		r2,#0
	beq		remove_ssnd_src2
	ldr		r5,[r1,#40]
	ldr		r10,[r1,#48]
	b		get_ssnd_src3
remove_ssnd_src2
	mov		r5,#0
	str		r5,[r1,#40]
	
get_ssnd_src3
	cmp		r6,#0
	beq		mix_ssnd_src
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r2,[r6],#2
  |
  	ldrsh	r2,[r6],#2
  ]
	add		r7,r7,r2
	sub		r11,r11,#1
	cmp		r11,#0
	bne		mix_ssnd_src
	ldr		r2,[r1,#76]
	cmp		r2,#0
	beq		remove_ssnd_src3
	ldr		r6,[r1,#60]
	ldr		r11,[r1,#68]
	b		mix_ssnd_src
remove_ssnd_src3
	mov		r6,#0
	str		r6,[r1,#60]
mix_ssnd_src
	cmp		r14,#1
	beq		mix_ssnd_src1
	cmp		r14,#2
	beq		mix_ssnd_src2
	cmp		r14,#3
	beq		mix_ssnd_src3
	cmp		r14,#4
	beq		mix_ssnd_src4
	b		exit_ssnd_mix
mix_ssnd_src1		;*src 1=============================
  [ PCM_MIXING_UNSIGNED
	mov		r14,r14,lsl #15	;r14 = 0x8000
	sub		r7,r7,r14
  ]
  	b		last_smix_snd
mix_ssnd_src2		;*src 2=============================
  [ PCM_MIXING_UNSIGNED
  	mov		r14,r14,lsl #15	;r14 = 0x10000
	sub		r7,r7,r14
  ]
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#14
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#13
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#11
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#10
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #1
  ENDIF
  	b		last_smix_snd
mix_ssnd_src3		;*src 3=============================
  [ PCM_MIXING_UNSIGNED
  	mov		r14,r14,lsl #15	;r14 = 0x18000
	sub		r7,r7,r14
  ]
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#10
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#9
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#8
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#7
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#6
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r2,#5
  ENDIF
	mul		r7,r2,r7
	mov		r7,r7,asr #4		;r7 = (r7*5)/16  	
	b		last_smix_snd
mix_ssnd_src4		;*src 4=============================
  [ PCM_MIXING_UNSIGNED
  	mov		r14,r14,lsl #15	;r14 = 0x20000
	sub		r7,r7,r14
  ]
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#8
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#7
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#6
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #2
  ENDIF
	;b		last_smix_snd
last_smix_snd
	strh	r7,[r12],#2
	add		r0,r0,#1
	ldr		r7,[sp]
	cmp		r0,r7
	beq		exit_ssnd_mix
	b		start_ssnd_mix
exit_ssnd_mix
	str		r3,[r1,#4]
	str		r4,[r1,#24]
	str		r5,[r1,#44]
	str		r6,[r1,#64]
	str		r8,[r1,#12] 
	str		r9,[r1,#32]
	str		r10,[r1,#52]
	str		r11,[r1,#72]
	
	ldmia	sp!,{r2,lr}
	ldmia	sp!,{r3-r12,pc}
	
;*=====================================================================================================
;*=====================================================================================================
_GpPcmMixingMono16
	stmdb	sp!,{r3-r12,lr}
	stmdb	sp!,{r2,lr}
	
	mov		r12,r0
	mov		r0,#0
	mov		r14,#0
	mov		r9,#0
	mov		r10,#0
;check_mmix_src0
	ldr		r3,[r1,#4]			;check _sndmixer[0].c_ptr
	cmp		r3,#0
	beq		check_mmix_src1
	ldr		r8,[r1,#12]
check_mmix_src1
	ldr		r4,[r1,#24]			;check _sndmixer[1].c_ptr
	cmp		r4,#0
	beq		check_mmix_src2
	ldr		r9,[r1,#32]
check_mmix_src2
	ldr		r5,[r1,#44]			;check _sndmixer[2].c_ptr
	cmp		r5,#0
	beq		check_mmix_src3
	ldr		r10,[r1,#52]
check_mmix_src3
	ldr		r6,[r1,#64]			;check _sndmixer[3].c_ptr
	cmp		r6,#0
	beq		start_msnd_mix
	ldr		r11,[r1,#72]
;*=================================================================================
;*r3,r4,r5,r6 ==> c_ptr, r9-> c_length of (src0 & src1), r10-> c_length of (src2 & src3)
;*r14 ==> repeatflag bitwise var
;*=================================================================================
start_msnd_mix
	mov		r7,#0
	mov		r14,#0
;*get_msnd_src0		
	cmp		r3,#0
	beq		get_msnd_src1
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r7,[r3],#2
  |
  	ldrsh	r7,[r3],#2
  ]
	sub		r8,r8,#1
	cmp		r8,#0
	bne		get_msnd_src1
	ldr		r2,[r1,#16]		;repeat flag
	cmp		r2,#0
	beq		remove_msnd_src0
	ldr		r3,[r1]			;r3 = _sndmixer[0].c_ptr = _sndmixer[0].i_ptr
	ldr		r8,[r1,#8]		;r8 = _sndmixer[0].c_length = _sndmixer[0].i_length
	b		get_msnd_src1
remove_msnd_src0
	mov		r3,#0
	str		r3,[r1]			;_sndmixer[0].i_ptr = _sndmixer[0].c_ptr = NULL
get_msnd_src1
	cmp		r4,#0
	beq		get_msnd_src2
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r2,[r4],#2
  |
  	ldrsh	r2,[r4],#2
  ]
	add		r7,r7,r2
	sub		r9,r9,#1
	cmp		r9,#0
	bne		get_msnd_src2
	ldr		r2,[r1,#36]	
	cmp		r2,#0
	beq		remove_msnd_src1
	ldr		r4,[r1,#20]
	ldr		r9,[r1,#28]
	b		get_msnd_src2
remove_msnd_src1
	mov		r4,#0
	str		r4,[r1,#20]
	
get_msnd_src2
	cmp		r5,#0
	beq		get_msnd_src3
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r2,[r5],#2
  |
  	ldrsh	r2,[r5],#2
  ]
	add		r7,r7,r2
	sub		r10,r10,#1
	cmp		r10,#0
	bne		get_msnd_src3
	ldr		r2,[r1,#56]
	cmp		r2,#0
	beq		remove_msnd_src2
	ldr		r5,[r1,#40]
	ldr		r10,[r1,#48]
	b		get_msnd_src3
remove_msnd_src2
	mov		r5,#0
	str		r5,[r1,#40]
	
get_msnd_src3
	cmp		r6,#0
	beq		mix_msnd_src
	
	add		r14,r14,#1
  [ PCM_MIXING_UNSIGNED
	ldrh	r2,[r6],#2
  |
  	ldrsh	r2,[r6],#2
  ]
	add		r7,r7,r2
	sub		r11,r11,#1
	cmp		r11,#0
	bne		mix_msnd_src
	ldr		r2,[r1,#76]
	cmp		r2,#0
	beq		remove_msnd_src3
	ldr		r6,[r1,#60]
	ldr		r11,[r1,#68]
	b		mix_msnd_src
remove_msnd_src3
	mov		r6,#0
	str		r6,[r1,#60]
	
mix_msnd_src
	cmp		r14,#1
	beq		mix_msnd_src1
	cmp		r14,#2
	beq		mix_msnd_src2
	cmp		r14,#3
	beq		mix_msnd_src3
	cmp		r14,#4
	beq		mix_msnd_src4
	b		exit_msnd_mix
	
mix_msnd_src1
  [ PCM_MIXING_UNSIGNED
	mov		r14,r14,lsl #15	;r14 = 0x8000
	sub		r7,r7,r14
  ]
	b		last_mmix_snd
mix_msnd_src2
  [ PCM_MIXING_UNSIGNED
  	mov		r14,r14,lsl #15	;r14 = 0x10000
	sub		r7,r7,r14
  ]
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#14
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#13
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#11
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#10
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #1
  ENDIF
	b		last_mmix_snd
mix_msnd_src3
  [ PCM_MIXING_UNSIGNED
  	mov		r14,r14,lsl #15	;r14 = 0x18000
	sub		r7,r7,r14
  ]
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#10
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#9
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#8
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#7
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#6
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r2,#5
  ENDIF
	mul		r7,r2,r7
	mov		r7,r7,asr #4		;r7 = (r7*5)/16  	
	b		last_mmix_snd
mix_msnd_src4
  [ PCM_MIXING_UNSIGNED
  	mov		r14,r14,lsl #15	;r14 = 0x20000
	sub		r7,r7,r14
  ]
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#8
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#7
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#6
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #2
  ENDIF
	;b		last_mmix_snd
last_mmix_snd
	strh	r7,[r12],#2
	strh	r7,[r12],#2		;*one more store, because of mono
	add		r0,r0,#2
	ldr		r7,[sp]
	cmp		r0,r7
	bge		exit_msnd_mix
	b		start_msnd_mix
exit_msnd_mix
	str		r3,[r1,#4]
	str		r4,[r1,#24]
	str		r5,[r1,#44]
	str		r6,[r1,#64]
	str		r8,[r1,#12] 
	str		r9,[r1,#32]
	str		r10,[r1,#52]
	str		r11,[r1,#72]
	
	ldmia	sp!,{r2,lr}
	ldmia	sp!,{r3-r12,pc}

;*=====================================================================================================
;*r0 = _sndmixedbuf
;*r1 = _sndmixer (i_ptr, c_ptr, i_length, c_length, repeatflag)
;*r2 = SNDMIXING_UNIT
;*=====================================================================================================
_GpPcmMixingStereo8
	stmdb	sp!,{r3-r12,lr}
	stmdb	sp!,{r2,lr}
	
	mov		r12,r0
	mov		r0,#0
	mov		r14,#0
	mov		r9,#0
	mov		r10,#0
;chk8_smix_src0
	ldr		r3,[r1,#4]			;check _sndmixer[0].c_ptr
	cmp		r3,#0
	beq		chk8_smix_src1
	ldr		r8,[r1,#12]
chk8_smix_src1
	ldr		r4,[r1,#24]			;check _sndmixer[1].c_ptr
	cmp		r4,#0
	beq		chk8_smix_src2
	ldr		r9,[r1,#32]
chk8_smix_src2
	ldr		r5,[r1,#44]			;check _sndmixer[2].c_ptr
	cmp		r5,#0
	beq		chk8_smix_src3
	ldr		r10,[r1,#52]
chk8_smix_src3
	ldr		r6,[r1,#64]			;check _sndmixer[3].c_ptr
	cmp		r6,#0
	beq		start8_ssnd_mix
	ldr		r11,[r1,#72]
;=================================================================================
;r3,r4,r5,r6 ==> c_ptr, r9-> c_length of (src0 & src1), r10-> c_length of (src2 & src3)
;r14 ==> repeatflag bitwise var
;=================================================================================
start8_ssnd_mix
	mov		r7,#0
	mov		r14,#0
;get8_ssnd_src0		
	cmp		r3,#0
	beq		get8_ssnd_src1
	
	add		r14,r14,#1
	ldrb	r7,[r3],#1
  	mov		r7,r7,lsl #8
	sub		r8,r8,#1
	cmp		r8,#0
	bne		get8_ssnd_src1
	ldr		r2,[r1,#16]		;repeat flag
	cmp		r2,#0
	beq		remove8_ssnd_src0
	ldr		r3,[r1]			;r3 = _sndmixer[0].c_ptr = _sndmixer[0].i_ptr
	ldr		r8,[r1,#8]		;r8 = _sndmixer[0].c_length = _sndmixer[0].i_length
	b		get8_ssnd_src1
remove8_ssnd_src0
	mov		r3,#0
	str		r3,[r1]			;_sndmixer[0].i_ptr = _sndmixer[0].c_ptr = NULL
	
get8_ssnd_src1
	cmp		r4,#0
	beq		get8_ssnd_src2 
	
	add		r14,r14,#1
	ldrb	r2,[r4],#1
	add		r7,r7,r2,lsl #8
	sub		r9,r9,#1
	cmp		r9,#0
	bne		get8_ssnd_src2
	ldr		r2,[r1,#36]	
	cmp		r2,#0
	beq		remove8_ssnd_src1
	ldr		r4,[r1,#20]
	ldr		r9,[r1,#28]
	b		get8_ssnd_src2
remove8_ssnd_src1
	mov		r4,#0
	str		r4,[r1,#20]
	
get8_ssnd_src2
	cmp		r5,#0
	beq		get8_ssnd_src3
	
	add		r14,r14,#1
	ldrb	r2,[r5],#1
	add		r7,r7,r2,lsl #8
	sub		r10,r10,#1
	cmp		r10,#0
	bne		get8_ssnd_src3
	ldr		r2,[r1,#56]
	cmp		r2,#0
	beq		remove8_ssnd_src2
	ldr		r5,[r1,#40]
	ldr		r10,[r1,#48]
	b		get8_ssnd_src3
remove8_ssnd_src2
	mov		r5,#0
	str		r5,[r1,#40]
	
get8_ssnd_src3
	cmp		r6,#0
	beq		mix8_ssnd_src
	
	add		r14,r14,#1
	ldrb	r2,[r6],#1
	add		r7,r7,r2,lsl #8
	sub		r11,r11,#1
	cmp		r11,#0
	bne		mix8_ssnd_src
	ldr		r2,[r1,#76]
	cmp		r2,#0
	beq		remove8_ssnd_src3
	ldr		r6,[r1,#60]
	ldr		r11,[r1,#68]
	b		mix8_ssnd_src
remove8_ssnd_src3
	mov		r6,#0
	str		r6,[r1,#60]
	
mix8_ssnd_src
	cmp		r14,#1
	beq		mix8_ssnd_src1
	cmp		r14,#2
	beq		mix8_ssnd_src2
	cmp		r14,#3
	beq		mix8_ssnd_src3
	cmp		r14,#4
	beq		mix8_ssnd_src4
	b		exit8_ssnd_mix
	
mix8_ssnd_src1
	mov		r14,r14,lsl #15	;r14 = 0x8000
	sub		r7,r7,r14
	b		last8_smix_snd
mix8_ssnd_src2
	mov		r14,r14,lsl #15	;r14 = 0x10000
	sub		r7,r7,r14
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#14
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#13
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#11
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#10
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #1
  ENDIF  
	b		last8_smix_snd
mix8_ssnd_src3
	mov		r14,r14,lsl #15	;r14 = 0x18000
	sub		r7,r7,r14
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#10
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#9
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#8
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#7
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#6
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r2,#5
  ENDIF
	mul		r7,r2,r7
	mov		r7,r7,asr #4		;r7 = (r7*5)/16  	
	b		last8_smix_snd
mix8_ssnd_src4
  	mov		r14,r14,lsl #15	;r14 = 0x20000
	sub		r7,r7,r14
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#8
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#7
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#6
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #2
  ENDIF
	;b		last8_smix_snd
last8_smix_snd
	strh	r7,[r12],#2
	add		r0,r0,#1
	ldr		r7,[sp]
	cmp		r0,r7
	beq		exit8_ssnd_mix
	b		start8_ssnd_mix
exit8_ssnd_mix
	str		r3,[r1,#4]
	str		r4,[r1,#24]
	str		r5,[r1,#44]
	str		r6,[r1,#64]
	str		r8,[r1,#12] 
	str		r9,[r1,#32]
	str		r10,[r1,#52]
	str		r11,[r1,#72]
	
	ldmia	sp!,{r2,lr}
	ldmia	sp!,{r3-r12,pc}
	
;*=====================================================================================================
;*=====================================================================================================

_GpPcmMixingMono8
	stmdb	sp!,{r3-r12,lr}
	stmdb	sp!,{r2,lr}
	
	mov		r12,r0
	mov		r0,#0
	mov		r14,#0
	mov		r9,#0
	mov		r10,#0
;chk8_mmix_src0
	ldr		r3,[r1,#4]			;chk _sndmixer[0].c_ptr
	cmp		r3,#0
	beq		chk8_mmix_src1
	ldr		r8,[r1,#12]
chk8_mmix_src1
	ldr		r4,[r1,#24]			;chk _sndmixer[1].c_ptr
	cmp		r4,#0
	beq		chk8_mmix_src2
	ldr		r9,[r1,#32]
chk8_mmix_src2
	ldr		r5,[r1,#44]			;chk _sndmixer[2].c_ptr
	cmp		r5,#0
	beq		chk8_mmix_src3
	ldr		r10,[r1,#52]
chk8_mmix_src3
	ldr		r6,[r1,#64]			;chk _sndmixer[3].c_ptr
	cmp		r6,#0
	beq		start8_msnd_mix
	ldr		r11,[r1,#72]
;=================================================================================
;r3,r4,r5,r6 ==> c_ptr, r9-> c_length of (src0 & src1), r10-> c_length of (src2 & src3)
;r14 ==> repeatflag bitwise var
;=================================================================================
start8_msnd_mix
	mov		r7,#0
	mov		r14,#0
;get8_msnd_src0		
	cmp		r3,#0
	beq		get8_msnd_src1
	
	add		r14,r14,#1
	ldrb	r7,[r3],#1
	mov		r7,r7,lsl #8
	sub		r8,r8,#1
	cmp		r8,#0
	bne		get8_msnd_src1
	ldr		r2,[r1,#16]		;repeat flag
	cmp		r2,#0
	beq		remove8_msnd_src0
	ldr		r3,[r1]			;r3 = _sndmixer[0].c_ptr = _sndmixer[0].i_ptr
	ldr		r8,[r1,#8]		;r8 = _sndmixer[0].c_length = _sndmixer[0].i_length
	b		get8_msnd_src1
remove8_msnd_src0
	mov		r3,#0
	str		r3,[r1]			;_sndmixer[0].i_ptr = _sndmixer[0].c_ptr = NULL
	
get8_msnd_src1
	cmp		r4,#0
	beq		get8_msnd_src2
	
	add		r14,r14,#1
	ldrb	r2,[r4],#1
	add		r7,r7,r2,lsl #8
	sub		r9,r9,#1
	cmp		r9,#0
	bne		get8_msnd_src2
	ldr		r2,[r1,#36]	
	cmp		r2,#0
	beq		remove8_msnd_src1
	ldr		r4,[r1,#20]
	ldr		r9,[r1,#28]
	b		get8_msnd_src2
remove8_msnd_src1
	mov		r4,#0
	str		r4,[r1,#20]
	
get8_msnd_src2
	cmp		r5,#0
	beq		get8_msnd_src3
	
	add		r14,r14,#1
	ldrb	r2,[r5],#1
	add		r7,r7,r2,lsl #8
	sub		r10,r10,#1
	cmp		r10,#0
	bne		get8_msnd_src3
	ldr		r2,[r1,#56]
	cmp		r2,#0
	beq		remove8_msnd_src2
	ldr		r5,[r1,#40]
	ldr		r10,[r1,#48]
	b		get8_msnd_src3
remove8_msnd_src2
	mov		r5,#0
	str		r5,[r1,#40]
	
get8_msnd_src3
	cmp		r6,#0
	beq		mix8_msnd_src
	
	add		r14,r14,#1
	ldrb	r2,[r6],#1
	add		r7,r7,r2,lsl #8
	sub		r11,r11,#1
	cmp		r11,#0
	bne		mix8_msnd_src
	ldr		r2,[r1,#76]
	cmp		r2,#0
	beq		remove8_msnd_src3
	ldr		r6,[r1,#60]
	ldr		r11,[r1,#68]
	b		mix8_msnd_src
remove8_msnd_src3
	mov		r6,#0
	str		r6,[r1,#60]
	
mix8_msnd_src
	cmp		r14,#1
	beq		mix8_msnd_src1
	cmp		r14,#2
	beq		mix8_msnd_src2
	cmp		r14,#3
	beq		mix8_msnd_src3
	cmp		r14,#4
	beq		mix8_msnd_src4
	b		exit8_msnd_mix
	
mix8_msnd_src1
	mov		r14,r14,lsl #15	;r14 = 0x8000
	sub		r7,r7,r14
	b		last8_mmix_snd
mix8_msnd_src2
	mov		r14,r14,lsl #15	;r14 = 0x10000
	sub		r7,r7,r14
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#14
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#13
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#11
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#10
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #1
  ENDIF  
	b		last8_mmix_snd
mix8_msnd_src3
	mov		r14,r14,lsl #15	;r14 = 0x18000
	sub		r7,r7,r14
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#10
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#9
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#8
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#7
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#6
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r2,#5
  ENDIF
	mul		r7,r2,r7
	mov		r7,r7,asr #4		;r7 = (r7*5)/16  	
	b		last8_mmix_snd
mix8_msnd_src4
  	mov		r14,r14,lsl #15	;r14 = 0x20000
	sub		r7,r7,r14
  IF PCM_MIXING_METHOD = PCM_MIXING_100
  	mov		r2,#8
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_90
  	mov		r2,#7
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_80
	mov		r2,#6
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_70
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_60
	mov		r2,#5
	mul		r7,r2,r7
	mov		r7,r7,asr #4
  ENDIF
  IF PCM_MIXING_METHOD = PCM_MIXING_50 
	mov		r7,r7,asr #2
  ENDIF
	;b		last8_mmix_snd
last8_mmix_snd
	strh	r7,[r12],#2
	strh	r7,[r12],#2
	add		r0,r0,#2
	ldr		r7,[sp]
	cmp		r0,r7
	bge		exit8_msnd_mix
	b		start8_msnd_mix
exit8_msnd_mix
	str		r3,[r1,#4]
	str		r4,[r1,#24]
	str		r5,[r1,#44]
	str		r6,[r1,#64]
	str		r8,[r1,#12] 
	str		r9,[r1,#32]
	str		r10,[r1,#52]
	str		r11,[r1,#72]
	
	ldmia	sp!,{r2,lr}
	ldmia	sp!,{r3-r12,pc}
		
	END