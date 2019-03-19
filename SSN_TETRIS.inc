; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; DEFINE_MODULE	SSN_TMPLT??? ; This isn't required on session definition
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
LOCMEM_PVT
LBIT_PVT
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;
;	SSN_TETRIS
;
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;	DESCRIPTION
; This is a session which defines the game 'TETRIS'.
; The left side of the screen will show gameplay screen, and the right 
;side of the screen will show current score.  
;
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
MEMALIGN 	LOCAL_MEMORY, 100H
LOCMEM	TT_BOARD,	100H; .Board data buffer. To allow fast accessing
		; and processing, a single block cell occupies
		; 1 byte memory space.

LOCMEM	TT_LEVEL,	1
LOCMEM	TT_TONEXTLV, 1; .Block destroy count left to next level.
LOCMEM	TT_DELAY, 	1; .This is the frame length which indicates
		; the period of dropping block. it begins with
		; 30frame, and it decreases per level.
		; NO_USE, THE DELAY SHOULD BE 30 - LEVEL * 2
LOCMEM	TT_CNTLEFT,	1; .Frame counts that are left until the block shifts down.
LOCMEM	TT_DELLEFT,	1; .Delay time left, will be reset on block movement
LOCMEM	TT_CB_TYPE, 1; .Current block type.
LOCMEM	TT_NB_TYPE,	1; .Next block type.

MEMALIGN	LOCAL_MEMORY, 4
LOCMEM	TT_CB_PVT,  4; .Location of four current active block cells. 
		; The first byte of PVT data indicates the absolute
		;location of block pivot, and the rest of three 
		;bytes indicate relative locations for each cells,
		;and those are represented as signed value.
		; On rotation, the pivot cell never be changed, 
		;but the rest of three locations will be changed 
		;to apply rotation.
LOCMEM	STRCACHE,	8; .STRING CACHE
MEMALIGN	LOCAL_MEMORY, 20H
LOCMEM	TT_TMP,	20H; Temporary memory for various usage.

; LOCATION INFO WILL BE ALIGNED AS  [YYYY XXXX], XXXX <= 1010
CONST	TT_WIDTH,	10
CONST	TT_HEIGHT,	16

CONST	TT_PV_BOARD, 00000110B
CONST	TT_PV_NEXT,  00100001B
CONST	TT_PV_LVL,	 11100001B
CONST	TT_PV_SCORE, 01110000B

LBIT	TT_SCOREDIRTY; .Score dirty flag for dynamic draw callback
LBIT	TT_NEXTDIRTY;  .Next block screen dirty flag
LBIT	TT_BOARDDIRTY; .Board dirty flag
LBIT	TT_LVLDIRTY;   .Level dirty flag 

LBIT	TT_FLGCLR;     .Flag clear signal flag. Should be reset on 
		;any dirty flag activation.
LBIT	TT_DROPBLOCK;  .Used to drop block immediately.

; --------------------------------------------------------------------
; To depict a tetris block shape, a byte of data will be used. 
; Upper 4bit will show upper line, Lower 4bit will show lower line.
; For example, the T block will be stored by this way.
; 	CONST TT_BLOCKSHAPE_T  00100111
;  	>>   * * ■ *
;	>>   * ■ ◆ ■  ◆ - Pivot location
; This data will be instanciated on block spawn, 1 will be set as
;filled block shape, 0 will indicate empty space.
CONST 	TT_BL_T, 	00100111B
CONST 	TT_BL_I,	00001111B
CONST 	TT_BL_LL, 	00010111B
CONST 	TT_BL_RL,	01000111B
CONST 	TT_BL_Z,	01100011B
CONST 	TT_BL_S,	00110110B
CONST 	TT_BL_SQ,	01100110B

TT_BLOCKSHAPE:DB	TT_BL_T, TT_BL_I, TT_BL_LL, TT_BL_RL, TT_BL_Z, TT_BL_S, TT_BL_SQ 
TT_BLOCK_COLOR:DB	101B, 001B, 011B, 100B, 010B, 101B, 110B
; --------------------------------------------------------------------
; #MACRO	BLKTODISP
; --------------------------------------------------------------------
; Transform 16 by 16 location to (F0)1 + 3 by 8 location
BLKTODISP	MACRO	_A
	PUSH	ACC
	; Upper 4bit to F0 + 3bit ofst
	ANL	A, 	#0F0H
	RLC	_A
	MOV	F0,	C
	; Lower 4bit and add 7 to shift right.
	ANL	A,	#11100000B
	POP	B
	; Masks B, its lower value.
	ANL	B,	#0FH
	ADD	A,	B
	ADD	A,	#6
	; now result in A
