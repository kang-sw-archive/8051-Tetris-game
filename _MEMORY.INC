
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; 	MEMORY LOCATION DEFINITIONS
; This area is consist of overall memory allocation and pre-definition
; $ Stack begins at last position of external memory offset.
; --------------------------------------------------------------------
; Global/internal memory helper macro.
;
; By using following macros, the offset and location of memories are
;  can be easily managed by identifier. Operation principle is simeple-
;  just declare a global data identifier with its offset, then internal
;  HEAD which pointing current usable memory address will define that
;  identifier as current HEAD location, and will be move forward for
;  the amount of given size. You can use this macro like-
;	GLBMEM SOME_DAT_ID, 4
;	LOCMEM SOME_DAT_ID, 35
; 	LOCMEM_PVT ; This macro will reset local memory assignement
;		  to 2000h. Should be used on situation which are
;		  perfectly independent and separted so that the
;  		  given memories are asured to never be used at
; 		  the same time.
;	INTMEM SOME_DAT_ID, 3; This macro will assign given id on
; 			internal memory location from 30H~,
;			and all the rest of internal memory
;			are for stack.
;	GBIT SOME_BIT_FLAG; 	This macro will assign a bit flag from
;			 2FH.7 ~ 28H.0. Since the memories from
;			 20H to 27H is used as bit-referenceable
;			 registers as an identifer of 'BR0 ~ BR7',
;			 the number of GBIT and LBIT should be
;			 less than 128.
;	LBIT ID; This is same with the relationship between GLBMEM
;	          and LOCMEM
;	LBIT_PVT
;
; To ensure valid operation on specific cases(for example, when you
;  have to avoid any carry occured to DPH while incrementing DPL),
;  a alignment macro is provided. When you call this macro with
;  alignment offset and HEAD identifier, the HEAD will be aligned
;  on given memory offset. Like-
; 	MEMALIGN GLOBAL_MEMORY, 100H 	; Align global memory by
;				   100H byte boundary
; 	MEMALIGN LOCAL_MEMORY, 20H	; Align local memory by
;				   20H byte boundary
; --------------------------------------------------------------------

; MEMALIGN
; 	GLOBAL_MEMORY
; 	LOCAL_MEMORY

; GLBMEM(ID, SIZE)
; GLBMEMAT(ID, SIZE, LOCATION)
; LOCMEM(ID, SIZE)
; GBIT(ID)

; LBIT_PVT()
; LBIT(ID)
; ==> GLOBAL MEMORY ASSIGNMENT
GLOBAL_MEMORY	SET	1000H

GLBMEM	MACRO	ID, SIZE
ID	EQU	GLOBAL_MEMORY
GLOBAL_MEMORY	SET	GLOBAL_MEMORY+SIZE
IF	GLOBAL_MEMORY>=2000H
$ERROR	("GLOBAL MEMORY OVERFLOW")
ENDIF
ENDM

GLBMEMAT	MACRO	ID, SIZE, LOC
IF	LOC<GLOBAL_MEMORY
$ERROR	("INVALID GLOBAL MEMORY OFFSET")
ENDIF
ID	EQU	LOC
GLOBAL_MEMORY	SET	LOC+SIZE
ENDM

; ==> EXTERNAL MEMORY MANAGEMENT
LOCAL_MEMORY SET	2000H

LOCMEM_PVT	MACRO
LOCAL_MEMORY SET	2000H 
ENDM 

LOCMEM	MACRO	ID, SIZE
ID	SET	LOCAL_MEMORY
LOCAL_MEMORY SET	LOCAL_MEMORY+SIZE
IF	LOCAL_MEMORY>=4000H
$ERROR	("EXTERNAL MEMORY OVERFLOW")
ENDIF
ENDM 

; ==> INTERNAL MEMORY ASSIGNMENT
__INT_OFST	SET	30H

INTMEM	MACRO	ID, SIZE
	LOCAL	ADDR
ADDR:
IF 	ADDR>=4000H
$ERROR	("INTMEM declaration should precede code segment.")
ENDIF
ID	EQU	__INT_OFST
__INT_OFST	SET	__INT_OFST+SIZE
ENDM

; ==> INTERNAL BIT DATA ASSIGNEMENT
__INTB_OFST	SET	07FH

GBIT	MACRO	ID
	LOCAL	ADDR
ADDR:
IF 	ADDR>=4000H
$ERROR	("GBIT declaration should precede code segment.")
ENDIF
ID	EQU	__INTB_OFST
__INTB_OFST	SET	__INTB_OFST-1
ENDM

; ==> LOCAL BIT DATA ASSIGNMENT
LBIT_PVT	MACRO
	LOCAL	ADDR
ADDR:
IF	ADDR < 4000H
$ERROR	("LBIT should be placed after code segment.")
ENDIF
__LB_PIVOT 	SET 	__INTB_OFST
ENDM

LBIT	MACRO	ID
ID	SET	__LB_PIVOT
__LB_PIVOT	SET	__LB_PIVOT -1
ENDM

; ==> These macro helps align data by data boundary
MEMALIGN	MACRO	HEAD, ALIGN
HEAD	SET	((HEAD+(ALIGN-1))/ALIGN)*ALIGN
ENDM

