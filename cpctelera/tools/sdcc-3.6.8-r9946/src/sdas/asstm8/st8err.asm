	.title	stm8  Assembly Errors Test

	.area	Prog(rel,con)


	abyt	=	0x0110		; Absolute 1-Byte Value
	awrd	=	0x015432	; Absolute 2-Byte Value
	aexa	=	0x01BA9876	; Absolute 3-Byte Value

	rbyt	=	. + 0x0010	; Relocatable 1-Byte Value
	rwrd	=	. + 0x5432	; Relocatable 2-Byte Value
	rexa	=	. + 0xBA9876	; Relocatable 3-Byte Value


	.page
	.sbttl	Base STM8 Instructions in Numerical Order (Absolute)

	; The 1-Byte abyt value has been changed to a 2-Byte value.
	; Those instructions not supporting a 2-Byte mode will
	; report an 'a' error except most immediate mode instructions
	; which simply use the lower 1-Byte value.
	;
	; The 2-Byte awrd value has been changed to a 3-Byte value.
	; All modes with awrd will report 'a' errors except the
	; immediate mode which simply uses the lower 2-Byte value.
	;
	; All 3-Byte instructions simply use the lower 3-Byte value.
	;
	;
	; Note:
	;	This file's comments are formatted so that
	;	the expected assembler errors can be seen
	;	and the generated code can be compared to
	;	the expected code using the asxscn program.
	;

agrp0:
	neg	(abyt,sp)	;a ; 00 10
	rrwa	x,a		;  ; 01
	rlwa	x,a		;  ; 02
	cpl	(abyt,sp)	;a ; 03 10
	srl	(abyt,sp)	;a ; 04 10
				;  ; 05
	rrc	(abyt,sp)	;a ; 06 10
	sra	(abyt,sp)	;a ; 07 10
	sla	(abyt,sp)	;a ; 08 10
	rlc	(abyt,sp)	;a ; 09 10
	dec	(abyt,sp)	;a ; 0A 10
				;  ; 0B 10
	inc	(abyt,sp)	;a ; 0C 10
	tnz	(abyt,sp)	;a ; 0D 10
	swap	(abyt,sp)	;a ; 0E 10
	clr	(abyt,sp)	;a ; 0F 10

agrp1:
	sub	a,(abyt,sp)	;a ; 10 10
	cp	a,(abyt,sp)	;a ; 11 10
	sbc	a,(abyt,sp)	;a ; 12 10
	cpw	x,(abyt,sp)	;a ; 13 10
	and	a,(abyt,sp)	;a ; 14 10
	bcp	a,(abyt,sp)	;a ; 15 10
	ldw	y,(abyt,sp)	;a ; 16 10
	ldw	(abyt,sp),y	;a ; 17 10
	xor	a,(abyt,sp)	;a ; 18 10
	adc	a,(abyt,sp)	;a ; 19 10
	or	a,(abyt,sp)	;a ; 1A 10
	add	a,(abyt,sp)	;a ; 1B 10
	addw	x,#awrd		;  ; 1C 54 32
	subw	x,#awrd		;  ; 1D 54 32
	ldw	x,(abyt,sp)	;a ; 1E 10
	ldw	(abyt,sp),x	;a ; 1F 10

	.page

agrp2:
	jra	1$		;  ; 20 0E
	jrf	1$		;  ; 21 0C
	jrugt	1$		;  ; 22 0A
	jrule	1$		;  ; 23 08
	jrnc	1$		;  ; 24 06
	jrc	1$		;  ; 25 04
	jrne	1$		;  ; 26 02
	jreq	1$		;  ; 27 00
1$:	jrnv	1$		;  ; 28 FE
	jrv	1$		;  ; 29 FC
	jrpl	1$		;  ; 2A FA
	jrmi	1$		;  ; 2B F8
	jrsgt	1$		;  ; 2C F6
	jrsle	1$		;  ; 2D F4
	jrsge	1$		;  ; 2E F2
	jrslt	1$		;  ; 2F F0

agrp3:
	neg	abyt		;  ; 72 50 01 10
	exg	a,awrd		;a ; 31 54 32
	pop	awrd		;a ; 32 54 32
	cpl	abyt		;  ; 72 53 01 10
	srl	abyt		;  ; 72 54 01 10
	mov	awrd,#abyt	;a ; 35 10 54 32
	rrc	abyt		;  ; 72 56 01 10
	sra	abyt		;  ; 72 57 01 10
	sla	abyt		;  ; 72 58 01 10
	rlc	abyt		;  ; 72 59 01 10
	dec	abyt		;  ; 72 5A 01 10
	push	awrd		;a ; 3B 54 32
	inc	abyt		;  ; 72 5C 01 10
	tnz	abyt		;  ; 72 5D 01 10
	swap	abyt		;  ; 72 5E 01 10
	clr	abyt		;  ; 72 5F 01 10

	.page

agrp4:
	neg	a		;  ; 40
	exg	a,xl		;  ; 41
	mul	x,a		;  ; 42
	cpl	a		;  ; 43
	srl	a		;  ; 44
	mov	abyt,abyt+0x11	;  ; 55 01 21 01 10
	rrc	a		;  ; 46
	sra	a		;  ; 47
	sla	a		;  ; 48
	rlc	a		;  ; 49
	dec	a		;  ; 4A
	push	#abyt		;  ; 4B 10
	inc	a		;  ; 4C
	tnz	a		;  ; 4D
	swap	a		;  ; 4E
	clr	a		;  ; 4F

agrp5:
	negw	x		;  ; 50
	exgw	x,y		;  ; 51
	sub	sp,#abyt	;a ; 52 10
	cplw	x		;  ; 53
	srlw	x		;  ; 54
	mov	awrd,awrd+0x1111;a ; 55 65 43 54 32
	rrcw	x		;  ; 56
	sraw	x		;  ; 57
	slaw	x		;  ; 58
	rlcw	x		;  ; 59
	decw	x		;  ; 5A
	add	sp,#abyt	;a ; 5B 10
	incw	x		;  ; 5C
	tnzw	x		;  ; 5D
	swapw	x		;  ; 5E
	clrw	x		;  ; 5F

	.page

agrp6:
	neg	(abyt,x)	;  ; 72 40 01 10
	exg	a,yl		;  ; 61
	div	x,a		;  ; 62
	cpl	(abyt,x)	;  ; 72 43 01 10
	srl	(abyt,x)	;  ; 72 44 01 10
	divw	x,y		;  ; 65
	rrc	(abyt,x)	;  ; 72 46 01 10
	sra	(abyt,x)	;  ; 72 47 01 10
	sla	(abyt,x)	;  ; 72 48 01 10
	rlc	(abyt,x)	;  ; 72 49 01 10
	dec	(abyt,x)	;  ; 72 4A 01 10
	ld	(abyt,sp),a	;a ; 6B 10
	inc	(abyt,x)	;  ; 72 4C 01 10
	tnz	(abyt,x)	;  ; 72 4D 01 10
	swap	(abyt,x)	;  ; 72 4E 01 10
	clr	(abyt,x)	;  ; 72 4F 01 10

agrp7:
	neg	(x)		;  ; 70
				;  ; 71
				;  ; 72
	cpl	(x)		;  ; 73
	srl	(x)		;  ; 74
				;  ; 75
	rrc	(x)		;  ; 76
	sra	(x)		;  ; 77
	sla	(x)		;  ; 78
	rlc	(x)		;  ; 79
	dec	(x)		;  ; 7A
	ld	a,(abyt,sp)	;a ; 7B 10
	inc	(x)		;  ; 7C
	tnz	(x)		;  ; 7D
	swap	(x)		;  ; 7E
	clr	(x)		;  ; 7F

	.page

agrp8:
	iret			;  ; 80
	ret			;  ; 81
				;  ; 82
	trap			;  ; 83
	pop	a		;  ; 84
	popw	x		;  ; 85
	pop	cc		;  ; 86
	retf			;  ; 87
	push	a		;  ; 88
	pushw	x		;  ; 89
	push	cc		;  ; 8A
				;  ; 8B
	ccf			;  ; 8C
	callf	aexa		;  ; 8D BA 98 76
	halt			;  ; 8E
	wfi			;  ; 8F

agrp9:
				;  ; 90
				;  ; 91
				;  ; 92
	ldw	x,y		;  ; 93
	ldw	sp,x		;  ; 94
	ld	xh,a		;  ; 95
	ldw	x,sp		;  ; 96
	ld	xl,a		;  ; 97
	rcf			;  ; 98
	scf			;  ; 99
	rim			;  ; 9A
	sim			;  ; 9B
	rvf			;  ; 9C
	nop			;  ; 9D
	ld	a,xh		;  ; 9E
	ld	a,xl		;  ; 9F

	.page

agrpA:
	sub	a,#abyt		;  ; A0 10
	cp	a,#abyt		;  ; A1 10
	sbc	a,#abyt		;  ; A2 10
	cpw	x,#awrd		;  ; A3 54 32
	and	a,#abyt		;  ; A4 10
	bcp	a,#abyt		;  ; A5 10
	ld	a,#abyt		;  ; A6 10
	ldf	(aexa,x),a	;  ; A7 BA 98 76
	xor	a,#abyt		;  ; A8 10
	adc	a,#abyt		;  ; A9 10
	or	a,#abyt		;  ; AA 10
	add	a,#abyt		;  ; AB 10
	jpf	aexa		;  ; AC BA 98 76
1$:	callr	1$		;  ; AD FE
	ldw	x,#awrd		;  ; AE 54 32
	ldf	a,(aexa,x)	;  ; AF BA 98 76

agrpB:
	sub	a,abyt		;  ; C0 01 10
	cp	a,abyt		;  ; C1 01 10
	sbc	a,abyt		;  ; C2 01 10
	cpw	x,abyt		;  ; C3 01 10
	and	a,abyt		;  ; C4 01 10
	bcp	a,abyt		;  ; C5 01 10
	ld	a,abyt		;  ; C6 01 10
	ld	abyt,a		;  ; C7 01 10
	xor	a,abyt		;  ; C8 01 10
	adc	a,abyt		;  ; C9 01 10
	or	a,abyt		;  ; CA 01 10
	add	a,abyt		;  ; CB 01 10
	ldf	a,aexa		;  ; BC BA 98 76
	ldf	aexa,a		;  ; BD BA 98 76
	ldw	x,abyt		;  ; CE 01 10
	ldw	abyt,x		;  ; CF 01 10

	.page

agrpC:
	sub	a,awrd		;a ; C0 54 32
	cp	a,awrd		;a ; C1 54 32
	sbc	a,awrd		;a ; C2 54 32
	cpw	x,awrd		;a ; C3 54 32
	and	a,awrd		;a ; C4 54 32
	bcp	a,awrd		;a ; C5 54 32
	ld	a,awrd		;a ; C6 54 32
	ld	awrd,a		;a ; C7 54 32
	xor	a,awrd		;a ; C8 54 32
	adc	a,awrd		;a ; C9 54 32
	or	a,awrd		;a ; CA 54 32
	add	a,awrd		;a ; CB 54 32
	jp	awrd		;  ; CC 54 32
	call	awrd		;  ; CD 54 32
	ldw	x,awrd		;a ; CE 54 32
	ldw	awrd,x		;a ; CF 54 32