ENDM
; Adds lower 4bit and higher 4bit separately.
; Invalidates B
ADDBLKRELC	MACRO	_A,	OP 
	PUSH	ACC
	; Adds upper 4bit first.
	ANL	A,	#0F0H
	ADD	_A,	OP
	ANL	A,	#0F0H
	; Adds lower 4bit
	MOV	B,	A
	POP	ACC
	ADD	A,	OP
	ANL	A,	#00FH
	; Adds each value other.
	ADD	A,	B
ENDM
; --------------------------------------------------------------------
;
; @PROC	SESSION_INIT
; @note	 It is recommended to define a general purpose session initializer
;	to easy access to different session.
;	 This procedure should contain the code segment calling TRANSITION
;	procedure, which requires following CALLBACK arguments:
;
; 	I X	DPH:DPL	INPUT
; 	I X	R1:R0	UPDATE
; 	I X	R3:R2	DYNAMIC DRAW CALL
; 	I X	R5:R4	SESSION INITIALIZER
; 	I X	R7:R6	STATIC DRAW CALL
;
; --------------------------------------------------------------------
rSESSION_TETRIS:
	; Clears memory
	MOV	DPTR,	#2000H
	LDCDW	R1,R0,	2000H
	MOV	B,	#0
	CALL	rMEMSET
	; Assign input procedure
	MOV	DPTR,	#cTETINPUT
	; Assign update procedure
	LDCDW	R1,R0, 	cTETUPDATE
	; Assign draw call procedure
	LDCDW	R3,R2,	cTETDNDRAW
	; Assign session initializer
	LDCDW	R5,R4,	cTETINIT
	; Assign static draw call
	LDCDW	R7,R6,	cTETSTDRAW
	; Clear if the timer should not be reset.
	; Reset timer for tetris.
	SETB	C
	; Calls the transition function.
	; It is recommended to place this line last.
	CALL	rTRANSITION	
	
	RET
; --------------------------------------------------------------------
; 
; @SPECT	CHECKBOUND
;
; I	R2 	NEXT PIVOT, y x align
; I 	R3	CURRENT PIVOT
; IX[3]	DPTR	NEXT PIVOT LINKINGS
; OUT 	F0	VALID
; 
; X	ACC
;
; --------------------------------------------------------------------
; Check if given next block is valid
sCHECKBOUND:
	USING 	GAMEBANK/8
VALID	SET	F0
PVNXT	SET	R2  
	PUSH	AR4
	PUSH	B
	; >>>>>
	
	; init
	SETB	VALID
	LDCDWP2	R1, 	TT_BOARD
	
	;SJMP	CHECKBOUND_LINKS
	
	; Y Axis distance between current and next should be less than 4.
	MOV	A,	R3
	MOV	B,	R2
	CALL	CHECKBOUND_YVALID
	
	; pivot validation
	MOV	R1,	AR2
	CALL	ssCHECKDOT 
	
CHECKBOUND_LINKS:
	
	MOV	R4,	#3
CHECKBOUND_FOREACH:
; check block validation for each block linkings.
; Apply absolute location to pivot
	; load value from ptr
	MOVX	A,	@DPTR
	SWAP	A
	ADDBLKRELC	A,	PVNXT
	MOV	R1,	A
	
	; Y Axis validation check
	MOV	B,	A
	MOV	A,	R3
	CALL	CHECKBOUND_YVALID
	
	; Call for checkdot function.
	CALL	ssCHECKDOT
	INC	DPTR
	DJNZ	R4,	CHECKBOUND_FOREACH
	
CHECKBOUND_EXT:
	; <<<<<
	POP	B
	POP	AR4 
	RET
	
CHECKBOUND_YVALID: 
	SWAP	A
	ANL	A,	#00FH
	XCH	A,	B
	SWAP	A
	ANL	A,	#00FH
	CLR	C
	SUBB	A,	B; NEXT - CURRENT
	
	; Check if (next - current) + 4 is positive and less than 8.
	ADD	A,	#4
	JNB	ACC.7,	CHECKBOUND_YVALID1; Check if negative.
	CLR	VALID
	RET
CHECKBOUND_YVALID1:
	CLR	C
	SUBB	A,	#8
	JC	CHECKBOUND_YVALID2; If the result is larger than 8...
	CLR	VALID 
CHECKBOUND_YVALID2:
	RET
	
; --------------------------------------------------------------------
;
; @SPECTT 	CHECKDOT
; I	P2:R1	BOARD PIVOT, R1: LOCATION
; I	F0	VALID_PREV
; OUT	F0 	VALID
;
; --------------------------------------------------------------------
ssCHECKDOT:
	USING	GAMEBANK/8
