	cpu ST7

init:	jp main

main:	ld a,#0
	jreq forever
	ld a,#1
forever:
	jra forever
