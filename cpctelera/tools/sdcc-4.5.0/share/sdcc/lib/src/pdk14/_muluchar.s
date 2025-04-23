	.module _muluchar
	.optsdcc -mpdk13

	.area DATA

	.area	OSEG (OVR,DATA)
__muluchar_PARM_1::
	.ds 1
__muluchar_PARM_2::
	.ds 1

	.area CODE

; unsigned int _muluchar (unsigned char x, unsigned char y)
__muluchar::

	mov	a, #0x00
	clear	p

	inc	p   ; {p,a} = 0x0100
0$:
	sl	a
	slc	p
	slc	_test_muluchar_PARM_1       ;   x <<= 1;
	t0sn.io f,c
	add	a, _test_muluchar_PARM_2    ;   result += y;
3$:
	addc	p
	t1sn	_test_muluchar_PARM_1, #0
	goto	0$
4$:
	ret

