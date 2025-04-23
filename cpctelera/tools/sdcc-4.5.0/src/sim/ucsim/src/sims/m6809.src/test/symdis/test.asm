	cpu 6809

init:	jmp main

main:	lda #0
	beq forever
	lda #1
forever:
	bra forever
