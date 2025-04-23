	cpu XAG3

init:	jmp main

main:	;movs r0,#0
	nop
	beq forever
	;movs r0,#1
	nop
forever:
	br forever