agrpD:
	sub	a,(awrd,x)	;a ; D0 54 32
	cp	a,(awrd,x)	;a ; D1 54 32
	sbc	a,(awrd,x)	;a ; D2 54 32
	cpw	y,(awrd,x)	;a ; D3 54 32
	and	a,(awrd,x)	;a ; D4 54 32
	bcp	a,(awrd,x)	;a ; D5 54 32
	ld	a,(awrd,x)	;a ; D6 54 32
	ld	(awrd,x),a	;a ; D7 54 32
	xor	a,(awrd,x)	;a ; D8 54 32
	adc	a,(awrd,x)	;a ; D9 54 32
	or	a,(awrd,x)	;a ; DA 54 32
	add	a,(awrd,x)	;a ; DB 54 32
	jp	(awrd,x)	;a ; DC 54 32
	call	(awrd,x)	;a ; DD 54 32
	ldw	x,(awrd,x)	;a ; DE 54 32
	ldw	(awrd,x),y	;a ; DF 54 32

	.page

agrpE:
	sub	a,(abyt,x)	;  ; D0 01 10
	cp	a,(abyt,x)	;  ; D1 01 10
	sbc	a,(abyt,x)	;  ; D2 01 10
	cpw	y,(abyt,x)	;  ; D3 01 10
	and	a,(abyt,x)	;  ; D4 01 10
	bcp	a,(abyt,x)	;  ; D5 01 10
	ld	a,(abyt,x)	;  ; D6 01 10
	ld	(abyt,x),a	;  ; D7 01 10
	xor	a,(abyt,x)	;  ; D8 01 10
	adc	a,(abyt,x)	;  ; D9 01 10
	or	a,(abyt,x)	;  ; DA 01 10
	add	a,(abyt,x)	;  ; DB 01 10
	jp	(abyt,x)	;  ; DC 01 10
	call	(abyt,x)	;  ; DD 01 10
	ldw	x,(abyt,x)	;  ; DE 01 10
	ldw	(abyt,x),y	;  ; DF 01 10

agrpF:
	sub	a,(x)		;  ; F0
	cp	a,(x)		;  ; F1
	sbc	a,(x)		;  ; F2
	cpw	y,(x)		;  ; F3
	and	a,(x)		;  ; F4
	bcp	a,(x)		;  ; F5
	ld	a,(x)		;  ; F6
	ld	(x),a		;  ; F7
	xor	a,(x)		;  ; F8
	adc	a,(x)		;  ; F9
	or	a,(x)		;  ; FA
	add	a,(x)		;  ; FB
	jp	(x)		;  ; FC
	call	(x)		;  ; FD
	ldw	x,(x)		;  ; FE
	ldw	(x),y		;  ; FF


	.page
	.sbttl	Page 72 STM8 Instructions in Numerical Order (Absolute)

agrp72_0:
	btjt	awrd,#0,1$	;a ; 72 00 54 32 23
	btjf	awrd,#0,1$	;a ; 72 01 54 32 1E
	btjt	awrd,#1,1$	;a ; 72 02 54 32 19
	btjf	awrd,#1,1$	;a ; 72 03 54 32 14
	btjt	awrd,#2,1$	;a ; 72 04 54 32 0F
	btjf	awrd,#2,1$	;a ; 72 05 54 32 0A
	btjt	awrd,#3,1$	;a ; 72 06 54 32 05
	btjf	awrd,#3,1$	;a ; 72 07 54 32 00
1$:	btjt	awrd,#4,1$	;a ; 72 08 54 32 FB
	btjf	awrd,#4,1$	;a ; 72 09 54 32 F6
	btjt	awrd,#5,1$	;a ; 72 0A 54 32 F1
	btjf	awrd,#5,1$	;a ; 72 0B 54 32 EC
	btjt	awrd,#6,1$	;a ; 72 0C 54 32 E7
	btjf	awrd,#6,1$	;a ; 72 0D 54 32 E2
	btjt	awrd,#7,1$	;a ; 72 0E 54 32 DD
	btjf	awrd,#7,1$	;a ; 72 0F 54 32 D8

agrp72_1:
	bset	awrd,#0		;a ; 72 10 54 32
	bres	awrd,#0		;a ; 72 11 54 32
	bset	awrd,#1		;a ; 72 12 54 32
	bres	awrd,#1		;a ; 72 13 54 32
	bset	awrd,#2		;a ; 72 14 54 32
	bres	awrd,#2		;a ; 72 15 54 32
	bset	awrd,#3		;a ; 72 16 54 32
	bres	awrd,#3		;a ; 72 17 54 32
	bset	awrd,#4		;a ; 72 18 54 32
	bres	awrd,#4		;a ; 72 19 54 32
	bset	awrd,#5		;a ; 72 1A 54 32
	bres	awrd,#5		;a ; 72 1B 54 32
	bset	awrd,#6		;a ; 72 1C 54 32
	bres	awrd,#6		;a ; 72 1D 54 32
	bset	awrd,#7		;a ; 72 1E 54 32
	bres	awrd,#7		;a ; 72 1F 54 32

agrp72_2:

	.page

agrp72_3:
	neg	[awrd]		;a ; 72 30 54 32
				;  ; 72 31
				;  ; 72 32
	cpl	[awrd]		;a ; 72 33 54 32
	srl	[awrd]		;a ; 72 34 54 32
				;  ; 72 35
	rrc	[awrd]		;a ; 72 36 54 32
	sra	[awrd]		;a ; 72 37 54 32
	sla	[awrd]		;a ; 72 38 54 32
	rlc	[awrd]		;a ; 72 39 54 32
	dec	[awrd]		;a ; 72 3A 54 32
				;  ; 72 3B
	inc	[awrd]		;a ; 72 3C 54 32
	tnz	[awrd]		;a ; 72 3D 54 32
	swap	[awrd]		;a ; 72 3E 54 32
	clr	[awrd]		;a ; 72 3F 54 32

agrp72_4:
	neg	(awrd,x)	;a ; 72 40 54 32
				;  ; 72 41
				;  ; 72 42
	cpl	(awrd,x)	;a ; 72 43 54 32
	srl	(awrd,x)	;a ; 72 44 54 32
				;  ; 72 45
	rrc	(awrd,x)	;a ; 72 46 54 32
	sra	(awrd,x)	;a ; 72 47 54 32
	sla	(awrd,x)	;a ; 72 48 54 32
	rlc	(awrd,x)	;a ; 72 49 54 32
	dec	(awrd,x)	;a ; 72 4A 54 32
				;  ; 72 4B
	inc	(awrd,x)	;a ; 72 4C 54 32
	tnz	(awrd,x)	;a ; 72 4D 54 32
	swap	(awrd,x)	;a ; 72 4E 54 32
	clr	(awrd,x)	;a ; 72 4F 54 32

	.page

agrp72_5:
	neg	awrd		;a ; 72 50 54 32
				;  ; 72 51
				;  ; 72 52
	cpl	awrd		;a ; 72 53 54 32
	srl	awrd		;a ; 72 54 54 32
				;  ; 72 55
	rrc	awrd		;a ; 72 56 54 32
	sra	awrd		;a ; 72 57 54 32
	sla	awrd		;a ; 72 58 54 32
	rlc	awrd		;a ; 72 59 54 32
	dec	awrd		;a ; 72 5A 54 32
				;  ; 72 5B
	inc	awrd		;a ; 72 5C 54 32
	tnz	awrd		;a ; 72 5D 54 32
	swap	awrd		;a ; 72 5E 54 32
	clr	awrd		;a ; 72 5F 54 32

agrp72_6:
	neg	([awrd],x)	;a ; 72 60 54 32
				;  ; 72 61
				;  ; 72 62
	cpl	([awrd],x)	;a ; 72 63 54 32
	srl	([awrd],x)	;a ; 72 64 54 32
				;  ; 72 65
	rrc	([awrd],x)	;a ; 72 66 54 32
	sra	([awrd],x)	;a ; 72 67 54 32
	sla	([awrd],x)	;a ; 72 68 54 32
	rlc	([awrd],x)	;a ; 72 69 54 32
	dec	([awrd],x)	;a ; 72 6A 54 32
				;  ; 72 6B
	inc	([awrd],x)	;a ; 72 6C 54 32
	tnz	([awrd],x)	;a ; 72 6D 54 32
	swap	([awrd],x)	;a ; 72 6E 54 32
	clr	([awrd],x)	;a ; 72 6F 54 32

	.page

agrp72_7:

agrp72_8:
	wfe			;  ; 72 8F

agrp72_9:

agrp72_A:
	subw	y,#awrd		;  ; 72 A2 54 32
	addw	y,#awrd		;  ; 72 A9 54 32

agrp72_B:
	subw	x,abyt		;a ; 72 B0 10
	subw	y,abyt		;a ; 72 B2 10
	addw	y,abyt		;a ; 72 B9 10
	addw	x,abyt		;a ; 72 BB 10

	.page

agrp72_C:
	sub	a,[awrd]	;a ; 72 C0 54 32
	cp	a,[awrd]	;a ; 72 C1 54 32
	sbc	a,[awrd]	;a ; 72 C2 54 32
	cpw	x,[awrd]	;a ; 72 C3 54 32
	and	a,[awrd]	;a ; 72 C4 54 32
	bcp	a,[awrd]	;a ; 72 C5 54 32
	ld	a,[awrd]	;a ; 72 C6 54 32
	ld	[awrd],a	;a ; 72 C7 54 32
	xor	a,[awrd]	;a ; 72 C8 54 32
	adc	a,[awrd]	;a ; 72 C9 54 32
	or	a,[awrd]	;a ; 72 CA 54 32
	add	a,[awrd]	;a ; 72 CB 54 32
	jp	[awrd]		;a ; 72 CC 54 32
	call	[awrd]		;a ; 72 CD 54 32
	ldw	x,[awrd]	;a ; 72 CE 54 32
	ldw	[awrd],x	;a ; 72 CF 54 32

agrp72_D:
	sub	a,([awrd],x)	;a ; 72 D0 54 32
	cp	a,([awrd],x)	;a ; 72 D1 54 32
	sbc	a,([awrd],x)	;a ; 72 D2 54 32
	cpw	y,([awrd],x)	;a ; 72 D3 54 32
	and	a,([awrd],x)	;a ; 72 D4 54 32
	bcp	a,([awrd],x)	;a ; 72 D5 54 32
	ld	a,([awrd],x)	;a ; 72 D6 54 32
	ld	([awrd],x),a	;a ; 72 D7 54 32
	xor	a,([awrd],x)	;a ; 72 D8 54 32
	adc	a,([awrd],x)	;a ; 72 D9 54 32
	or	a,([awrd],x)	;a ; 72 DA 54 32
	add	a,([awrd],x)	;a ; 72 DB 54 32
	jp	([awrd],x)	;a ; 72 DC 54 32
	call	([awrd],x)	;a ; 72 DD 54 32
	ldw	x,([awrd],x)	;a ; 72 DE 54 32
	ldw	([awrd],x),y	;a ; 72 DF 54 32

agrp72_E:

agrp72_F:
	subw	x,(abyt,sp)	;a ; 72 F0 10
	subw	y,(abyt,sp)	;a ; 72 F2 10
	addw	y,(abyt,sp)	;a ; 72 F9 10
	addw	x,(abyt,sp)	;a ; 72 FB 10


	.page
	.sbttl	Page 90 STM8 Instructions in Numerical Order (Absolute)

agrp90_0:
	rrwa	y,a		;  ; 90 01
	rlwa	y,a		;  ; 90 02

agrp90_1:
	bcpl	awrd,#0		;a ; 90 10 54 32
	bccm	awrd,#0		;a ; 90 11 54 32
	bcpl	awrd,#1		;a ; 90 12 54 32
	bccm	awrd,#1		;a ; 90 13 54 32
	bcpl	awrd,#2		;a ; 90 14 54 32
	bccm	awrd,#2		;a ; 90 15 54 32
	bcpl	awrd,#3		;a ; 90 16 54 32
	bccm	awrd,#3		;a ; 90 17 54 32
	bcpl	awrd,#4		;a ; 90 18 54 32
	bccm	awrd,#4		;a ; 90 19 54 32
	bcpl	awrd,#5		;a ; 90 1A 54 32
	bccm	awrd,#5		;a ; 90 1B 54 32
	bcpl	awrd,#6		;a ; 90 1C 54 32
	bccm	awrd,#6		;a ; 90 1D 54 32
	bcpl	awrd,#7		;a ; 90 1E 54 32
	bccm	awrd,#7		;a ; 90 1F 54 32

