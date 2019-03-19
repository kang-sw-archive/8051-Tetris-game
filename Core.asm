$ NOPAGING DEBUG NOTABS 
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;
; 	      ** MULTIPLE GAME PLATFORM      **
;	      ** MICROPROCESSOR I FINAL TASK **
;
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;	PROGRAM DESCRIPTION
; ...
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; 	FILE DESCRIPTION
; This file is the topmost asm file which provides definitions 
;  and locations of memory segments, logic elements, etc.
; The overall program is consist of lots of submodules defined
;  in inc files. The communication between submodules are controled
;  by this core file. 
; Since the Macro Assembler 51 doesn't provide linking through
;  multiple source files, every submodule should be wrote on inc file
;  and be included in somewhere.
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; 	RULES
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; 	SYSTEMIC MACROS
; This area defines several macros which should be included on each
;  inc files to prevent multiple inclusion.
; --------------------------------------------------------------------
;* Every single inc file have to instanciate this macro via its name.
;  These macros prevent duplicated inclusion of every single inc files
;  therefore enables to keep label definitions unique.
DEFINE_MODULE MACRO	MODULENAME
	IFN 	____&MODULENAME
____&MODULENAME 	SET	1
	ENDM 
;* This macro should be defined for each inc files.
DECLARE_MODULE MACRO	MODULENAME
;; To prevent multiple instanciation of this macro ...
____&MODULENAME&_DEF	EQU	0
____&MODULENAME	SET	0
	ENDM
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
$INCLUDE	(_MEMORY.INC)
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; 	INTERRUPT SUBROUTINES
; The gameplay update logic will be assigned on Timer0's inturrupt
;  service routine.
; The sound management logic will be assigned on Timer1's inturrupt
;  service routine.
; The graphics processing subroutine will be plaeced in main loop,
;  will operate without any idling. 
; --------------------------------------------------------------------
	ORG	4000H
	LJMP	MAIN	; long jump since will jump over
			;  huge code segment made from
			; lots of submodules.
	ORG	4003H	 
	
	ORG	400BH	
	LJMP	iUPDATE_DISPLAY
			; Timer 0 interrupt will trigger
			; the display-a-line procedure
	ORG	4013H   
	ORG	401BH 
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; --------------------------------------------------------------------
; 	MODULE DECLARATION 
; --------------------------------------------------------------------
DECLARE_MODULE 	GRAPHICS
DECLARE_MODULE 	UTILS
; --------------------------------------------------------------------
;	MODULE INCLUSION
; --------------------------------------------------------------------
$INCLUDE	(GRAPHICS.INC)
$INCLUDE 	(UTILS.INC) 
$INCLUDE	(RANKING.INC)
; --------------------------------------------------------------------
; Session inclusions should be placed below this line.
$INCLUDE 	(SSN_TITLE.INC)
$INCLUDE 	(SSN_TETRIS.INC)
$INCLUDE	(SSN_JIRUNG.INC)
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
; > > > > > > > > > > > > > > > >  > > > > > > > > > > > > > > > > > >
; > > > > > > > > > > CODE SEGMENT > > > > > > > > > > > > > > > > > >
; > > > > > > > > > >  > > > > > > > > > > > > > > > > > > > > > > > >
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; --------------------------------------------------------------------
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
; @PROC	NULLFUNC(void)
;
; --------------------------------------------------------------------
; A nullfunc can be assigned any void return type callbacks.
; This null function exists to not define any functionality on
;  specific callback argument.D
NULLF:	RET ; do nothing ... 
; --------------------------------------------------------------------
;
; @PROC	SET_TIMER
; @note	TimerCallback(void)
; 
; I 2	BR1:BR0	Frames to wait.
; I	R1:R0	Callback address
; 
; X	DPTR, ACC
;
; --------------------------------------------------------------------
; Assign a timer callback. This will iterate each timer until find empty one.
; 4 Timers are enabled.
rSET_TIMER:
	MOV	DPTR,	#TCNT

	MOV	A,	R2
	PUSH	ACC
	
	; >>> >>>
	MOV	R2,	#4
