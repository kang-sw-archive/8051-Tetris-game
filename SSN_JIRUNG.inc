; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; DEFINE_MODULE	SSN_TMPLT??? ; This isn't required on session definition
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LOCMEM_PVT
LBIT_PVT
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;
;	SSN_JIRUNG
;
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;	DESCRIPTION
; The jirungi game.
; Uses 16*16 size of screen to describe jirungi game session.
; 1 byte of memory allocated for represent current PIVOT location,
;and the 16*16 size board itself represents current game state.
; 1 byte of memory allocated to store current direction, u d r l
; The worm dies when the pivot located on biting its body.
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ######### SUBROUTINES & MACROS ##################################### 
; --------------------------------------------------------------------
; @NOTE	LOADCDW	HI, LO, CONST
; @NOTE	MOVP2	VAL
; --------------------------------------------------------------------
;
; 	LOCAL MEMORY ALLOCATIONS
;
; TYPE     |IDENTIFIER |SIZE/DESCRIPTION
; ---------|-----------|----------------------------------------------  
MEMALIGN	LOCAL_MEMORY, 100H
LOCMEM	JR_BOARD,100H; .Main game board of jirungi game.
LOCMEM	JR_PVT, 1	; .The pivot location of jirungi. Should be aligned as Y X order.
LOCMEM	JR_DIR, 1	; .Direction of jirungi's pivot. Should be aligned as Y X order.
LOCMEM	JR_LVL, 1	; .Number of layers of jirungi. 
LOCMEM	JR_FOOD, 1	; .Current food location.
LOCMEM	JR_TICKS, 1	; .Tick count to next frame.
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
rSESSION_JIRUNG:
	; Clear memory
	MOV	DPTR,	#2000H
	LDCDW	R1,R0,	2000H
	MOV	B,	#0
	CALL	rMEMSET
	; Assign input procedure
	MOV	DPTR,	#cJRINPUT
	; Assign update procedure
	LDCDW	R1,R0, 	cJRUPDATE
	; Assign draw call procedure
	LDCDW	R3,R2,	cJRDRAW
	; Assign session initializer
	LDCDW	R5,R4,	cJRINIT
	; Assign static draw call
	LDCDW	R7,R6,	NULLF
	; Clear if the timer should not be reset.
	SETB	C
	; Calls the transition function.
	; It is recommended to place this line last.
	CALL	rTRANSITION	
	RET
; --------------------------------------------------------------------
; @CALBAK	JRUPDATE
; --------------------------------------------------------------------
cJRUPDATE:	
	USING	GAMEBANK/8
; <algorithm overview>
; [] moves head as its direction
; <> if there's worm's body on new pivot location...
;	[] game overs, call ranking. 
; <> if there's <<<NO>>> food on the new pivot location ...
;	[] iterate all the board elements to decrease them by 1 for each, only when it is larger than 0.
;	==> this means, there's no decreasing if there's any food.
; ><> else ... // if there's any food...
;	[] increase current level count by 1
;	[] places a food on random point.
; [] fills the point which is current pivot location as current level.
; ~done.
	; All the action begins only when the ticks equals zero.
	MOV	DPTR,	#JR_TICKS
	MOVX	A,	@DPTR
	JZ	JRUPDATE_BEGIN
	; [] Decreases tick count, and escapes.
	DEC	A
	MOVX	@DPTR,	A
	RET
JRUPDATE_BEGIN:
	; Resets ticks left
	MOV	A,	#3
	MOVX	@DPTR,	A
	
	; [] MOVE HEAD LOC
	; get direction
	MOV	DPTR,	#JR_DIR
	MOVX	A,	@DPTR
	PUSH	ACC
	
	; get cur pivot.
	MOV	DPTR,	#JR_PVT
	MOVX	A,	@DPTR
	
	; add them each.
	POP	AR2
	CALL	rADDBLKSEP
	MOVX	@DPTR,	A
	