agrp90_2:
	jrnh	1$		;  ; 90 28 0C
	jrh	1$		;  ; 90 29 09
	jrnm	1$		;  ; 90 2C 06
	jrm	1$		;  ; 90 2D 03
	jril	1$		;  ; 90 2E 00
1$:	jrih	1$		;  ; 90 2F FD

agrp90_3:

	.page

agrp90_4:
	neg	(awrd,y)	;a ; 90 40 54 32
				;  ; 90 41
	mul	y,a		;  ; 90 42
	cpl	(awrd,y)	;a ; 90 43 54 32
	srl	(awrd,y)	;a ; 90 44 54 32
				;  ; 90 45
	rrc	(awrd,y)	;a ; 90 46 54 32
	sra	(awrd,y)	;a ; 90 47 54 32
	sla	(awrd,y)	;a ; 90 48 54 32
	rlc	(awrd,y)	;a ; 90 49 54 32
	dec	(awrd,y)	;a ; 90 4A 54 32
				;  ; 90 4B
	inc	(awrd,y)	;a ; 90 4C 54 32
	tnz	(awrd,y)	;a ; 90 4D 54 32
	swap	(awrd,y)	;a ; 90 4E 54 32
	clr	(awrd,y)	;a ; 90 4F 54 32

 agrp90_5:
	negw	y		;  ; 90 50
				;  ; 90 51
				;  ; 90 52
	cplw	y		;  ; 90 53
	srlw	y		;  ; 90 54
				;  ; 90 55
	rrcw	y		;  ; 90 56
	sraw	y		;  ; 90 57
	slaw	y		;  ; 90 58
	rlcw	y		;  ; 90 59
	decw	y		;  ; 90 5A
				;  ; 90 5B
	incw	y		;  ; 90 5C
	tnzw	y		;  ; 90 5D
	swapw	y		;  ; 90 5E
	clrw	y		;  ; 90 5F

.page

agrp90_6:
	neg	(abyt,y)	;  ; 90 40 01 10
				;  ; 90 61
	div	y,a		;  ; 90 62
	cpl	(abyt,y)	;  ; 90 43 01 10
	srl	(abyt,y)	;  ; 90 44 01 10
				;  ; 90 65
	rrc	(abyt,y)	;  ; 90 46 01 10
	sra	(abyt,y)	;  ; 90 47 01 10
	sla	(abyt,y)	;  ; 90 48 01 10
	rlc	(abyt,y)	;  ; 90 49 01 10
	dec	(abyt,y)	;  ; 90 4A 01 10
				;  ; 90 6B
	inc	(abyt,y)	;  ; 90 4C 01 10
	tnz	(abyt,y)	;  ; 90 4D 01 10
	swap	(abyt,y)	;  ; 90 4E 01 10
	clr	(abyt,y)	;  ; 90 4F 01 10

 agrp90_7:
	neg	(y)		;  ; 90 70
				;  ; 90 71
				;  ; 90 72
	cpl	(y)		;  ; 90 73
	srl	(y)		;  ; 90 74
				;  ; 90 75
	rrc	(y)		;  ; 90 76
	sra	(y)		;  ; 90 77
	sla	(y)		;  ; 90 78
	rlc	(y)		;  ; 90 79
	dec	(y)		;  ; 90 7A
				;  ; 90 7B
	inc	(y)		;  ; 90 7C
	tnz	(y)		;  ; 90 7D
	swap	(y)		;  ; 90 7E
	clr	(y)		;  ; 90 7F

.page

agrp90_8:
	popw	y		;  ; 90 85
	pushw	y		;  ; 90 89

agrp90_9:
	ldw	y,x		;  ; 90 93
	ldw	sp,y		;  ; 90 94
	ld	yh,a		;  ; 90 95
	ldw	y,sp		;  ; 90 96
	ld	yl,a		;  ; 90 97
	ld	a,yh		;  ; 90 9E
	ld	a,yl		;  ; 90 9F

agrp90_A:
	cpw	y,#awrd		;  ; 90 A3 54 32
	ldf	(aexa,y),a	;  ; 90 A7 BA 98 76
	ldw	y,#awrd		;  ; 90 AE 54 32
	ldf	a,(aexa,y)	;  ; 90 AF BA 98 76

agrp90_B:
	cpw	y,abyt		;  ; 90 C3 01 10
	ldw	y,abyt		;  ; 90 CE 01 10
	ldw	abyt,y		;  ; 90 CF 01 10

agrp90_C:
	cpw	y,awrd		;a ; 90 C3 54 32
	ldw	y,awrd		;a ; 90 CE 54 32
	ldw	awrd,y		;a ; 90 CF 54 32

agrp90_D:
	sub	a,(awrd,y)	;a ; 90 D0 54 32
	cp	a,(awrd,y)	;a ; 90 D1 54 32
	sbc	a,(awrd,y)	;a ; 90 D2 54 32
	cpw	x,(awrd,y)	;a ; 90 D3 54 32
	and	a,(awrd,y)	;a ; 90 D4 54 32
	bcp	a,(awrd,y)	;a ; 90 D5 54 32
	ld	a,(awrd,y)	;a ; 90 D6 54 32
	ld	(awrd,y),a	;a ; 90 D7 54 32
	xor	a,(awrd,y)	;a ; 90 D8 54 32
	adc	a,(awrd,y)	;a ; 90 D9 54 32
	or	a,(awrd,y)	;a ; 90 DA 54 32
	add	a,(awrd,y)	;a ; 90 DB 54 32
	jp	(awrd,y)	;a ; 90 DC 54 32
	call	(awrd,y)	;a ; 90 DD 54 32
	ldw 	y,(awrd,y)	;a ; 90 DE 54 32
	ldw 	(awrd,y),x	;a ; 90 DF 54 32

	.page

agrp90_E:
	sub	a,(abyt,y)	;  ; 90 D0 01 10
	cp	a,(abyt,y)	;  ; 90 D1 01 10
	sbc	a,(abyt,y)	;  ; 90 D2 01 10
	cpw	x,(abyt,y)	;  ; 90 D3 01 10
	and	a,(abyt,y)	;  ; 90 D4 01 10
	bcp	a,(abyt,y)	;  ; 90 D5 01 10
	ld	a,(abyt,y)	;  ; 90 D6 01 10
	ld	(abyt,y),a	;  ; 90 D7 01 10
	xor	a,(abyt,y)	;  ; 90 D8 01 10
	adc	a,(abyt,y)	;  ; 90 D9 01 10
	or	a,(abyt,y)	;  ; 90 DA 01 10
	add	a,(abyt,y)	;  ; 90 DB 01 10
	jp	(abyt,y)	;  ; 90 DC 01 10
	call	(abyt,y)	;  ; 90 DD 01 10
	ldw 	y,(abyt,y)	;  ; 90 DE 01 10
	ldw 	(abyt,y),x	;  ; 90 DF 01 10

agrp90_F:
	sub	a,(y)		;  ; 90 F0
	cp	a,(y)		;  ; 90 F1
	sbc	a,(y)		;  ; 90 F2
	cpw	x,(y)		;  ; 90 F3
	and	a,(y)		;  ; 90 F4
	bcp	a,(y)		;  ; 90 F5
	ld	a,(y)		;  ; 90 F6
	ld	(y),a		;  ; 90 F7
	xor	a,(y)		;  ; 90 F8
	adc	a,(y)		;  ; 90 F9
	or	a,(y)		;  ; 90 FA
	add	a,(y)		;  ; 90 FB
	jp	(y)		;  ; 90 FC
	call	(y)		;  ; 90 FD
	ldw 	y,(y)		;  ; 90 FE
	ldw 	(y),x		;  ; 90 FF


	.page
	.sbttl	Page 91 STM8 Instructions in Numerical Order (Absolute)

agrp91_0:

agrp91_1:

agrp91_2:

agrp91_3:

agrp91_4:

agrp91_5:

agrp91_6:
	neg	([abyt],y)	;a ; 91 60 10
				;  ; 91 61
				;  ; 91 62
	cpl	([abyt],y)	;a ; 91 63 10
	srl	([abyt],y)	;a ; 91 64 10
				;  ; 91 65
	rrc	([abyt],y)	;a ; 91 66 10
	sra	([abyt],y)	;a ; 91 67 10
	sla	([abyt],y)	;a ; 91 68 10
	rlc	([abyt],y)	;a ; 91 69 10
	dec	([abyt],y)	;a ; 91 6A 10
				;  ; 91 6B
	inc	([abyt],y)	;a ; 91 6C 10
	tnz	([abyt],y)	;a ; 91 6D 10
	swap	([abyt],y)	;a ; 91 6E 10
	clr	([abyt],y)	;a ; 91 6F 10

agrp91_7:

	.page

agrp91_8:

agrp91_9:

agrp91_A:
	ldf	([awrd],y),a	;a ; 91 A7 54 32
	ldf	a,([awrd],y)	;a ; 91 AF 54 32

agrp91_B:

agrp91_C:
	cpw	y,[abyt]	;a ; 91 C3 10
	ldw	y,[abyt]	;a ; 91 CE 10
	ldw	[abyt],y	;a ; 91 CF 10

agrp91_D:
	sub	a,([abyt],y)	;a ; 91 D0 10
	cp	a,([abyt],y)	;a ; 91 D1 10
	sbc	a,([abyt],y)	;a ; 91 D2 10
	cpw	x,([abyt],y)	;a ; 91 D3 10
	and	a,([abyt],y)	;a ; 91 D4 10
	bcp	a,([abyt],y)	;a ; 91 D5 10
	ld	a,([abyt],y)	;a ; 91 D6 10
	ld	([abyt],y),a	;a ; 91 D7 10
	xor	a,([abyt],y)	;a ; 91 D8 10
	adc	a,([abyt],y)	;a ; 91 D9 10
	or	a,([abyt],y)	;a ; 91 DA 10
	add	a,([abyt],y)	;a ; 91 DB 10
	jp	([abyt],y)	;a ; 91 DC 10
	call	([abyt],y)	;a ; 91 DD 10
	ldw 	y,([abyt],y)	;a ; 91 DE 10
	ldw 	([abyt],y),x	;a ; 91 DF 10

agrp91_E:

agrp91_F:

	.page
	.sbttl	Page 92 STM8 Instructions in Numerical Order (Absolute)

agrp92_0:

agrp92_1:

agrp92_2:

agrp92_3:
	neg	[abyt]		;  ; 72 30 01 10
				;  ; 92 31
				;  ; 92 32
	cpl	[abyt]		;  ; 72 33 01 10
	srl	[abyt]		;  ; 72 34 01 10
				;  ; 92 35
	rrc	[abyt]		;  ; 72 36 01 10
	sra	[abyt]		;  ; 72 37 01 10
	sla	[abyt]		;  ; 72 38 01 10
	rlc	[abyt]		;  ; 72 39 01 10
	dec	[abyt]		;  ; 72 3A 01 10
				;  ; 92 3B
	inc	[abyt]		;  ; 72 3C 01 10
	tnz	[abyt]		;  ; 72 3D 01 10
	swap	[abyt]		;  ; 72 3E 01 10
	clr	[abyt]		;  ; 72 3F 01 10

agrp92_4:

agrp92_5:

