; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DEFINE_MODULE	TEMPLATE??? ; Change this as you want.
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
;
; @PROC 	SAMPLE
;
; I 	ACC	INPUT PARAM 1 DESC
; IX	20H	INPUT PARAM 2 DESC
; I  B	F0	INPUT BIT
;
; OUT 3:0	ACC	OUTPUT PARAM 1 DESC, LOWER 4BITS ONLY
; OUT 	B	OUTPUT PARAM 2 DESC
;
; X	R1	INVALIDATES R1
;
; --------------------------------------------------------------------
rSAMPLE:	RET
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ENDIF