NEWPIVOT	SET	R3 
	MOV	NEWPIVOT,	A
	
	; <> check if there's worm body.
	; The dpl will directly be the coordinate.
	MOV	DPH,	#HIGH(JR_BOARD)
	MOV	DPL,	NEWPIVOT
	
	; If it's not zero, that means there's worm body.
	MOVX	A,	@DPTR
	JZ	JRUPDATE_NOOVER
	; [] game over sequence.
	MOV	DPTR,	#RANKING_RETURN
	MOV	A,	#LOW(rSESSION_JIRUNG)
	MOVX	@DPTR,	A

	INC	DPL
	MOV	A,	#HIGH(rSESSION_JIRUNG)
	MOVX	@DPTR,	A
	
	MOV	DPTR,	#RANKING_GAMEINFO
	MOV	A,	#1
	MOVX	@DPTR,	A
	
	CALL	rTORANKING
	RET
JRUPDATE_NOOVER:
	; <> if there's no food on new pivot loc...
	; gets food location.
	MOV	DPTR,	#JR_FOOD
	MOVX	A,	@DPTR
	MOV	B,	NEWPIVOT
	; compare with ..
	CJNE	A,	B,	JRUPDATE_NOFOOD
	
; ==> there's food. 
	; [] increases current level by 1. Increases score.
	MOV	DPTR,	#JR_LVL
	MOVX	A,	@DPTR
	INC	A
	MOVX	@DPTR,	A
	; [] places food on random point.
	MOV	DPH,	#HIGH(JR_BOARD)
	; random pivot
	MOV	A,	RANDNUM 
	MOV	DPL,	A
	; determine direction to proceed.
	MOV	C,	ACC.0
	MOV	F0,	C
	
JRUPDATE_FINDPUTFOOD:
; <> if specified board element location indicates 0, put food there.
	; :: else, iterate until find valid place.
	MOVX	A,	@DPTR
	JZ	JRUPDATE_FOUND	
	; move up or down 
	JB	F0,	JRUPDATE_INC
	DEC	DPL
	SJMP	JRUPDATE_FINDPUTFOOD
JRUPDATE_INC:
	INC	DPL
	SJMP	JRUPDATE_FINDPUTFOOD
JRUPDATE_FOUND:
	
	; Now, dpl points an empty place.
	; locates new food.
	MOV	A,	DPL
	MOV	DPTR,	#(JR_FOOD)
	MOVX	@DPTR,	A

	; [] increases score.
	MOV	DPTR,	#JR_LVL
	MOVX	A,	@DPTR
	; adds score repeatedly, 50 per 10 level.
JRUPDATE_SCORELP:
	MOV	R2,	A
	; adds score.
	MOV	B,	#50
	CALL	rADDSCORE
	; check for iteration condition
	MOV	A,	R2
	CLR	C
	SUBB	A,	#10
	JNC	JRUPDATE_SCORELP
	
	; done.	
	SJMP 	JRUPDATE_FILLCURRENT
JRUPDATE_NOFOOD:
; ==> there's no food.
	; [] iterates every element of board, and if it's not zero, decreases each cells.
	MOV	DPTR,	#JR_BOARD
JRUPDATE_LIFESPAN:
	MOVX	A,	@DPTR
	; <> if lifespan left in this cell is larger than 0 ...
	JZ	JRUPDATE_SKIP
	; [] decreases it by 1.
	DEC	A
	MOVX	@DPTR,	A
JRUPDATE_SKIP:
	DJNZ	DPL,	JRUPDATE_LIFESPAN
; ~~JRUPDATE_LIFESPAN
JRUPDATE_FILLCURRENT:
	; [] fills current pivot location by current level.
	; current pivot location.
	MOV	DPTR,	#JR_PVT
	MOVX	A,	@DPTR
	PUSH	ACC
		
	; current level.
	MOV	DPTR,	#JR_LVL
	MOVX	A,	@DPTR
	
	; Sets it by ...
	POP	DPL
	MOV	DPH,	#HIGH(JR_BOARD)
	
	MOVX	@DPTR,	A
	
	RET
