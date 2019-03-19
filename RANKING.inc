; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; DEFINE_MODULE	SSN_TMPLT??? ; This isn't required on session definition
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LOCMEM_PVT
LBIT_PVT
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;
;	SSN_TMPLT
;
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;	DESCRIPTION
; This is ranking session which will be used for recording final score 
;of any game. Transition to this session will be occured when the end 
;of any game session or pressing home button. It transites to title 
;session when the home button pressed, with or without recording score 
;by recording flag.
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ######### SUBROUTINES & MACROS #####################################
;
; § LIST OF PREFIXES:: PROC, SPEC[0-9a-zA-Z_]*, CALBAK
;
;	Prefix PROC indicates a general callable procedure which
;	 indicates an interface function.
;
;	Prefix SPEC[..] indicates a special purpose function which
;	 should be called only in specific sequence.
;	 All of the rules about prefix words and procedure concepts
;	 should be concretely defined.
;
;	Prefix CALBAK indicates a procedure is callback which fits
; 	 in specific calling protocol.
;
; --------------------------------------------------------------------
; @NOTE	LOADCDW	HI, LO, CONST
; @NOTE	MOVP2	VAL
; --------------------------------------------------------------------
;
; 	LOCAL MEMORY ALLOCATIONS
;
; TYPE     |IDENTIFIER |SIZE/DESCRIPTION
; ---------|-----------|----------------------------------------------  

; --------------------------------------------------------------------
;
; @PROC	SESSION_INIT
; @note	It is recommended to define a general purpose session initializer
;	 to easy access to different session.
;	This procedure should contain the code segment calling TRANSITION
;	 procedure, which requires following CALLBACK arguments:
;
; 	I X	DPH:DPL	INPUT 
; 	I X	R1:R0	UPDATE
; 	I X	R3:R2	DYNAMIC DRAW CALL
; 	I X	R5:R4	SESSION INITIALIZER
; 	I X	R7:R6	STATIC DRAW CALL
;
; --------------------------------------------------------------------
rTORANKING:
; Ranking initialize ... if score is lower than current max, does nothing.
	; <> Check given game score is larger than current highst score.
; ++++ compares RANKING_SCORE and SCORES + RANKING_GAMEINFO*4
	; Gets current highst score's address
	LDCDWP2	R1,	RANKING_HIGHST
	MOV	DPTR,	#RANKING_GAMEINFO
	MOVX	A,	@DPTR
	; A << 2 = A * 4
	RL	A
	RL	A
	; Add this to R1, which indicates lower word of ext. address
	ADD	A,	R1
	MOV	R1,	A
	MOV	R4,	A
	; Get current score	
	MOV	DPTR,	#RANKING_SCORE
	
	; for 4 bytes integer...
	MOV	R2,	#4
	CLR	C
TORANKING_0:
	; Game info score - incoming score... C ? HIGHST : NOP
	MOVX	A,	@DPTR
	MOV	B,	A
	MOVX	A,	@R1
	SUBB	A,	B
	
	INC	R1
	INC	DPL
	
	DJNZ	R2,	TORANKING_0
; ~~TORANKING_0
	; If carry bit is activated, assign incoming score as new value.
	JC	TORANKING_DISP
	; f. 	calls return callback.
	CALL	cRETURNTOSESSION
	RET
TORANKING_DISP:	
	; t. 	[] sets given score as new highst score.
	MOV	A,	R4
	MOV	R1,	A
	MOV	DPTR,	#RANKING_SCORE
	
	MOV	R2,	#4
TORANKING_1:
	; Copies new value into ranking.
	MOVX	A,	@DPTR
	MOVX	@R1,	A
	
	INC	R1
	INC	DPL	
	
	DJNZ	R2,	TORANKING_1
; ~~TORANKING_1
	; Display "NEW HIGHST SCORE" screen for 3 secs. 
	; Clear memory
	;MOV	DPTR,	#2000H
	;LDCDW	R1,R0,	2000H
	;MOV	B,	#0
	;CALL	rMEMSET
	; Assign input procedure
	MOV	DPTR,	#NULLF
	; Assign update procedure
	LDCDW	R1,R0, 	NULLF
	; Assign draw call procedure
	LDCDW	R3,R2,	NULLF
	; Assign session initializer
	LDCDW	R5,R4,	cRANKINIT
	; Assign static draw call
	LDCDW	R7,R6,	cRANKDRAW
	; Clear if the timer should not be reset.
	SETB	C
	; Calls the transition function.
	; It is recommended to place this line last.
	CALL	rTRANSITION	
	RET
; --------------------------------------------------------------------
; @CALBAK	RANKINIT
; --------------------------------------------------------------------
; Initializes return timer.
cRANKINIT:	
	LDCDW	BR1,	BR0,	60
	LDCDW	R1,	R0,	cRETURNTOSESSION
	CALL	rSET_TIMER
	RET 