; ==> CONSTANT
CONST	MACRO	ID, VALUE
ID	EQU	VALUE
ENDM

; ==> PORT DEFINITION
LCDPORT	EQU	8000H
OUTPUTPORT	EQU	0C000H
INPUTPORT	EQU	0C000H
; ####################################################################
; ####################################################################
; ####################################################################
; ##################  MEMORY ALLOCATION  #############################
; ####################################################################
; ####################################################################
; ####################################################################
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; --------------------------------------------------------------------
; TYPE     |IDENTIFIER |SIZE/DESCRIPTION
; ---------|-----------|----------------------------------------------  
GLBMEMAT 	GBUFFER1, 	100H, 1000H;.2nd graphics buffer
GLBMEMAT	GBUFFER2, 	100H, 1100H;.2nd graphics buffer
GBIT	CURBUFF;	.A flag notices which
		; buffer is idle now. If this flag indicates
		; 1, then the gameplay loop can call draw
		; operations targetting GBUFFER1.
GBIT	FRAMEBOUND60; .A flag which notices 
		; a framerate boundary, which will be set per 16 ms. 
GBIT	FRAMEBOUND30; .30FPS BOUNDARY
		
;GBIT	SPK_1;	.Flags which drive
;GBIT	SPK_2;	.extern speaker.
		; These will be bound into NAND gate with SPK
		; Enable signal, therefore can mute the
		; speaker.
		
;CONST	SPKEN, LCDPORT;	.D4 and D5 bit will
		; be used to indicate the speaker enable
		; signal.
		
;INTMEM	SPK_HZ1,	1;	.These bytes indicate
;INTMEM	SPK_HZ2,	1;	.the counts left until
		; the timer inturruption routine would flip
		; the SPK output. 
		
;INTMEM	SPK_LD1,	1;	
;INTMEM	SPK_LD2,	1;	.These bytes indicate
		; the counts which will be reloaded onto
		; SPK_HZ register when the output flipping
		; occurs.
 
RANDNUM	SET	TL1
INTMEM	CS_INPUTPR,	2;	.CALLBACKS
INTMEM	CS_UPDATE,	2;
INTMEM	CS_DRAW,	2;

INTMEM	GLB_P2CACHE,1;	.The global byte location
		; which is used for caching P2 value, this exists
		; to protect P2 value while interrupt subroutine.

INTMEM	PREV_INPUT, 1;	.Will be used on input preprocessor.

MEMALIGN 	GLOBAL_MEMORY, 20H
GLBMEM	TCNT, 8;		.Timer memory allocation.
GLBMEM	TCALBAKOFST, 2

GLBMEM	RANKING_RETURN, 2; 	.At the end of any game session,
		;  the program will ask player's name and will
		;  record it into SCORE_GAMENAME. Any game ending
		;  will cache the returning session after
		;  recording and score recorded in 4 digit BCD
		;  which was calculated in the session's own.
		; Then it transite into recording session.
		; On the recording session, the session will
		;  take player's name and read game informations
		;  from SCORECACHE_GAME, SCORECACHE_SCORE_BCD
		;  and record it to the game's own memory space.

GBIT	RANKING_RECORDEN; .Determines whether record score to 
		;  ranking or not.
MEMALIGN	GLOBAL_MEMORY, 20H
GLBMEM	RANKING_GAMEINFO, 1;
GLBMEM	RANKING_SCORE, 4;

; SCORES = TETRIS, SCORES + 4 = JIRUNG, SCORES + 8 = ? 
GLBMEM	RANKING_HIGHST, 12
ERRORBIT	EQU	P1.2
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; --------------------------------------------------------------------
;	CONSTANT IDENTIFIERS
; IDENTIFIER	VALUE	DESCRIPTION
; ---------------------|-----------|----------------------------------
BR0	EQU	20H	; Usable bit-refrence registers...
BR1	EQU	21H
BR2	EQU	22H
BR3	EQU	23H
BR4	EQU	24H
BR5	EQU	25H
BR6	EQU	26H
BR7	EQU	27H

DBGBANK	EQU	00H;
GPUBANK	EQU	08H;	register bank used in Graphics
		; processing sequence.
GAMEBANK	EQU	10H;	register bank used in gameplay
		; update procedure. 
BANK2	EQU	18H;	temporary banks which can be used 
;BANK3	EQU	18H; in primitive functions.

_SSN_BANK	EQU	18H;	register bank used in session
_FRAME	EQU	0C43BH;	C43BH = 60FPS, 881DH = 30FPS

BTN0	EQU	01H
BTN1	EQU	02H
BTN2	EQU	04H
BTN3	EQU	08H
BTN4	EQU	10H
BTN5	EQU	20H
BTN6	EQU	40H
BTN7	EQU	80H
; --------------------------------------------------------------------
; Options
__OPT_SIZE__	EQU	1
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ####################################################################
; ####################################################################
; ####################################################################
; ####################################################################
; ####################################################################
; ####################################################################
; ####################################################################
; ####################################################################
; ####################################################################