; --------------------------------------------------------------------
; @CALBAK	JRINIT
; --------------------------------------------------------------------
; Initializes jirungi game.
cJRINIT:
	; Resets current level as 1.
	MOV	DPTR,	#JR_LVL
	MOV	A,	#1
	MOVX	@DPTR,	A
	
	; Resets pivot
	MOV	DPTR,	#JR_PVT
	MOV	A,	#01110111B; 7, 7.
	MOVX	@DPTR,	A
	
	; Locates the first food. It'll be fixed location, 9'9.
	MOV	DPTR,	#JR_FOOD
	MOV	A,	#10011001B
	MOVX	@DPTR,	A
	
	; Adjust heading direction. Default is 0. game begins when 
	;the player starts to move.
	MOV	DPTR,	#JR_DIR
	MOV	A,	#00010000B
	MOVX	@DPTR,	A
	
	; Set initial tick timer.
	MOV	DPTR,	#JR_TICKS
	MOV	A,	#0
	MOVX	@DPTR,	A

	RET
; --------------------------------------------------------------------
; @CALBAK	JRDRAW
; --------------------------------------------------------------------
; Draws jirungi game board every tick.
cJRDRAW:
	; clears screen
	MOV	A,	#0
	CALL	rFILLBUFFER
	
	; draws border
	; border will be like this ..
	; +----+      +----+
	; |    | GAME |    |
	; +----+      +----+
	; The first border.
	MOV	DPL,	#0
	MOV	B,	#001B
	MOV	R1,	#8
	MOV	R2,	#16
	CLR	C
	CALL	rDRAWBOX
	
	; The second border.
	MOV	DPL,	#24 ; Y = 0, X = 24
	MOV	B,	#001B
	MOV	R1,	#8
	MOV	R2,	#16
	CLR	C
	CALL	rDRAWBOX
	 
	; iterate all the board element, and draws color if it's not 0.
	; Radix point = Y0 X8
	LDCDWP2	R1,	JR_BOARD
	; draws activated food.
JRDRAW_BOARD_0:
	; <> if the board element is not zero, fill white color.
	MOVX	A,	@R1
	JZ	JRDRAW_BOARD_1
	; [] draw dot.
	; transpose location from board to disp.
	MOV	A,	R1
	CALL	JRDRAW_BLKTODISP
	; call drawdot
	MOV	DPL,	A
	MOV	B,	#111B
	MOV	C,	F0
	CALL	rDRAWDOT
	
JRDRAW_BOARD_1:	; NODRAW
	
	DJNZ	R1,	JRDRAW_BOARD_0
; ~~JRDRAW_BOARD_0

	; [] Draws food location
	; adjusts location
	LDCDWP2	R1,	JR_FOOD
	MOVX	A,	@R1
	CALL	JRDRAW_BLKTODISP
	
	; draws dot
	MOV	DPL,	A
	MOV	B,	#110B; will be yellow.
	MOV	C,	F0
	CALL	rDRAWDOT
	
	RET
	
; Transform 16 by 16 location to (F0)1 + 3 by 8 location
; I 	A	INPUT LOCATION
; OUT	F0	SCREEN SELECTION
; OUT	A	OUTPUT LOCATION (3, 5)
; X 	B
JRDRAW_BLKTODISP:
	PUSH	ACC
	; Upper 4bit to F0 + 3bit ofst
	ANL	A, 	#0F0H
	RLC	A
	MOV	F0,	C
	; Lower 4bit and add 7 to shift right.
	ANL	A,	#11100000B
	POP	B
	; Masks B, its lower value.
	ANL	B,	#0FH
	ADD	A,	B
	ADD	A,	#8
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
cJRINPUT:	
	USING 	GAMEBANK/8
	
	MOV	BR0,	R0
	MOV	DPTR,	#JR_DIR
	MOVX	A,	@DPTR
	
JRINPUT_U:	JNB	BR0.4,	JRINPUT_D
	MOV	A,	#11110000B
	SJMP	JRINPUT_X
	
JRINPUT_D:	JNB	BR0.5,	JRINPUT_L
	MOV	A,	#00010000B
	SJMP	JRINPUT_X
	
JRINPUT_L:	JNB	BR0.6,	JRINPUT_R
	MOV	A,	#00001111B
	SJMP	JRINPUT_X
	
JRINPUT_R:	JNB	BR0.3,	JRINPUT_X
	MOV	A,	#00000001B
	SJMP	JRINPUT_X
	
JRINPUT_X:	MOVX	@DPTR,	A
	RET
; --------------------------------------------------------------------
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