; --------------------------------------------------------------------
; @CALBAK	RANKDRAW
; --------------------------------------------------------------------
; Displays new highscore
STRING	RANKSTR,	'HISCORE!'
LOCMEM	STRCACHE,	10
cRANKDRAW:
	; Clears screen
	MOV	A,	#0
	CALL	rFILLBUFFER
	
	; Draws 'HISCORE' string.
	CLR	F0
	CALL	rBUFFER_BACK
	MOVP2	A
	
	MOV	R1,	#00100000B
	MOV	DPTR,	#RANKSTR
	MOV	B,	#111B
	
	CALL	rDRAWSTRING
	
	; Draws current score.
	; Make current score to string
	MOV	DPTR,	#STRCACHE
	LDCDWP2	R1,	RANKING_SCORE+3
	
	MOV	R3,	#4
RANKDRAW_DTOA:
	; Get number
	MOVX	A,	@R1
	MOV	B,	A
	DEC	R1
	; to string
	CALL	rDTOA	
	DJNZ	R3,	RANKDRAW_DTOA
; ~~RANKDRAW_DTOA
	
	; print string 
	SETB	F0
	CALL	rBUFFER_BACK
	MOVP2	A
	
	MOV	R1,	#00100000B
	MOV	DPTR,	#STRCACHE
	MOV	B,	#110B
	
	CALL	rDRAWSTRING
	
	; done.
	RET
; --------------------------------------------------------------------
; @CALBAK	RETURN_ORG
; --------------------------------------------------------------------
cRETURNTOSESSION:
	; CLEAR SCORE.
	MOV	R2,	#4
	MOV	DPTR,	#RANKING_SCORE
	MOV	A,	#0
RETURNTOSESSION_FINALIZE:
	MOVX	@DPTR,	A
	INC	DPL	
	DJNZ	R2,	RETURNTOSESSION_FINALIZE
; ~~ RETURNTOSESSION_FINALIZE
	
	LDCDWP2	R1,	RANKING_RETURN
	; Load returning session to DPTR
	MOVX	A,	@R1
	MOV	DPL,	A
	INC	R1
	MOVX	A,	@R1
	MOV	DPH,	A
	; DCALL
	DCALL	DPTR
	RET 
; --------------------------------------------------------------------
;	
; @PROC	InitScore
;
; X	DPTR, ACC
;
; --------------------------------------------------------------------
rINITSCORE:
	PUSH	BR0
	
	; >>>>>>
	MOV	DPTR,	#RANKING_SCORE
	MOV	BR0,	#4
	MOV	A,	#0
	
INITSCORE0:	MOVX	@DPTR,	A
	INC	DPL
	DJNZ	BR0,	INITSCORE0
	
	; <<<<<<
	POP	BR0
	RET
; --------------------------------------------------------------------
;
; @PROC	AddScore
;
; I	B	SCORE_BCD
;
; X	A, DPTR
;
; --------------------------------------------------------------------\
; Adds score to ranking_score cache.
rADDSCORE: 	PUSH	BR0
	
	; >>>>>>
	; Set memory location
	MOV	DPTR,	#RANKING_SCORE
	
	; 1st byte
	MOVX	A,	@DPTR
	ADD	A,	B
	DA	A
	MOVX	@DPTR,	A
	
	; 2nd-4th byte
	MOV	BR0,	#3
ADDSCORE_LP:
; Adds score by repeatation.
	; Next addres
	INC	DPTR
	MOVX	A,	@DPTR
	; Adds carry
	ADDC	A,	#0
	; BCD correction
	DA	A
	; Load to memory
	MOVX	@DPTR,	A
	DJNZ	BR0,	ADDSCORE_LP
	
	; <<<<<<
	POP	BR0
	RET
; --------------------------------------------------------------------
; @CALBAK 	INPUTCALLBACK
; @NOTE 	BUTTON.6 	L
;	BUTTON.5 	D
;	BUTTON.4 	U
;	BUTTON.3 	R
;	BUTTON.2 	ACTA
;	BUTTON.1 	ACTB
;	BUTTON.0 	HOME
;
; I	R0	KEYDOWN
; I 	R1	KEYUP
; I 	R2	KEYHOLD
; --------------------------------------------------------------------
; @CALBAK	UPDATECALLBACK(VOID)
; --------------------------------------------------------------------
; @CALBAK	DYNAMICDRAWCALLBACK
; I 	B	BACKBUFFERHI
; --------------------------------------------------------------------
; @CALBAK	INITIALIZERCALLBACK(VOID)
; --------------------------------------------------------------------
; @CALBAK	STATICDRAWCALLBAK
; I	B	BACKBUFFERHI
; --------------------------------------------------------------------   
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
