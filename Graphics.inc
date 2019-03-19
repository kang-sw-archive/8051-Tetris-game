; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DEFINE_MODULE	Graphics ; Change this as you want.
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
$INCLUDE	(UTILS.INC)
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;
;	Graphics Module
;
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;	DESCRIPTION
;
; This module contains most of graphical utilities and subroutines.
; Every procedures are designed to enable to drive "DFR0471" smoothly
;  which is 32*16 RGB display whose pixel count is 512.
;
; Every procedure labels begin with prefix 'GP' assumes that they
;  are called only in main graphics processing loop, therefore there
;  is not any memory allocation sequence to secure this process. 
;
; Procedures wich consists the rest of code segment are interface
;  functions which draw primitive elements on the graphics back buffer.
;
; Since drawing a single dot repeatively is pretty inefficient process,
;  this module provides many types of drawing functionality.
;
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
;
; @NOTICE	DRAWING API OVERALL
;
; ┌───────────────────────────────────┐ ↑
; │ x                                 │ 8   SCREEN SELECTION 0
; │                                   │ ↓
; │                                   │ ↑
; │                                   │ 8   SCREEN SELECTION 1
; └───────────────────────────────────┘ ↓
; |←---------------32----------------→|
;  x : Origin.
;
; The first dot of display is at left and topmost.
; To draw a dot on display, you should specify the address of target
;  dot which is indicated by a bit and a byte. A bit indicates a
;  selection between screens divided upper and lower segment,
;  assigning 0 on this bit targets upper screen.
; The higher 3bit of address byte indicates the vertical line, and
;  lower 5bit of byte indicates horizontal line.
; Additionally, (You don't need to remember this thing..) the OUTPUT
;  port of this 8051 kit will be used to indicate 6 bit of RGB output
;  which will be pushed into display's register. This will be separted
;  into upper and lower 4bits, and their topmost bit will be ignored.
;  This targets to adapt SWAP operation onto them smoothly therefore
;  to achieve higher performance.
; --------------------------------------------------------------------
;
; @PROC	FILLBUFFER
; $MC 1047
; I 	A	INPUT COLOR. Uses only lower 4bits
; OUT	DPH	BACK BUFFER
; 
; XX 	DPL	WILL BE 0.
;
; --------------------------------------------------------------------
; Fills back buffer as given color input. 
rFILLBUFFER:
	PUSH	B
	; >>>>>
	; copies lower 4bits' data onto upper 4bit
	ANL	A,	#0FH
	MOV	B,	A
	SWAP	A
	ADD	A,	B
	MOV	B,	A
	
	; gets current back buffer
	CALL 	rBUFFER_BACK
	
	; set initial buffer point
	MOV	DPH,	A
	MOV	DPL,	#0
	
	MOV	A,	B
	
; fills all of the cells as given color.
; this will operate by decrementing direction.
FILLBUFFER_0:
	MOVX	@DPTR,	A
	DJNZ	DPL,	FILLBUFFER_0
	
	; <<<<<	
	POP	B
	RET	
; --------------------------------------------------------------------
; #MACRO	DP_ASMLOC	AA, BB
; AA 	I A	Y value. 0~15
; BB 	IO B	X value. 0~31. Will contain result.
;	O F0	Screen selection output.
; --------------------------------------------------------------------
DP_ASMLOC	MACRO	AA, BB
	MOV	F0,	ACC.3
	SWAP	AA
	RL	A
	ANL	B	#00011111B
	ANL	A	#11100000B
	ORL	B,	A
	ENDM
; --------------------------------------------------------------------
;
; @PROC	DRAWBOX
; $MC 40*WIDTH + 51*HEIGHT - 34
;
; I  	DPH	BACK BUFFER
; IX 	CY	SCRREEN SELECTION
; IX 	DPL	[Y3:X5] LOCATION
; IX [3:0]	B	COLOR
; I  	R1	WIDTH
; IX 	R2	HEIGHT 
;
; X BIT	F0
;
; --------------------------------------------------------------------
; Draws a box on given location with given width and height.
rDRAWBOX:
	USING	GAMEBANK/8
WIDTH	SET 	AR1
HEIGHT	SET	AR2
CACHE	SET	R3
COLOR	SET	R4

	PUSH	AR3
	PUSH	AR4
	PUSH	ACC
	
	; >>>>>
	; Uses lower 4bit of color only.
	ANL	B,	#0FH
	; caches color.
	MOV	AR4,	B
	
	; this will define a HEAD, and will draw a box via moving
	;  the HEAD as the amount of it should move on.
	
	; the first it will move into right, and then go down,
	;  and then go left, and will go back to beginning point.

	; caches CY
	MOV	F0,	C

	; 1. Goes by right direction. R3 will indicate
	;  counts left to move.
	MOV	CACHE,	WIDTH 
DRAWBOX_R:	; Draws dot until it reached at given width.
	; Select screen
	MOV	C,	F0
	MOV	B,	COLOR
	; draws dot on current HEAD location
	CALL	rDRAWDOT
	; moves HEAD right
	INC	DPL
	; repeat...
	DJNZ	CACHE,	DRAWBOX_R

;	SJMP	DRAWBOX_X
	
	; since the HEAD is pointing 1 dot further, roll it back.
	DEC 	DPL
	; 2. Goes down. 
	; @the value in HEIGHT will not be used anymore.
	; since the first dot was already drawn...
	DEC	HEIGHT
; at this point, the HEAD is pointing the latest drawn dot.
;  to avoid duplication, this sequence moves HEAD first,
;  and then begins drawing dot.
DRAWBOX_D:	; downs HEAD by adding 0b00100000
	MOV	A,	#00100000B
	ADD	A,	DPL
	MOV	DPL,	A
	; adapt ORL to C and F0. since the screen selection
	;  value never be cleared after set ever once.
	ORL	C,	F0
	MOV	F0,	C  
	; draw dot. 
	MOV	B,	COLOR
	CALL	rDRAWDOT
	; draw left side also. it can be easily adapted by
	;  subtracting (Width-1) from DPL 
	; WIDTH-1
	MOV	A,	WIDTH
	DEC	A
	; gets the 2's complementary of A.
	CPL	A
	INC	A
	; cache DPL
	MOV	CACHE,	DPL
	; add this onto DPL, actually indicates the operation
	;  "DPL - (WIDTH - 1)"
	ADD	A,	DPL
	MOV	DPL,	A
	; draw a dot
	MOV	B,	COLOR
	MOV	C,	F0
	CALL	rDRAWDOT
	; revert DPL.
	MOV	DPL,	CACHE
	; repeat this for height count ...
	DJNZ	HEIGHT, 	DRAWBOX_D
	
	; start to draw left side ... 
	; to avoid duplication, it should draw dots for
	;  (WIDTH - 2) times.
	MOV	CACHE,	WIDTH
	DEC	CACHE
	DEC	CACHE
	; at this point the HEAD is pointing the rightmost and
	;  downmost dot of the box boundary.
DRAWBOX_L: 	; draws dots by moving left...
	DEC	DPL
	MOV	B,	COLOR
	MOV	C,	F0
	CALL	rDRAWDOT
	DJNZ	CACHE,	DRAWBOX_L
	
DRAWBOX_X:	; <<<<<	
	POP	ACC
	POP	AR4
	POP	AR3
	RET
; --------------------------------------------------------------------
;
; @PROC	FILLBOX
; $MC 23 + HEIGHT*(15 + WIDTH*20)
;
; I  	DPH	BACK BUFFER
; IX B	CY	SCRREEN SELECTION
; IX 	DPL	[Y3:X5] LOCATION
; IX [3:0]	B	COLOR
; I 	R1	WIDTH
; IX 	R2	HEIGHT 
;
; X  B	F0
;
; --------------------------------------------------------------------
; Fill a box with given color.
rFILLBOX:
	USING	GAMEBANK/8
WIDTH	SET	R1
HEIGHT	SET	R2
CACHE	SET	R3
COLOR	SET	R4
WID_IT	SET	AR5

	PUSH	ACC
	PUSH	AR3
	PUSH	AR4
	PUSH	AR5
	; >>>>>
; fills horizontal lines one by one.
	; masks input color for lower 4bits.
	ANL	B,	#0FH
	; caches CY and input color
	MOV	F0,	C
	MOV	COLOR,	B
	
FILLBOX_VERT:
; the outer loop does vertical indexing.
	; caches DPL before filling a horizontal line.
	MOV	CACHE,	DPL
	MOV	WID_IT,	WIDTH
FILLBOX_HORI:
; the inner loop does horizontal line filling.
	; .draws a dot on HEAD
	; assign screen seleciton
	MOV	C,	F0
	MOV	B,	COLOR
	CALL	rDRAWDOT
	; move HEAD to next.
	INC	DPL
	; repeats this for WIDTH
	DJNZ	WID_IT,	FILLBOX_HORI
; ~FILLBOX_HORI
	; .moves HEAD down for 1 dot.
	; rolls back dpl from cache
	MOV	DPL,	CACHE
	; moves down HEAD by adding 0010 0000 = 20h
	MOV	A,	#20H
	ADD	A,	DPL
	MOV	DPL,	A
	; adapt ORL to C and F0. since the screen selection
	;  value never be cleared after set ever once.
	ORL	C,	F0
	MOV	F0,	C
	DJNZ	HEIGHT,	FILLBOX_VERT
; ~FILLBOX_VERT
	; <<<<<	
	POP	AR5
	POP	AR4
	POP	AR3
	POP	ACC
	RET
; --------------------------------------------------------------------
;
; @PROC	DRAWIMAGE
;
; I  	P2	BACK BUFFER
; IX B 	CY	SCREEN_SELECTION
; IX 	R1	[Y3:X5] PIVOT LOCATION
; I  2 	DPTR	IMAGE LOCATION
;
; X  B	F0	
; X	B	
;	
; --------------------------------------------------------------------
; Draws image onto given location. An image format is 
; 	{0:1bit}{1:7}...{{2:x}{'0b10001000'}}*{1}
; ex.
;	00000100
;		0110:0111 0010:0001 0100:0001 10001000
;		0100:0001 0100:1000 1000:0010 0001:0010 10001000
; 		....
;     (0b100 th row)	0001:0010 10001000
;
; The first byte is header. The first bit determines whether the
;  drawing HEAD treats RGB 000 as void or black color.
; Next 7bits are number of rows. 
; From then, it will start to draw colors by 4bits one by one.
; The constant '10001000' is treated like a null character. 
; If the drawing HEAD encounters the null character, it will
;  jump to next line.
; However only fully filled byte with null character will be treated
;  as null. If when the only half of a byte is filled with '0b1000',
;  then it will be treated just like void.
rDRAWIMAGE:
	USING 	GAMEBANK/8
HEAD	SET	R1
LINELEFT	SET	R2
SCRNSELEC	SET	BR0.0
CACHE	SET	R3
C_HEAD	SET	R4
BLTOVOID	SET	F0

	PUSH 	ACC
	PUSH	AR2
	PUSH	AR3
	PUSH	AR4
	PUSH	BR0
	; >>>>>
	; cache screen selection
	MOV	SCRNSELEC,	C
	
	; parse header
	MOV	A,	#0
	MOVC 	A,	@A+DPTR
	MOV	C,	ACC.7
	MOV	BLTOVOID,	C 
	ANL	A,	#01111111B
	MOV	LINELEFT,	A
	
	; move head forward.
	INC	DPTR
	
	; .start draw image
DRAWIMAGE_LINE_ENTRY:
; This outer loop indicates starting draw the first position of an image.
	; cache current HEAD location
	MOV	AR4,	HEAD
DRAWIMAGE_DRAW_LINE:
; This inner loop indicates drawing two dots per tick.
	; get two dots contained in a byte of data from external memory.
	MOV	A,	#0	
	MOVC	A,	@A+DPTR
	MOV	CACHE,	A
	INC	DPTR
	; checks if head is on the end of the line.
	CJNE	A,	#10001000B,	DRAWIMAGE_DRAW_DOT
	
	; .if end of line ...
	; rolls back HEAD to the beginning point of current line.
	MOV	AR1,	C_HEAD
	; shifts HEAD downward
	MOV	A,	#20H
	ADD	A,	HEAD
	MOV	HEAD,	A
	ORL	C,	SCRNSELEC
	MOV	SCRNSELEC,	C
	; go to draw next line.
	DJNZ	LINELEFT,	DRAWIMAGE_LINE_ENTRY
	SJMP	DRAWIMAGE_DONE
	
DRAWIMAGE_DRAW_DOT:
; upper 4 bits precedes.
	; masks upper 4bits.
	SWAP	A
	ANL	A,	#0FH
	; if A is zero(=RGB000) and BLTOVOID is 1, skip this dot
	JNZ	DRAWIMAGE_0
	JB	BLTOVOID, 	DRAWIMAGE_1
DRAWIMAGE_0:
; draws upper 4bits
	MOV	B,	A
	MOV	C,	SCRNSELEC
	CALL	rDRAWDOTB
DRAWIMAGE_1:
; lower 4bits follows.
	; extract lower 4bits
	MOV	A,	CACHE
	ANL	A,	#0FH 
	; moves HEAD forward.
	INC	HEAD
	; if A is zero(=RGB000) and BLTOVOID is 1, skip this dot
	JNZ	DRAWIMAGE_2
	JB	BLTOVOID,	DRAWIMAGE_NEXTDOT
DRAWIMAGE_2:
; draws lower 4bits
	MOV	B,	A
	MOV	C,	SCRNSELEC
	CALL	rDRAWDOTB
	
DRAWIMAGE_NEXTDOT:
	; moves HEAD forward.
	INC	HEAD
	SJMP	DRAWIMAGE_DRAW_LINE 
DRAWIMAGE_DONE:

	; <<<<<
	POP	BR0
	POP	AR4
	POP	AR3
	POP	AR2
	POP 	ACC
	RET

; --------------------------------------------------------------------
; 
; @PROC	DRAWSTRING
;
; I  B	F0	Screen selection
; I	P2	Current back buffer
; I  	R1	[X3:Y5] Drawing HEAD
; IX	DPTR	String begin address
; I	B	Color
;
; X	A
;
; --------------------------------------------------------------------
; Draw string on given locaiton. 8 letters maximum per line.
rDRAWSTRING:
	USING 	GAMEBANK/8
IS_SPACE	SET	BR0.0
F0CACHE	SET	BR0.1
	PUSH	AR1
	PUSH	BR0
	; >>>>>
	CLR	IS_SPACE
	MOV	C,	F0
	MOV	F0CACHE,	C
DRAWSTRING_DRAW:
; Loop sustains until the HEAD found '\0' 
	; Get ASCII on pointing location
	MOV	A,	#0
	MOVC	A,	@A+DPTR
	
	; If it is space, the cursor will only shift two lines right.
	CJNE	A,	#' ',	DRAWSTRING_NOSPACE
	SETB	IS_SPACE
DRAWSTRING_NOSPACE:
	
	; If it is null letter, end drawing sequence.
	JZ	DRAWSTRING_DONE
	
	; Cache current DPTR location
	PUSH	DPH
	PUSH	DPL
	
	; Draw character on given location
	CALL	rDRAWASCII
	
	; Adjust R1 location to right.
	MOV	A,	#4
	JNB	IS_SPACE,	DRAWSTRING_STEP
	MOV	A,	#2
DRAWSTRING_STEP:
	CLR	IS_SPACE
; To jump half when it is space ...
	ADD	A,	R1
	MOV	R1,	A
		
	; Restores DPTR location
	POP	DPL
	POP	DPH
	
	; Targets next character
	INC 	DPTR
	SJMP	DRAWSTRING_DRAW
DRAWSTRING_DONE:
	; <<<<< 
	POP	BR0
	POP	AR1
	RET
; --------------------------------------------------------------------
;
; @PROC	DRAWASCII
; 
; I  B	F0	Screen seleciton
; I 	P2	Current buffer
; I  	R1	[X3:Y5] Drawing HEAD
; IX	A	Target ASCII character.
; I 	B	Color 
; 
; X  2	DPTR
; X/BANK2	R0, R2, R3, R5
;
; --------------------------------------------------------------------
; ASCII code will be drawn in X4*Y8 boundary. Its format on external
;  memory will be defined with 4 bytes of continuous data.
; A single bit indicates whether the dot should be drawn or not.
; @see file	RES.ASCII.INC
rDRAWASCII:
; ASCII drawing format is consist of continuous 3bytes.
; A byte will be seperated by 4bits, each half byte will indicate a
;  horizontal data.
; For example, a character '0' will be represented like this.
; 	. 0 0 .   }
; 	0 . . 0   } first byte
;	0 . 0 0   ] 
;	0 0 . 0   ] second byte
;	0 . . 0   )
;	. 0 0 .   ) third byte
	USING 	BANK2/8
