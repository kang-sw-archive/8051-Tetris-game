
ASCII: 
; SP, !, ", #, $, %, &, '
DB	0, 0, 0	  	; SPACE
DB	66h, 66h, 06h	; !
DB	55h, 55h, 0A0H	; "
DB	0, 0, 0		; #
DB	0, 0, 0		; $
DB	0, 0, 0		; %
DB 	0, 0, 0		; &
DB	44H, 80H, 0		; '
; (, ), *, +, `, -, ., /
DB	42H, 22H, 24H	; (
DB	24H, 44H, 42H	; )
DB	00H, 0A4H, 0A0H	; *
DB	00H, 4EH, 40H	; +
DB	0, 0, 0		; `
DB	00H, 0EH, 00H	; -
DB 	00H, 00H, 0CCH	; .
DB	12H, 24H, 48H	; /
ASCII_NUM: ; where the letter offset begins
; 0 1 2 3 4 5 6 7 8 9 :

; . 0 0 .   >--* MSB
; 0 . . 0   *-->  LSB
; 0 . 0 0       
; 0 0 . 0      
; 0 . . 0   
; . 0 0 .           
DB	069H, 0BDH, 96H	; 0     

; . 1 1 .   >--* MSB
; 1 . 1 .   *-->  LSB
; . . 1 .       
; . . 1 .      
; . . 1 .   
; . 1 1 1           
DB	6AH, 22H, 2FH
	
; . 2 2 .   >--* MSB
; 2 . . 2   *-->  LSB
; . . . 2       
; . . 2 .      
; . 2 . .   
; 2 2 2 2          
DB	69H, 12H, 4FH

; . 3 3 .   >--* MSB
; 3 . . 3   *-->  LSB
; . . 3 .       
; . . . 3      
; 3 . . 3   
; . 3 3 .          
DB	69H, 21H, 96H

; . . 4 .   >--* MSB
; . 4 4 .   *-->  LSB
; 4 . 4 .       
; 4 4 4 4      
; . . 4 .   
; . . 4 .          
DB	26H, 0AFH, 22H
	
; 5 5 5 5   >--* MSB
; 5 . . .   *-->  LSB
; 5 5 5 .       
; . . . 5      
; . . . 5   
; 5 5 5 .    
DB	0F8H, 0E1H, 1EH

; . 6 . .   >--* MSB
; 6 . . .   *-->  LSB
; 6 6 6 .       
; 6 . . 6      
; 6 . . 6   
; . 6 6 .   
DB	48H, 0E9H, 96H

; 7 7 7 7   >--* MSB
; 7 . . 7   *-->  LSB
; . . 7 .       
; . . 7 .      
; . 7 . .   
; . 7 . .   
DB	0F9H, 22H, 44H 

; . 8 8 .   >--* MSB
; 8 . . 8   *-->  LSB
; . 8 8 .       
; 8 . . 8      
; 8 . . 8   
; . 8 8 .   
DB	69H, 69H, 96H 

; . 9 9 .   >--* MSB
; 9 . . 9   *-->  LSB
; . 9 9 9       
; . . . 9      
; . . 9 .   
; . 9 . .   
DB	69H, 71H, 24H 

