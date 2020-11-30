		GET asm_def.a
	AREA ASM_COMM,CODE,READONLY
	
	EXPORT	GpCommCreate
	EXPORT	GpCommDelete
	
GpCommCreate
	;int GpCommCreate(GPN_DESC * p_desc, GPN_COMM * p_comm)
	stmdb	sp!,{lr}
	swi		(0x100:OR:_SCMD_COMM_RESET)
	ldmia	sp!,{pc}
GpCommDelete
	;void GpCommDelete(GPN_DESC * p_desc);
	stmdb	sp!,{lr}
	swi		(0x100:OR:_SCMD_COMM_DELETE)
	ldmia	sp!,{pc}
	
	END