COLOR	SET	R5  

	PUSH	PSW
	; get parameter, R1 indicates current bank's parameter,
	;  AR1 indicates temporary bank's address.
	; by this, the original input parameter R1 won't be
	;  changed as long as caller procedure does not use BANK2.
	MOV	AR1,	R1
	
	; now switch the bank.
	SETB	RS1
	SETB	RS0	
	
	; >>>>>
	; .Moves DPTR offset as three times of input ASCII.
	; to do this... 
	; cache color B
	MOV	COLOR,	B
	; subtract ' ' from input ascii
	CLR	C
	SUBB	A,	#' '
	; do A * 3, since a character occupies 3 byte offset...
	MOV	B,	#3
	MUL	AB
	; add this to DPTR
	MOV	DPTR,	#ASCII
	ADD	A,	DPL
	MOV	DPL,	A
	MOV	A,	B
	ADDC	A,	DPH
	MOV	DPH,	A
	
	; draw ASCII on given location
	MOV	B,	COLOR
	CALL	r__DRAWASCII
	
	; <<<<<
	POP	PSW
	
	; output values to allow continuous call.
	MOV	B,	AR5 	; since the address still 
				;  indicates BANK2's...
	RET
; --------------------------------------------------------------------
;
; @SPECTMP	__DRAWASCII
;
; IX	F0	Screen selection
; I	P2	Current buffer
; IX	R1	[X3:Y5] Drawing HEAD
; IX	DPTR	Target ASCII byte begin location
; IX  3:0	B	Input color
; 
; X	ACC, R7, R6, R5, R4, R3
; 
; --------------------------------------------------------------------
; Internal function which draws given single ASCII character into
;  given location. 
r__DRAWASCII:
	USING 	BANK2/8
