	.title	Test of Z80 / HD64180 / eZ80 assembler

	.ez80
	.adl	1

	offset	=	0x55		;arbitrary constants
	n	=	0x20
	nn	=	0x0584

	; notes:
	;	Leading 'a' operand is optional.
	;	If offset is ommitted 0 is assumed.

	;***********************************************************
	; add with carry to 'a'
	adc	a,(hl)			;8E
	adc.s	a,(hl)			;52 8E
	adc	a,ixh			;DD 8C
	adc	a,ixl			;DD 8D
	adc	a,iyh			;FD 8C
	adc	a,iyl			;FD 8D
	adc	a,offset(ix)		;DD 8E 55
	adc.s	a,offset(ix)		;52 DD 8E 55
	adc	a,offset(iy)		;FD 8E 55
	adc.s	a,offset(iy)		;52 FD 8E 55
	adc	a,a			;8F
	adc	a,b			;88
	adc	a,c			;89
	adc	a,d			;8A
	adc	a,e			;8B
	adc	a,h			;8C
	adc	a,l			;8D
	adc	a,#n			;CE 20
	;***********************************************************
	adc	(hl)			;8E
	adc.s	(hl)			;52 8E
	adc	ixh			;DD 8C
	adc	ixl			;DD 8D
	adc	iyh			;FD 8C
	adc	iyl			;FD 8D
	adc	offset(ix)		;DD 8E 55
	adc.s	offset(ix)		;52 DD 8E 55
	adc	offset(iy)		;FD 8E 55
	adc.s	offset(iy)		;52 FD 8E 55
	adc	a			;8F
	adc	b			;88
	adc	c			;89
	adc	d			;8A
	adc	e			;8B
	adc	h			;8C
	adc	l			;8D
	adc	#n			;CE 20
	;***********************************************************
	; add with carry register pair to 'hl'
	adc	hl,bc			;ED 4A
	adc	hl,de			;ED 5A
	adc	hl,hl			;ED 6A
	adc.s	hl,bc			;52 ED 4A
	adc.s	hl,de			;52 ED 5A
	adc.s	hl,hl			;52 ED 6A
	adc	hl,sp			;ED 7A
	adc.s	hl,sp			;52 ED 7A
	;***********************************************************
	; add operand to 'a'
	add	a,(hl)			;86
	add.s	a,(hl)			;52 86
	add	a,ixh			;DD 84
	add	a,ixl			;DD 85
	add	a,iyh			;FD 84
	add	a,iyl			;FD 85
	add	a,offset(ix)		;DD 86 55
	add.s	a,offset(ix)		;52 DD 86 55
	add	a,offset(iy)		;FD 86 55
	add.s	a,offset(iy)		;52 FD 86 55
	add	a,a			;87
	add	a,b			;80
	add	a,c			;81
	add	a,d			;82
	add	a,e			;83
	add	a,h			;84
	add	a,l			;85
	add	a,#n			;C6 20
	;***********************************************************
	; add register pair to 'hl'
	add	hl,bc			;09
	add	hl,de			;19
	add	hl,hl			;29
	add.s	hl,bc			;52 09
	add.s	hl,de			;52 19
	add.s	hl,hl			;52 29
	add	hl,sp			;39
	add.s	hl,sp			;52 39
	;***********************************************************
	; add register pair to 'ix'
	add	ix,bc			;DD 09
	add	ix,de			;DD 19
	add	ix,ix			;DD 29
	add.s	ix,bc			;52 DD 09
	add.s	ix,de			;52 DD 19
	add.s	ix,ix			;52 DD 29
	add	ix,sp			;DD 39
	add.s	ix,sp			;52 DD 39
	;***********************************************************
	; add register pair to 'iy'
	add	iy,bc			;FD 09
	add	iy,de			;FD 19
	add	iy,iy			;FD 29
	add.s	iy,bc			;52 FD 09
	add.s	iy,de			;52 FD 19
	add.s	iy,iy			;52 FD 29
	add	iy,sp			;FD 39
	add.s	iy,sp			;52 FD 39
	;***********************************************************
	; logical 'and' operand with 'a'
	and	a,(hl)			;A6
	and.s	a,(hl)			;52 A6
	and	a,ixh			;DD A4
	and	a,ixl			;DD A5
	and	a,iyh			;FD A4
	and	a,iyl			;FD A5
	and	a,offset(ix)		;DD A6 55
	and.s	a,offset(ix)		;52 DD A6 55
	and	a,offset(iy)		;FD A6 55
	and.s	a,offset(iy)		;52 FD A6 55
	and	a,a			;A7
	and	a,b			;A0
	and	a,c			;A1
	and	a,d			;A2
	and	a,e			;A3
	and	a,h			;A4
	and	a,l			;A5
	and	a,#n			;E6 20
	;***********************************************************
	; test bit of location or register
	bit	0,(hl)			;CB 46
	bit.s	0,(hl)			;52 CB 46
	bit	0,offset(ix)		;DD CB 55 46
	bit.s	0,offset(ix)		;52 DD CB 55 46
	bit	0,offset(iy)		;FD CB 55 46
	bit.s	0,offset(iy)		;52 FD CB 55 46
	bit	0,a			;CB 47
	bit	0,b			;CB 40
	bit	0,c			;CB 41
	bit	0,d			;CB 42
	bit	0,e			;CB 43
	bit	0,h			;CB 44
	bit	0,l			;CB 45
	bit	1,(hl)			;CB 4E
	bit.s	1,(hl)			;52 CB 4E
	bit	1,offset(ix)		;DD CB 55 4E
	bit.s	1,offset(ix)		;52 DD CB 55 4E
	bit	1,offset(iy)		;FD CB 55 4E
	bit.s	1,offset(iy)		;52 FD CB 55 4E
	bit	1,a			;CB 4F
	bit	1,b			;CB 48
	bit	1,c			;CB 49
	bit	1,d			;CB 4A
	bit	1,e			;CB 4B
	bit	1,h			;CB 4C
	bit	1,l			;CB 4D
	bit	2,(hl)			;CB 56
	bit.s	2,(hl)			;52 CB 56
	bit	2,offset(ix)		;DD CB 55 56
	bit.s	2,offset(ix)		;52 DD CB 55 56
	bit	2,offset(iy)		;FD CB 55 56
	bit.s	2,offset(iy)		;52 FD CB 55 56
	bit	2,a			;CB 57
	bit	2,b			;CB 50
	bit	2,c			;CB 51
	bit	2,d			;CB 52
	bit	2,e			;CB 53
	bit	2,h			;CB 54
	bit	2,l			;CB 55
	bit	3,(hl)			;CB 5E
	bit.s	3,(hl)			;52 CB 5E
	bit	3,offset(ix)		;DD CB 55 5E
	bit.s	3,offset(ix)		;52 DD CB 55 5E
	bit	3,offset(iy)		;FD CB 55 5E
	bit.s	3,offset(iy)		;52 FD CB 55 5E
	bit	3,a			;CB 5F
	bit	3,b			;CB 58
	bit	3,c			;CB 59
	bit	3,d			;CB 5A
	bit	3,e			;CB 5B
	bit	3,h			;CB 5C
	bit	3,l			;CB 5D
	bit	4,(hl)			;CB 66
	bit.s	4,(hl)			;52 CB 66
	bit	4,offset(ix)		;DD CB 55 66
	bit.s	4,offset(ix)		;52 DD CB 55 66
	bit	4,offset(iy)		;FD CB 55 66
	bit.s	4,offset(iy)		;52 FD CB 55 66
	bit	4,a			;CB 67
	bit	4,b			;CB 60
	bit	4,c			;CB 61
	bit	4,d			;CB 62
	bit	4,e			;CB 63
	bit	4,h			;CB 64
	bit	4,l			;CB 65
	bit	5,(hl)			;CB 6E
	bit.s	5,(hl)			;52 CB 6E
	bit	5,offset(ix)		;DD CB 55 6E
	bit.s	5,offset(ix)		;52 DD CB 55 6E
	bit	5,offset(iy)		;FD CB 55 6E
	bit.s	5,offset(iy)		;52 FD CB 55 6E
	bit	5,a			;CB 6F
	bit	5,b			;CB 68
	bit	5,c			;CB 69
	bit	5,d			;CB 6A
	bit	5,e			;CB 6B
	bit	5,h			;CB 6C
	bit	5,l			;CB 6D
	bit	6,(hl)			;CB 76
	bit.s	6,(hl)			;52 CB 76
	bit	6,offset(ix)		;DD CB 55 76
	bit.s	6,offset(ix)		;52 DD CB 55 76
	bit	6,offset(iy)		;FD CB 55 76
	bit.s	6,offset(iy)		;52 FD CB 55 76
	bit	6,a			;CB 77
	bit	6,b			;CB 70
	bit	6,c			;CB 71
	bit	6,d			;CB 72
	bit	6,e			;CB 73
	bit	6,h			;CB 74
	bit	6,l			;CB 75
	bit	7,(hl)			;CB 7E
	bit.s	7,(hl)			;52 CB 7E
	bit	7,offset(ix)		;DD CB 55 7E
	bit.s	7,offset(ix)		;52 DD CB 55 7E
	bit	7,offset(iy)		;FD CB 55 7E
	bit.s	7,offset(iy)		;52 FD CB 55 7E
	bit	7,a			;CB 7F
	bit	7,b			;CB 78
	bit	7,c			;CB 79
	bit	7,d			;CB 7A
	bit	7,e			;CB 7B
	bit	7,h			;CB 7C
	bit	7,l			;CB 7D
	;***********************************************************
	; call subroutine at nn if condition is true
	call	C,nn			;DC 84 05 00
	call.is	C,nn			;49 DC 84 05
	call	M,nn			;FC 84 05 00
	call.is	M,nn			;49 FC 84 05
	call	NC,nn			;D4 84 05 00
	call.is	NC,nn			;49 D4 84 05
	call	NZ,nn			;C4 84 05 00
	call.is	NZ,nn			;49 C4 84 05
	call	P,nn			;F4 84 05 00
	call.is	P,nn			;49 F4 84 05
	call	PE,nn			;EC 84 05 00
	call.is	PE,nn			;49 EC 84 05
	call	PO,nn			;E4 84 05 00
	call.is	PO,nn			;49 E4 84 05
	call	Z,nn			;CC 84 05 00
	call.is	Z,nn			;49 CC 84 05
	;***********************************************************
	; unconditional call to subroutine at nn
	call	nn			;CD 84 05 00
	call.is	nn			;49 CD 84 05
	;***********************************************************
	; complement carry flag
	ccf				;3F
	;***********************************************************
	; compare operand with 'a'
	cp	a,(hl)			;BE
	cp.s	a,(hl)			;52 BE
	cp	a,ixh			;DD BC
	cp	a,ixl			;DD BD
	cp	a,iyh			;FD BC
	cp	a,iyl			;FD BD
	cp	a,offset(ix)		;DD BE 55
	cp.s	a,offset(ix)		;52 DD BE 55
	cp	a,offset(iy)		;FD BE 55
	cp.s	a,offset(iy)		;52 FD BE 55
	cp	a,a			;BF
	cp	a,b			;B8
	cp	a,c			;B9
	cp	a,d			;BA
	cp	a,e			;BB
	cp	a,h			;BC
	cp	a,l			;BD
	cp	a,#n			;FE 20
	;***********************************************************
	; compare location (hl) and 'a'
	; decrement 'hl' and 'bc'
	cpd				;ED A9
	cpd.s				;52 ED A9
	;***********************************************************
	; compare location (hl) and 'a'
	; decrement 'hl' and 'bc'
	; repeat until 'bc' = 0
	cpdr				;ED B9
	cpdr.s				;52 ED B9
	;***********************************************************
	; compare location (hl) and 'a'
	; increment 'hl' and decrement 'bc'
	cpi				;ED A1
	cpi.s				;52 ED A1
	;***********************************************************
	; compare location (hl) and 'a'
	; increment 'hl' and decrement 'bc'
	; repeat until 'bc' = 0
	cpir				;ED B1
	cpir.s				;52 ED B1
	;***********************************************************
	; 1's complement of 'a'
	cpl				;2F
	;***********************************************************
	; decimal adjust 'a'
	daa				;27
	;***********************************************************
	; decrement operand
	dec	(hl)			;35
	dec.s	(hl)			;52 35
	dec	ixh			;DD 25
	dec	ixl			;DD 2D
	dec	iyh			;FD 25
	dec	iyl			;FD 2D
	dec	offset(ix)		;DD 35 55
	dec.s	offset(ix)		;52 DD 35 55
	dec	offset(iy)		;FD 35 55
	dec.s	offset(iy)		;52 FD 35 55
	dec	a			;3D
	dec	b			;05
	dec	bc			;0B
	dec.s	bc			;52 0B
	dec	c			;0D
	dec	d			;15
	dec	de			;1B
	dec.s	de			;52 1B
	dec	e			;1D
	dec	h			;25
	dec	hl			;2B
	dec.s	hl			;52 2B
	dec	ix			;DD 2B
	dec.s	ix			;52 DD 2B
	dec	iy			;FD 2B
	dec.s	iy			;52 FD 2B
	dec	l			;2D
	dec	sp			;3B
	dec.s	sp			;52 3B
	;***********************************************************
	; disable interrupts
	di				;F3
	;***********************************************************
	; decrement b and jump relative if b # 0
	djnz	.+0x12			;10 10
	;***********************************************************
	; enable interrupts
	ei				;FB
	;***********************************************************
	; exchange location and (sp)
	ex	(sp),hl			;E3
	ex.s	(sp),hl			;52 E3
	ex	(sp),ix			;DD E3
	ex.s	(sp),ix			;52 DD E3
	ex	(sp),iy			;FD E3
	ex.s	(sp),iy			;52 FD E3
	;***********************************************************
	; exchange af and af'
	ex	af,af'			;08
	;***********************************************************
	; exchange de and hl
	ex	de,hl			;EB
	;***********************************************************
	; exchange:
	;	bc <-> bc'
	;	de <-> de'
	;	hl <-> hl'
	exx				;D9
	;***********************************************************
	; halt (wait for interrupt or reset)
	halt				;76
	;***********************************************************
	; set interrupt mode
	im	0			;ED 46
	im	1			;ED 56
	im	2			;ED 5E
	;***********************************************************
	; load 'a' with input from device n
	in	a,(n)			;DB 20
	;***********************************************************
	; load register with input from (c)
	in	a,(c)			;ED 78
	in	b,(c)			;ED 40
	in	c,(c)			;ED 48
	in	d,(c)			;ED 50
	in	e,(c)			;ED 58
	in	h,(c)			;ED 60
	in	l,(c)			;ED 68
	;***********************************************************
	; load register with input from port (n)
	in0	a,(n)			;ED 38 20
	in0	b,(n)			;ED 00 20
	in0	c,(n)			;ED 08 20
	in0	d,(n)			;ED 10 20
	in0	e,(n)			;ED 18 20
	in0	h,(n)			;ED 20 20
	in0	l,(n)			;ED 28 20
	;***********************************************************
	; increment operand
	inc	(hl)			;34
	inc.s	(hl)			;52 34
	inc	ixh			;DD 24
	inc	ixl			;DD 2C
	inc	iyh			;FD 24
	inc	iyl			;FD 2C
	inc	offset(ix)		;DD 34 55
	inc.s	offset(ix)		;52 DD 34 55
	inc	offset(iy)		;FD 34 55
	inc.s	offset(iy)		;52 FD 34 55
	inc	a			;3C
	inc	b			;04
	inc	bc			;03
	inc.s	bc			;52 03
	inc	c			;0C
	inc	d			;14
	inc	de			;13
	inc.s	de			;52 13
	inc	e			;1C
	inc	h			;24
	inc	hl			;23
	inc.s	hl			;52 23
	inc	ix			;DD 23
	inc.s	ix			;52 DD 23
	inc	iy			;FD 23
	inc.s	iy			;52 FD 23
	inc	l			;2C
	inc	sp			;33
	inc.s	sp			;52 33
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; decrement 'hl' and 'b'
	ind				;ED AA
	ind.s				;52 ED AA
	;***********************************************************
	ind2				;ED 8C
	ind2.s				;52 ED 8C
	;***********************************************************
	ind2r				;ED 9C
	ind2r.s				;52 ED 9C
	;***********************************************************
	indm				;ED 8A
	indm.s				;52 ED 8A
	;***********************************************************
	indmr				;ED 9A
	indmr.s				;52 ED 9A
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; decrement 'hl' and 'b'
	; repeat until 'b' = 0
	indr				;ED BA
	indr.s				;52 ED BA
	;***********************************************************
	indrx				;ED CA
	indrx.s				;52 ED CA
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; increment 'hl' and decrement 'b'
	ini				;ED A2
	ini.s				;52 ED A2
	;***********************************************************
	ini2				;ED 84
	ini2.s				;52 ED 84
	;***********************************************************
	ini2r				;ED 94
	ini2r.s				;52 ED 94
	;***********************************************************
	inim				;ED 82
	inim.s				;52 ED 82
	;***********************************************************
	inimr				;ED 92
	inimr.s				;52 ED 92
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; increment 'hl' and decrement 'b'
	; repeat until 'b' = 0
	inir				;ED B2
	inir.s				;52 ED B2
	;***********************************************************
	inirx				;ED C2
	inirx.s				;52 ED C2
	;***********************************************************
	; unconditional jump to location nn
	jp	nn			;C3 84 05 00
	jp.sis	nn			;49 C3 84 05
	jp	(hl)			;E9
	jp.s	(hl)			;52 E9
	jp	(ix)			;DD E9
	jp.s	(ix)			;52 DD E9
	jp	(iy)			;FD E9
	jp.s	(iy)			;52 FD E9
	;***********************************************************
	; jump to location if condition is true
	jp	C,nn			;DA 84 05 00
	jp.sis	C,nn			;49 DA 84 05
	jp	M,nn			;FA 84 05 00
	jp.sis	M,nn			;49 FA 84 05
	jp	NC,nn			;D2 84 05 00
	jp.sis	NC,nn			;49 D2 84 05
	jp	NZ,nn			;C2 84 05 00
	jp.sis	NZ,nn			;49 C2 84 05
	jp	P,nn			;F2 84 05 00
	jp.sis	P,nn			;49 F2 84 05
	jp	PE,nn			;EA 84 05 00
	jp.sis	PE,nn			;49 EA 84 05
	jp	PO,nn			;E2 84 05 00
	jp.sis	PO,nn			;49 E2 84 05
	jp	Z,nn			;CA 84 05 00
	jp.sis	Z,nn			;49 CA 84 05
	;***********************************************************
	; unconditional jump relative to PC+e
	jr	jr1+0x10		;18 10
	;***********************************************************
	; jump relative to PC+e if condition is true