LOCATION	SET	R1 
	; Boundary check, x < 10
	MOV	A,	R1
	; Masks lower 4bits to compare only lower 4bits
	ANL	A,	#0FH
	CLR	C
	SUBB	A,	#10; Now A should be less than 10.
	JNC	CHECKDOT_FAIL
	
	; Data check
	MOVX	A,	@LOCATION
	JNZ	CHECKDOT_FAIL
	SJMP	CHECKDOT_END
CHECKDOT_FAIL:
	CLR	F0
CHECKDOT_END:
	RET 
; --------------------------------------------------------------------
;
; @SPECT	GENBLOCK
;
; X	R7, R6, R5, R3, P2, R1
;
; --------------------------------------------------------------------
; Generates current block link information, for pvt +1, 2, 3.
sGENBLOCK:	
	USING	GAMEBANK/8
	; [-2-1] [-1-1] [ 0-1] [ 1-1]
	; [-2 0] [-1 0] [ PVT] [ 1 0]
	
	; LOAD BLOCK SHAPE INFO
	MOV	DPTR,	#TT_CB_TYPE
	MOVX	A,	@DPTR
	MOV	DPTR,	#TT_BLOCKSHAPE
	MOVC	A,	@A+DPTR
	
BLSHAPE	SET	R7
CURIDX	SET	R6
LEFT	SET	R5

	; Caches it
	MOV	BLSHAPE, 	A
	MOV	CURIDX,	#0
	MOV	LEFT,	#8
	
	; Get pivot link address
	LDCDWP2	R1,	TT_CB_PVT+1
	MOV	DPTR,	#GENBLOCK_TABLE
	
GENBLOCK_LP:
; visits each bits of block shape info, without in case of pivot location(6th) 
	; if curidx == 6, skip all
	CJNE	CURIDX,	#6,	GENBLOCK_VALID
	SJMP	GENBLOCK_ESC
	
GENBLOCK_VALID: 
	; Extract leftmost bit of blshape
	MOV	A,	BLSHAPE
	RLC	A
	MOV	BLSHAPE,	A
	; if CY is active, it means it's pivot is valid
	JNC	GENBLOCK_ESC
	; Load pivot information from table
	MOV	A,	CURIDX
	MOVC	A,	@A+DPTR
	MOVX	@R1,	A
	; mov head to next pivot
	INC	R1
	
GENBLOCK_ESC:
	INC	CURIDX
	DJNZ	LEFT,	GENBLOCK_LP
	
	RET
GENBLOCK_TABLE:
DB	11101111B,	11111111B,	00001111B,	00011111B
DB	11100000B,	11110000B,	00000000B,	00010000B 
; --------------------------------------------------------------------
; @CALBAK	TETUPDATE
; --------------------------------------------------------------------
; The main update routine of this session. 
; This procedure performs following operation.
; 1. Decrease block drop counter by 1.
; 2. If Block drop counter goes 0, reset it as specific number and drop 
;   block by 1.
; 3. If there's nowhere to move when dropping block, then hold the
;   block on the board, and get next block.
cTETUPDATE:
	USING 	GAMEBANK/8
	
	; Check if the block count indicates zero.
	MOV	DPTR,	#TT_CNTLEFT
	MOVX	A,	@DPTR
	JZ	TETUPDATE_ON_CNT_ZERO
	
	; Store the count left 
	DEC	A
	MOVX	@DPTR,	A
	
	RET

TETUPDATE_ON_CNT_ZERO:
; Determines where the count zero event should go.
	; Check if the block can be moved.
	MOV	DPTR,	#TT_CB_PVT
	MOVX	A,	@DPTR
	; To call check bound
	MOV	R3,	A
	ADD	A,	#00010000B
	MOV	R2,	A
	
	INC	DPL
	CALL 	sCHECKBOUND
	
	; VALIDATION INFO = F0
	JNB	F0,	TETUPDATE_ON_BLOCK_FIX
TETUPDATE_ON_BLOCK_DOWN:
	; Store new block info.
	MOV	DPTR,	#TT_CB_PVT
	MOV	A,	R2
	MOVX	@DPTR,	A
	
	; If drop block, jump to entry again.
	JB	TT_DROPBLOCK,	cTETUPDATE
	; done.
	AJMP 	TETUPDATE_BLOCK_CNT_RESET

TETUPDATE_ON_BLOCK_FIX:
; Draws block color on board
; Tries to generate new block, and if cannot do so, game overs.
; And checks block lines to find if there's any score gain.
; If it exists, remove it and update score.
	CLR	TT_DROPBLOCK; On fix, cancel drop block.
	; ADD SCORE BY 1 BASICALLY.
	MOV	B,	#1H
	CALL	rADDSCORE
	SETB	TT_SCOREDIRTY
	
	; Draws block color on given location.
	; First, gets the color.
	MOV	DPTR,	#TT_CB_TYPE
	MOVX	A,	@DPTR
	MOV	DPTR,	#TT_BLOCK_COLOR
	MOVC	A,	@A+DPTR; Gets block color
	MOV	BR0,	A; Caches it
	
	; pivot point.
	LDCDWP2	R0,	TT_CB_PVT
	MOVX	A,	@R0