HEAD	SET	R1
COLOR	SET	R5
COUNT	SET	R7
HEAD_C	SET	AR4
HEAD_C0	SET	R4

	; >>>>> 
	MOV	COLOR,	B	; cache input color.
	MOV	COUNT,	#3	; count of bytes.
__DRAWASCII_0:
	; caches HEAD location
	MOV	HEAD_C,	HEAD
	; extracts byte from program memory
	MOV	A,	#0
	MOVC	A,	@A+DPTR
	; to read next byte...
	INC	DPTR
	; cache
	MOV	R3,	A
; .Each bit in A indicates if a dot should be drawn on
;  given location or not.
__DRAWASCII_DRAWDOT	MACRO	BT
	LOCAL	NODRAW
	; >>>>>> MACRO BEGIN >>>>>>
	;; IF BT == 7, [MOV A, R3] IS NOT REQUIRED. 
	IFN	BT-7
	ELSE
	MOV	A,	R3
	ENDIF
	;; if current bit indicates 0, skip to next dot.
	JNB	ACC.&BT,	NODRAW
	MOV	B,	COLOR
	MOV	C,	F0
	CALL	RDRAWDOTB
NODRAW:	;; IF BT == 0 OR BT == 4, R1 SHOULD BE SHIFTED DOWNWARD. 
	IFN	BT*(BT-4)
	;; rolls back HEAD location onto leftmost point.
	;; shifts HEAD downward.
	MOV	A,	#20H
	ADD	A,	HEAD_C0
	MOV	HEAD_C0,	A
	ORL	C,	F0
	MOV	F0,	C
	MOV	HEAD,	HEAD_C
	;ELSEIFN	BT
	;;; rolls back HEAD location onto leftmost point.
	;;; shifts HEAD downward.
	;MOV	A,	#20H
	;ADD	A,	HEAD_C0
	;MOV	HEAD_C0,	A
	;ORL	C,	F0
	;MOV	F0,	C
	;MOV	HEAD,	HEAD_C
	ELSE
	INC	HEAD
	ENDIF
	; <<<<<< MACRO END   <<<<<<
	ENDM
	; draw bits.
	__DRAWASCII_DRAWDOT 7
	__DRAWASCII_DRAWDOT 6
	__DRAWASCII_DRAWDOT 5
	__DRAWASCII_DRAWDOT 4 
	__DRAWASCII_DRAWDOT 3
	__DRAWASCII_DRAWDOT 2
	__DRAWASCII_DRAWDOT 1
	__DRAWASCII_DRAWDOT 0
	DJNZ	COUNT,	__DRAWASCII_0 
	
