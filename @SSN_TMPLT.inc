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
rSESSION_NAME:
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