PIVOT	SET	R4
	MOV	PIVOT,	A; CACHE PIVOT
	
	; fix it to board.
	MOV	DPTR,	#TT_BOARD
	MOV	DPL,	A
	MOV	A,	BR0
	MOVX	@DPTR,	A
	
	; For each block links..
	MOV	R3,	#3
TETUPDATE_FIXING_BLOCK:	
	; for next block link ..
	INC	R0
	MOVX	A,	@R0
	; caculate exact location
	SWAP	A
	ADDBLKRELC	A,	PIVOT
	; fix it.
	MOV	DPL,	A
	MOV	A,	BR0
	MOVX	@DPTR,	A
	DJNZ	R3,	TETUPDATE_FIXING_BLOCK
	
	; If there was any score gain, pull down all the line elements from upper.
	MOV	R6,	#0; SCORE_GETD
	MOV	R7,	#0
	; scans the line from upper.
TETUPDATE_SCAN: 
HEAD	SET	R3
	MOV	HEAD,	#16
	
TETUPDATE_SCAN_0:
	MOV	DPTR,	#TT_BOARD
	CALL	TUP_SCANL
	; Check if there's any score gain.
	JNB	F0,	TETUPDATE_SCAN_1
	; Line destroied
	CALL	TUP_COPYL
	INC	R6
	MOV	A,	R6
	ADD	A,	R7
	MOV	R7,	A
	
	; Level counter
	MOV	DPTR,	#TT_TONEXTLV
	MOVX	A,	@DPTR
	DEC	A
	MOVX	@DPTR,	A
	JNZ	TETUPDATE_SCAN_0
	; --> to next level...
	MOV	DPTR,	#TT_LEVEL
	MOVX	A,	@DPTR
	INC	A
	MOVX	@DPTR,	A
	
	; Level up count = LEVEL * 7
	MOV	B,	#7
	MUL	AB
	
	MOV	DPTR,	#TT_TONEXTLV
	MOVX	@DPTR,	A
	
	; Set flags
	SETB	TT_LVLDIRTY
	
	SJMP 	TETUPDATE_SCAN_0
TETUPDATE_SCAN_1:
	DJNZ	HEAD,	TETUPDATE_SCAN_0

	; Apply score gain.
	MOV	A,	R7
	MOV	B,	#3
	MUL	AB
	DA	A
	MOV	B,	A
	
	MOV	DPTR,	#TT_LEVEL
	MOVX	A,	@DPTR
	MUL	AB
	MOV	B,	A
	
	CALL	rADDSCORE
	
	
	; Skip subroutine area
	SJMP	TETUPDATE_CONTINUE
; ---------------- SUBROUTINES.
TUP_SCANL:	
; I 	R3 	HEAD + 1
; I 	DPTR	BOARD
; OUT	F0	LINE FILLED?
; X	R4
	; Scan specified line and returns result if the line has fully filled.
	; The head address indicates Y index.
	MOV	A,	R3
	DEC	A
	SWAP	A
	MOV	DPL,	A
	
	; Iteration count
	MOV	R4,	#10
	
	; Result.
	SETB	F0
TUP_SCANL_LP:
	MOVX	A,	@DPTR
	JNZ	TUP_SCANL_KEEP; If any line indicates 0, this line is empty.
	CLR	F0
	RET
TUP_SCANL_KEEP:
	INC	DPL
	DJNZ	R4,	TUP_SCANL_LP
; ~F0
	RET
; ~~FUNC

; --------------------
TUP_COPYL:
; I 	R3	HEAD
; Coppies upper line into lower line.
	MOVP2	DPH
	
	; line HEAD will be removed.
	MOV	A,	R3
	MOV	R4,	A
	
TUP_COPYL_0:
; Until all the line'd be shifted down...
	; Destination 
	MOV	A,	R4
	DEC	A
	SWAP	A
	MOV	R0,	A
	; Source
	MOV	A,	R4
	DEC	A
	DEC	A
	SWAP	A
	MOV	R1,	A
	
	MOV	R5,	#10; Iteration count
	
TUP_COPYL_1:
; Copies data from upper to lower.
	MOVX	A,	@R1
	MOVX	@R0,	A
	INC	R0
	INC	R1
	DJNZ	R5,	TUP_COPYL_1
	DJNZ	R4,	TUP_COPYL_0
; ~~TUP_COPYL_0
TUP_COPYL_3:

	MOV	R5,	#10
	MOV	R0,	#0
	MOV	A,	#0