SET_TIMER_BEGIN:
; Iterate each timer count register until find one which is in idle state.
	MOVX	A,	@DPTR
	INC	DPL
	; Zero count indicates it is not initialized yet.
	JNZ	SET_TIMER_NEXT
	; The TCNT data is ensured to be aligned in 20 byte boundary,
	;  therefore it is safe to increase only DPL.
	JZ	SET_TIMER_FOUND
SET_TIMER_NEXT:
; Moves HEAD location to next address.
	INC	DPL
	DJNZ	R2, 	SET_TIMER_BEGIN
	SJMP	SET_TIMER_DONE
SET_TIMER_FOUND:
; If found, sets timer trigger time and callback.
; At this point, DPL is pointing 1 higher point than pivot.
	MOV	A,	BR1
	MOVX	@DPTR,	A
	DEC	DPL
	MOV	A,	BR0
	MOVX	@DPTR,	A
	; Can find callback external memory location by adding 8 to address.
	MOV	A,	#8
	ADD	A,	DPL
	MOV	DPL,	A
	; Assigning callbacks
	MOV	A,	R0
	MOVX	@DPTR,	A
	INC	DPL
	MOV	A,	R1
	MOVX	@DPTR,	A
	; Done.
	; <<< <<<
SET_TIMER_DONE:
	POP	ACC
	MOV	R2,	A
	RET 
; --------------------------------------------------------------------
;
; @PROC	TRANSITION
;
; IX B	CY	Set 1 if all gameplay timers are should be reset
; IX 2	DPH:DPL	Input procedure callback to assign at next session
		; @CALBAK 	InputProcCallback
		;	I R0 	KeysDown
		;	I R1	KeysUp
		;	I R2	KeysHolding	
; IX 2	R1:R0	Update procedure callback to assign at next session
		; @CALBAK 	UpdateCallback(void)
; IX 2	R3:R2	Callback which handles all dynamic draw call.
		; @CALBAK 	DynamicDrawCallback 
; IX 2	R5:R4	Initializer callback
		; @CALBAK	SessionInitCallback
; IX 2	R7:R6	Static object draw callback
		; @CALBAK	StaticDrawCallback 
;
; --------------------------------------------------------------------
; Transition between sessions...
rTRANSITION: 
	; Stop rendering while transition
	CLR	TR0
	; Decide whether game timer should be reinitialized
	JNC	TRANSITION_ASSIGNMENT
	
	PUSH 	B
	PUSH	DPH
	PUSH	DPL
	
	; [] Resets timer
	MOV	DPTR,	#TCNT
	MOV	B, 	#8
	
	; timer count zero indicates it is deactivated now.
TRANSITION_0:
	MOV	A,	#0
	MOVX	@DPTR,	A
	INC	DPL
	DJNZ	B,	TRANSITION_0
	
	POP	DPL
	POP	DPH
	POP 	B
	
TRANSITION_ASSIGNMENT:
	; Assign global callbacks
	MOV 	CS_INPUTPR,		DPL
	MOV 	CS_INPUTPR+1,	DPH
	MOV	CS_UPDATE,		R0
	MOV	CS_UPDATE+1,	R1
	MOV	CS_DRAW,		R2
	MOV	CS_DRAW+1,		R3
	
	; Before call initializer function, cache input parameter
	PUSH	ACC
	MOV	A,	R7
	PUSH	ACC
	MOV	A,	R6
	PUSH	ACC
	
	; Call initializer dynamically 
	MOV	DPL,	R4
	MOV	DPH,	R5
	DCALL	DPTR
	
	; Values which were pushed, and are static draw function. 
	POP	DPL
	POP	DPH
	PUSH	DPH
	PUSH	DPL
	DCALL	DPTR
	
	; Draw on both side of the buffer.
	CPL	CURBUFF
	POP	DPL
	POP	DPH
	DCALL	DPTR
	
	; Resotre acc value.
	POP 	ACC
	
	; Restart rendering
	SETB	TR0
	RET
; -------------------------------------------------------------------- 
TO_MONITOR:	MOV	PSW,	#0
	MOV	SP,	#5FH
	MOV	R0,	#58H
	CLR	TR0
		
	JMP	0057H