agrp92_6:
	neg	([abyt],x)	;  ; 72 60 01 10
				;  ; 92 61
				;  ; 92 62
	cpl	([abyt],x)	;  ; 72 63 01 10
	srl	([abyt],x)	;  ; 72 64 01 10
				;  ; 92 65
	rrc	([abyt],x)	;  ; 72 66 01 10
	sra	([abyt],x)	;  ; 72 67 01 10
	sla	([abyt],x)	;  ; 72 68 01 10
	rlc	([abyt],x)	;  ; 72 69 01 10
	dec	([abyt],x)	;  ; 72 6A 01 10
				;  ; 92 6B
	inc	([abyt],x)	;  ; 72 6C 01 10
	tnz	([abyt],x)	;  ; 72 6D 01 10
	swap	([abyt],x)	;  ; 72 6E 01 10
	clr	([abyt],x)	;  ; 72 6F 01 10

agrp92_7:

	.page

agrp92_8:
	callf	[awrd]		;a ; 92 8D 54 32

agrp92_9:

agrp92_A:
	ldf	([awrd],x),a	;a ; 92 A7 54 32
	jpf	[awrd]		;a ; 92 AC 54 32
	ldf	a,([awrd],x)	;a ; 92 AF 54 32

agrp92_B:
	ldf	a,[awrd]	;a ; 92 BC 54 32
	ldf	[awrd],a	;a ; 92 BD 54 32

agrp92_C:
	sub	a,[abyt]	;  ; 72 C0 01 10
	cp	a,[abyt]	;  ; 72 C1 01 10
	sbc	a,[abyt]	;  ; 72 C2 01 10
	cpw	x,[abyt]	;  ; 72 C3 01 10
	and	a,[abyt]	;  ; 72 C4 01 10
	bcp	a,[abyt]	;  ; 72 C5 01 10
	ld	a,[abyt]	;  ; 72 C6 01 10
	ld	[abyt],a	;  ; 72 C7 01 10
	xor	a,[abyt]	;  ; 72 C8 01 10
	adc	a,[abyt]	;  ; 72 C9 01 10
	or	a,[abyt]	;  ; 72 CA 01 10
	add	a,[abyt]	;  ; 72 CB 01 10
	jp	[abyt]		;  ; 72 CC 01 10
	call	[abyt]		;  ; 72 CD 01 10
	ldw 	x,[abyt]	;  ; 72 CE 01 10
	ldw 	[abyt],x	;  ; 72 CF 01 10

agrp92_D:
	sub	a,([abyt],x)	;  ; 72 D0 01 10
	cp	a,([abyt],x)	;  ; 72 D1 01 10
	sbc	a,([abyt],x)	;  ; 72 D2 01 10
	cpw	y,([abyt],x)	;  ; 72 D3 01 10
	and	a,([abyt],x)	;  ; 72 D4 01 10
	bcp	a,([abyt],x)	;  ; 72 D5 01 10
	ld	a,([abyt],x)	;  ; 72 D6 01 10
	ld	([abyt],x),a	;  ; 72 D7 01 10
	xor	a,([abyt],x)	;  ; 72 D8 01 10
	adc	a,([abyt],x)	;  ; 72 D9 01 10
	or	a,([abyt],x)	;  ; 72 DA 01 10
	add	a,([abyt],x)	;  ; 72 DB 01 10
	jp	([abyt],x)	;  ; 72 DC 01 10
	call	([abyt],x)	;  ; 72 DD 01 10
	ldw 	x,([abyt],x)	;  ; 72 DE 01 10
	ldw 	([abyt],x),y	;  ; 72 DF 01 10

agrp92_E:

agrp92_F:


	.page
	.sbttl	Base STM8 Instructions External 1-Byte Promotion to 2-Byte

	; Note: 1-Byte external references are promoted to
	;	2-Byte references if the  2-Byte addressing
	;	mode is allowed.
	;
	;	If the 1-Byte does not promote to the 2-Byte
	;	mode and the external reference is not within
	;	the range 0x00 - 0xFF the linker will report
	;	an error.
	;
	; The externally defined zbyt, zwrd, and zexa values
	; will result in linker errors for all references to
	; zbyt which cannot be promoted to 2-Byte modes.  All
	; zwrd references will result in linker errors.
	; Immediate references for zbyt and zwrd will simply
	; be truncated to 1-Byte or 2-Byte values respectively.
	; All references to zexa will simply use the lower 3-Byte
	; values.

	.define	rbyt, /zbyt+0x010/
	.define	rwrd, /zwrd+0x15432/
	.define	rexa, /zexa+0x1BA9876/

xgrp0:
	neg	(rbyt,sp)	;  ; 00u10
	rrwa	x,a		;  ; 01
	rlwa	x,a		;  ; 02
	cpl	(rbyt,sp)	;  ; 03u10
	srl	(rbyt,sp)	;  ; 04u10
				;  ; 05
	rrc	(rbyt,sp)	;  ; 06u10
	sra	(rbyt,sp)	;  ; 07u10
	sla	(rbyt,sp)	;  ; 08u10
	rlc	(rbyt,sp)	;  ; 09u10
	dec	(rbyt,sp)	;  ; 0Au10
				;  ; 0B
	inc	(rbyt,sp)	;  ; 0Cu10
	tnz	(rbyt,sp)	;  ; 0Du10
	swap	(rbyt,sp)	;  ; 0Eu10
	clr	(rbyt,sp)	;  ; 0Fu10

xgrp1:
	sub	a,(rbyt,sp)	;  ; 10u10
	cp	a,(rbyt,sp)	;  ; 11u10
	sbc	a,(rbyt,sp)	;  ; 12u10
	cpw	x,(rbyt,sp)	;  ; 13u10
	and	a,(rbyt,sp)	;  ; 14u10
	bcp	a,(rbyt,sp)	;  ; 15u10
	ldw	y,(rbyt,sp)	;  ; 16u10
	ldw	(rbyt,sp),y	;  ; 17u10
	xor	a,(rbyt,sp)	;  ; 18u10
	adc	a,(rbyt,sp)	;  ; 19u10
	or	a,(rbyt,sp)	;  ; 1Au10
	add	a,(rbyt,sp)	;  ; 1Bu10
	addw	x,#rwrd		;  ; 1Cs54r32
	subw	x,#rwrd		;  ; 1Ds54r32
	ldw	x,(rbyt,sp)	;  ; 1Eu10
	ldw	(rbyt,sp),x	;  ; 1Fu10

	.page

xgrp2:
	jra	1$		;  ; 20 0E
	jrf	1$		;  ; 21 0C
	jrugt	1$		;  ; 22 0A
	jrule	1$		;  ; 23 08
	jrnc	1$		;  ; 24 06
	jrc	1$		;  ; 25 04
	jrne	1$		;  ; 26 02
	jreq	1$		;  ; 27 00
1$:	jrnv	1$		;  ; 28 FE
	jrv	1$		;  ; 29 FC
	jrpl	1$		;  ; 2A FA
	jrmi	1$		;  ; 2B F8
	jrsgt	1$		;  ; 2C F6
	jrsle	1$		;  ; 2D F4
	jrsge	1$		;  ; 2E F2
	jrslt	1$		;  ; 2F F0

xgrp3:
	neg	rbyt		;  ; 72 50v00u10
	exg	a,rwrd		;  ; 31v54u32
	pop	rwrd		;  ; 32v54u32
	cpl	rbyt		;  ; 72 53v00u10
	srl	rbyt		;  ; 72 54v00u10
	mov	rwrd,#rbyt	;  ; 35r10v54u32
	rrc	rbyt		;  ; 72 56v00u10
	sra	rbyt		;  ; 72 57v00u10
	sla	rbyt		;  ; 72 58v00u10
	rlc	rbyt		;  ; 72 59v00u10
	dec	rbyt		;  ; 72 5Av00u10
	push	rwrd		;  ; 3Bv54u32
	inc	rbyt		;  ; 72 5Cv00u10
	tnz	rbyt		;  ; 72 5Dv00u10
	swap	rbyt		;  ; 72 5Ev00u10
	clr	rbyt		;  ; 72 5Fv00u10

	.page

xgrp4:
	neg	a		;  ; 40
	exg	a,xl		;  ; 41
	mul	x,a		;  ; 42
	cpl	a		;  ; 43
	srl	a		;  ; 44
	mov	rbyt,rbyt+0x11	;  ; 55v00u21v00u10
	rrc	a		;  ; 46
	sra	a		;  ; 47
	sla	a		;  ; 48
	rlc	a		;  ; 49
	dec	a		;  ; 4A
	push	#rbyt		;  ; 4Br10
	inc	a		;  ; 4C
	tnz	a		;  ; 4D
	swap	a		;  ; 4E
	clr	a		;  ; 4F

xgrp5:
	negw	x		;  ; 50
	exgw	x,y		;  ; 51
	sub	sp,#rbyt	;  ; 52u10
	cplw	x		;  ; 53
	srlw	x		;  ; 54
	mov	rwrd,rwrd+0x1111;  ; 55v65u43v54u32
	rrcw	x		;  ; 56
	sraw	x		;  ; 57
	slaw	x		;  ; 58
	rlcw	x		;  ; 59
	decw	x		;  ; 5A
	add	sp,#rbyt	;  ; 5Bu10
	incw	x		;  ; 5C
	tnzw	x		;  ; 5D
	swapw	x		;  ; 5E
	clrw	x		;  ; 5F

	.page

xgrp6:
	neg	(rbyt,x)	;  ; 72 40v00u10
	exg	a,yl		;  ; 61
	div	x,a		;  ; 62
	cpl	(rbyt,x)	;  ; 72 43v00u10
	srl	(rbyt,x)	;  ; 72 44v00u10
	divw	x,y		;  ; 65
	rrc	(rbyt,x)	;  ; 72 46v00u10
	sra	(rbyt,x)	;  ; 72 47v00u10
	sla	(rbyt,x)	;  ; 72 48v00u10
	rlc	(rbyt,x)	;  ; 72 49v00u10
	dec	(rbyt,x)	;  ; 72 4Av00u10
	ld	(rbyt,sp),a	;  ; 6Bu10
	inc	(rbyt,x)	;  ; 72 4Cv00u10
	tnz	(rbyt,x)	;  ; 72 4Dv00u10
	swap	(rbyt,x)	;  ; 72 4Ev00u10
	clr	(rbyt,x)	;  ; 72 4Fv00u10

xgrp7:
	neg	(x)		;  ; 70
				;  ; 71
				;  ; 72
	cpl	(x)		;  ; 73
	srl	(x)		;  ; 74
				;  ; 75
	rrc	(x)		;  ; 76
	sra	(x)		;  ; 77
	sla	(x)		;  ; 78
	rlc	(x)		;  ; 79
	dec	(x)		;  ; 7A
	ld	a,(rbyt,sp)	;  ; 7Bu10
	inc	(x)		;  ; 7C
	tnz	(x)		;  ; 7D
	swap	(x)		;  ; 7E
	clr	(x)		;  ; 7F

	.page

xgrp8:
	iret			;  ; 80
	ret			;  ; 81
				;  ; 82
	trap			;  ; 83
	pop	a		;  ; 84
	popw	x		;  ; 85
	pop	cc		;  ; 86
	retf			;  ; 87
	push	a		;  ; 88
	pushw	x		;  ; 89
	push	cc		;  ; 8A
				;  ; 8B
	ccf			;  ; 8C
	callf	rexa		;  ; 8DRBAs98r76
	halt			;  ; 8E
	wfi			;  ; 8F

