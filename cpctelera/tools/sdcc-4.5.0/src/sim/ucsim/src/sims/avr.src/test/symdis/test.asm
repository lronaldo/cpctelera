	cpu ATMEGA8

init:	call main

main:	clr r0
	rjmp forever
	inc r0
forever:
	rjmp forever