; --------------------------------------------------------------------
;
; #### 	MAIN ROUTINE
; 
; The main loop of this program will initiate overall program
;  instance. 
; 1. Will initialize game timer
; 2. Will initialize sound organizer
; 3. Will initialize graphics processor
; 4... Will eternally repeat graphics processing
; --------------------------------------------------------------------
MAIN:	MOV	SP,	#__INT_OFST
	MOV	P1,	#0FFH
	MOV	PSW,	#GAMEBANK
	USING 	GAMEBANK/8
	
	; Clear global memory
	MOV	DPTR,	#1000H
	LDCDW	R1,R0,	1000H
	MOV	B,	#0
	CALL	rMEMSET 
	
	; *** INITIALIZE GRAPHICS PROCDURE
	MOV	R0,	#GPUBANK
	MOV	R1,	#8
MAIN_GPINIT:
	MOV	@R0,	#0
	INC	R0
	DJNZ	R1,	MAIN_GPINIT
	
	; *** CLEANING BUFFERS
	MOV	A,	#0
	MOV	DPTR,	#RFILLBUFFER
	DCALL	DPTR
	CPL	CURBUFF
	MOV	DPTR,	#RFILLBUFFER
	DCALL	DPTR 
	
	; *** INITIALIZING HARDWARE TIMER & GAME TIMER
	; Initializing timer 0 for core gameplay routine ...
	SETB	ET0
	SETB	TR0
	; Initializes timer 1 to generate random numbers...
	SETB	TR1
	MOV	TH1,	#99
	
	; Reset all gameplay timers.
	MOV	DPTR,	#TCNT
	MOV	R2,	#8
	
	; Initializing timer 1 which takes charge of controlling
	; sound system.
	; SETB	ET1
	
	; Timer 0 will run as mod 1, Timer 1 will run as mod 2.
	MOV	TMOD,	#00100001B
	SETB	EA
	
MAIN_TIMER:	MOV	A,	#0
	MOVX	@DPTR,	A
	INC	DPL 
	DJNZ	R2,	MAIN_TIMER 
	
	; *** ENTERING ENTRY SESSION ...
	CALL	rTITLE_SESSION
	  
	; Function test.
	$include	(test\function.inc)	
	
	; This is main grahpics processing loop.
	; It will operate without any delay or idling. 
	
	; *** MAIN LOOP
GP_MAIN:	; .The role of this processing loop is simple: Runs gameloop.
GP_WAIT:	JNB	FRAMEBOUND30, GP_WAIT; Wait for 33 ms...
	CLR	FRAMEBOUND30
	
	LCALL 	rGAMELOOP 
	; swaps front and back buffer.
	CPL	CURBUFF
	SJMP 	GP_MAIN	

;   the prgoram will not end until you power down the system... 
; --------------------------------------------------------------------
;	GAMEPLAY ROUTINE
; --------------------------------------------------------------------
rGAMELOOP:	MOV	PSW,	#GAMEBANK
	USING	GAMEBANK/8
	
	; # timer management 
	CALL	rUPDATE_TIMER

	; # sound management
	
	
	; # input procedure 		CONTROLLER
	CALL 	rPREPROCESS_INPUT
	MOV	DPL,	CS_INPUTPR
	MOV	DPH,	CS_INPUTPR + 1
	DCALL	DPTR
	
	; # update gameplay		MODEL
	MOV	DPL,	CS_UPDATE
	MOV	DPH,	CS_UPDATE + 1
	DCALL	DPTR

	; # draw call 		VIEW 
	MOV	DPL,	CS_DRAW
	MOV	DPH,	CS_DRAW + 1
	DCALL 	DPTR
	
	RET
; --------------------------------------------------------------------
;
; @SPECMAIN	UPDATE_TIMER(VOID)
;
; --------------------------------------------------------------------
; This specified procedure iterates each timer instances and update it all.
rUPDATE_TIMER:
	USING	GAMEBANK/8
	; Short timer location
	LDCDWP2	R1,	TCNT
	
	; Of number of timers...
	MOV	R2,	#4