xgrp9:
				;  ; 90
				;  ; 91
				;  ; 92
	ldw	x,y		;  ; 93
	ldw	sp,x		;  ; 94
	ld	xh,a		;  ; 95
	ldw	x,sp		;  ; 96
	ld	xl,a		;  ; 97
	rcf			;  ; 98
	scf			;  ; 99
	rim			;  ; 9A
	sim			;  ; 9B
	rvf			;  ; 9C
	nop			;  ; 9D
	ld	a,xh		;  ; 9E
	ld	a,xl		;  ; 9F

	.page

xgrpA:
	sub	a,#rbyt		;  ; A0r10
	cp	a,#rbyt		;  ; A1r10
	sbc	a,#rbyt		;  ; A2r10
	cpw	x,#rwrd		;  ; A3s54r32
	and	a,#rbyt		;  ; A4r10
	bcp	a,#rbyt		;  ; A5r10
	ld	a,#rbyt		;  ; A6r10
	ldf	(rexa,x),a	;  ; A7RBAs98r76
	xor	a,#rbyt		;  ; A8r10
	adc	a,#rbyt		;  ; A9r10
	or	a,#rbyt		;  ; AAr10
	add	a,#rbyt		;  ; ABr10
	jpf	rexa		;  ; ACRBAs98r76
1$:	callr	1$		;  ; AD FE
	ldw	x,#rwrd		;  ; AEs54r32
	ldf	a,(rexa,x)	;  ; AFRBAs98r76

xgrpB:
	sub	a,rbyt		;  ; C0v00u10
	cp	a,rbyt		;  ; C1v00u10
	sbc	a,rbyt		;  ; C2v00u10
	cpw	x,rbyt		;  ; C3v00u10
	and	a,rbyt		;  ; C4v00u10
	bcp	a,rbyt		;  ; C5v00u10
	ld	a,rbyt		;  ; C6v00u10
	ld	rbyt,a		;  ; C7v00u10
	xor	a,rbyt		;  ; C8v00u10
	adc	a,rbyt		;  ; C9v00u10
	or	a,rbyt		;  ; CAv00u10
	add	a,rbyt		;  ; CBv00u10
	ldf	a,rexa		;  ; BCRBAs98r76
	ldf	rexa,a		;  ; BDRBAs98r76
	ldw	x,rbyt		;  ; CEv00u10
	ldw	rbyt,x		;  ; CFv00u10

	.page

xgrpC:
	sub	a,rwrd		;  ; C0v54u32
	cp	a,rwrd		;  ; C1v54u32
	sbc	a,rwrd		;  ; C2v54u32
	cpw	x,rwrd		;  ; C3v54u32
	and	a,rwrd		;  ; C4v54u32
	bcp	a,rwrd		;  ; C5v54u32
	ld	a,rwrd		;  ; C6v54u32
	ld	rwrd,a		;  ; C7v54u32
	xor	a,rwrd		;  ; C8v54u32
	adc	a,rwrd		;  ; C9v54u32
	or	a,rwrd		;  ; CAv54u32
	add	a,rwrd		;  ; CBv54u32
	jp	rwrd		;  ; CCs54r32
	call	rwrd		;  ; CDs54r32
	ldw	x,rwrd		;  ; CEv54u32
	ldw	rwrd,x		;  ; CFv54u32

xgrpD:
	sub	a,(rwrd,x)	;  ; D0v54u32
	cp	a,(rwrd,x)	;  ; D1v54u32
	sbc	a,(rwrd,x)	;  ; D2v54u32
	cpw	y,(rwrd,x)	;  ; D3v54u32
	and	a,(rwrd,x)	;  ; D4v54u32
	bcp	a,(rwrd,x)	;  ; D5v54u32
	ld	a,(rwrd,x)	;  ; D6v54u32
	ld	(rwrd,x),a	;  ; D7v54u32
	xor	a,(rwrd,x)	;  ; D8v54u32
	adc	a,(rwrd,x)	;  ; D9v54u32
	or	a,(rwrd,x)	;  ; DAv54u32
	add	a,(rwrd,x)	;  ; DBv54u32
	jp	(rwrd,x)	;  ; DCv54u32
	call	(rwrd,x)	;  ; DDv54u32
	ldw	x,(rwrd,x)	;  ; DEv54u32
	ldw	(rwrd,x),y	;  ; DFv54u32

	.page

xgrpE:
	sub	a,(rbyt,x)	;  ; D0v00u10
	cp	a,(rbyt,x)	;  ; D1v00u10
	sbc	a,(rbyt,x)	;  ; D2v00u10
	cpw	y,(rbyt,x)	;  ; D3v00u10
	and	a,(rbyt,x)	;  ; D4v00u10
	bcp	a,(rbyt,x)	;  ; D5v00u10
	ld	a,(rbyt,x)	;  ; D6v00u10
	ld	(rbyt,x),a	;  ; D7v00u10
	xor	a,(rbyt,x)	;  ; D8v00u10
	adc	a,(rbyt,x)	;  ; D9v00u10
	or	a,(rbyt,x)	;  ; DAv00u10
	add	a,(rbyt,x)	;  ; DBv00u10
	jp	(rbyt,x)	;  ; DCv00u10
	call	(rbyt,x)	;  ; DDv00u10
	ldw	x,(rbyt,x)	;  ; DEv00u10
	ldw	(rbyt,x),y	;  ; DFv00u10

xgrpF:
	sub	a,(x)		;  ; F0
	cp	a,(x)		;  ; F1
	sbc	a,(x)		;  ; F2
	cpw	y,(x)		;  ; F3
	and	a,(x)		;  ; F4
	bcp	a,(x)		;  ; F5
	ld	a,(x)		;  ; F6
	ld	(x),a		;  ; F7
	xor	a,(x)		;  ; F8
	adc	a,(x)		;  ; F9
	or	a,(x)		;  ; FA
	add	a,(x)		;  ; FB
	jp	(x)		;  ; FC
	call	(x)		;  ; FD
	ldw	x,(x)		;  ; FE
	ldw	(x),y		;  ; FF


	.page
	.sbttl	Page 72 STM8 Instructions External 1-Byte Promotion to 2-Byte

xgrp72_0:
	btjt	rwrd,#0,1$	;  ; 72 00v54u32 23
	btjf	rwrd,#0,1$	;  ; 72 01v54u32 1E
	btjt	rwrd,#1,1$	;  ; 72 02v54u32 19
	btjf	rwrd,#1,1$	;  ; 72 03v54u32 14
	btjt	rwrd,#2,1$	;  ; 72 04v54u32 0F
	btjf	rwrd,#2,1$	;  ; 72 05v54u32 0A
	btjt	rwrd,#3,1$	;  ; 72 06v54u32 05
	btjf	rwrd,#3,1$	;  ; 72 07v54u32 00
1$:	btjt	rwrd,#4,1$	;  ; 72 08v54u32 FB
	btjf	rwrd,#4,1$	;  ; 72 09v54u32 F6
	btjt	rwrd,#5,1$	;  ; 72 0Av54u32 F1
	btjf	rwrd,#5,1$	;  ; 72 0Bv54u32 EC
	btjt	rwrd,#6,1$	;  ; 72 0Cv54u32 E7
	btjf	rwrd,#6,1$	;  ; 72 0Dv54u32 E2
	btjt	rwrd,#7,1$	;  ; 72 0Ev54u32 DD
	btjf	rwrd,#7,1$	;  ; 72 0Fv54u32 D8

xgrp72_1:
	bset	rwrd,#0		;  ; 72 10v54u32
	bres	rwrd,#0		;  ; 72 11v54u32
	bset	rwrd,#1		;  ; 72 12v54u32
	bres	rwrd,#1		;  ; 72 13v54u32
	bset	rwrd,#2		;  ; 72 14v54u32
	bres	rwrd,#2		;  ; 72 15v54u32
	bset	rwrd,#3		;  ; 72 16v54u32
	bres	rwrd,#3		;  ; 72 17v54u32
	bset	rwrd,#4		;  ; 72 18v54u32
	bres	rwrd,#4		;  ; 72 19v54u32
	bset	rwrd,#5		;  ; 72 1Av54u32
	bres	rwrd,#5		;  ; 72 1Bv54u32
	bset	rwrd,#6		;  ; 72 1Cv54u32
	bres	rwrd,#6		;  ; 72 1Dv54u32
	bset	rwrd,#7		;  ; 72 1Ev54u32
	bres	rwrd,#7		;  ; 72 1Fv54u32

xgrp72_2:

	.page

xgrp72_3:
	neg	[rwrd]		;  ; 72 30v54u32
				;  ; 72 31
				;  ; 72 32
	cpl	[rwrd]		;  ; 72 33v54u32
	srl	[rwrd]		;  ; 72 34v54u32
				;  ; 72 35
	rrc	[rwrd]		;  ; 72 36v54u32
	sra	[rwrd]		;  ; 72 37v54u32
	sla	[rwrd]		;  ; 72 38v54u32
	rlc	[rwrd]		;  ; 72 39v54u32
	dec	[rwrd]		;  ; 72 3Av54u32
				;  ; 72 3B
	inc	[rwrd]		;  ; 72 3Cv54u32
	tnz	[rwrd]		;  ; 72 3Dv54u32
	swap	[rwrd]		;  ; 72 3Ev54u32
	clr	[rwrd]		;  ; 72 3Fv54u32

xgrp72_4:
	neg	(rwrd,x)	;  ; 72 40v54u32
				;  ; 72 41
				;  ; 72 42
	cpl	(rwrd,x)	;  ; 72 43v54u32
	srl	(rwrd,x)	;  ; 72 44v54u32
				;  ; 72 45
	rrc	(rwrd,x)	;  ; 72 46v54u32
	sra	(rwrd,x)	;  ; 72 47v54u32
	sla	(rwrd,x)	;  ; 72 48v54u32
	rlc	(rwrd,x)	;  ; 72 49v54u32
	dec	(rwrd,x)	;  ; 72 4Av54u32
				;  ; 72 4B
	inc	(rwrd,x)	;  ; 72 4Cv54u32
	tnz	(rwrd,x)	;  ; 72 4Dv54u32
	swap	(rwrd,x)	;  ; 72 4Ev54u32
	clr	(rwrd,x)	;  ; 72 4Fv54u32

	.page

xgrp72_5:
	neg	rwrd		;  ; 72 50v54u32
				;  ; 72 51
				;  ; 72 52
	cpl	rwrd		;  ; 72 53v54u32
	srl	rwrd		;  ; 72 54v54u32
				;  ; 72 55
	rrc	rwrd		;  ; 72 56v54u32
	sra	rwrd		;  ; 72 57v54u32
	sla	rwrd		;  ; 72 58v54u32
	rlc	rwrd		;  ; 72 59v54u32
	dec	rwrd		;  ; 72 5Av54u32
				;  ; 72 5B
	inc	rwrd		;  ; 72 5Cv54u32
	tnz	rwrd		;  ; 72 5Dv54u32
	swap	rwrd		;  ; 72 5Ev54u32
	clr	rwrd		;  ; 72 5Fv54u32

xgrp72_6:
	neg	([rwrd],x)	;  ; 72 60v54u32
				;  ; 72 61
				;  ; 72 62
	cpl	([rwrd],x)	;  ; 72 63v54u32
	srl	([rwrd],x)	;  ; 72 64v54u32
				;  ; 72 65
	rrc	([rwrd],x)	;  ; 72 66v54u32
	sra	([rwrd],x)	;  ; 72 67v54u32
	sla	([rwrd],x)	;  ; 72 68v54u32
	rlc	([rwrd],x)	;  ; 72 69v54u32
	dec	([rwrd],x)	;  ; 72 6Av54u32
				;  ; 72 6B
	inc	([rwrd],x)	;  ; 72 6Cv54u32
	tnz	([rwrd],x)	;  ; 72 6Dv54u32
	swap	([rwrd],x)	;  ; 72 6Ev54u32
	clr	([rwrd],x)	;  ; 72 6Fv54u32

	.page

