;--------------------------------------------------------------------------
;  setjmp.s
;
;  Copyright (C) 2011-2024, Philipp Klaus Krause
;  Copyright (C) 2024 Janko Stamenović
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

	.area	_CODE

	.globl ___setjmp

___setjmp:
	; store ret addr
	pop	de
	push	de
	ld	(hl), e
	inc	hl
	ld	(hl), d
	inc	hl

	; store SP
	xor	a, a
	ld	e, a
	ld	d, a
	ex	de, hl
	add	hl, sp
	ex	de, hl
	ld	(hl), e
	inc	hl
	ld	(hl), d
	inc	hl

	; store frame ptr
	push	ix
	pop	de
	ld	(hl), e
	inc	hl
	ld	(hl), d

	; ret 0
	ld	l, a
	ld	h, a
	ret


.globl _longjmp

_longjmp:
	pop	de
	pop	de
	; pass retval as is, only if 0 pass 1
	ld	a, e
	or	a, d
	jr	nz, s1
	inc	e
s1:
	; save retval
	push	de

	; fetch stored jumpaddr
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	inc	hl

	; fetch  spval
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	inc	hl
	; save	spval
	push	de

	; fetch and set IX
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	push	de
	pop	ix

	; restore spval
	pop	hl
	; restore retval
	pop	de
	; adjust the stack
	ld	sp, hl
	pop	hl

	; retval to hl
	ex	de, hl

	; jump to jumpaddr
	push	bc
	ret