; ~__DRAWASCII_0
	; <<<<<
	RET  
; --------------------------------------------------------------------
;
; @PROC	DRAWDOT
; $MC 14
;
; I  	DPH	BACK BUFFER 
; I 	DPL	[3(Y):5(X)]
; IX 	CY	SCREEN_SELECTION
; IX  3:0	B	COLOR. should only be lower 4bit value
;
; X  	ACC
;
; --------------------------------------------------------------------
; Draws a dot on given location.
rDRAWDOT:
	; get original value to adapt input value only into lower
	;  or higher 4 bit.
	MOVX	A,	@DPTR
	
	; if F0 is 1, then the pixel should be drawn on lower 
	;  screen, which is indicated by lower 4bit RGB value.
	JNC	DRAWDOT_LOW
	ANL	A,	#0F0H
	ADD	A,	B
	SJMP	DRAWDOT_END
DRAWDOT_LOW:
	ANL	A,	#00FH
	XCH	A,	B
	SWAP	A
	ADD	A,	B
DRAWDOT_END:
	MOVX	@DPTR,	A
	RET
; --------------------------------------------------------------------
;
; @PROC	DRAWDOTB
; $MC 14
;
; I  	P2	BACK BUFFER 
; I 	R1	[3(Y):5(X)]
; IX 	CY	SCREEN_SELECTION
; IX [3:0]	B	COLOR. should only be lower 4bit value
;
; X  	ACC
;
; --------------------------------------------------------------------
; Draws a dot on given location. Uses P2 and R1 register instead of DPTR.
rDRAWDOTB:
	; get original value to adapt input value only into lower
	;  or higher 4 bit.
	MOVX	A,	@R1
	
	; if F0 is 1, then the pixel should be drawn on lower 
	;  screen, which is indicated by lower 4bit RGB value.
	JNC	DRAWDOTB_LOW
	ANL	A,	#0F0H
	ADD	A,	B
	SJMP	DRAWDOTB_END