TUP_COPYL_2:
; Clears topmost line as 0.
	;MOVX	@R0,	A
	;INC 	R0
	;DJNZ	R5,	TUP_COPYL_2
; ~~TUP_COPYL_2 

	RET

	
TETUPDATE_CONTINUE:
	; Reset pivot
	MOV	DPTR,	#TT_CB_PVT
	MOV	A,	#014H
	MOVX	@DPTR,	A
	
	; Generates next block. 
	; Gets rand number.
	MOV	A,	RANDNUM  
	MOV	B,	#7
	DIV	AB
	
	; Next block will be current.
	MOV	DPTR,	#TT_NB_TYPE 
	MOVX	A,	@DPTR
	MOV	DPTR,	#TT_CB_TYPE
	MOVX	@DPTR,	A
	
	; Rand % 7 will be next block.
	MOV	DPTR,	#TT_NB_TYPE
	MOV	A,	B
	MOVX	@DPTR,	A
	
	SETB	TT_NEXTDIRTY 
	
	CALL	sGENBLOCK
	
	; Check if new block location is valid.
	; If not, game overs. 
	; @todo
	; calling CHECKBOUND
	MOV	DPTR,	#TT_CB_PVT
	MOVX	A,	@DPTR
	MOV	R2,	A
	MOV	R3,	A
	
	INC	DPL
	CALL	sCHECKBOUND
	
	JB	F0,	TETUPDATE_BLOCK_CNT_RESET
	; --> if generated block is on invalid location ==> game over.
	MOV	DPTR,	#RANKING_RETURN
	MOV	A,	#LOW(rSESSION_TETRIS)
	MOVX	@DPTR,	A

	INC	DPL
	MOV	A,	#HIGH(rSESSION_TETRIS)
	MOVX	@DPTR,	A
	
	MOV	DPTR,	#RANKING_GAMEINFO
	MOV	A,	#0
	MOVX	@DPTR,	A
	
	CALL	rTORANKING
	RET
TETUPDATE_BLOCK_CNT_RESET:
	; 30 - 3*LEVEL should be new delay count
	MOV	DPTR,	#TT_LEVEL
	MOVX	A,	@DPTR
	MOV	B,	#3
	MUL	AB
	; ~(A - 30) + 1 = 30 - A
	ADD	A,	#-30
	CPL	A
	INC	A
	; Stores it.
	MOV	DPTR,	#TT_CNTLEFT
	MOVX	@DPTR,	A
	
	; Refresh dirty flags. 
	SETB	TT_BOARDDIRTY
	CLR	TT_FLGCLR
	; done.
	RET
; --------------------------------------------------------------------
; @CALBAK	TETINIT
; --------------------------------------------------------------------
cTETINIT:	; TETRIS GAME LOGIC INITIALIZE 
	SETB	TT_SCOREDIRTY
	SETB	TT_LVLDIRTY
	SETB	TT_BOARDDIRTY
	SETB	TT_NEXTDIRTY
	CLR	TT_FLGCLR
	
	CALL	rINITSCORE
	
	; RANDOM BLOCKS
	MOV	A,	RANDNUM
	MOV	B,	#7
	DIV	AB
	
	; A % 8 will be stored as current block
	MOV	DPTR,	#TT_NB_TYPE
	PUSH	ACC
	MOV	A,	B
	MOVX	@DPTR,	A
	
	; B will be stored as current block
	MOV	DPTR,	#TT_CB_TYPE
	POP	ACC
	MOV	B,	#7
	DIV	AB
	MOV	A,	B
	MOVX	@DPTR,	A
	
	; Reset active block pivot
	MOV	DPTR,	#TT_CB_PVT
	MOV	A,	#014H
	MOVX	@DPTR,	A
	
	; Generate current block
	CALL	sGENBLOCK 
	
	; Reset level
	MOV	DPTR,	#TT_LEVEL
	MOV	A,	#1
	MOVX	@DPTR,	A
	
	; Reset level up count
	MOV	DPTR,	#TT_TONEXTLV
	MOV	A,	#5
	MOVX	@DPTR,	A
	
	
	; clear flags
	CLR	TT_DROPBLOCK
	
	RET
; --------------------------------------------------------------------
; @CALBAK	TETDNDRAW
; --------------------------------------------------------------------
cTETDNDRAW: ; TETRIS DYNAMIC DRAW
	USING 	GAMEBANK/8
	; CACHES BACK BUFFER
	CALL	rBUFFER_BACK
	MOV	R7,	A
BACKBUFFER 	SET	R7

