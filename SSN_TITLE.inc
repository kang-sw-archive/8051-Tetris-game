; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; DEFINE_MODULE	SSN_TITLE ; Change this as you want.
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LOCMEM_PVT
LBIT_PVT
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;
;	SSN_TITLE
;
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;	DESCRIPTION
; Write down the description of this module.
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
CONST	MAXCURSOR,	2;	.Indicates maximum cursor location.

LOCMEM	STRCACHE,	0FH;	.Used to cache assembled string
LOCMEM	CURSOR, 	1;	.Indicates current cursor location.
; --------------------------------------------------------------------
;
; @PROC	SESSION_INIT
; @note	It is recommended to define a general purpose session initializer
;	 to easy access to different session.
;	This procedure should contain the code segment calling TRANSITION
;	 procedure, which requires following CALLBACK arguments:
;
;	I X	DPH:DPL	INPUT 
;	I X	R1:R0	UPDATE
;	I X	R3:R2	DYNAMIC DRAW CALL
;	I X	R5:R4	SESSION INITIALIZER
;	I X	R7:R6	STATIC DRAW CALL
;
;
; --------------------------------------------------------------------
rTITLE_SESSION:
	; Clear memory
	MOV	DPTR,	#2000H
	LDCDW	R1,R0,	2000H
	MOV	B,	#0
	CALL	rMEMSET
	; Assign input procedure
	MOV	DPTR,	#NULLF
	; Assign update procedure
	LDCDW	R1,R0, 	NULLF
	; Assign draw call procedure
	LDCDW	R3,R2,	NULLF
	; Assign session initializer
	LDCDW	R5,R4,	cTITLE_INIT
	; Assign static draw call
	LDCDW	R7,R6,	cTITLE_STATIC_DRAW
	; Clear if the timer should not be reset.
	SETB	C
	; Calls the transition function.
	; It is recommended to place this line last.
	CALL	rTRANSITION	
	RET
; --------------------------------------------------------------------
rMAIN_MENU: 
	; Assign input procedure
	MOV	DPTR,	#cMAIN_INPUT
	; Assign update procedure
	LDCDW	R1,R0, 	NULLF
	; Assign draw call procedure
	LDCDW	R3,R2,	RMAIN_DRAW
	; Assign session initializer
	LDCDW	R5,R4,	NULLF
	; Assign static draw call
	LDCDW	R7,R6,	NULLF
	; Clear if the timer should not be reset.
	SETB	C
	; Calls the transition function.
	; It is recommended to place this line last.
	CALL	rTRANSITION	
	RET
; --------------------------------------------------------------------
; @CALBAK 	MAIN_DRAW
; --------------------------------------------------------------------
; Draw callback of MAIN menu session.
rMAIN_DRAW:	; Get back buffer
	CALL	RBUFFER_BACK
	; Pushes current back buffer element to stack,
	;  will be extracted by @R0 which is containing #SP
	PUSH	ACC
	; >>> >>> >>>
	; Clears background with black
	MOV	A,	#0
	CALL	rFILLBUFFER
	
	; Get cursor from memory
	MOV	DPTR,	#CURSOR
	MOVX	A,	@DPTR
	
	CJNE	A,	#0,	MAIN_DRAW_1
	MOV	DPTR,	#MAIN_DRAW_TETRIS 
	MOV	B,	#010B
STRING	MAIN_DRAW_TETRIS, 'TETRIS'
	SJMP	MAIN_DRAW_SWDONE
MAIN_DRAW_1:
	CJNE	A,	#1,	MAIN_DRAW_2
	MOV	DPTR,	#MAIN_DRAW_JIRUNG
	MOV	B,	#101B
STRING	MAIN_DRAW_JIRUNG, 'JIRUNG'
	SJMP	MAIN_DRAW_SWDONE
MAIN_DRAW_2:
	CJNE	A,	#2,	MAIN_DRAW_DEFAULT
	MOV	B,	#001B
	SJMP	MAIN_DRAW_DEFAULT
;MAIN_DRAW_3:
;	CJNE	A,	#3,	MAIN_DRAW_4
;	MOV	B,	#100B
;	; FILL
;	SJMP	MAIN_DRAW_DEFAULT
;MAIN_DRAW_4:
;	CJNE	A,	#4,	MAIN_DRAW_5
;	MOV	B,	#010B
;	; FILL
;	SJMP	MAIN_DRAW_DEFAULT
;MAIN_DRAW_5:
;	CJNE	A,	#5,	MAIN_DRAW_DEFAULT
;	MOV	DPTR,	#MAIN_DRAW_RANKING
;	MOV	B,	#011B
;STRING	MAIN_DRAW_RANKING, 'RANKING'
;	SJMP	MAIN_DRAW_SWDONE
MAIN_DRAW_DEFAULT:
	MOV	DPTR,	#STRCACHE
	PUSH	B
	MOV	B,	A
	CALL	rDTOA
	MOV	DPTR,	#STRCACHE
	POP	B