DRAWDOTB_LOW:
	ANL	A,	#00FH
	XCH	A,	B
	SWAP	A
	ADD	A,	B
DRAWDOTB_END:
	MOVX	@R1,	A
	RET
; --------------------------------------------------------------------
;
; @PROC	BUFFER_FRONT
; $MC 7
;
; OUT 	ACC	Returns higher byte of current front buffer's
		; address.
; 
; -------------------------------------------------------------------
; This interface returns higher byte of current graphics front buffer
;  which is mainly used on display update.	
rBUFFER_FRONT:	
	; Loads current buffer value directly onto return value
	JNB	CURBUFF,	BUFFER_FRONT_0
	MOV	A,	#HIGH(GBUFFER1)
	RET
BUFFER_FRONT_0:
	MOV	A,	#HIGH(GBUFFER2) 
	RET
; -------------------------------------------------------------------
;
; @PROC	BUFFER_BACK
; $MC 7
;
; 0 	ACC	Returns higher byte of current back buffer's
		; address.
;
; --------------------------------------------------------------------
; This interface returns higher byte of current graphics back buffer
;  which is mainly used on gameplay routine's draw call.
rBUFFER_BACK:
	; Loads flipped current buffer value onto return value.
	JNB	CURBUFF,	BUFFER_BACK_0
	MOV	A,	#HIGH(GBUFFER2)
	RET