; Dirty flag will be drawn later ...	
TETDNDRAW_LV:
; Meanless label for identification
	JNB	TT_LVLDIRTY,	TETDNDRAW_BRD
	; Clear target board area
	MOV	DPH,	BACKBUFFER
	MOV	DPL,	#TT_PV_LVL
	MOV	B,	#000B
	MOV	R1,	#4
	MOV	R2,	#6
	CLR	C
	CALL	rFILLBOX
	; Draw level information
	MOV	DPTR,	#TT_LEVEL
	MOVX	A,	@DPTR
	; To ASCII
	ADD	A,	#'0'
	; @See RDRAWASCII
	MOV	B,	#011B
	MOVP2	R7
	MOV	R1,	#TT_PV_LVL
	CLR	F0
	CALL	rDRAWASCII
TETDNDRAW_BRD:
	JB	TT_BOARDDIRTY, 	TETDNDRAW_BRD_ENTRY
	JMP	TETDNDRAW_SCORE
	
TETDNDRAW_BRD_ENTRY:
	; >> Clears board 
	; ##deprecated since drawing black dots at duplicated location on the code below wastes the performance.
	;MOV	DPH,	BACKBUFFER
	;MOV	DPL,	#TT_PV_BOARD
	;MOV	B,	#0
	;MOV	R1,	#10
	;MOV	R2,	#16
	;CLR	C
	;CALL	rFILLBOX
	
	; >> Draw board geometry
	; Draws dot for 0x00 ~ 0x9f
	LDCDWP2 	R0,	TT_BOARD
	MOV	DPH,	BACKBUFFER
	
	MOV	R4,	#16; Vertical index
TETDNDRAW_BRD_BRD0:
	MOV	R3,	#10; Horizontal index
TETDNDRAW_BRD_BRD1:
	; Assemble r4 and r3, then use it to draw board color.
	MOV	A,	R4
	DEC	A; should be decreased for 1.
	SWAP	A; R4 indicates Y coordinate.
	ADD	A,	R3
	DEC	A; R3 indicates X coordinate.
	; extracts color from board and caches it
	MOV	R0,	A
	MOVX	A,	@R0
	PUSH	ACC
	
	; find location on board.
	MOV	A,	R0
	BLKTODISP	A
	MOV	DPL,	A
	MOV	C,	F0
	POP	B; color which was cached.
	CALL	rDRAWDOT

	DJNZ	R3,	TETDNDRAW_BRD_BRD1
; ~~LP ------
	DJNZ	R4,	TETDNDRAW_BRD_BRD0
; ~LP -------
	
	; >> Draw current active block instance
	; Extracts color information
	MOV	DPTR,	#TT_CB_TYPE
	MOVX	A,	@DPTR
	MOV	DPTR,	#TT_BLOCK_COLOR
	MOVC	A,	@A+DPTR
	; Caches color
COLOR	SET	R5
	MOV	COLOR,	A
	; Draw dot on pivot
	; Extracts pivot location
	MOV	DPTR,	#TT_CB_PVT
	MOVX	A,	@DPTR
	; Caches pivot location
PIVOT	SET	R6
	MOV	PIVOT,	A
	
	; Draw dot
	MOV	DPH,	BACKBUFFER
	BLKTODISP 	A
	MOV	DPL,	A
	MOV	C,	F0
	MOV	B,	COLOR
	CALL	rDRAWDOT
	  
	; Load pivot, and calculate for 4 bits.
	MOV	R2,	#3
	MOV	DPTR,	#TT_CB_PVT
TETDNDRAW_BRD_AB:
; To reuse DPL for 3 times, push this.
	INC	DPL	; SInce its value is in PVT now..
	PUSH	DPL
	PUSH	DPH
	; Gets relative offset.
	MOVX	A,	@DPTR
	SWAP	A
	ADDBLKRELC	A,	PIVOT
	; Now there is relative offset in A, changes this to absolute 
	;offset by adding block alignment info.
	BLKTODISP	A
	MOV	DPH,	BACKBUFFER
	MOV	DPL,	A
	MOV	C,	F0
	MOV	B,	COLOR
	CALL 	rDRAWDOT
	; Pops DPL.
	POP	DPH
	POP	DPL
	DJNZ	R2,	TETDNDRAW_BRD_AB
; ~~~ LOOP DONE
	
	; Draw dots on pivot links
TETDNDRAW_SCORE:
	JNB	TT_SCOREDIRTY,	TETDNDRAW_NEXT
	; Draws score information
	
	; lower 2 bytes will be used.
	LDCDWP2	R0,	(RANKING_SCORE+1)
	MOV	DPTR,	#STRCACHE; result string will be loaded here
	MOVX	A,	@R0
	MOV	B,	A
	CALL	rDTOA
	
	DEC	R0
	MOVX	A,	@R0
	MOV	B,	A
	CALL	rDTOA
	
	; Clear target board area
	MOV	DPH,	BACKBUFFER
	MOV	DPL,	#TT_PV_SCORE
	MOV	B,	#000B
	MOV	R1,	#16
	MOV	R2,	#6
	CLR	C
	CALL	rFILLBOX
	
	; Draw string on there.
	CLR	F0
	MOVP2	BACKBUFFER
	MOV	R1,	#TT_PV_SCORE
	MOV	DPTR,	#STRCACHE
	MOV	B,	#110B
	CALL	rDRAWSTRING
	
