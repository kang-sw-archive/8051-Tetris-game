
	; Assumed if input number of people is stored in R0
	MOV	A,	R0
	MOV	DPTR,	#STRING0
	
	MOV	B,	#20
	MUL	AB
	
	ADD	A,	DPL
	MOV	DPL,	A
	MOV	A,	DPH
	ADDC	A,	#0
	MOV	DPH,	A
	
	; Now here, DPTR stores begin location of string.


	JMP 	0057H
		
STRING0:	DB	'NO PERSON', 0

	ORG	STRING0 + 20
STRING1:	DB	'ONE PERSON', 0

	ORG	STRING1 + 20
STRING2:	DB	'TWO PEOPLE', 0
	
	ORG	STRING2 + 20
STRING3:	DB	'THREE PEOPLE', 0

END