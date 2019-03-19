; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DEFINE_MODULE	UTILS ; Change this as you want.
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;
;	TemplateModule
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
; #MACRO	STRING	LITERAL
; --------------------------------------------------------------------
STRING	MACRO	LBL, LTRL
	LOCAL	ECC
	SJMP	ECC
LBL:	DB	LTRL, 0
ECC:
ENDM
; --------------------------------------------------------------------
; #MACRO	NEG	A
; --------------------------------------------------------------------
; Negates A
NEG	MACRO	_A
	CPL	_A
	INC	_A
ENDM
; --------------------------------------------------------------------
; --------------------------------------------------------------------
; #MACRO	MOVP2	VAL
; --------------------------------------------------------------------
MOVP2	MACRO	VAL
	MOV	GLB_P2CACHE,VAL
	MOV	P2,	VAL
	ENDM
; --------------------------------------------------------------------
; #MACRO	LDP2CDW	P2, RN, CONSTDW
; --------------------------------------------------------------------
LDCDWP2	MACRO	_RN, CONSTDW
	MOVP2		#HIGH(CONSTDW)
	MOV	_RN,	#LOW(CONSTDW)
	ENDM
; --------------------------------------------------------------------
; #MACRO	LOADCDW	HI, LO, CONSTDW
; --------------------------------------------------------------------
LDCDW	MACRO	HI, LO, CONSTDW
	MOV	HI,	#HIGH(CONSTDW)
	MOV	LO,	#LOW(CONSTDW)
	ENDM
; --------------------------------------------------------------------
; #MACRO	PEAKP2	TGET
; --------------------------------------------------------------------
PEAKP2	MACRO	TGET
	MOV	TGET,	GLB_P2CACHE
	ENDM
; --------------------------------------------------------------------
; #MACRO	PUSHP2
; --------------------------------------------------------------------
PUSHP2	MACRO
	PUSH	GLB_P2CACHE
	ENDM 
; --------------------------------------------------------------------
; #MACRO	POPP2
; --------------------------------------------------------------------
POPP2	MACRO
	POP	GLB_P2CACHE
	PEAKP2	P2
	ENDM
; --------------------------------------------------------------------
; #MACRO 	DCALL	IDPTR
;	X A, DPTR
; --------------------------------------------------------------------
; Calls function dynamically on dptr
DCALL	MACRO	IDPTR
	LOCAL	BACK
	MOV	A,	#LOW(BACK)
	PUSH	ACC
	MOV	A,	#HIGH(BACK)
	PUSH	ACC
	MOV	A,	#0
	JMP	@A+IDPTR
BACK:	
	ENDM
; --------------------------------------------------------------------
; #MACRO	MOVW	OP1(W), OP2(DW)
; --------------------------------------------------------------------
; Moves a word from op2 to op1. Operand should be directly addressed.
MOVW	MACRO	OP1, OP2
	MOV	OP1, 	OP2
	MOV	OP1+1,	OP2+1	
	ENDM
; --------------------------------------------------------------------
;
; @PROC	ADDSEP
;	
; I	R1	OPERAND 1
; I	R2	OPERAND 2
; OUT	A	RESULT
;
; X	B
;
; --------------------------------------------------------------------	
; Performs signed addition operation, seprately upper and lower 4bits
rADDSEP:	MOV	A,	R1
	ADD	A,	R2
	PUSH	ACC
	MOV	A,	R1
	ANL	A,	#0F0H
	ADD	A,	R2
	ANL	A,	#0F0H
	MOV	B,	A
	POP	ACC
	ANL	A,	#00FH
	ADD	A,	B
	RET
; --------------------------------------------------------------------	
;
; @PROC	MEMSET
;
; I	B	MEM TO SET
; IX	DPTR	BEGIN
; IX	R1:R0	SIZE TO CLEAR
;
; X	A
;
; --------------------------------------------------------------------	
rMEMSET:	; Set memory
	MOV	A,	B
	MOVX	@DPTR,	A
	INC	DPTR
	; To next index
	MOV	A,	R0
	CLR	C
	SUBB	A,	#1
	MOV	R0,	A
	MOV	A,	R1
	SUBB	A,	#0
	MOV	R1,	A
	; Check if done
	JNZ	rMEMSET
	MOV	A,	R0
	JNZ	rMEMSET
	; If input size has turned to zero, return.
	RET	
; --------------------------------------------------------------------	
;
; @PROC	DTOA
;
; I	B	Input BCD
; I	DPTR	String location
; OUT	DPTR	New string location, '\0' stored.
; 
; X 	ACC
;
; -------------------------------------------------------------------	
; Appends decimal to ascii result into given string location.
; Behavior is undefined if given B value is not a BCD number.
rDTOA:	MOV	A,	B
	; First, masks upper 4bit and draw it to lower place.
	ANL	A,	#0F0H
	SWAP	A
	; Then makes it a ASCII code
	ADD	A,	#'0'
	; Store it
	MOVX	@DPTR,	A
	; Repeat this to lower 4bits
	INC	DPTR
	MOV	A,	B
	ANL	A,	#0FH
	ADD	A,	#'0'
	MOVX	@DPTR,	A
	; Append null character at the end of the string.
	INC	DPTR
	MOV	A,	#0
	MOVX	@DPTR,	A
	RET 	 
; --------------------------------------------------------------------	
;
; @PROC	ADDBLKSEP
;
; IX	A	OP1
; I 	R2	OP2
; OUT 	A 	RESULT
;
; X 	B
;
; --------------------------------------------------------------------
; Adds two signed number, which are the 4bit-4bit separated format.
rADDBLKSEP:	
	PUSH	ACC
	; Adds upper 4bit first.
	ANL	A,	#0F0H
	ADD	A,	R2
	ANL	A,	#0F0H
	; Adds lower 4bit
	MOV	B,	A
	POP	ACC
	ADD	A,	R2
	ANL	A,	#00FH
	; Adds each value other.
	ADD	A,	B
	RET
; --------------------------------------------------------------------	
;
; @PROC	DELAY
;
; I 	A	Milliseconds to delay
; XX 	B
;
; --------------------------------------------------------------------	
rDELAY:	MOV	B,	#230
DELAY_0:	NOP
	NOP
	DJNZ	B,	DELAY_0
	DJNZ	ACC,	rDELAY
	RET  
; --------------------------------------------------------------------
;
; @PROC	ITOA
;
; -------------------------------------------------------------------- 

; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ENDIF