BUFFER_BACK_0:
	MOV	A,	#HIGH(GBUFFER1) 
	RET
; --------------------------------------------------------------------
;
; @SPECINT	UPDATE_DISPLAY(void)
; $MC 275~339(OPT_SIZE)
;	
; --------------------------------------------------------------------
; This interruption subroutine displays a line onto LED dot matrix per call.
; To enable seamless and light operation, this procedure uses separated register bank.
; This procedure is called per 1840 ticks, and costs 275 ticks for
;  itself, therefore (1840 - 275) * 8 = 12520 ticks can be used for
;  gameplay sequence per frame.
iUPDATE_DISPLAY:
	USING	GPUBANK/8
DP_CLK	SET	P1.0
DP_LAT	SET	P1.1
DP_A	SET	P1.2 ; @notice. this subroutine does not use this.
DP_B	SET	P1.3
DP_C	SET	P1.4
DP_D	SET	P1.5  
DP_RGBHL	SET	R1
	; Reset timer period 
	; To keep 60Hz ... 16 ms per frame, 16ms/8 per line,
	;  ... 2ms/1.085us ticks per line ==> 1840 ticks per line.
	; Therefore the timer register value should be reset as
	;	 0xffff - 1840 = 0xf8cf
	MOV	TH0,	#0F8H
	MOV	TL0,	#0cfH
	
	PUSH	PSW
	MOV	PSW,	#GPUBANK
	
	PUSH	ACC
	PUSH	B
	PUSH	DPL
	PUSH	DPH
	; >>> >>> >>>
	
	; Targets OUTPUT port.
	MOV	DPTR,	#0C000H
	; P2 indicates back buffer.
	CALL 	rBUFFER_FRONT
	MOV	P2,	A
	
	; Clear latch
	CLR	DP_LAT
	
	; cache current R1 value to keep display line value
	MOV	AR3,	R1
	
	; Uses macro to reduce cost from jump command.
	; Since the R1 register is holding the latest reading index
	;  information and this routine uses separated register
	;  bank, any other additional process to ensure its purence
	;  doesn't required.