jr1:	jr	C,jr2+0x10		;38 10
jr2:	jr	NC,jr3+0x10		;30 10
jr3:	jr	NZ,jr4+0x10		;20 10
jr4:	jr	Z,jr5+0x10		;28 10
jr5:
	;***********************************************************
	; load source to destination
	ld	a,(hl)			;7E
	ld.s	a,(hl)			;52 7E
	ld	a,offset(ix)		;DD 7E 55
	ld.s	a,offset(ix)		;52 DD 7E 55
	ld	a,offset(iy)		;FD 7E 55
	ld.s	a,offset(iy)		;52 FD 7E 55
	ld	a,a			;7F
	ld	a,b			;78
	ld	a,c			;79
	ld	a,d			;7A
	ld	a,e			;7B
	ld	a,h			;7C
	ld	a,l			;7D
	ld	a,#n			;3E 20
	ld	b,(hl)			;46
	ld.s	b,(hl)			;52 46
	ld	b,offset(ix)		;DD 46 55
	ld.s	b,offset(ix)		;52 DD 46 55
	ld	b,offset(iy)		;FD 46 55
	ld.s	b,offset(iy)		;52 FD 46 55
	ld	b,a			;47
	ld	b,b			;40
	ld	b,c			;41
	ld	b,d			;42
	ld	b,e			;43
	ld	b,h			;44
	ld	b,l			;45
	ld	b,#n			;06 20
	ld	c,(hl)			;4E
	ld.s	c,(hl)			;52 4E
	ld	c,offset(ix)		;DD 4E 55
	ld.s	c,offset(ix)		;52 DD 4E 55
	ld	c,offset(iy)		;FD 4E 55
	ld.s	c,offset(iy)		;52 FD 4E 55
	ld	c,a			;4F
	ld	c,b			;48
	ld	c,c			;49
	ld	c,d			;4A
	ld	c,e			;4B
	ld	c,h			;4C
	ld	c,l			;4D
	ld	c,#n			;0E 20
	ld	d,(hl)			;56
	ld.s	d,(hl)			;52 56
	ld	d,offset(ix)		;DD 56 55
	ld.s	d,offset(ix)		;52 DD 56 55
	ld	d,offset(iy)		;FD 56 55
	ld.s	d,offset(iy)		;52 FD 56 55
	ld	d,a			;57
	ld	d,b			;50
	ld	d,c			;51
	ld	d,d			;52
	ld	d,e			;53
	ld	d,h			;54
	ld	d,l			;55
	ld	d,#n			;16 20
	ld	e,(hl)			;5E
	ld.s	e,(hl)			;52 5E
	ld	e,offset(ix)		;DD 5E 55
	ld.s	e,offset(ix)		;52 DD 5E 55
	ld	e,offset(iy)		;FD 5E 55
	ld.s	e,offset(iy)		;52 FD 5E 55
	ld	e,a			;5F
	ld	e,b			;58
	ld	e,c			;59
	ld	e,d			;5A
	ld	e,e			;5B
	ld	e,h			;5C
	ld	e,l			;5D
	ld	e,#n			;1E 20
	ld	h,(hl)			;66
	ld.s	h,(hl)			;52 66
	ld	h,offset(ix)		;DD 66 55
	ld.s	h,offset(ix)		;52 DD 66 55
	ld	h,offset(iy)		;FD 66 55
	ld.s	h,offset(iy)		;52 FD 66 55
	ld	h,a			;67
	ld	h,b			;60
	ld	h,c			;61
	ld	h,d			;62
	ld	h,e			;63
	ld	h,h			;64
	ld	h,l			;65
	ld	h,#n			;26 20
	ld	l,(hl)			;6E
	ld.s	l,(hl)			;52 6E
	ld	l,offset(ix)		;DD 6E 55
	ld.s	l,offset(ix)		;52 DD 6E 55
	ld	l,offset(iy)		;FD 6E 55
	ld.s	l,offset(iy)		;52 FD 6E 55
	ld	l,a			;6F
	ld	l,b			;68
	ld	l,c			;69
	ld	l,d			;6A
	ld	l,e			;6B
	ld	l,h			;6C
	ld	l,l			;6D
	ld	l,#n			;2E 20
	;***********************************************************
	ld	i,a			;ED 47
	ld	r,a			;ED 4F
	ld	a,i			;ED 57
	ld	a,r			;ED 5F
	ld	a,mb			;ED 6E
	ld	mb,a			;ED 6D
	ld	hl,i			;ED D7
	ld	i,hl			;ED C7
	;***********************************************************
	ld	(bc),a			;02
	ld.s	(bc),a			;52 02
	ld	(de),a			;12
	ld.s	(de),a			;52 12
	ld	a,(bc)			;0A
	ld.s	a,(bc)			;52 0A
	ld	a,(de)			;1A
	ld.s	a,(de)			;52 1A
	;***********************************************************
	ld	(hl),a			;77
	ld.s	(hl),a			;52 77
	ld	(hl),b			;70
	ld.s	(hl),b			;52 70
	ld	(hl),c			;71
	ld.s	(hl),c			;52 71
	ld	(hl),d			;72
	ld.s	(hl),d			;52 72
	ld	(hl),e			;73
	ld.s	(hl),e			;52 73
	ld	(hl),h			;74
	ld.s	(hl),h			;52 74
	ld	(hl),l			;75
	ld.s	(hl),l			;52 75
	ld	(hl),#n			;36 20
	ld.s	(hl),#n			;52 36 20
	;***********************************************************
	ld	offset(ix),a		;DD 77 55
	ld.s	offset(ix),a		;52 DD 77 55
	ld	offset(ix),b		;DD 70 55
	ld.s	offset(ix),b		;52 DD 70 55
	ld	offset(ix),c		;DD 71 55
	ld.s	offset(ix),c		;52 DD 71 55
	ld	offset(ix),d		;DD 72 55
	ld.s	offset(ix),d		;52 DD 72 55
	ld	offset(ix),e		;DD 73 55
	ld.s	offset(ix),e		;52 DD 73 55
	ld	offset(ix),h		;DD 74 55
	ld.s	offset(ix),h		;52 DD 74 55
	ld	offset(ix),l		;DD 75 55
	ld.s	offset(ix),l		;52 DD 75 55
	ld	offset(ix),#n		;DD 36 55 20
	ld.s	offset(ix),#n		;52 DD 36 55 20
	;***********************************************************
	ld	offset(iy),a		;FD 77 55
	ld.s	offset(iy),a		;52 FD 77 55
	ld	offset(iy),b		;FD 70 55
	ld.s	offset(iy),b		;52 FD 70 55
	ld	offset(iy),c		;FD 71 55
	ld.s	offset(iy),c		;52 FD 71 55
	ld	offset(iy),d		;FD 72 55
	ld.s	offset(iy),d		;52 FD 72 55
	ld	offset(iy),e		;FD 73 55
	ld.s	offset(iy),e		;52 FD 73 55
	ld	offset(iy),h		;FD 74 55
	ld.s	offset(iy),h		;52 FD 74 55
	ld	offset(iy),l		;FD 75 55
	ld.s	offset(iy),l		;52 FD 75 55
	ld	offset(iy),#n		;FD 36 55 20
	ld.s	offset(iy),#n		;52 FD 36 55 20
	;***********************************************************
	ld	(nn),a			;32 84 05 00
	ld.is	(nn),a			;49 32 84 05
	ld	(nn),bc			;ED 43 84 05 00
	ld.is	(nn),bc			;49 ED 43 84 05
	ld	(nn),de			;ED 53 84 05 00
	ld.is	(nn),de			;49 ED 53 84 05
	ld	(nn),hl			;22 84 05 00
	ld.is	(nn),hl			;49 22 84 05
	ld	(nn),sp			;ED 73 84 05 00
	ld.is	(nn),sp			;49 ED 73 84 05
	ld	(nn),ix			;DD 22 84 05 00
	ld.is	(nn),ix			;49 DD 22 84 05
	ld	(nn),iy			;FD 22 84 05 00
	ld.is	(nn),iy			;49 FD 22 84 05
	;***********************************************************
	ld	a,(nn)			;3A 84 05 00
	ld.is	a,(nn)			;49 3A 84 05
	ld	bc,(nn)			;ED 4B 84 05 00
	ld.is	bc,(nn)			;49 ED 4B 84 05
	ld	de,(nn)			;ED 5B 84 05 00
	ld.is	de,(nn)			;49 ED 5B 84 05
	ld	hl,(nn)			;2A 84 05 00
	ld.is	hl,(nn)			;49 2A 84 05
	ld	sp,(nn)			;ED 7B 84 05 00
	ld.is	sp,(nn)			;49 ED 7B 84 05
	ld	ix,(nn)			;DD 2A 84 05 00
	ld.is	ix,(nn)			;49 DD 2A 84 05
	ld	iy,(nn)			;FD 2A 84 05 00
	ld.is	iy,(nn)			;49 FD 2A 84 05
	;***********************************************************
	ld	bc,#nn			;01 84 05 00
	ld.is	bc,#nn			;49 01 84 05
	ld	de,#nn			;11 84 05 00
	ld.is	de,#nn			;49 11 84 05
	ld	hl,#nn			;21 84 05 00
	ld.is	hl,#nn			;49 21 84 05
	ld	sp,#nn			;31 84 05 00
	ld.is	sp,#nn			;49 31 84 05
	ld	ix,#nn			;DD 21 84 05 00
	ld.is	ix,#nn			;49 DD 21 84 05
	ld	iy,#nn			;FD 21 84 05 00
	ld.is	iy,#nn			;49 FD 21 84 05
	;***********************************************************
	ld	sp,hl			;F9
	ld	sp,ix			;DD F9
	ld	sp,iy			;FD F9
	;***********************************************************
	ld	(hl),ix			;ED 3F
	ld.s	(hl),ix			;52 ED 3F
	ld	(hl),iy			;ED 3E
	ld.s	(hl),iy			;52 ED 3E
	ld	(hl),bc			;ED 0F
	ld.s	(hl),bc			;52 ED 0F
	ld	(hl),de			;ED 1F
	ld.s	(hl),de			;52 ED 1F
	ld	(hl),hl			;ED 2F
	ld.s	(hl),hl			;52 ED 2F
	ld	bc,(hl)			;ED 07
	ld.s	bc,(hl)			;52 ED 07
	ld	de,(hl)			;ED 17
	ld.s	de,(hl)			;52 ED 17
	ld	hl,(hl)			;ED 27
	ld.s	hl,(hl)			;52 ED 27
	ld	ix,(hl)			;ED 37
	ld.s	ix,(hl)			;52 ED 37
	ld	iy,(hl)			;ED 31
	ld.s	iy,(hl)			;52 ED 31
	;***********************************************************
	ld	ixh,ixh			;DD 64
	ld	ixh,ixl			;DD 65
	ld	ixl,ixh			;DD 6C
	ld	ixl,ixl			;DD 6D
	ld	iyh,iyh			;FD 64
	ld	iyh,iyl			;FD 65
	ld	iyl,iyh			;FD 6C
	ld	iyl,iyl			;FD 6D
	ld	ixh,#n			;DD 26 20
	ld	ixl,#n			;DD 2E 20
	ld	iyh,#n			;FD 26 20
	ld	iyl,#n			;FD 2E 20
	ld	ixh,a			;DD 67
	ld	ixh,b			;DD 60
	ld	ixh,c			;DD 61
	ld	ixh,d			;DD 62
	ld	ixh,e			;DD 63
	ld	ixl,a			;DD 6F
	ld	ixl,b			;DD 68
	ld	ixl,c			;DD 69
	ld	ixl,d			;DD 6A
	ld	ixl,e			;DD 6B
	ld	iyh,a			;FD 67
	ld	iyh,b			;FD 60
	ld	iyh,c			;FD 61
	ld	iyh,d			;FD 62
	ld	iyh,e			;FD 63
	ld	iyl,a			;FD 6F
	ld	iyl,b			;FD 68
	ld	iyl,c			;FD 69
	ld	iyl,d			;FD 6A
	ld	iyl,e			;FD 6B
	ld	a,ixh			;DD 7C
	ld	a,ixl			;DD 7D
	ld	a,iyh			;FD 7C
	ld	a,iyl			;FD 7D
	ld	b,ixh			;DD 44
	ld	b,ixl			;DD 45
	ld	b,iyh			;FD 44
	ld	b,iyl			;FD 45
	ld	c,ixh			;DD 4C
	ld	c,ixl			;DD 4D
	ld	c,iyh			;FD 4C
	ld	c,iyl			;FD 4D
	ld	d,ixh			;DD 54
	ld	d,ixl			;DD 55
	ld	d,iyh			;FD 54
	ld	d,iyl			;FD 55
	ld	e,ixh			;DD 5C
	ld	e,ixl			;DD 5D
	ld	e,iyh			;FD 5C
	ld	e,iyl			;FD 5D
	;***********************************************************
	ld	bc,offset(ix)		;DD 07 55
	ld.s	bc,offset(ix)		;52 DD 07 55
	ld	de,offset(ix)		;DD 17 55
	ld.s	de,offset(ix)		;52 DD 17 55
	ld	hl,offset(ix)		;DD 27 55
	ld.s	hl,offset(ix)		;52 DD 27 55
	ld	bc,offset(iy)		;FD 07 55
	ld.s	bc,offset(iy)		;52 FD 07 55
	ld	de,offset(iy)		;FD 17 55
	ld.s	de,offset(iy)		;52 FD 17 55
	ld	hl,offset(iy)		;FD 27 55
	ld.s	hl,offset(iy)		;52 FD 27 55
	ld	ix,offset(ix)		;DD 37 55
	ld.s	ix,offset(ix)		;52 DD 37 55
	ld	iy,offset(ix)		;DD 31 55
	ld.s	iy,offset(ix)		;52 DD 31 55
	ld	ix,offset(iy)		;FD 31 55
	ld.s	ix,offset(iy)		;52 FD 31 55
	ld	iy,offset(iy)		;FD 37 55
	ld.s	iy,offset(iy)		;52 FD 37 55
	ld	offset(ix),bc		;DD 0F 55
	ld.s	offset(ix),bc		;52 DD 0F 55
	ld	offset(ix),de		;DD 1F 55
	ld.s	offset(ix),de		;52 DD 1F 55
	ld	offset(ix),hl		;DD 2F 55
	ld.s	offset(ix),hl		;52 DD 2F 55
	ld	offset(iy),bc		;FD 0F 55
	ld.s	offset(iy),bc		;52 FD 0F 55
	ld	offset(iy),de		;FD 1F 55
	ld.s	offset(iy),de		;52 FD 1F 55
	ld	offset(iy),hl		;FD 2F 55
	ld.s	offset(iy),hl		;52 FD 2F 55
	ld	offset(ix),ix		;DD 3F 55
	ld.s	offset(ix),ix		;52 DD 3F 55
	ld	offset(ix),iy		;DD 3E 55
	ld.s	offset(ix),iy		;52 DD 3E 55
	ld	offset(iy),ix		;FD 3E 55
	ld.s	offset(iy),ix		;52 FD 3E 55
	ld	offset(iy),iy		;FD 3F 55
	ld.s	offset(iy),iy		;52 FD 3F 55
	;***********************************************************
	; load location (hl)
	; with location (de)
	; decrement de, hl
	; decrement bc
	ldd				;ED A8
	ldd.s				;52 ED A8
	;***********************************************************
	; load location (hl)
	; with location (de)
	; decrement de, hl
	; decrement bc
	; repeat until bc = 0
	lddr				;ED B8
	lddr.s				;52 ED B8
	;***********************************************************
	; load location (hl)
	; with location (de)
	; increment de, hl
	; decrement bc
	ldi				;ED A0
	ldi.s				;52 ED A0
	;***********************************************************
	; load location (hl)
	; with location (de)
	; increment de, hl
	; decrement bc
	; repeat until bc = 0
	ldir				;ED B0
	ldir.s				;52 ED B0
	;***********************************************************
	lea	ix,ix,#offset		;ED 32 55
	lea.s	ix,ix,#offset		;52 ED 32 55
	lea	iy,ix,#offset		;ED 55 55
	lea.s	iy,ix,#offset		;52 ED 55 55
	lea	ix,iy,#offset		;ED 54 55
	lea.s	ix,iy,#offset		;52 ED 54 55
	lea	iy,iy,#offset		;ED 33 55
	lea.s	iy,iy,#offset		;52 ED 33 55
	lea	bc,ix,#offset		;ED 02 55
	lea	de,ix,#offset		;ED 12 55
	lea	hl,ix,#offset		;ED 22 55
	lea.s	bc,ix,#offset		;52 ED 02 55
	lea.s	de,ix,#offset		;52 ED 12 55
	lea.s	hl,ix,#offset		;52 ED 22 55
	lea	bc,iy,#offset		;ED 03 55
	lea	de,iy,#offset		;ED 13 55
	lea	hl,iy,#offset		;ED 23 55
	lea.s	bc,iy,#offset		;52 ED 03 55
	lea.s	de,iy,#offset		;52 ED 13 55
	lea.s	hl,iy,#offset		;52 ED 23 55
	;***********************************************************
	; multiplication of each half
	; of the specified register pair
	; with the 16-bit result going to
	; the specified register pair
	mlt	bc			;ED 4C
	mlt	de			;ED 5C
	mlt	hl			;ED 6C
	mlt	sp			;ED 7C
	mlt.s	sp			;52 ED 7C
	;***********************************************************
	; 2's complement of 'a'
	neg				;ED 44
	;***********************************************************
	; no operation
	nop				;00
	;***********************************************************
	; logical 'or' operand with 'a'
	or	a,(hl)			;B6
	or.s	a,(hl)			;52 B6
	or	a,ixh			;DD B4
	or	a,ixl			;DD B5
	or	a,iyh			;FD B4
	or	a,iyl			;FD B5
	or	a,offset(ix)		;DD B6 55
	or.s	a,offset(ix)		;52 DD B6 55
	or	a,offset(iy)		;FD B6 55
	or.s	a,offset(iy)		;52 FD B6 55
	or	a,a			;B7
	or	a,b			;B0
	or	a,c			;B1
	or	a,d			;B2
	or	a,e			;B3
	or	a,h			;B4
	or	a,l			;B5
	or	a,#n			;F6 20
	;***********************************************************
	otd2r				;ED BC
	otd2r.s				;52 ED BC
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; decrement hl and b
	; decrement c
	otdm				;ED 8B
	otdm.s				;52 ED 8B
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; decrement hl and c
	; decrement b
	; repeat until b = 0
	otdmr				;ED 9B
	otdmr.s				;52 ED 9B
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; decrement hl and decrement b
	; repeat until b = 0
	otdr				;ED BB
	otdr.s				;52 ED BB
	;***********************************************************
	otdrx				;ED CB
	otdrx.s				;52 ED CB
	;***********************************************************
	oti2r				;ED B4
	oti2r.s				;52 ED B4
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; increment hl and b
	; decrement c
	otim				;ED 83
	otim.s				;52 ED 83
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; increment hl and c
	; decrement b
	; repeat until b = 0
	otimr				;ED 93
	otimr.s				;52 ED 93
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; increment hl and decrement b
	; repeat until b = 0
	otir				;ED B3
	otir.s				;52 ED B3
	;***********************************************************
	otirx				;ED C3
	otirx.s				;52 ED C3
	;***********************************************************
	; load output port (c) with reg
	out	(c),a			;ED 79
	out	(c),b			;ED 41
	out	(c),c			;ED 49
	out	(c),d			;ED 51
	out	(c),e			;ED 59
	out	(c),h			;ED 61
	out	(c),l			;ED 69
	;***********************************************************
	; load output port (n) with 'a'
	out	(n),a			;D3 20
	;***********************************************************
	; load output port (n) from register
	out0	(n),a			;ED 39 20
	out0	(n),b			;ED 01 20
	out0	(n),c			;ED 09 20
	out0	(n),d			;ED 11 20
	out0	(n),e			;ED 19 20
	out0	(n),h			;ED 21 20
	out0	(n),l			;ED 29 20
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; decrement hl and decrement b
	outd				;ED AB
	outd.s				;52 ED AB
	;***********************************************************
	outd2				;ED AC
	outd2.s				;52 ED AC
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; increment hl and decrement b
	outi				;ED A3
	outi.s				;52 ED A3
	;***********************************************************
	outi2				;ED A4
	outi2.s				;52 ED A4
	;***********************************************************
	pea	ix,#offset		;ED 65 55
	pea.s	ix,#offset		;52 ED 65 55
	pea	iy,#offset		;ED 66 55
	pea.s	iy,#offset		;52 ED 66 55
	;***********************************************************
	; load destination with top of stack
	pop	af			;F1
	pop.s	af			;52 F1
	pop	bc			;C1
	pop.s	bc			;52 C1
	pop	de			;D1
	pop.s	de			;52 D1
	pop	hl			;E1
	pop.s	hl			;52 E1
	pop	ix			;DD E1
	pop.s	ix			;52 DD E1
	pop	iy			;FD E1
	pop.s	iy			;52 FD E1
	;***********************************************************
	; put source on stack
	push	af			;F5
	push.s	af			;52 F5
	push	bc			;C5
	push.s	bc			;52 C5
	push	de			;D5
	push.s	de			;52 D5
	push	hl			;E5
	push.s	hl			;52 E5
	push	ix			;DD E5
	push.s	ix			;52 DD E5
	push	iy			;FD E5
	push.s	iy			;52 FD E5
	;***********************************************************
	; reset bit of location or register
	res	0,(hl)			;CB 86
	res.s	0,(hl)			;52 CB 86
	res	0,offset(ix)		;DD CB 55 86
	res.s	0,offset(ix)		;52 DD CB 55 86
	res	0,offset(iy)		;FD CB 55 86
	res.s	0,offset(iy)		;52 FD CB 55 86
	res	0,a			;CB 87
	res	0,b			;CB 80
	res	0,c			;CB 81
	res	0,d			;CB 82
	res	0,e			;CB 83
	res	0,h			;CB 84
	res	0,l			;CB 85
	res	1,(hl)			;CB 8E
	res.s	1,(hl)			;52 CB 8E
	res	1,offset(ix)		;DD CB 55 8E
	res.s	1,offset(ix)		;52 DD CB 55 8E
	res	1,offset(iy)		;FD CB 55 8E
	res.s	1,offset(iy)		;52 FD CB 55 8E
	res	1,a			;CB 8F
	res	1,b			;CB 88
	res	1,c			;CB 89
	res	1,d			;CB 8A
	res	1,e			;CB 8B
	res	1,h			;CB 8C
	res	1,l			;CB 8D
	res	2,(hl)			;CB 96
	res.s	2,(hl)			;52 CB 96
	res	2,offset(ix)		;DD CB 55 96
	res.s	2,offset(ix)		;52 DD CB 55 96
	res	2,offset(iy)		;FD CB 55 96
	res.s	2,offset(iy)		;52 FD CB 55 96
	res	2,a			;CB 97
	res	2,b			;CB 90
	res	2,c			;CB 91
	res	2,d			;CB 92
	res	2,e			;CB 93
	res	2,h			;CB 94
	res	2,l			;CB 95
	res	3,(hl)			;CB 9E
	res.s	3,(hl)			;52 CB 9E
	res	3,offset(ix)		;DD CB 55 9E
	res.s	3,offset(ix)		;52 DD CB 55 9E
	res	3,offset(iy)		;FD CB 55 9E
	res.s	3,offset(iy)		;52 FD CB 55 9E
	res	3,a			;CB 9F
	res	3,b			;CB 98
	res	3,c			;CB 99
	res	3,d			;CB 9A
	res	3,e			;CB 9B
	res	3,h			;CB 9C
	res	3,l			;CB 9D
	res	4,(hl)			;CB A6
	res.s	4,(hl)			;52 CB A6
	res	4,offset(ix)		;DD CB 55 A6
	res.s	4,offset(ix)		;52 DD CB 55 A6
	res	4,offset(iy)		;FD CB 55 A6
	res.s	4,offset(iy)		;52 FD CB 55 A6
	res	4,a			;CB A7
	res	4,b			;CB A0
	res	4,c			;CB A1
	res	4,d			;CB A2
	res	4,e			;CB A3
	res	4,h			;CB A4
	res	4,l			;CB A5
	res	5,(hl)			;CB AE
	res.s	5,(hl)			;52 CB AE
	res	5,offset(ix)		;DD CB 55 AE
	res.s	5,offset(ix)		;52 DD CB 55 AE
	res	5,offset(iy)		;FD CB 55 AE
	res.s	5,offset(iy)		;52 FD CB 55 AE
	res	5,a			;CB AF
	res	5,b			;CB A8
	res	5,c			;CB A9
	res	5,d			;CB AA
	res	5,e			;CB AB
	res	5,h			;CB AC
	res	5,l			;CB AD
	res	6,(hl)			;CB B6
	res.s	6,(hl)			;52 CB B6
	res	6,offset(ix)		;DD CB 55 B6
	res.s	6,offset(ix)		;52 DD CB 55 B6
	res	6,offset(iy)		;FD CB 55 B6
	res.s	6,offset(iy)		;52 FD CB 55 B6
	res	6,a			;CB B7
	res	6,b			;CB B0
	res	6,c			;CB B1
	res	6,d			;CB B2
	res	6,e			;CB B3
	res	6,h			;CB B4
	res	6,l			;CB B5
	res	7,(hl)			;CB BE
	res.s	7,(hl)			;52 CB BE
	res	7,offset(ix)		;DD CB 55 BE
	res.s	7,offset(ix)		;52 DD CB 55 BE
	res	7,offset(iy)		;FD CB 55 BE
	res.s	7,offset(iy)		;52 FD CB 55 BE
	res	7,a			;CB BF
	res	7,b			;CB B8
	res	7,c			;CB B9
	res	7,d			;CB BA
	res	7,e			;CB BB
	res	7,h			;CB BC
	res	7,l			;CB BD
	;***********************************************************
	; return from subroutine
	ret				;C9
	ret.s				;52 C9
	;***********************************************************
	; return from subroutine if condition is true
	ret	C			;D8
	ret.s	C			;52 D8
	ret	M			;F8
	ret.s	M			;52 F8
	ret	NC			;D0
	ret.s	NC			;52 D0
	ret	NZ			;C0
	ret.s	NZ			;52 C0
	ret	P			;F0
	ret.s	P			;52 F0
	ret	PE			;E8
	ret.s	PE			;52 E8
	ret	PO			;E0
	ret.s	PO			;52 E0
	ret	Z			;C8
	ret.s	Z			;52 C8
	;***********************************************************
	; return from interrupt
	reti				;ED 4D
	reti.s				;52 ED 4D
	;***********************************************************
	; return from non-maskable interrupt
	retn				;ED 45
	retn.s				;52 ED 45
	;***********************************************************
	; rotate left through carry
	rl	a,(hl)			;CB 16
	rl.s	a,(hl)			;52 CB 16
	rl	a,offset(ix)		;DD CB 55 16
	rl.s	a,offset(ix)		;52 DD CB 55 16
	rl	a,offset(iy)		;FD CB 55 16
	rl.s	a,offset(iy)		;52 FD CB 55 16
	rl	a,a			;CB 17
	rl	a,b			;CB 10
	rl	a,c			;CB 11
	rl	a,d			;CB 12
	rl	a,e			;CB 13
	rl	a,h			;CB 14
	rl	a,l			;CB 15
	;***********************************************************
	; rotate left 'a' with carry
	rla				;17
	;***********************************************************
	; rotate left circular
	rlc	a,(hl)			;CB 06
	rlc.s	a,(hl)			;52 CB 06
	rlc	a,offset(ix)		;DD CB 55 06
	rlc.s	a,offset(ix)		;52 DD CB 55 06
	rlc	a,offset(iy)		;FD CB 55 06
	rlc.s	a,offset(iy)		;52 FD CB 55 06
	rlc	a,a			;CB 07
	rlc	a,b			;CB 00
	rlc	a,c			;CB 01
	rlc	a,d			;CB 02
	rlc	a,e			;CB 03
	rlc	a,h			;CB 04
	rlc	a,l			;CB 05
	;***********************************************************
	; rotate left 'a' circular
	rlca				;07
	;***********************************************************
	; rotate digit left and right
	; between 'a' and location (hl)
	rld				;ED 6F
	;***********************************************************
	; rotate right through carry
	rr	a,(hl)			;CB 1E
	rr.s	a,(hl)			;52 CB 1E
	rr	a,offset(ix)		;DD CB 55 1E
	rr.s	a,offset(ix)		;52 DD CB 55 1E
	rr	a,offset(iy)		;FD CB 55 1E
	rr.s	a,offset(iy)		;52 FD CB 55 1E
	rr	a,a			;CB 1F
	rr	a,b			;CB 18
	rr	a,c			;CB 19
	rr	a,d			;CB 1A
	rr	a,e			;CB 1B
	rr	a,h			;CB 1C
	rr	a,l			;CB 1D
	;***********************************************************
	; rotate 'a' right with carry
	rra				;1F
	;***********************************************************
	; rotate right circular
	rrc	a,(hl)			;CB 0E
	rrc.s	a,(hl)			;52 CB 0E
	rrc	a,offset(ix)		;DD CB 55 0E
	rrc.s	a,offset(ix)		;52 DD CB 55 0E
	rrc	a,offset(iy)		;FD CB 55 0E
	rrc.s	a,offset(iy)		;52 FD CB 55 0E
	rrc	a,a			;CB 0F
	rrc	a,b			;CB 08
	rrc	a,c			;CB 09
	rrc	a,d			;CB 0A
	rrc	a,e			;CB 0B
	rrc	a,h			;CB 0C
	rrc	a,l			;CB 0D
	;***********************************************************
	; rotate 'a' right circular
	rrca				;0F
	;***********************************************************
	; rotate digit right and left
	; between 'a' and location (hl)
	rrd				;ED 67
	;***********************************************************
	rsmix				;ED 7E
	;***********************************************************
	; restart location
	rst	0x00			;C7
	rst	0x08			;CF
	rst	0x10			;D7
	rst	0x18			;DF
	rst	0x20			;E7
	rst	0x28			;EF
	rst	0x30			;F7
	rst	0x38			;FF
	rst.s	0x00			;52 C7
	rst.s	0x08			;52 CF
	rst.s	0x10			;52 D7
	rst.s	0x18			;52 DF
	rst.s	0x20			;52 E7
	rst.s	0x28			;52 EF
	rst.s	0x30			;52 F7
	rst.s	0x38			;52 FF
	;***********************************************************
	; subtract with carry to 'a'
	sbc	a,(hl)			;9E
	sbc.s	a,(hl)			;52 9E
	sbc	a,ixh			;DD 9C
	sbc	a,ixl			;DD 9D
	sbc	a,iyh			;FD 9C
	sbc	a,iyl			;FD 9D
	sbc	a,offset(ix)		;DD 9E 55
	sbc.s	a,offset(ix)		;52 DD 9E 55
	sbc	a,offset(iy)		;FD 9E 55
	sbc.s	a,offset(iy)		;52 FD 9E 55
	sbc	a,a			;9F
	sbc	a,b			;98
	sbc	a,c			;99
	sbc	a,d			;9A
	sbc	a,e			;9B
	sbc	a,h			;9C
	sbc	a,l			;9D
	sbc	a,#n			;DE 20
	;***********************************************************
	; add with carry register pair to 'hl'
	sbc	hl,bc			;ED 42
	sbc	hl,de			;ED 52
	sbc	hl,hl			;ED 62
	sbc.s	hl,bc			;52 ED 42
	sbc.s	hl,de			;52 ED 52
	sbc.s	hl,hl			;52 ED 62
	sbc	hl,sp			;ED 72
	sbc.s	hl,sp			;52 ED 72
	;***********************************************************
	; set carry flag (C=1)
	scf				;37
	;***********************************************************
	; set bit of location or register
	set	0,(hl)			;CB C6
	set.s	0,(hl)			;52 CB C6
	set	0,offset(ix)		;DD CB 55 C6
	set.s	0,offset(ix)		;52 DD CB 55 C6
	set	0,offset(iy)		;FD CB 55 C6
	set.s	0,offset(iy)		;52 FD CB 55 C6
	set	0,a			;CB C7
	set	0,b			;CB C0
	set	0,c			;CB C1
	set	0,d			;CB C2
	set	0,e			;CB C3
	set	0,h			;CB C4
	set	0,l			;CB C5
	set	1,(hl)			;CB CE
	set.s	1,(hl)			;52 CB CE
	set	1,offset(ix)		;DD CB 55 CE
	set.s	1,offset(ix)		;52 DD CB 55 CE
	set	1,offset(iy)		;FD CB 55 CE
	set.s	1,offset(iy)		;52 FD CB 55 CE
	set	1,a			;CB CF
	set	1,b			;CB C8
	set	1,c			;CB C9
	set	1,d			;CB CA
	set	1,e			;CB CB
	set	1,h			;CB CC
	set	1,l			;CB CD
	set	2,(hl)			;CB D6
	set.s	2,(hl)			;52 CB D6
	set	2,offset(ix)		;DD CB 55 D6
	set.s	2,offset(ix)		;52 DD CB 55 D6
	set	2,offset(iy)		;FD CB 55 D6
	set.s	2,offset(iy)		;52 FD CB 55 D6
	set	2,a			;CB D7
	set	2,b			;CB D0
	set	2,c			;CB D1
	set	2,d			;CB D2
	set	2,e			;CB D3
	set	2,h			;CB D4
	set	2,l			;CB D5
	set	3,(hl)			;CB DE
	set.s	3,(hl)			;52 CB DE
	set	3,offset(ix)		;DD CB 55 DE
	set.s	3,offset(ix)		;52 DD CB 55 DE
	set	3,offset(iy)		;FD CB 55 DE
	set.s	3,offset(iy)		;52 FD CB 55 DE
	set	3,a			;CB DF
	set	3,b			;CB D8
	set	3,c			;CB D9
	set	3,d			;CB DA
	set	3,e			;CB DB
	set	3,h			;CB DC
	set	3,l			;CB DD
	set	4,(hl)			;CB E6
	set.s	4,(hl)			;52 CB E6
	set	4,offset(ix)		;DD CB 55 E6
	set.s	4,offset(ix)		;52 DD CB 55 E6
	set	4,offset(iy)		;FD CB 55 E6
	set.s	4,offset(iy)		;52 FD CB 55 E6
	set	4,a			;CB E7
	set	4,b			;CB E0
	set	4,c			;CB E1
	set	4,d			;CB E2
	set	4,e			;CB E3
	set	4,h			;CB E4
	set	4,l			;CB E5
	set	5,(hl)			;CB EE
	set.s	5,(hl)			;52 CB EE
	set	5,offset(ix)		;DD CB 55 EE
	set.s	5,offset(ix)		;52 DD CB 55 EE
	set	5,offset(iy)		;FD CB 55 EE
	set.s	5,offset(iy)		;52 FD CB 55 EE
	set	5,a			;CB EF
	set	5,b			;CB E8
	set	5,c			;CB E9
	set	5,d			;CB EA
	set	5,e			;CB EB
	set	5,h			;CB EC
	set	5,l			;CB ED
	set	6,(hl)			;CB F6
	set.s	6,(hl)			;52 CB F6
	set	6,offset(ix)		;DD CB 55 F6
	set.s	6,offset(ix)		;52 DD CB 55 F6
	set	6,offset(iy)		;FD CB 55 F6
	set.s	6,offset(iy)		;52 FD CB 55 F6
	set	6,a			;CB F7
	set	6,b			;CB F0
	set	6,c			;CB F1
	set	6,d			;CB F2
	set	6,e			;CB F3
	set	6,h			;CB F4
	set	6,l			;CB F5
	set	7,(hl)			;CB FE
	set.s	7,(hl)			;52 CB FE
	set	7,offset(ix)		;DD CB 55 FE
	set.s	7,offset(ix)		;52 DD CB 55 FE
	set	7,offset(iy)		;FD CB 55 FE
	set.s	7,offset(iy)		;52 FD CB 55 FE
	set	7,a			;CB FF
	set	7,b			;CB F8
	set	7,c			;CB F9
	set	7,d			;CB FA
	set	7,e			;CB FB
	set	7,h			;CB FC
	set	7,l			;CB FD
	;***********************************************************
	; shift operand left arithmetic
	sla	a,(hl)			;CB 26
	sla.s	a,(hl)			;52 CB 26
	sla	a,offset(ix)		;DD CB 55 26
	sla.s	a,offset(ix)		;52 DD CB 55 26
	sla	a,offset(iy)		;FD CB 55 26
	sla.s	a,offset(iy)		;52 FD CB 55 26
	sla	a,a			;CB 27
	sla	a,b			;CB 20
	sla	a,c			;CB 21
	sla	a,d			;CB 22
	sla	a,e			;CB 23
	sla	a,h			;CB 24
	sla	a,l			;CB 25
	;***********************************************************
	; enter sleep mode
	slp				;ED 76
	;***********************************************************
	; shift operand right arithmetic
	sra	a,(hl)			;CB 2E
	sra.s	a,(hl)			;52 CB 2E
	sra	a,offset(ix)		;DD CB 55 2E
	sra.s	a,offset(ix)		;52 DD CB 55 2E
	sra	a,offset(iy)		;FD CB 55 2E
	sra.s	a,offset(iy)		;52 FD CB 55 2E
	sra	a,a			;CB 2F
	sra	a,b			;CB 28
	sra	a,c			;CB 29
	sra	a,d			;CB 2A
	sra	a,e			;CB 2B
	sra	a,h			;CB 2C
	sra	a,l			;CB 2D
	;***********************************************************
	; shift operand right logical
	srl	a,(hl)			;CB 3E
	srl.s	a,(hl)			;52 CB 3E
	srl	a,offset(ix)		;DD CB 55 3E
	srl.s	a,offset(ix)		;52 DD CB 55 3E
	srl	a,offset(iy)		;FD CB 55 3E
	srl.s	a,offset(iy)		;52 FD CB 55 3E
	srl	a,a			;CB 3F
	srl	a,b			;CB 38
	srl	a,c			;CB 39
	srl	a,d			;CB 3A
	srl	a,e			;CB 3B
	srl	a,h			;CB 3C
	srl	a,l			;CB 3D
	;***********************************************************
	stmix				;ED 7D
	;***********************************************************
	; subtract operand from 'a'
	sub	a,(hl)			;96
	sub.s	a,(hl)			;52 96
	sub	a,ixh			;DD 94
	sub	a,ixl			;DD 95
	sub	a,iyh			;FD 94
	sub	a,iyl			;FD 95
	sub	a,offset(ix)		;DD 96 55
	sub.s	a,offset(ix)		;52 DD 96 55
	sub	a,offset(iy)		;FD 96 55
	sub.s	a,offset(iy)		;52 FD 96 55
	sub	a,a			;97
	sub	a,b			;90
	sub	a,c			;91
	sub	a,d			;92
	sub	a,e			;93
	sub	a,h			;94
	sub	a,l			;95
	sub	a,#n			;D6 20
	;***********************************************************
	; non-destructive'and' with accumulator and specified operand
	tst	a			;ED 3C
	tst	b			;ED 04
	tst	c			;ED 0C
	tst	d			;ED 14
	tst	e			;ED 1C
	tst	h			;ED 24
	tst	l			;ED 2C
	tst	#n			;ED 64 20
	tst	(hl)			;ED 34
	tst.s	(hl)			;52 ED 34
	;***********************************************************
	; non-destructive 'and' of n and the contents of port (c)
	tstio	#n			;ED 74 20
	;***********************************************************
	; logical 'xor' operand with 'a'
	xor	a,(hl)			;AE
	xor.s	a,(hl)			;52 AE
	xor	a,ixh			;DD AC
	xor	a,ixl			;DD AD
	xor	a,iyh			;FD AC
	xor	a,iyl			;FD AD
	xor	a,offset(ix)		;DD AE 55
	xor.s	a,offset(ix)		;52 DD AE 55
	xor	a,offset(iy)		;FD AE 55
	xor.s	a,offset(iy)		;52 FD AE 55
	xor	a,a			;AF
	xor	a,b			;A8
	xor	a,c			;A9
	xor	a,d			;AA
	xor	a,e			;AB
	xor	a,h			;AC
	xor	a,l			;AD
	xor	a,#n			;EE 20

	;***********************************************************
	; eZ80 short mode
	;***********************************************************

	.adl	0

	;***********************************************************
	adc.l	a,(hl)			;49 8E
	adc.l	a,offset(ix)		;49 DD 8E 55
	adc.l	a,offset(iy)		;49 FD 8E 55
	;***********************************************************
	adc.l	(hl)			;49 8E
	adc.l	offset(ix)		;49 DD 8E 55
	adc.l	offset(iy)		;49 FD 8E 55
	;***********************************************************
	; add with carry register pair to 'hl'
	adc.l	hl,bc			;49 ED 4A
	adc.l	hl,de			;49 ED 5A
	adc.l	hl,hl			;49 ED 6A
	adc.l	hl,sp			;49 ED 7A
	;***********************************************************
	; add operand to 'a'
	add.l	a,(hl)			;49 86
	add.l	a,offset(ix)		;49 DD 86 55
	add.l	a,offset(iy)		;49 FD 86 55
	;***********************************************************
	; add register pair to 'hl'
	add.l	hl,bc			;49 09
	add.l	hl,de			;49 19
	add.l	hl,hl			;49 29
	add.l	hl,sp			;49 39
	;***********************************************************
	; add register pair to 'ix'
	add.l	ix,bc			;49 DD 09
	add.l	ix,de			;49 DD 19
	add.l	ix,ix			;49 DD 29
	add.l	ix,sp			;49 DD 39
	;***********************************************************
	; add register pair to 'iy'
	add.l	iy,bc			;49 FD 09
	add.l	iy,de			;49 FD 19
	add.l	iy,iy			;49 FD 29
	add.l	iy,sp			;49 FD 39
	;***********************************************************
	; logical 'and' operand with 'a'
	and.l	a,(hl)			;49 A6
	and.l	a,offset(ix)		;49 DD A6 55
	and.l	a,offset(iy)		;49 FD A6 55
	;***********************************************************
	; test bit of location or register
	bit.l	0,(hl)			;49 CB 46
	bit.l	0,offset(ix)		;49 DD CB 55 46
	bit.l	0,offset(iy)		;49 FD CB 55 46
	bit.l	1,(hl)			;49 CB 4E
	bit.l	1,offset(ix)		;49 DD CB 55 4E
	bit.l	1,offset(iy)		;49 FD CB 55 4E
	bit.l	2,(hl)			;49 CB 56
	bit.l	2,offset(ix)		;49 DD CB 55 56
	bit.l	2,offset(iy)		;49 FD CB 55 56
	bit.l	3,(hl)			;49 CB 5E
	bit.l	3,offset(ix)		;49 DD CB 55 5E
	bit.l	3,offset(iy)		;49 FD CB 55 5E
	bit.l	4,(hl)			;49 CB 66
	bit.l	4,offset(ix)		;49 DD CB 55 66
	bit.l	4,offset(iy)		;49 FD CB 55 66
	bit.l	5,(hl)			;49 CB 6E
	bit.l	5,offset(ix)		;49 DD CB 55 6E
	bit.l	5,offset(iy)		;49 FD CB 55 6E
	bit.l	6,(hl)			;49 CB 76
	bit.l	6,offset(ix)		;49 DD CB 55 76
	bit.l	6,offset(iy)		;49 FD CB 55 76
	bit.l	7,(hl)			;49 CB 7E
	bit.l	7,offset(ix)		;49 DD CB 55 7E
	bit.l	7,offset(iy)		;49 FD CB 55 7E
	;***********************************************************
	; call subroutine at nn if condition is true
	call.il	C,nn			;52 DC 84 05 00
	call.il	M,nn			;52 FC 84 05 00
	call.il	NC,nn			;52 D4 84 05 00
	call.il	NZ,nn			;52 C4 84 05 00
	call.il	P,nn			;52 F4 84 05 00
	call.il	PE,nn			;52 EC 84 05 00
	call.il	PO,nn			;52 E4 84 05 00
	call.il	Z,nn			;52 CC 84 05 00
	;***********************************************************
	; unconditional call to subroutine at nn
	call.il	nn			;52 CD 84 05 00
	;***********************************************************
	; compare operand with 'a'
	cp.l	a,(hl)			;49 BE
	cp.l	a,offset(ix)		;49 DD BE 55
	cp.l	a,offset(iy)		;49 FD BE 55
	;***********************************************************
	; compare location (hl) and 'a'
	; decrement 'hl' and 'bc'
	cpd.l				;49 ED A9
	;***********************************************************
	; compare location (hl) and 'a'
	; decrement 'hl' and 'bc'
	; repeat until 'bc' = 0
	cpdr.l				;49 ED B9
	;***********************************************************
	; compare location (hl) and 'a'
	; increment 'hl' and decrement 'bc'
	cpi.l				;49 ED A1
	;***********************************************************
	; compare location (hl) and 'a'
	; increment 'hl' and decrement 'bc'
	; repeat until 'bc' = 0
	cpir.l				;49 ED B1
	;***********************************************************
	; decrement operand
	dec.l	(hl)			;49 35
	dec.l	offset(ix)		;49 DD 35 55
	dec.l	offset(iy)		;49 FD 35 55
	dec.l	bc			;49 0B
	dec.l	de			;49 1B
	dec.l	hl			;49 2B
	dec.l	ix			;49 DD 2B
	dec.l	iy			;49 FD 2B
	dec.l	sp			;49 3B
	;***********************************************************
	; exchange location and (sp)
	ex.l	(sp),hl			;49 E3
	ex.l	(sp),ix			;49 DD E3
	ex.l	(sp),iy			;49 FD E3
	;***********************************************************
	; increment operand
	inc.l	(hl)			;49 34
	inc.l	offset(ix)		;49 DD 34 55
	inc.l	offset(iy)		;49 FD 34 55
	inc.l	bc			;49 03
	inc.l	de			;49 13
	inc.l	hl			;49 23
	inc.l	ix			;49 DD 23
	inc.l	iy			;49 FD 23
	inc.l	sp			;49 33
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; decrement 'hl' and 'b'
	ind.l				;49 ED AA
	;***********************************************************
	ind2.l				;49 ED 8C
	;***********************************************************
	ind2r.l				;49 ED 9C
	;***********************************************************
	indm.l				;49 ED 8A
	;***********************************************************
	indmr.l				;49 ED 9A
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; decrement 'hl' and 'b'
	; repeat until 'b' = 0
	indr.l				;49 ED BA
	;***********************************************************
	indrx.l				;49 ED CA
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; increment 'hl' and decrement 'b'
	ini.l				;49 ED A2
	;***********************************************************
	ini2.l				;49 ED 84
	;***********************************************************
	ini2r.l				;49 ED 94
	;***********************************************************
	inim.l				;49 ED 82
	;***********************************************************
	inimr.l				;49 ED 92
	;***********************************************************
	; load location (hl) with input
	; from port (c)
	; increment 'hl' and decrement 'b'
	; repeat until 'b' = 0
	inir.l				;49 ED B2
	;***********************************************************
	inirx.l				;49 ED C2
	;***********************************************************
	; unconditional jump to location nn
	jp.lil	nn			;52 C3 84 05 00
	jp.l	(hl)			;49 E9
	jp.l	(ix)			;49 DD E9
	jp.l	(iy)			;49 FD E9
	;***********************************************************
	; jump to location if condition is true
	jp.lil	C,nn			;52 DA 84 05 00
	jp.lil	M,nn			;52 FA 84 05 00
	jp.lil	NC,nn			;52 D2 84 05 00
	jp.lil	NZ,nn			;52 C2 84 05 00
	jp.lil	P,nn			;52 F2 84 05 00
	jp.lil	PE,nn			;52 EA 84 05 00
	jp.lil	PO,nn			;52 E2 84 05 00
	jp.lil	Z,nn			;52 CA 84 05 00
	;***********************************************************
	; load source to destination
	ld.l	a,(hl)			;49 7E
	ld.l	a,offset(ix)		;49 DD 7E 55
	ld.l	a,offset(iy)		;49 FD 7E 55
	ld.l	b,(hl)			;49 46
	ld.l	b,offset(ix)		;49 DD 46 55
	ld.l	b,offset(iy)		;49 FD 46 55
	ld.l	c,(hl)			;49 4E
	ld.l	c,offset(ix)		;49 DD 4E 55
	ld.l	c,offset(iy)		;49 FD 4E 55
	ld.l	d,(hl)			;49 56
	ld.l	d,offset(ix)		;49 DD 56 55
	ld.l	d,offset(iy)		;49 FD 56 55
	ld.l	e,(hl)			;49 5E
	ld.l	e,offset(ix)		;49 DD 5E 55
	ld.l	e,offset(iy)		;49 FD 5E 55
	ld.l	h,(hl)			;49 66
	ld.l	h,offset(ix)		;49 DD 66 55
	ld.l	h,offset(iy)		;49 FD 66 55
	ld.l	l,(hl)			;49 6E
	ld.l	l,offset(ix)		;49 DD 6E 55
	ld.l	l,offset(iy)		;49 FD 6E 55
	;***********************************************************
	ld.l	(bc),a			;49 02
	ld.l	(de),a			;49 12
	ld.l	a,(bc)			;49 0A
	ld.l	a,(de)			;49 1A
	;***********************************************************
	ld.l	(hl),a			;49 77
	ld.l	(hl),b			;49 70
	ld.l	(hl),c			;49 71
	ld.l	(hl),d			;49 72
	ld.l	(hl),e			;49 73
	ld.l	(hl),h			;49 74
	ld.l	(hl),l			;49 75
	ld.l	(hl),#n			;49 36 20
	;***********************************************************
	ld.l	offset(ix),a		;49 DD 77 55
	ld.l	offset(ix),b		;49 DD 70 55
	ld.l	offset(ix),c		;49 DD 71 55
	ld.l	offset(ix),d		;49 DD 72 55
	ld.l	offset(ix),e		;49 DD 73 55
	ld.l	offset(ix),h		;49 DD 74 55
	ld.l	offset(ix),l		;49 DD 75 55
	ld.l	offset(ix),#n		;49 DD 36 55 20
	;***********************************************************
	ld.l	offset(iy),a		;49 FD 77 55
	ld.l	offset(iy),b		;49 FD 70 55
	ld.l	offset(iy),c		;49 FD 71 55
	ld.l	offset(iy),d		;49 FD 72 55
	ld.l	offset(iy),e		;49 FD 73 55
	ld.l	offset(iy),h		;49 FD 74 55
	ld.l	offset(iy),l		;49 FD 75 55
	ld.l	offset(iy),#n		;49 FD 36 55 20
	;***********************************************************
	ld.il	(nn),a			;52 32 84 05 00
	ld.il	(nn),bc			;52 ED 43 84 05 00
	ld.il	(nn),de			;52 ED 53 84 05 00
	ld.il	(nn),hl			;52 22 84 05 00
	ld.il	(nn),sp			;52 ED 73 84 05 00
	ld.il	(nn),ix			;52 DD 22 84 05 00
	ld.il	(nn),iy			;52 FD 22 84 05 00
	;***********************************************************
	ld.il	a,(nn)			;52 3A 84 05 00
	ld.il	bc,(nn)			;52 ED 4B 84 05 00
	ld.il	de,(nn)			;52 ED 5B 84 05 00
	ld.il	hl,(nn)			;52 2A 84 05 00
	ld.il	sp,(nn)			;52 ED 7B 84 05 00
	ld.il	ix,(nn)			;52 DD 2A 84 05 00
	ld.il	iy,(nn)			;52 FD 2A 84 05 00
	;***********************************************************
	ld.il	bc,#nn			;52 01 84 05 00
	ld.il	de,#nn			;52 11 84 05 00
	ld.il	hl,#nn			;52 21 84 05 00
	ld.il	sp,#nn			;52 31 84 05 00
	ld.il	ix,#nn			;52 DD 21 84 05 00
	ld.il	iy,#nn			;52 FD 21 84 05 00
	;***********************************************************
	ld.l	(hl),ix			;49 ED 3F
	ld.l	(hl),iy			;49 ED 3E
	ld.l	(hl),bc			;49 ED 0F
	ld.l	(hl),de			;49 ED 1F
	ld.l	(hl),hl			;49 ED 2F
	ld.l	bc,(hl)			;49 ED 07
	ld.l	de,(hl)			;49 ED 17
	ld.l	hl,(hl)			;49 ED 27
	ld.l	ix,(hl)			;49 ED 37
	ld.l	iy,(hl)			;49 ED 31
	;***********************************************************
	ld.l	bc,offset(ix)		;49 DD 07 55
	ld.l	de,offset(ix)		;49 DD 17 55
	ld.l	hl,offset(ix)		;49 DD 27 55
	ld.l	bc,offset(iy)		;49 FD 07 55
	ld.l	de,offset(iy)		;49 FD 17 55
	ld.l	hl,offset(iy)		;49 FD 27 55
	ld.l	ix,offset(ix)		;49 DD 37 55
	ld.l	iy,offset(ix)		;49 DD 31 55
	ld.l	ix,offset(iy)		;49 FD 31 55
	ld.l	iy,offset(iy)		;49 FD 37 55
	ld.l	offset(ix),bc		;49 DD 0F 55
	ld.l	offset(ix),de		;49 DD 1F 55
	ld.l	offset(ix),hl		;49 DD 2F 55
	ld.l	offset(iy),bc		;49 FD 0F 55
	ld.l	offset(iy),de		;49 FD 1F 55
	ld.l	offset(iy),hl		;49 FD 2F 55
	ld.l	offset(ix),ix		;49 DD 3F 55
	ld.l	offset(ix),iy		;49 DD 3E 55
	ld.l	offset(iy),ix		;49 FD 3E 55
	ld.l	offset(iy),iy		;49 FD 3F 55
	;***********************************************************
	; load location (hl)
	; with location (de)
	; decrement de, hl
	; decrement bc
	ldd.l				;49 ED A8
	;***********************************************************
	; load location (hl)
	; with location (de)
	; decrement de, hl
	; decrement bc
	; repeat until bc = 0
	lddr.l				;49 ED B8
	;***********************************************************
	; load location (hl)
	; with location (de)
	; increment de, hl
	; decrement bc
	ldi.l				;49 ED A0
	;***********************************************************
	; load location (hl)
	; with location (de)
	; increment de, hl
	; decrement bc
	; repeat until bc = 0
	ldir.l				;49 ED B0
	;***********************************************************
	lea.l	ix,ix,#offset		;49 ED 32 55
	lea.l	iy,ix,#offset		;49 ED 55 55
	lea.l	ix,iy,#offset		;49 ED 54 55
	lea.l	iy,iy,#offset		;49 ED 33 55
	lea.l	bc,ix,#offset		;49 ED 02 55
	lea.l	de,ix,#offset		;49 ED 12 55
	lea.l	hl,ix,#offset		;49 ED 22 55
	lea.l	bc,iy,#offset		;49 ED 03 55
	lea.l	de,iy,#offset		;49 ED 13 55
	lea.l	hl,iy,#offset		;49 ED 23 55
	;***********************************************************
	; multiplication of each half
	; of the specified register pair
	; with the 16-bit result going to
	; the specified register pair
	mlt.l	sp			;49 ED 7C
	;***********************************************************
	; logical 'or' operand with 'a'
	or.l	a,(hl)			;49 B6
	or.l	a,offset(ix)		;49 DD B6 55
	or.l	a,offset(iy)		;49 FD B6 55
	;***********************************************************
	otd2r.l				;49 ED BC
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; decrement hl and b
	; decrement c
	otdm.l				;49 ED 8B
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; decrement hl and c
	; decrement b
	; repeat until b = 0
	otdmr.l				;49 ED 9B
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; decrement hl and decrement b
	; repeat until b = 0
	otdr.l				;49 ED BB
	;***********************************************************
	otdrx.l				;49 ED CB
	;***********************************************************
	oti2r.l				;49 ED B4
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; increment hl and b
	; decrement c
	otim.l				;49 ED 83
	;***********************************************************
	; load output port (c) with
	; location (hl),
	; increment hl and c
	; decrement b
	; repeat until b = 0
	otimr.l				;49 ED 93
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; increment hl and decrement b
	; repeat until b = 0
	otir.l				;49 ED B3
	;***********************************************************
	otirx.l				;49 ED C3
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; decrement hl and decrement b
	outd.l				;49 ED AB
	;***********************************************************
	outd2.l				;49 ED AC
	;***********************************************************
	; load output port (c)
	; with location (hl)
	; increment hl and decrement b
	outi.l				;49 ED A3
	;***********************************************************
	outi2.l				;49 ED A4
	;***********************************************************
	pea.l	ix,#offset		;52 ED 65 55
	pea.l	iy,#offset		;52 ED 66 55
	;***********************************************************
	; load destination with top of stack
	pop.l	af			;49 F1
	pop.l	bc			;49 C1
	pop.l	de			;49 D1
	pop.l	hl			;49 E1
	pop.l	ix			;49 DD E1
	pop.l	iy			;49 FD E1
	;***********************************************************
	; put source on stack
	push.l	af			;49 F5
	push.l	bc			;49 C5
	push.l	de			;49 D5
	push.l	hl			;49 E5
	push.l	ix			;49 DD E5
	push.l	iy			;49 FD E5
	;***********************************************************
	; reset bit of location or register
	res.l	0,(hl)			;49 CB 86
	res.l	0,offset(ix)		;49 DD CB 55 86
	res.l	0,offset(iy)		;49 FD CB 55 86
	res.l	1,(hl)			;49 CB 8E
	res.l	1,offset(ix)		;49 DD CB 55 8E
	res.l	1,offset(iy)		;49 FD CB 55 8E
	res.l	2,(hl)			;49 CB 96
	res.l	2,offset(ix)		;49 DD CB 55 96
	res.l	2,offset(iy)		;49 FD CB 55 96
	res.l	3,(hl)			;49 CB 9E
	res.l	3,offset(ix)		;49 DD CB 55 9E
	res.l	3,offset(iy)		;49 FD CB 55 9E
	res.l	4,(hl)			;49 CB A6
	res.l	4,offset(ix)		;49 DD CB 55 A6
	res.l	4,offset(iy)		;49 FD CB 55 A6
	res.l	5,(hl)			;49 CB AE
	res.l	5,offset(ix)		;49 DD CB 55 AE
	res.l	5,offset(iy)		;49 FD CB 55 AE
	res.l	6,(hl)			;49 CB B6
	res.l	6,offset(ix)		;49 DD CB 55 B6
	res.l	6,offset(iy)		;49 FD CB 55 B6
	res.l	7,(hl)			;49 CB BE
	res.l	7,offset(ix)		;49 DD CB 55 BE
	res.l	7,offset(iy)		;49 FD CB 55 BE
	;***********************************************************
	; return from subroutine
	ret.l				;49 C9
	;***********************************************************
	; return from subroutine if condition is true
	ret.l	C			;49 D8
	ret.l	M			;49 F8
	ret.l	NC			;49 D0
	ret.l	NZ			;49 C0
	ret.l	P			;49 F0
	ret.l	PE			;49 E8
	ret.l	PO			;49 E0
	ret.l	Z			;49 C8
	;***********************************************************
	; return from interrupt
	reti.l				;49 ED 4D
	;***********************************************************
	; return from non-maskable interrupt
	retn.l				;49 ED 45
	;***********************************************************
	; rotate left through carry
	rl.l	a,(hl)			;49 CB 16
	rl.l	a,offset(ix)		;49 DD CB 55 16
	rl.l	a,offset(iy)		;49 FD CB 55 16
	;***********************************************************
	; rotate left circular
	rlc.l	a,(hl)			;49 CB 06
	rlc.l	a,offset(ix)		;49 DD CB 55 06
	rlc.l	a,offset(iy)		;49 FD CB 55 06
	;***********************************************************
	; rotate right through carry
	rr.l	a,(hl)			;49 CB 1E
	rr.l	a,offset(ix)		;49 DD CB 55 1E
	rr.l	a,offset(iy)		;49 FD CB 55 1E
	;***********************************************************
	; rotate right circular
	rrc.l	a,(hl)			;49 CB 0E
	rrc.l	a,offset(ix)		;49 DD CB 55 0E
	rrc.l	a,offset(iy)		;49 FD CB 55 0E
	;***********************************************************
	; restart location
	rst.l	0x00			;49 C7
	rst.l	0x08			;49 CF
	rst.l	0x10			;49 D7
	rst.l	0x18			;49 DF
	rst.l	0x20			;49 E7
	rst.l	0x28			;49 EF
	rst.l	0x30			;49 F7
	rst.l	0x38			;49 FF
	;***********************************************************
	; subtract with carry to 'a'
	sbc.l	a,(hl)			;49 9E
	sbc.l	a,offset(ix)		;49 DD 9E 55
	sbc.l	a,offset(iy)		;49 FD 9E 55
	;***********************************************************
	; add with carry register pair to 'hl'
	sbc.l	hl,bc			;49 ED 42
	sbc.l	hl,de			;49 ED 52
	sbc.l	hl,hl			;49 ED 62
	sbc.l	hl,sp			;49 ED 72
	;***********************************************************
	; set bit of location or register
	set.l	0,(hl)			;49 CB C6
	set.l	0,offset(ix)		;49 DD CB 55 C6
	set.l	0,offset(iy)		;49 FD CB 55 C6
	set.l	1,(hl)			;49 CB CE
	set.l	1,offset(ix)		;49 DD CB 55 CE
	set.l	1,offset(iy)		;49 FD CB 55 CE
	set.l	2,(hl)			;49 CB D6
	set.l	2,offset(ix)		;49 DD CB 55 D6
	set.l	2,offset(iy)		;49 FD CB 55 D6
	set.l	3,(hl)			;49 CB DE
	set.l	3,offset(ix)		;49 DD CB 55 DE
	set.l	3,offset(iy)		;49 FD CB 55 DE
	set.l	4,(hl)			;49 CB E6
	set.l	4,offset(ix)		;49 DD CB 55 E6
	set.l	4,offset(iy)		;49 FD CB 55 E6
	set.l	5,(hl)			;49 CB EE
	set.l	5,offset(ix)		;49 DD CB 55 EE
	set.l	5,offset(iy)		;49 FD CB 55 EE
	set.l	6,(hl)			;49 CB F6
	set.l	6,offset(ix)		;49 DD CB 55 F6
	set.l	6,offset(iy)		;49 FD CB 55 F6
	set.l	7,(hl)			;49 CB FE
	set.l	7,offset(ix)		;49 DD CB 55 FE
	set.l	7,offset(iy)		;49 FD CB 55 FE
	;***********************************************************
	; shift operand left arithmetic
	sla.l	a,(hl)			;49 CB 26
	sla.l	a,offset(ix)		;49 DD CB 55 26
	sla.l	a,offset(iy)		;49 FD CB 55 26
	;***********************************************************
	; shift operand right arithmetic
	sra.l	a,(hl)			;49 CB 2E
	sra.l	a,offset(ix)		;49 DD CB 55 2E
	sra.l	a,offset(iy)		;49 FD CB 55 2E
	;***********************************************************
	; shift operand right logical
	srl.l	a,(hl)			;49 CB 3E
	srl.l	a,offset(ix)		;49 DD CB 55 3E
	srl.l	a,offset(iy)		;49 FD CB 55 3E
	;***********************************************************
	; subtract operand from 'a'
	sub.l	a,(hl)			;49 96
	sub.l	a,offset(ix)		;49 DD 96 55
	sub.l	a,offset(iy)		;49 FD 96 55
	;***********************************************************
	; non-destructive'and' with accumulator and specified operand
	tst.l	(hl)			;49 ED 34
	;***********************************************************
	; logical 'xor' operand with 'a'
	xor.l	a,(hl)			;49 AE
	xor.l	a,offset(ix)		;49 DD AE 55
	xor.l	a,offset(iy)		;49 FD AE 55
	;***********************************************************
