	.title	Test of TLCS-90 assembler
	; see http://www.bitsavers.org/components/toshiba/_dataBook/1988_Toshiba_TLCS-48_90_8_Bit_Microcontroller.pdf

	offset	=	0x11		;arbitrary constants
	n	=	0x20
	nn	=	0x0584
	stack	=	0x0FFA0
	stack8	=	0xA0

	;***********************************************************
	; 4T (just loads for initialization)
	ld	a, #n			; 36 20
	ld	d, #n			; 32 20
	ld	e, d			; FA 33
	ld	hl, de			; 41
	ld	bc, hl			; 48

	;***********************************************************
	; 2T
	nop				; 00
	ld	b, a			; 28
	ld	a, b			; 20
	inc	a			; 86
	dec	e			; 8B
	rlca				; A0
	rrca				; A1
	rla				; A2
	rra				; A3
	;slaa				; A4
	;sraa				; A5
	;slla				; A6
	;srla				; A7
	cpl				; 10
	neg				; 11
	ex	de, hl			; 08
	ex	af, af			; 09
	exx				; 0A

	;***********************************************************
	; 4T
	add	a, b			; F8 60
	add	a, #n			; 68 20
	adc	a, l			; FD 61
	sub	a, h			; FC 62
	sub	l, #n			; FD 6A 20
	xor	a, a			; FE 65
	rlc	b			; F8 A0
	set	2, b			; F8 BA
	sla	a			; FE A4
	sra	b			; F8 A5
	sll	c			; F9 A6
	srl	d			; Fa A7

	;***********************************************************
	; 6T
	ld	sp, #stack		; 3E A0 FF
	ld	ix, sp			; FE 3C
	ld	(sp), b			; EE 20
	ld	c, (sp)			; E6 29
	add	a, (sp)			; E6 60
	;xor	b, b			; illegal
	xor	a, b			; F8 65 ; as 4T
	bit	3, (sp)			; E6 AB

	;***********************************************************
	; 8T (b 0x5e)
	ld	de, (sp)		; E6 49 ; as 6T
	push	hl			; 52
	ld	c, (stack)		; E7 A0 29
	add	a, (stack)		; 60 A0 ; as 4T
	rrc	(sp)			; E6 A1 ; as 6T
	tset	4, b			; F8 1C
	xor	hl, hl			; FA 75

	;***********************************************************
	; 10T
	pop	hl			; 5A
	ld	offset(sp), e		; F6 11 23
	ld	d, offset(sp)		; F2 11 2A ; as 8T
	add	a, offset(sp)		; F2 11 60 ; as 8T
	;rr	(stack)			; E7 A0 A3
	res	5, (sp)			; E6 B5 ; as 6T

	;***********************************************************
	; 12T (b 0x78)
	and	(stack), #n		; EF A0 6C 20 ; as 6T
	inc	1(sp)			; F2 01 87 ; as 8T
	cp	1(sp), #n		; F6 01 6F 20 ; as 10T

	;***********************************************************
	; 14T
	or	1(sp), #n		; F6 01 6E 20 ; as 10T

	;***********************************************************
	; 16T
	mul	hl, #3			; 12 03 ; as 4T

	;***********************************************************
	; 18T (b 0x89)
	div	hl, b			; F8 13 ; as 4T

	;***********************************************************
	; 20T
	mul	hl, (stack)		; E7 A0 12 ; as 6T
	;mul	hl, (stack8)		; E7 A0 12

	;***********************************************************
	; 22T
	div	hl, offset(sp)		; F2 11 13 ; as 8T

	;***********************************************************
	; just some setup
	sub	a, a			; FE 62
	ld	hl, #stack + 2		; 3A A2 FF

	;***********************************************************
	; 26T
	;div	hl, (hl+a)		; EF 13

	;***********************************************************
	; jumps
	;xor	c, c			; illegal
	and	c, #0			; F9 6C 00 ; as 4T (not 6T)
	call	fun2			; 1Cr85r00
	ld	bc, #fun2		; 38r85r00
	;call	NC, bc			; E8 DF
	;jp	C, bc			; E8 E7
	call	NZ, fun1		; EBr83r00 DE
	call	Z, fun1			; EBr83r00 D6
	jr	NE, end			; CE 05
	jr	EQ, end			; C6 03
fun1:
	ret	NZ			; FE DE
	ret	Z			; FE D6
fun2:
	ret				; 1E
end:
	jp	end			; 1Ar86r00