IF	__OPT_SIZE__
	MOV	R2,	#32
UPDATE_DISPLAY_LINE:
ELSE
	REPT	32
ENDIF
	;; read data from current address
	MOVX	A,	@R1 
	;; load data onto output RGB port
	MOVX	@DPTR,	A
	;; prepare to read next data ..
	INC	R1
	CLR	DP_CLK
	SETB	DP_CLK
IF	__OPT_SIZE__
	DJNZ	R2,	UPDATE_DISPLAY_LINE
ELSE
	ENDM
ENDIF
	
	; Select line which will be rendered
	MOV	A,	R3
	MOV	C,	ACC.7
	MOV	DP_B,	C
	MOV	C,	ACC.6
	MOV	DP_C,	C
	MOV	C,	ACC.5
	MOV	DP_D,	C
	
	; Sets latch to display a RGB line sent.
	SETB	DP_LAT
	
	; .When the register R1 is reaches 0, which can notify a
	;  frame boundary.
	; since A is holding the latest R1 value...
	JNZ	UPDATE_DISPLAY_FRAME_BOUNDARY
	CPL	FRAMEBOUND60
	JNB	FRAMEBOUND60, UPDATE_DISPLAY_FRAME_BOUNDARY
	SETB	FRAMEBOUND30
UPDATE_DISPLAY_FRAME_BOUNDARY:
	
	; <<< <<< <<<
	POP	DPH
	POP	DPL
	POP	B
	POP	ACC
	
	POP	PSW
	
	; Restore P2 value
	PEAKP2	P2
	
	;INC 	RANDNUM
	
	RETI
; -------------------------------------------------------------------- 
;; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
ENDIF