TETDNDRAW_NEXT:
	JNB	TT_NEXTDIRTY,	TETDNDRAW_DONE
	; Draw next block information
	MOV	DPTR,	#TT_NB_TYPE
	MOVX	A,	@DPTR
	; Caches it
	MOV	B,	A
	; Get block draw information
	MOV	DPTR,	#TT_BLOCKSHAPE
	MOVC	A,	@A+DPTR
	; Caches it
	MOV	R3,	A
	
	; Extract color info
	MOV	DPTR,	#TT_BLOCK_COLOR
	MOV	A,	B
	MOVC	A,	@A+DPTR
	; Caches it
	MOV	R4,	A
	
CONST 	TT_NEXT_PIVOT, 01000001B
	MOV	DPH,	R7
	MOV	DPL,	#TT_NEXT_PIVOT
	; Draw next block loop... UPPER LINE
	MOV	R2,	#4
TETDNDRAW_NBLP:
; 0 is black, 1 is given block color
	; If the rightmost bit is 1...
	; Don't need to care current carry bit which will be set in leftmost.
	MOV	A,	R3
	RLC	A
	MOV	R3,	A
	JNC	TETDNDDRAW_DRAWBLACK
	; DRAW COLOR
	MOV	B,	R4
	CLR	C
	CALL	rDRAWDOT
	SJMP	TETDNDRAW_NBDONE	
TETDNDDRAW_DRAWBLACK:
	; DRAW BLACK
	CLR	C
	MOV	B,	#0
	CALL	rDRAWDOT
TETDNDRAW_NBDONE:
	INC	DPL
	DJNZ	R2,	TETDNDRAW_NBLP
	MOV	A,	DPL
	JB	ACC.5,	TETDNDRAW_DONE
	; Draw LOWER LINE
	MOV	DPL,	#TT_NEXT_PIVOT + 20H
	MOV	R2,	#4
	SJMP	TETDNDRAW_NBLP
TETDNDRAW_DONE:
	; To avoid situation that the elements are drawn only 
	;in single buffer, this flag is used. On the first draw 
	;call, not any flag will be cleard though they are already 
	;drawn on a single buffer, but they are cleared on the 
	;second call. However, there is possibility that the flag 
	;information has changed while in the middle of dirty flag 
	;clearing, therefore this flag should be reset on any 
	;dirty flag activation.
	JB	TT_FLGCLR,	TETDNDRAW_CLR
	SETB	TT_FLGCLR
	RET
TETDNDRAW_CLR:
	CLR	TT_FLGCLR
	CLR	TT_LVLDIRTY
	CLR	TT_NEXTDIRTY
	CLR	TT_BOARDDIRTY
	CLR	TT_SCOREDIRTY
	RET
; --------------------------------------------------------------------
; @CALBAK	TETSTDRAW
; --------------------------------------------------------------------
cTETSTDRAW: ; TETRIS STATIC DRAW
; [0...5, 0...5] 	NEXT BLOCK WITH BOUNDARY
; [0...4, 6...11]	LEVEL DISPLAY
; [5...16, 0...15] 	TETRIS BOARD WITH BOUNDARY
; [17...31, 3...10] 	SCORE DISPLAY WITH BOUNDARY
	USING	GAMEBANK/8
	
	; Cache current back buffer
	CALL	rBUFFER_BACK
	MOV	R7,	A
	
	; Clears screen
	MOV	A,	#0
	CALL	rFILLBUFFER
	
	; DRAW NEXT BLOCK BOUNDARY
	; @SEE 	GRAPHICS.INC/rDRAWBOX
	MOV	DPH,	R7	; [0, 0]
	MOV	DPL,	#0
	MOV	B,	#111B 	; [RGB]
	MOV	R1,	#6
	MOV	R2,	#6
	CLR	C
	CALL	rDRAWBOX
	
	; DRAW BOARD BOUNDARY
	MOV	DPL,	#0101B 	; [5, 0]
	MOV	B,	#111B 	; [RGB]
	MOV	R1,	#12
	MOV	R2,	#16
	CLR	C
	CALL	rDRAWBOX
	
	; To use maximum area, the upper and low lines of boundary
	;will be removed
	MOV	DPL,	#0110B 	; [6, 0]
	MOV	B,	#000B 	; [RGB]
	MOV	R1,	#10
	MOV	R2,	#16
	CLR	C
	CALL	rDRAWBOX
	
	; To draw score boundary, firstly fills right side as white.
	MOV	DPL,	#00010001B	; [17, 0]
	MOV	B,	#111B
	MOV	R1,	#15
	MOV	R2,	#16
	CLR	C
	CALL	rFILLBOX
	
	; ... Score area will be cleared on dynamic draw call	
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
cTETINPUT:
	USING 	GAMEBANK/8
