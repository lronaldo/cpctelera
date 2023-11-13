;--------------------------------------------------------------------------
;  memcpy.s
;
;  Copyright (C) 2020, Sergey Belyashov
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

	.area   _CODE

	.globl	_memcpy
	.globl	___memcpy

_memcpy::
___memcpy::
	lda	hl, 7(sp)
	ld	a, (hl-)
	ld	d, a
	ld	a, (hl-)
	ld	e, a
	or	a, d
	jr	Z, 190$
	push	bc
	ld	a, (hl-)
	ld	b, a
	ld	a, (hl-)
	ld	c, a
	ld	a, (hl-)
	ld	l, (hl)
	ld	h, a
	inc	d
	inc	e
	jr	110$
100$:
	ld	a, (bc)
	inc	bc
	ld	(hl+), a
110$:
	dec	e
	jr	nz, 100$
	dec	d
	jr	nz, 100$
	pop	bc
190$:
	lda	hl, 2(sp)
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	ret