xgrp72_7:

xgrp72_8:
	wfe			;  ; 72 8F

xgrp72_9:

xgrp72_A:
	subw	y,#rwrd		;  ; 72 A2s54r32
	addw	y,#rwrd		;  ; 72 A9s54r32

xgrp72_B:
	subw	x,rbyt		;  ; 72 B0u10
	subw	y,rbyt		;  ; 72 B2u10
	addw	y,rbyt		;  ; 72 B9u10
	addw	x,rbyt		;  ; 72 BBu10

	.page

xgrp72_C:
	sub	a,[rwrd]	;  ; 72 C0v54u32
	cp	a,[rwrd]	;  ; 72 C1v54u32
	sbc	a,[rwrd]	;  ; 72 C2v54u32
	cpw	x,[rwrd]	;  ; 72 C3v54u32
	and	a,[rwrd]	;  ; 72 C4v54u32
	bcp	a,[rwrd]	;  ; 72 C5v54u32
	ld	a,[rwrd]	;  ; 72 C6v54u32
	ld	[rwrd],a	;  ; 72 C7v54u32
	xor	a,[rwrd]	;  ; 72 C8v54u32
	adc	a,[rwrd]	;  ; 72 C9v54u32
	or	a,[rwrd]	;  ; 72 CAv54u32
	add	a,[rwrd]	;  ; 72 CBv54u32
	jp	[rwrd]		;  ; 72 CCv54u32
	call	[rwrd]		;  ; 72 CDv54u32
	ldw	x,[rwrd]	;  ; 72 CEv54u32
	ldw	[rwrd],x	;  ; 72 CFv54u32

xgrp72_D:
	sub	a,([rwrd],x)	;  ; 72 D0v54u32
	cp	a,([rwrd],x)	;  ; 72 D1v54u32
	sbc	a,([rwrd],x)	;  ; 72 D2v54u32
	cpw	y,([rwrd],x)	;  ; 72 D3v54u32
	and	a,([rwrd],x)	;  ; 72 D4v54u32
	bcp	a,([rwrd],x)	;  ; 72 D5v54u32
	ld	a,([rwrd],x)	;  ; 72 D6v54u32
	ld	([rwrd],x),a	;  ; 72 D7v54u32
	xor	a,([rwrd],x)	;  ; 72 D8v54u32
	adc	a,([rwrd],x)	;  ; 72 D9v54u32
	or	a,([rwrd],x)	;  ; 72 DAv54u32
	add	a,([rwrd],x)	;  ; 72 DBv54u32
	jp	([rwrd],x)	;  ; 72 DCv54u32
	call	([rwrd],x)	;  ; 72 DDv54u32
	ldw	x,([rwrd],x)	;  ; 72 DEv54u32
	ldw	([rwrd],x),y	;  ; 72 DFv54u32

xgrp72_E:

xgrp72_F:
	subw	x,(rbyt,sp)	;  ; 72 F0u10
	subw	y,(rbyt,sp)	;  ; 72 F2u10
	addw	y,(rbyt,sp)	;  ; 72 F9u10
	addw	x,(rbyt,sp)	;  ; 72 FBu10


	.page
	.sbttl	Page 90 STM8 Instructions External 1-Byte Promotion to 2-Byte

xgrp90_0:
	rrwa	y,a		;  ; 90 01
	rlwa	y,a		;  ; 90 02

xgrp90_1:
	bcpl	rwrd,#0		;  ; 90 10v54u32
	bccm	rwrd,#0		;  ; 90 11v54u32
	bcpl	rwrd,#1		;  ; 90 12v54u32
	bccm	rwrd,#1		;  ; 90 13v54u32
	bcpl	rwrd,#2		;  ; 90 14v54u32
	bccm	rwrd,#2		;  ; 90 15v54u32
	bcpl	rwrd,#3		;  ; 90 16v54u32
	bccm	rwrd,#3		;  ; 90 17v54u32
	bcpl	rwrd,#4		;  ; 90 18v54u32
	bccm	rwrd,#4		;  ; 90 19v54u32
	bcpl	rwrd,#5		;  ; 90 1Av54u32
	bccm	rwrd,#5		;  ; 90 1Bv54u32
	bcpl	rwrd,#6		;  ; 90 1Cv54u32
	bccm	rwrd,#6		;  ; 90 1Dv54u32
	bcpl	rwrd,#7		;  ; 90 1Ev54u32
	bccm	rwrd,#7		;  ; 90 1Fv54u32

xgrp90_2:
	jrnh	1$		;  ; 90 28 0C
	jrh	1$		;  ; 90 29 09
	jrnm	1$		;  ; 90 2C 06
	jrm	1$		;  ; 90 2D 03
	jril	1$		;  ; 90 2E 00
1$:	jrih	1$		;  ; 90 2F FD

xgrp90_3:

	.page

xgrp90_4:
	neg	(rwrd,y)	;  ; 90 40v54u32
				;  ; 90 41
	mul	y,a		;  ; 90 42
	cpl	(rwrd,y)	;  ; 90 43v54u32
	srl	(rwrd,y)	;  ; 90 44v54u32
				;  ; 90 45
	rrc	(rwrd,y)	;  ; 90 46v54u32
	sra	(rwrd,y)	;  ; 90 47v54u32
	sla	(rwrd,y)	;  ; 90 48v54u32
	rlc	(rwrd,y)	;  ; 90 49v54u32
	dec	(rwrd,y)	;  ; 90 4Av54u32
				;  ; 90 4B
	inc	(rwrd,y)	;  ; 90 4Cv54u32
	tnz	(rwrd,y)	;  ; 90 4Dv54u32
	swap	(rwrd,y)	;  ; 90 4Ev54u32
	clr	(rwrd,y)	;  ; 90 4Fv54u32

 xgrp90_5:
	negw	y		;  ; 90 50
				;  ; 90 51
				;  ; 90 52
	cplw	y		;  ; 90 53
	srlw	y		;  ; 90 54
				;  ; 90 55
	rrcw	y		;  ; 90 56
	sraw	y		;  ; 90 57
	slaw	y		;  ; 90 58
	rlcw	y		;  ; 90 59
	decw	y		;  ; 90 5A
				;  ; 90 5B
	incw	y		;  ; 90 5C
	tnzw	y		;  ; 90 5D
	swapw	y		;  ; 90 5E
	clrw	y		;  ; 90 5F

.page

xgrp90_6:
	neg	(rbyt,y)	;  ; 90 40v00u10
				;  ; 90 61
	div	y,a		;  ; 90 62
	cpl	(rbyt,y)	;  ; 90 43v00u10
	srl	(rbyt,y)	;  ; 90 44v00u10
				;  ; 90 65
	rrc	(rbyt,y)	;  ; 90 46v00u10
	sra	(rbyt,y)	;  ; 90 47v00u10
	sla	(rbyt,y)	;  ; 90 48v00u10
	rlc	(rbyt,y)	;  ; 90 49v00u10
	dec	(rbyt,y)	;  ; 90 4Av00u10
				;  ; 90 6B
	inc	(rbyt,y)	;  ; 90 4Cv00u10
	tnz	(rbyt,y)	;  ; 90 4Dv00u10
	swap	(rbyt,y)	;  ; 90 4Ev00u10
	clr	(rbyt,y)	;  ; 90 4Fv00u10

 xgrp90_7:
	neg	(y)		;  ; 90 70
				;  ; 90 71
				;  ; 90 72
	cpl	(y)		;  ; 90 73
	srl	(y)		;  ; 90 74
				;  ; 90 75
	rrc	(y)		;  ; 90 76
	sra	(y)		;  ; 90 77
	sla	(y)		;  ; 90 78
	rlc	(y)		;  ; 90 79
	dec	(y)		;  ; 90 7A
				;  ; 90 7B
	inc	(y)		;  ; 90 7C
	tnz	(y)		;  ; 90 7D
	swap	(y)		;  ; 90 7E
	clr	(y)		;  ; 90 7F

.page

xgrp90_8:
	popw	y		;  ; 90 85
	pushw	y		;  ; 90 89

xgrp90_9:
	ldw	y,x		;  ; 90 93
	ldw	sp,y		;  ; 90 94
	ld	yh,a		;  ; 90 95
	ldw	y,sp		;  ; 90 96
	ld	yl,a		;  ; 90 97
	ld	a,yh		;  ; 90 9E
	ld	a,yl		;  ; 90 9F

xgrp90_A:
	cpw	y,#rwrd		;  ; 90 A3s54r32
	ldf	(rexa,y),a	;  ; 90 A7RBAs98r76
	ldw	y,#rwrd		;  ; 90 AEs54r32
	ldf	a,(rexa,y)	;  ; 90 AFRBAs98r76

xgrp90_B:
	cpw	y,rbyt		;  ; 90 C3v00u10
	ldw	y,rbyt		;  ; 90 CEv00u10
	ldw	rbyt,y		;  ; 90 CFv00u10

xgrp90_C:
	cpw	y,rwrd		;  ; 90 C3v54u32
	ldw	y,rwrd		;  ; 90 CEv54u32
	ldw	rwrd,y		;  ; 90 CFv54u32

xgrp90_D:
	sub	a,(rwrd,y)	;  ; 90 D0v54u32
	cp	a,(rwrd,y)	;  ; 90 D1v54u32
	sbc	a,(rwrd,y)	;  ; 90 D2v54u32
	cpw	x,(rwrd,y)	;  ; 90 D3v54u32
	and	a,(rwrd,y)	;  ; 90 D4v54u32
	bcp	a,(rwrd,y)	;  ; 90 D5v54u32
	ld	a,(rwrd,y)	;  ; 90 D6v54u32
	ld	(rwrd,y),a	;  ; 90 D7v54u32
	xor	a,(rwrd,y)	;  ; 90 D8v54u32
	adc	a,(rwrd,y)	;  ; 90 D9v54u32
	or	a,(rwrd,y)	;  ; 90 DAv54u32
	add	a,(rwrd,y)	;  ; 90 DBv54u32
	jp	(rwrd,y)	;  ; 90 DCv54u32
	call	(rwrd,y)	;  ; 90 DDv54u32
	ldw 	y,(rwrd,y)	;  ; 90 DEv54u32
	ldw 	(rwrd,y),x	;  ; 90 DFv54u32

	.page

xgrp90_E:
	sub	a,(rbyt,y)	;  ; 90 D0v00u10
	cp	a,(rbyt,y)	;  ; 90 D1v00u10
	sbc	a,(rbyt,y)	;  ; 90 D2v00u10
	cpw	x,(rbyt,y)	;  ; 90 D3v00u10
	and	a,(rbyt,y)	;  ; 90 D4v00u10
	bcp	a,(rbyt,y)	;  ; 90 D5v00u10
	ld	a,(rbyt,y)	;  ; 90 D6v00u10
	ld	(rbyt,y),a	;  ; 90 D7v00u10
	xor	a,(rbyt,y)	;  ; 90 D8v00u10
	adc	a,(rbyt,y)	;  ; 90 D9v00u10
	or	a,(rbyt,y)	;  ; 90 DAv00u10
	add	a,(rbyt,y)	;  ; 90 DBv00u10
	jp	(rbyt,y)	;  ; 90 DCv00u10
	call	(rbyt,y)	;  ; 90 DDv00u10
	ldw 	y,(rbyt,y)	;  ; 90 DEv00u10
	ldw 	(rbyt,y),x	;  ; 90 DFv00u10