; TEMPORARY CODE... INPUT PROCESS 
	MOV	B,	R0
	MOV	DPTR,	#TT_CB_PVT
	MOV	BR2,	R2
	
?IPN:	JNB	B.1,	?IPL
	; Immediately drops block.
	SETB	TT_DROPBLOCK
	SJMP 	?IPD0
	
?IPL:	JNB	B.6,	?IPR
	MOV	R3,	#00001111B
	CALL	TETINPUT_MVB
	RET
	
?IPR:	JNB	B.3,	?IPD
	MOV	R3,	#00000001B 
	CALL	TETINPUT_MVB
	RET
	
?IPD:	JNB	BR2.5,	?IPU 
?IPD0:	MOV	DPTR,	#TT_CNTLEFT
	MOV	A,	#0
	MOVX	@DPTR,	A
	RET
	
?IPU:	JNB	B.2,	TETINPUT_END 
	; rotates block for clockwise 90 degree
	MOV	DPTR,	#TT_CB_PVT
	MOVX	A,	@DPTR
	MOV	R2,	A
	INC	DPL
	; tries rotation
	CALL	sROTATER90
	JB	F0,	?IPU_VALID
	
	; tries for some other cases...
	DEC	R2
	MOV	DPTR,	#TT_CB_PVT+1
	CALL	sROTATER90
	JB	F0,	?IPU_VALID_OTHER
	
	INC	R2
	INC	R2
	MOV	DPTR,	#TT_CB_PVT+1
	CALL	sROTATER90
	JB	F0,	?IPU_VALID_OTHER
	
	RET
?IPU_VALID_OTHER:
	LDCDWP2	R0,	TT_CB_PVT
	MOV	A,	R2
	MOVX	@R0,	A
	
?IPU_VALID:	; if rotation was valid...
	LDCDWP2 	R0, 	(TT_CB_PVT+1)
	MOV	R2,	#3

?IPU_VALID_0:	
	MOVX	A,	@DPTR
	MOVX	@R0,	A
	INC	R0
	INC	DPL
	DJNZ	R2,	?IPU_VALID_0
	
	SETB	TT_BOARDDIRTY
	CLR	TT_FLGCLR
	
	RET
	
TETINPUT_MVB:
	MOVX	A,	@DPTR
	PUSH	ACC
	ADDBLKRELC	A,	R3
	POP	AR3 ; Current location cache.
	; check boundary.
	MOV	R2,	A
	INC	DPL; For the rest of pivot...
	CALL	sCHECKBOUND
	; if result is valid...
	JB	F0, 	TETINPUT_CMITMV 
	RET
TETINPUT_CMITMV:
	; re-adjust dptr and store value
	MOV	DPTR,	#TT_CB_PVT
	MOV	A,	R2
	MOVX	@DPTR,	A
	; dirty flag reset.
	SETB	TT_BOARDDIRTY
	CLR	TT_FLGCLR
	
	RET
TETINPUT_END:
	RET
; --------------------------------------------------------------------
; 
; @SPECT	ROTATER90
;
; I	R2	PIVOT LOCATION
; I[3]	DPTR	LINK INFO
; OUT[3]	DPTR	rotated result.
; OUT	F0	Was valid ?
;
; X 	R0, B, BR1, R3
;
; --------------------------------------------------------------------
MEMALIGN	LOCAL_MEMORY, 8
LOCMEM	ROTATE_CACHE, 8
; Rotates given block links to right.
sROTATER90:
	USING 	GAMEBANK/8
	
	LDCDWP2	R0, 	ROTATE_CACHE
	
	MOV	BR1,	#3
ROTATE90_0:	; (x, y) -> (y, -x)
	MOVX	A,	@DPTR
	
	SWAP	A
	PUSH	ACC
	
	CPL	A
	INC	A
	ANL	A,	#00FH
	MOV	B,	A
	
	POP	ACC
	ANL	A,	#0F0H
	ADD	A,	B
	
	; cache it
	MOVX	@R0,	A
	
	INC	R0
	INC	DPL
	
	DJNZ	BR1,	ROTATE90_0
; ~LP
	DEC	DPL
	DEC	DPL 
	
ROTATE90_ISVALID:
; Checks if the upcoming location is valid
	MOV	DPTR,	#ROTATE_CACHE
	MOV	AR3,	R2
	CALL	sCHECKBOUND
	
	; return value.
	MOV	DPTR,	#ROTATE_CACHE
	RET	
	
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