; . . . .   >--* MSB
; . 0 . .   *-->  LSB
; . 0 . .       
; . . . .      
; . 0 . .   
; . 0 . .   
DB	04H, 40H, 44H
; ; < = > ? @ A B C D E 
; F G H I J K L M N O P
; Q R S T U V W X Y Z [
DS 	12

; . ? ? .   >--* MSB
; ? . . ?   *-->  LSB
; . . ? .       
; . ? . .      
; . . . .   
; . ? . .   
DB	69H, 24H, 04H
DS	3

; . A A .   >--* MSB
; A . . A   *-->  LSB
; A . . A       
; A A A A      
; A . . A   
; A . . A   
DB	01101001B,10011111B,10011001B

; B B B .   >--* MSB
; B . . B   *-->  LSB
; B B B .       
; B . . B      
; B . . B   
; B B B .   
DB	11101001B, 11101001B, 10011110B

; . C C C   >--* MSB
; C . . .   *-->  LSB
; C . . .       
; C . . .      
; C . . .   
; . C C C   
DB	01111000B, 10001000B, 10000111B
 
; D D D .   >--* MSB
; D . . D   *-->  LSB
; D . . D       
; D . . D      
; D . . D   
; D D D .   
DB	11101001B, 10011001B, 10011110B
 
; E E E E   >--* MSB
; E . . .   *-->  LSB
; E E E .       
; E . . .      
; E . . .   
; E E E E   
DB	11111000B, 11101000B, 10001111B

 
; F F F F   >--* MSB
; F . . .   *-->  LSB
; F F F .       
; F . . .      
; F . . .   
; F . . .   
DB	11101000B, 11101000B, 10001000B

; . G G .   >--* MSB
; G . . G   *-->  LSB
; G . . .       
; G . G G      
; G . . G   
; . G G .   
DB	69H, 10001011B, 96H 
 
; H . . H   >--* MSB
; H . . H   *-->  LSB
; H H H H       
; H . . H      
; H . . H   
; H . . H   
DB	99H, 11111001B, 99H
 
; I I I .   >--* MSB
; . I . .   *-->  LSB
; . I . .       
; . I . .      
; . I . .   
; I I I .   
DB	11100100B, 	44H, 	4EH
 
; J J J J   >--* MSB
; . . J .   *-->  LSB
; . . J .       
; . . J .      
; J . J .   
; J J . .   
DB	11110010B, 	22H, 	10101100B 
 
; K . . K   >--* MSB
; K . K .   *-->  LSB
; K K . .       
; K . K .      
; K . . K   
; K . . K   
DB	10011010B, 	11001010B, 	10011001B
 
; L . . .   >--* MSB
; L . . .   *-->  LSB
; L . . .       
; L . . .      
; L . . .   
; L L L L   
DB	88H,	88H, 	8FH
 
; M M M M   >--* MSB
; M M . M   *-->  LSB
; M M . M       
; M M . M      
; M M . M   
; M M . M   
DB	0FDH,	0DDH,	0DDH
 
 
; N . . N   >--* MSB
; N N . N   *-->  LSB
; N . N N       
; N . . N      
; N . . N   
; N . . N   
DB	9DH,	0B9H,	99H
 
 
; . O O .   >--* MSB
; O . . O   *-->  LSB
; O . . O       
; O . . O      
; O . . O   
; . O O .   
DB	69H,	99H,	96H
 
; P P P .   >--* MSB
; P . . P   *-->  LSB
; P . . P       
; P P P .      
; P . . .   
; P . . .   
DB	0E9H,	9EH,	88H
 
; . Q Q .   >--* MSB
; Q . . Q   *-->  LSB
; Q . . Q       
; Q . . Q      
; . Q Q .   
; . . . Q   
DB	69H,	99H,	61H
 
; R R R .   >--* MSB
; R . . R   *-->  LSB
; R R R .       
; R . . R      
; R . . R   
; R . . R   
DB	0E9H,	0E9H,	99H

; . S S .   >--* MSB
; S . . S   *-->  LSB
; . S . .       
; . . S S      
; S . . S   
; . S S .   
DB	69H,	43H,	96H 
 
; T T T .   >--* MSB
; . T . .   *-->  LSB
; . T . .       
; . T . .      
; . T . .   
; . T . .   
DB	11100100B,	01000100B,	01000100B
 
; U . . U   >--* MSB
; U . . U   *-->  LSB
; U . . U       
; U . . U      
; U . . U   
; . U U .   
DB	99H,	99H,	96H
 
; . . . V   >--* MSB
; V . . V   *-->  LSB
; V . . V       
; . V . V      
; . . V V   
; . . . V   
DB	19H,	95H,	31H 
 
; W . . W   >--* MSB
; W W . W   *-->  LSB
; W W . W       
; W W . W      
; W W . W   
; W W W .   
 DB	9DH,	0DDH,	0DEH
 
; X . . X   >--* MSB
; X . . X   *-->  LSB
; . X X .       
; . X X .      
; X . . X   
; X . . X   
DB	99H,	66H,	099H
 
; Y . . Y   >--* MSB
; Y . Y .   *-->  LSB
; . Y . .       
; . Y . .      
; . Y . .   
; . Y . .   
DB	9AH,	44H,	44H
 
; Z Z Z Z   >--* MSB
; . . . Z   *-->  LSB
; Z . Z .       
; . Z Z Z      
; Z . . .   
; Z Z Z Z   
DB	0F1H,	0A7H,	8FH

; [ [ . .   ---
; [ . . .   
; [ . . .   
; [ . . .   
; [ . . .   
; [ [ . .   
; \ ] ^ _ ` a b c d e f
; g h i j k l m n o p q
; r s t u v w x y z
DS	18 
 
; . . . .   ---
; . a a .   
; . . . a   
; . a a a   
; a . . a   
; . a a a   
 
; . . . .   ---
; b . . .   
; b . . .   
; b b b .   
; b   . b   
; . b b .   
 
; . . . .   ---
; . . . .   
; . c c .   
; c . . .   
; c . . .   
; . c c .   
 
; . . . .   ---
; . . . d   
; . . . d   
; . d d d   
; d . . d   
; . d d .   
 
; . . . .   ---
; . e e .   
; e . . e   
; e e e .   
; e . . .   
; . e e .   
  
; . . f f   ---
; . f . .   
; f f f f   
; . f . .   
; . f . .   
; . f . .   
 
; . . . .   ---
; . g g .   
; g . . g   
; . g g g   
; . . . g   
; . g g .   
 
; . . . . . ---
; h       . 
; h       . 
; h       . 
; h h h   . 
; h     h . 
; h     h . 
; . . . . .
; . . . . . ---
;         . 
;   i i   . 
;         . 
;   i i   . 
;   i     . 
;   i     . 
; . . . . .
; . . . . . ---
;   j j   . 
;         . 
;   j j   . 
;     j   . 
; j   j   . 
;   j j   . 
; . . . . .
; . . . . . ---
;         . 
; k       . 
; k   k   . 
; k k     . 
; k   k   . 
; k     k . 
; . . . . .
; . . . . . ---
;     l   . 
;   l     . 
;   l     . 
;   l     . 
;   l     . 
; l l     . 
; . . . . .
; . . . . . ---
;         . 
;         . 
;         . 
; m m m   . 
; m m   m . 
; m m   m . 
; . . . . .
; . . . . . ---
;         . 
;         . 
;         . 
; n n     . 
; n   n   . 
; n   n   . 
; . . . . .
; . . . . . ---
;         . 
;         . 
;   o o   . 
; o     o . 
; o     o . 
;   o o   . 
; . . . . .
; . . . . . ---
;         . 
;   p p   . 
; p     p . 
; p p p   . 
; p       . 
; p       . 
; . . . . .
; . . . . . ---
;         . 
;   q q   . 
; q   q   . 
; q q q   . 
;     q   . 
;       q . 
; . . . . .
; . . . . . ---
;         . 
;         . 
; r   r r . 
; r r     . 
; r       . 
; r       . 
; . . . . .
; . . . . . ---
;         . 
;   s s s . 
; s       . 
;   s s   . 
;       s . 
; s s s   . 
; . . . . .
; . . . . . ---
;         . 
;         . 
;   t     . 
; t t t   . 
;   t     . 
;   t t   . 
; . . . . .
; . . . . . ---
;         . 
;         . 
;         . 
; u   u   . 
; u   u   . 
;   u u u . 
; . . . . .
; . . . . . ---
;         . 
;         . 
;         . 
; v   v   . 
; v   v   . 
;   v     . 
; . . . . .
; . . . . . ---
;         . 
;         . 
;         . 
; w w   w . 
; w w   w . 
; w w w   . 
; . . . . .
; . . . . . ---
;         . 
;         . 
; x   x   . 
; x   x   . 
;   x     . 
; x   x   . 
; . . . . .
; . . . . . ---
;         . 
;         . 
; y   y   . 
; y   y   . 
;   y     . 
;     y   . 
; . . . . .
; . . . . . ---
;         . 
;         . 
; z z z   . 
;     z   . 
;   z     . 
; z z z   . 
; . . . . .