MAIN_DRAW_SWDONE: 
	; Draw string on location
	MOV	R0,	SP
	MOVP2	@R0
	MOV	R1,	#00000010B
	SETB	F0
	PUSH	B
	CALL	rDRAWSTRING
	
	; Draw boundary box
	PEAKP2	DPH
	MOV	DPL,	#0
	POP	B
	MOV	R1,	#32
	MOV	R2,	#16
	CLR	C
	CALL	rDRAWBOX
	
	MOV	DPL,	#00100001B
	MOV	B,	#111B
	MOV	R1,	#30
	MOV	R2,	#14
	CLR	C
	CALL	rDRAWBOX
	
	; Print the title's highst score
	; Get its highst score
	MOV	DPTR,	#CURSOR
	MOVX	A,	@DPTR
	RL	A
	RL	A
	
	; RANKING_HIGHST + CURSOR*4 = Its highst score location.
	LDCDWP2	R1,	RANKING_HIGHST
	ADD	A,	R1
	ADD	A,	#2; Since it's little endian....
	MOV	R1,	A
	
	; String to assemble.
	MOV	DPTR,	#STRCACHE
	MOV	R2,	#3
MAIN_DRAW_SCORE:
	; Change BCD value to string.
	MOVX	A,	@R1
	MOV	B,	A
	DEC	R1
	CALL	rDTOA
	DJNZ	R2,	MAIN_DRAW_SCORE
	
	; print string.
	MOV	DPTR,	#STRCACHE
	MOV	B,	#110B
	CLR	F0
	MOV	R0,	SP
	MOVP2	@R0
	MOV	R1,	#01000010B
	CALL	rDRAWSTRING
	
;STRING	MAIN_DRAW_STR_SELECT, 'SELECT'
;	MOV	DPTR,	#MAIN_DRAW_STR_SELECT
;	MOV	B,	#111B
;	CLR	F0
;	MOV	R0,	SP
;	MOVP2	@R0
;	MOV	R1,	#01000010B
;	CALL	rDRAWSTRING

	; <<< <<< <<<
	POP	ACC
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
; @CALBAK	MAIN_INPUT
; --------------------------------------------------------------------
cMAIN_INPUT:
BIT_U 	SET	0
BIT_D 	SET	1
BIT_R 	SET	2
BIT_L 	SET	3
BIT_A 	SET	4
BIT_B 	SET	5
BIT_	SET	6
BIT_HOME	SET	7
	USING	GAMEBANK/8  
	; If button 0 pressed...
	MOV	A,	R0 ; KEY DOWN
	JB	ACC.5,	MAIN_INPUT_NEXT
	JB	ACC.4,	MAIN_INPUT_PREV
	JB	ACC.2,	MAIN_INPUT_SELECT
	RET
	
	; Cursor location will be ranking, game info.
	;MOV	DPTR,	#CURSOR
	;MOVX	A,	@DPTR
	;MOV	DPTR,	#RANKING_GAMEINFO
	;MOVX	@DPTR,	A
	
MAIN_INPUT_SELECT:
	; Initiates selected session
	MOV	DPTR,	#CURSOR
	MOVX	A,	@DPTR
	
	CJNE	A,	#0,	MAIN_INPUT_S1
	CALL	rSESSION_TETRIS
	RET
MAIN_INPUT_S1:
	CJNE	A,	#1,	MAIN_INPUT_S2
	CALL	rSESSION_JIRUNG
	RET	
MAIN_INPUT_S2:
	RET
MAIN_INPUT_NEXT:
	; Rotate cursor 
	MOV	DPTR,	#CURSOR
	MOVX	A,	@DPTR
	; NEXT = CURSOR + 1
	INC	A
	; NEXT = NEXT == MAX ? 0 : NEXT
	CJNE	A,	#MAXCURSOR, MAIN_INPUT_STORE
	MOV	A,	#0
MAIN_INPUT_STORE:
	MOVX	@DPTR,	A
	RET
MAIN_INPUT_PREV:
	; Rotate cursor 
	MOV	DPTR,	#CURSOR
	MOVX	A,	@DPTR
	; NEXT = CURSOR + 1
	DEC	A
	; NEXT = NEXT == MAX ? 0 : NEXT
	CJNE	A,	#0FFH,	MAIN_INPUT_STORE
	MOV	A,	#MAXCURSOR - 1
	SJMP	MAIN_INPUT_STORE
; --------------------------------------------------------------------
; @CALBAK	UPDATECALLBACK(VOID)
; --------------------------------------------------------------------
; @CALBAK	DYNAMICDRAWCALLBACK 
; --------------------------------------------------------------------
cTITLE_STATIC_DRAW:
; Draw title image.
	CALL	RBUFFER_BACK
	MOV	P2,	A
	MOV	R1,	#0
	MOV	DPTR,	#IMG_TITLE
	CLR	CY
	CALL	RDRAWIMAGE	
	RET 
; --------------------------------------------------------------------
; @CALBAK	INITIALIZERCALLBACK(VOID)
; --------------------------------------------------------------------
cTITLE_INIT:  
	LDCDW	BR1,	BR0,	45
	LDCDW	R1,	R0,	rMAIN_MENU	
	CALL	rSET_TIMER  
	RET
; --------------------------------------------------------------------
; @CALBAK	STATICDRAWCALLBAK 
; --------------------------------------------------------------------   
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