UPDATE_TIMER_NEXT:
; Visit each short timers and decrease items which are not zero.
	MOVX	A,	@R1
	INC	R1
	; Zero value indicates it is not a running timer.
	JNZ	UPDATE_TIMER_UPDATE
	MOVX	A,	@R1
	JNZ	UPDATE_TIMER_UPDATE
	; Find next timer
UPDATE_TIMER_TO_NEXT:
	INC	R1
	DJNZ	R2,	UPDATE_TIMER_NEXT
	RET
UPDATE_TIMER_UPDATE:
	; Decrease double byte value and store it.
	DEC	R1

	; Decreases lower value	
	MOVX	A,	@R1
	DEC	A
	MOVX	@R1,	A
	INC	R1
	JNZ	UPDATE_TIMER_TO_NEXT
	; If A has switched into zero...
	; Higher value
	MOVX	A,	@R1
	JZ	UPDATE_TIMER_RUN
	DEC	A
	MOVX	@R1,	A
	DEC	R1
	MOV	A,	#0FFH
	MOVX	@R1,	A
	INC	R1
	SJMP	UPDATE_TIMER_TO_NEXT
	
UPDATE_TIMER_RUN:
	DEC	R1
	; @CALL CALLBACK
	; Cache locals
	PUSH	AR1
	PUSH	AR2
	PUSHP2 
	; Adds offset to find callback location.
	MOV	A,	#8
	ADD	A,	R1
	MOV	R1,	A
	; Load callback funciton
	MOVX	A,	@R1
	MOV	DPL,	A
	INC	R1
	MOVX	A,	@R1
	MOV	DPH,	A
	; Call timer callback
	DCALL	DPTR
	; Resotre values
	POPP2
	POP	AR2
	POP	AR1
	SJMP	UPDATE_TIMER_TO_NEXT 
; --------------------------------------------------------------------
;
; @SPECMAIN	PREPROCESS_INPUT
; OUT	R0	BUTTONDN
; OUT	R1	BUTTONUP
; OUT	R2	BUTTONHELD
;
; --------------------------------------------------------------------
rPREPROCESS_INPUT:
; @Input data of previous frame is stored in PREV_INPUT.
	USING	GAMEBANK/8

; R3 = CACHE INPUT
; R4 = CACHE DELTA VALUE
	
;	; If home button pressed, call title session
	MOV	DPTR,	#INPUTPORT
;	MOVX	A,	@DPTR
;	JB	ACC.0, 	PREPROCESS_INPUT_MAIN
;	CALL 	rTITLE_SESSION
;	RET
;PREPROCESS_INPUT_MAIN:	
; Reads current input from INPUT PORT, and caches it into R3
	MOVX	A,	@DPTR
	MOV	C,	ACC.0
	MOV	P1.7,	C
	; Input button will be pulled-up. Changes this via invert.
	CPL	A
	MOV	R3,	A
	
	; @Compares previous input state with current input state,
	;  then assembles outputs
	; Finds newly pushed buttons. R4 indicates delta bits
	;  between previous and current input.
	MOV	R4,	PREV_INPUT
	XRL	AR4,	A
	
	; Newly pushed buttons can be found by AND operation
	;  between DELTA value and present input.
	ANL	A,	R4
	MOV	R0,	A
	
	; Finds newly released buttons by AND operation
	;  between DELTA value and previous input.
	MOV	A,	PREV_INPUT
	ANL	A,	R4
	MOV	R1,	A
	
	; Finds held buttons by AND operation between previous
	;  and current input value.
	MOV	A,	R3
	ANL	A,	PREV_INPUT
	MOV	R2,	A
	
	; Assign PREV_INPUT
	MOV	PREV_INPUT,	R3
	
	; If home button downed, go to ranking or title.
	; @todo
	MOV	B,	R0
	JB	B.0,	PREPROCESS_INPUT_HOME
	RET
PREPROCESS_INPUT_HOME:
	CALL 	rTITLE_SESSION
	RET
; --------------------------------------------------------------------
; RESOURCE DATA
; --------------------------------------------------------------------
$include	(res.image.inc)
$include	(res.ascii.inc)
; --------------------------------------------------------------------
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


END