xgrp90_F:
	sub	a,(y)		;  ; 90 F0
	cp	a,(y)		;  ; 90 F1
	sbc	a,(y)		;  ; 90 F2
	cpw	x,(y)		;  ; 90 F3
	and	a,(y)		;  ; 90 F4
	bcp	a,(y)		;  ; 90 F5
	ld	a,(y)		;  ; 90 F6
	ld	(y),a		;  ; 90 F7
	xor	a,(y)		;  ; 90 F8
	adc	a,(y)		;  ; 90 F9
	or	a,(y)		;  ; 90 FA
	add	a,(y)		;  ; 90 FB
	jp	(y)		;  ; 90 FC
	call	(y)		;  ; 90 FD
	ldw 	y,(y)		;  ; 90 FE
	ldw 	(y),x		;  ; 90 FF


	.page
	.sbttl	Page 91 STM8 Instructions External 1-Byte Promotion to 2-Byte

xgrp91_0:

xgrp91_1:

xgrp91_2:

xgrp91_3:

xgrp91_4:

xgrp91_5:

xgrp91_6:
	neg	([rbyt],y)	;  ; 91 60u10
				;  ; 91 61
				;  ; 91 62
	cpl	([rbyt],y)	;  ; 91 63u10
	srl	([rbyt],y)	;  ; 91 64u10
				;  ; 91 65
	rrc	([rbyt],y)	;  ; 91 66u10
	sra	([rbyt],y)	;  ; 91 67u10
	sla	([rbyt],y)	;  ; 91 68u10
	rlc	([rbyt],y)	;  ; 91 69u10
	dec	([rbyt],y)	;  ; 91 6Au10
				;  ; 91 6B
	inc	([rbyt],y)	;  ; 91 6Cu10
	tnz	([rbyt],y)	;  ; 91 6Du10
	swap	([rbyt],y)	;  ; 91 6Eu10
	clr	([rbyt],y)	;  ; 91 6Fu10

xgrp91_7:

	.page

xgrp91_8:

xgrp91_9:

xgrp91_A:
	ldf	([rwrd],y),a	;  ; 91 A7v54u32
	ldf	a,([rwrd],y)	;  ; 91 AFv54u32

xgrp91_B:

xgrp91_C:
	cpw	y,[rbyt]	;  ; 91 C3u10
	ldw	y,[rbyt]	;  ; 91 CEu10
	ldw	[rbyt],y	;  ; 91 CFu10

xgrp91_D:
	sub	a,([rbyt],y)	;  ; 91 D0u10
	cp	a,([rbyt],y)	;  ; 91 D1u10
	sbc	a,([rbyt],y)	;  ; 91 D2u10
	cpw	x,([rbyt],y)	;  ; 91 D3u10
	and	a,([rbyt],y)	;  ; 91 D4u10
	bcp	a,([rbyt],y)	;  ; 91 D5u10
	ld	a,([rbyt],y)	;  ; 91 D6u10
	ld	([rbyt],y),a	;  ; 91 D7u10
	xor	a,([rbyt],y)	;  ; 91 D8u10
	adc	a,([rbyt],y)	;  ; 91 D9u10
	or	a,([rbyt],y)	;  ; 91 DAu10
	add	a,([rbyt],y)	;  ; 91 DBu10
	jp	([rbyt],y)	;  ; 91 DCu10
	call	([rbyt],y)	;  ; 91 DDu10
	ldw 	y,([rbyt],y)	;  ; 91 DEu10
	ldw 	([rbyt],y),x	;  ; 91 DFu10

xgrp91_E:

xgrp91_F:

	.page
	.sbttl	Page 92 STM8 Instructions External 1-Byte Promotion to 2-Byte

xgrp92_0:

xgrp92_1:

xgrp92_2:

xgrp92_3:
	neg	[rbyt]		;  ; 72 30v00u10
				;  ; 92 31
				;  ; 92 32
	cpl	[rbyt]		;  ; 72 33v00u10
	srl	[rbyt]		;  ; 72 34v00u10
				;  ; 92 35
	rrc	[rbyt]		;  ; 72 36v00u10
	sra	[rbyt]		;  ; 72 37v00u10
	sla	[rbyt]		;  ; 72 38v00u10
	rlc	[rbyt]		;  ; 72 39v00u10
	dec	[rbyt]		;  ; 72 3Av00u10
				;  ; 92 3B
	inc	[rbyt]		;  ; 72 3Cv00u10
	tnz	[rbyt]		;  ; 72 3Dv00u10
	swap	[rbyt]		;  ; 72 3Ev00u10
	clr	[rbyt]		;  ; 72 3Fv00u10

xgrp92_4:

xgrp92_5:

xgrp92_6:
	neg	([rbyt],x)	;  ; 72 60v00u10
				;  ; 92 61
				;  ; 92 62
	cpl	([rbyt],x)	;  ; 72 63v00u10
	srl	([rbyt],x)	;  ; 72 64v00u10
				;  ; 92 65
	rrc	([rbyt],x)	;  ; 72 66v00u10
	sra	([rbyt],x)	;  ; 72 67v00u10
	sla	([rbyt],x)	;  ; 72 68v00u10
	rlc	([rbyt],x)	;  ; 72 69v00u10
	dec	([rbyt],x)	;  ; 72 6Av00u10
				;  ; 92 6B
	inc	([rbyt],x)	;  ; 72 6Cv00u10
	tnz	([rbyt],x)	;  ; 72 6Dv00u10
	swap	([rbyt],x)	;  ; 72 6Ev00u10
	clr	([rbyt],x)	;  ; 72 6Fv00u10

xgrp92_7:

	.page

xgrp92_8:
	callf	[rwrd]		;  ; 92 8Dv54u32

xgrp92_9:

xgrp92_A:
	ldf	([rwrd],x),a	;  ; 92 A7v54u32
	jpf	[rwrd]		;  ; 92 ACv54u32
	ldf	a,([rwrd],x)	;  ; 92 AFv54u32

xgrp92_B:
	ldf	a,[rwrd]	;  ; 92 BCv54u32
	ldf	[rwrd],a	;  ; 92 BDv54u32

xgrp92_C:
	sub	a,[rbyt]	;  ; 72 C0v00u10
	cp	a,[rbyt]	;  ; 72 C1v00u10
	sbc	a,[rbyt]	;  ; 72 C2v00u10
	cpw	x,[rbyt]	;  ; 72 C3v00u10
	and	a,[rbyt]	;  ; 72 C4v00u10
	bcp	a,[rbyt]	;  ; 72 C5v00u10
	ld	a,[rbyt]	;  ; 72 C6v00u10
	ld	[rbyt],a	;  ; 72 C7v00u10
	xor	a,[rbyt]	;  ; 72 C8v00u10
	adc	a,[rbyt]	;  ; 72 C9v00u10
	or	a,[rbyt]	;  ; 72 CAv00u10
	add	a,[rbyt]	;  ; 72 CBv00u10
	jp	[rbyt]		;  ; 72 CCv00u10
	call	[rbyt]		;  ; 72 CDv00u10
	ldw 	x,[rbyt]	;  ; 72 CEv00u10
	ldw 	[rbyt],x	;  ; 72 CFv00u10

xgrp92_D:
	sub	a,([rbyt],x)	;  ; 72 D0v00u10
	cp	a,([rbyt],x)	;  ; 72 D1v00u10
	sbc	a,([rbyt],x)	;  ; 72 D2v00u10
	cpw	y,([rbyt],x)	;  ; 72 D3v00u10
	and	a,([rbyt],x)	;  ; 72 D4v00u10
	bcp	a,([rbyt],x)	;  ; 72 D5v00u10
	ld	a,([rbyt],x)	;  ; 72 D6v00u10
	ld	([rbyt],x),a	;  ; 72 D7v00u10
	xor	a,([rbyt],x)	;  ; 72 D8v00u10
	adc	a,([rbyt],x)	;  ; 72 D9v00u10
	or	a,([rbyt],x)	;  ; 72 DAv00u10
	add	a,([rbyt],x)	;  ; 72 DBv00u10
	jp	([rbyt],x)	;  ; 72 DCv00u10
	call	([rbyt],x)	;  ; 72 DDv00u10
	ldw 	x,([rbyt],x)	;  ; 72 DEv00u10
	ldw 	([rbyt],x),y	;  ; 72 DFv00u10

xgrp92_E:

xgrp92_F:

	.undefine	rbyt
	.undefine	rwrd
	.undefine	rexa


	.page
	.sbttl	Modes Test

	ival	=	0x0010		; Absolute 1-Byte Value
	iwrd	=	0x005432	; Absolute 2-Byte Value
	iexa	=	0x00BA9876	; Absolute 3-Byte Value

	; S_AOP
	add	a,(ival,x).e	;a ; DB 00 10
	add	a,(ival,x).w	;  ; DB 00 10
	add	a,(ival,x).b	;  ; EB 10
	add	a,(ival,x)	;  ; EB 10

	add	a,[ival].e	;a ; 72 CB 00 10
	add	a,[ival].w	;  ; 72 CB 00 10
	add	a,[ival].b	;  ; 92 CB 10
	add	a,[ival]	;  ; 92 CB 10

	add	a,([ival].e,x)	;a ; 72 DB 00 10
	add	a,([ival].w,x)	;  ; 72 DB 00 10
	add	a,([ival].b,x)	;  ; 92 DB 10
	add	a,([ival],x)	;  ; 92 DB 10

	add	a,([ival],x).e	;a ; 72 DB 00 10
	add	a,([ival],x).w	;  ; 72 DB 00 10
	add	a,([ival],x).b	;  ; 92 DB 10
	add	a,([ival],x)	;  ; 92 DB 10

	; S_ADDW
	addw	x,(ival,sp).e	;a ; 72 FB 10
	addw	x,(ival,sp).w	;a ; 72 FB 10
	addw	x,(ival,sp).b	;  ; 72 FB 10
	addw	x,(ival,sp)	;  ; 72 FB 10

	; S_SUBW
	subw	x,(ival,sp).e	;a ; 72 F0 10
	subw	x,(ival,sp).w	;a ; 72 F0 10
	subw	x,(ival,sp).b	;  ; 72 F0 10
	subw	x,(ival,sp)	;  ; 72 F0 10

	; S_CPW
	cpw	x,(ival,y).e	;a ; 90 D3 00 10
	cpw	x,(ival,y).w	;  ; 90 D3 00 10
	cpw	x,(ival,y).b	;  ; 90 E3 10
	cpw	x,(ival,y)	;  ; 90 E3 10

	cpw	y,(ival,x).e	;a ; D3 00 10
	cpw	y,(ival,x).w	;  ; D3 00 10
	cpw	y,(ival,x).b	;  ; E3 10
	cpw	y,(ival,x)	;  ; E3 10

	cpw	x,[ival].e	;a ; 72 C3 00 10
	cpw	x,[ival].w	;  ; 72 C3 00 10
	cpw	x,[ival].b	;  ; 92 C3 10
	cpw	x,[ival]	;  ; 92 C3 10

	cpw	y,[ival].e	;a ; 91 C3 10
	cpw	y,[ival].w	;a ; 91 C3 10
	cpw	y,[ival].b	;  ; 91 C3 10
	cpw	y,[ival]	;  ; 91 C3 10

	cpw	x,([ival].e,y)	;a ; 91 D3 10
	cpw	x,([ival].w,y)	;a ; 91 D3 10
	cpw	x,([ival].b,y)	;  ; 91 D3 10
	cpw	x,([ival],y)	;  ; 91 D3 10

	cpw	x,([ival],y).e	;a ; 91 D3 10
	cpw	x,([ival],y).w	;a ; 91 D3 10
	cpw	x,([ival],y).b	;  ; 91 D3 10
	cpw	x,([ival],y)	;  ; 91 D3 10

	cpw	y,([ival].e,x)	;a ; 72 D3 00 10
	cpw	y,([ival].w,x)	;  ; 72 D3 00 10
	cpw	y,([ival].b,x)	;  ; 92 D3 10
	cpw	y,([ival],x)	;  ; 92 D3 10

	cpw	y,([ival],x).e	;a ; 72 D3 00 10
	cpw	y,([ival],x).w	;  ; 72 D3 00 10
	cpw	y,([ival],x).b	;  ; 92 D3 10
	cpw	y,([ival],x)	;  ; 92 D3 10

	cpw	x,(ival,sp).e	;a ; 13 10
	cpw	x,(ival,sp).w	;a ; 13 10
	cpw	x,(ival,sp).b	;  ; 13 10
	cpw	x,(ival,sp)	;  ; 13 10

	; S_BOP
	neg	(ival,x).e	;a ; 72 40 00 10
	neg	(ival,x).w	;  ; 72 40 00 10
	neg	(ival,x).b	;  ; 60 10
	neg	(ival,x)	;  ; 60 10

	neg	(ival,sp).e	;a ; 00 10
	neg	(ival,sp).w	;a ; 00 10
	neg	(ival,sp).b	;  ; 00 10
	neg	(ival,sp)	;  ; 00 10

	neg	[ival].e	;a ; 72 30 00 10
	neg	[ival].w	;  ; 72 30 00 10
	neg	[ival].b	;  ; 92 30 10
	neg	[ival]		;  ; 92 30 10

	neg	([ival].e,x)	;a ; 72 60 00 10
	neg	([ival].w,x)	;  ; 72 60 00 10
	neg	([ival].b,x)	;  ; 92 60 10
	neg	([ival],x)	;  ; 92 60 10

	neg	([ival],x).e	;a ; 72 60 00 10
	neg	([ival],x).w	;  ; 72 60 00 10
	neg	([ival],x).b	;  ; 92 60 10
	neg	([ival],x)	;  ; 92 60 10

	neg	([ival].e,y)	;a ; 91 60 10
	neg	([ival].w,y)	;a ; 91 60 10
	neg	([ival].b,y)	;  ; 91 60 10
	neg	([ival],y)	;  ; 91 60 10

	neg	([ival],y).e	;a ; 91 60 10
	neg	([ival],y).w	;a ; 91 60 10
	neg	([ival],y).b	;  ; 91 60 10
	neg	([ival],y)	;  ; 91 60 10

	; S_LD
	ld	a,(ival,x).e	;a ; D6 00 10
	ld	a,(ival,x).w	;  ; D6 00 10
	ld	a,(ival,x).b	;  ; E6 10
	ld	a,(ival,x)	;  ; E6 10

	ld	a,(ival,sp).e	;a ; 7B 10
	ld	a,(ival,sp).w	;a ; 7B 10
	ld	a,(ival,sp).b	;  ; 7B 10
	ld	a,(ival,sp)	;  ; 7B 10

	ld	a,[ival].e	;a ; 72 C6 00 10
	ld	a,[ival].w	;  ; 72 C6 00 10
	ld	a,[ival].b	;  ; 92 C6 10
	ld	a,[ival]	;  ; 92 C6 10

	ld	a,([ival].e,x)	;a ; 72 D6 00 10
	ld	a,([ival].w,x)	;  ; 72 D6 00 10
	ld	a,([ival].b,x)	;  ; 92 D6 10
	ld	a,([ival],x)	;  ; 92 D6 10

	ld	a,([ival],x).e	;a ; 72 D6 00 10
	ld	a,([ival],x).w	;  ; 72 D6 00 10
	ld	a,([ival],x).b	;  ; 92 D6 10
	ld	a,([ival],x)	;  ; 92 D6 10

	ld	a,([ival].e,y)	;a ; 91 D6 10
	ld	a,([ival].w,y)	;a ; 91 D6 10
	ld	a,([ival].b,y)	;  ; 91 D6 10
	ld	a,([ival],y)	;  ; 91 D6 10

	ld	a,([ival],y).e	;a ; 91 D6 10
	ld	a,([ival],y).w	;a ; 91 D6 10
	ld	a,([ival],y).b	;  ; 91 D6 10
	ld	a,([ival],y)	;  ; 91 D6 10

	; S_LDF
	ldf	a,(ival,x).e	;  ; AF 00 00 10
	ldf	a,(ival,x).w	;a ; AF 00 00 10
	ldf	a,(ival,x).b	;a ; AF 00 00 10
	ldf	a,(ival,x)	;  ; AF 00 00 10

	ldf	a,[ival].e	;  ; 92 BC 00 10
	ldf	a,[ival].w	;a ; 92 BC 00 10
	ldf	a,[ival].b	;a ; 92 BC 00 10
	ldf	a,[ival]	;  ; 92 BC 00 10

	ldf	a,([ival].e,x)	;  ; 92 AF 00 10
	ldf	a,([ival].w,x)	;a ; 92 AF 00 10
	ldf	a,([ival].b,x)	;a ; 92 AF 00 10
	ldf	a,([ival],x)	;  ; 92 AF 00 10

	ldf	a,([ival],x).e	;  ; 92 AF 00 10
	ldf	a,([ival],x).w	;a ; 92 AF 00 10
	ldf	a,([ival],x).b	;a ; 92 AF 00 10
	ldf	a,([ival],x)	;  ; 92 AF 00 10

	; S_LDW
	; ldw  x,---
	ldw	x,(ival,x).e	;a ; DE 00 10
	ldw	x,(ival,x).w	;  ; DE 00 10
	ldw	x,(ival,x).b	;  ; EE 10
	ldw	x,(ival,x)	;  ; EE 10

	ldw	x,(ival,sp).e	;a ; 1E 10
	ldw	x,(ival,sp).w	;a ; 1E 10
	ldw	x,(ival,sp).b	;  ; 1E 10
	ldw	x,(ival,sp)	;  ; 1E 10

	ldw	x,[ival].e	;a ; 72 CE 00 10
	ldw	x,[ival].w	;  ; 72 CE 00 10
	ldw	x,[ival].b	;  ; 92 CE 10
	ldw	x,[ival]	;  ; 92 CE 10

	ldw	x,([ival].e,x)	;a ; 72 DE 00 10
	ldw	x,([ival].w,x)	;  ; 72 DE 00 10
	ldw	x,([ival].b,x)	;  ; 92 DE 10
	ldw	x,([ival],x)	;  ; 92 DE 10

	ldw	x,([ival],x).e	;a ; 72 DE 00 10
	ldw	x,([ival],x).w	;  ; 72 DE 00 10
	ldw	x,([ival],x).b	;  ; 92 DE 10
	ldw	x,([ival],x)	;  ; 92 DE 10

	; ldw  y,---
	ldw	y,(ival,y).e	;a ; 90 DE 00 10
	ldw	y,(ival,y).w	;  ; 90 DE 00 10
	ldw	y,(ival,y).b	;  ; 90 EE 10
	ldw	y,(ival,y)	;  ; 90 EE 10

	ldw	y,(ival,sp).e	;a ; 16 10
	ldw	y,(ival,sp).w	;a ; 16 10
	ldw	y,(ival,sp).b	;  ; 16 10
	ldw	y,(ival,sp)	;  ; 16 10

	ldw	y,[ival].e	;a ; 91 CE 10
	ldw	y,[ival].w	;a ; 91 CE 10
	ldw	y,[ival].b	;  ; 91 CE 10
	ldw	y,[ival]	;  ; 91 CE 10

	ldw	y,([ival].e,y)	;a ; 91 DE 10
	ldw	y,([ival].w,y)	;a ; 91 DE 10
	ldw	y,([ival].b,y)	;  ; 91 DE 10
	ldw	y,([ival],y)	;  ; 91 DE 10

	ldw	y,([ival],y).e	;a ; 91 DE 10
	ldw	y,([ival],y).w	;a ; 91 DE 10
	ldw	y,([ival],y).b	;  ; 91 DE 10
	ldw	y,([ival],y)	;  ; 91 DE 10

	; ldw  ---,x
	ldw	(ival,y).e,x	;a ; 90 DF 00 10
	ldw	(ival,y).w,x	;  ; 90 DF 00 10
	ldw	(ival,y).b,x	;  ; 90 EF 10
	ldw	(ival,y),x	;  ; 90 EF 10

	ldw	(ival,sp).e,x	;a ; 1F 10
	ldw	(ival,sp).w,x	;a ; 1F 10
	ldw	(ival,sp).b,x	;  ; 1F 10
	ldw	(ival,sp),x	;  ; 1F 10

	ldw	[ival].e,x	;a ; 72 CF 00 10
	ldw	[ival].w,x	;  ; 72 CF 00 10
	ldw	[ival].b,x	;  ; 92 CF 10
	ldw	[ival],x	;  ; 92 CF 10

	ldw	([ival].e,y),x	;a ; 91 DF 10
	ldw	([ival].w,y),x	;a ; 91 DF 10
	ldw	([ival].b,y),x	;  ; 91 DF 10
	ldw	([ival],y),x	;  ; 91 DF 10

	ldw	([ival],y).e,x	;a ; 91 DF 10
	ldw	([ival],y).w,x	;a ; 91 DF 10
	ldw	([ival],y).b,x	;  ; 91 DF 10
	ldw	([ival],y),x	;  ; 91 DF 10

	; ldw  ---,y
	ldw	(ival,x).e,y	;a ; DF 00 10
	ldw	(ival,x).w,y	;  ; DF 00 10
	ldw	(ival,x).b,y	;  ; EF 10
	ldw	(ival,x),y	;  ; EF 10

	ldw	(ival,sp).e,y	;a ; 17 10
	ldw	(ival,sp).w,y	;a ; 17 10
	ldw	(ival,sp).b,y	;  ; 17 10
	ldw	(ival,sp),y	;  ; 17 10

	ldw	[ival].e,y	;a ; 91 CF 10
	ldw	[ival].w,y	;a ; 91 CF 10
	ldw	[ival].b,y	;  ; 91 CF 10
	ldw	[ival],y	;  ; 91 CF 10

	ldw	([ival].e,x),y	;a ; 72 DF 00 10
	ldw	([ival].w,x),y	;  ; 72 DF 00 10
	ldw	([ival].b,x),y	;  ; 92 DF 10
	ldw	([ival],x),y	;  ; 92 DF 10

	ldw	([ival],x).e,y	;a ; 72 DF 00 10
	ldw	([ival],x).w,y	;  ; 72 DF 00 10
	ldw	([ival],x).b,y	;  ; 92 DF 10
	ldw	([ival],x),y	;  ; 92 DF 10

	; S_CLJP
	call	(ival,x).e	;a ; DD 00 10
	call	(ival,x).w	;  ; DD 00 10
	call	(ival,x).b	;  ; ED 10
	call	(ival,x)	;  ; ED 10

	call	[ival].e	;a ; 72 CD 00 10
	call	[ival].w	;  ; 72 CD 00 10
	call	[ival].b	;  ; 92 CD 10
	call	[ival]		;  ; 92 CD 10

	call	([ival].e,x)	;a ; 72 DD 00 10
	call	([ival].w,x)	;  ; 72 DD 00 10
	call	([ival].b,x)	;  ; 92 DD 10
	call	([ival],x)	;  ; 92 DD 10

	call	([ival],x).e	;a ; 72 DD 00 10
	call	([ival],x).w	;  ; 72 DD 00 10
	call	([ival],x).b	;  ; 92 DD 10
	call	([ival],x)	;  ; 92 DD 10

	; S_CLJPF
	callf	[ival].e	;  ; 92 8D 00 10
	callf	[ival].w	;a ; 92 8D 00 10
	callf	[ival].b	;a ; 92 8D 00 10
	callf	[ival]		;  ; 92 8D 00 10




	.end

