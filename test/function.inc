; FUNCTIONALITY TEST
IF	0; SET THIS TO 0 IF RUN
	
	
	MOV	DPTR,	#2000H
	LDCDW	R1,R0,	2000H
	MOV	B,	#0
	CALL	RMEMSET


	JMP	TO_MONITOR






ENDIF