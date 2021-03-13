;--------------------------------------------------------------------------
;  setjmp.s
;
;  Copyright (C) 2011-2014, Philipp Klaus Krause
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
	.ez80

	.area   _CODE

	.globl ___setjmp

___setjmp:
	pop	hl
	pop	iy
	push	af
	push	hl

	; Store return address.
	ld	0(iy), hl

	; Store stack pointer.
	xor	a, a
	ld	l, a
	ld	h, a
	add	hl, sp
	ld	2(iy), hl

	; Store frame pointer.
	push	ix
	pop	hl
	ld	4(iy), hl

	; Return 0.
	ld	l, a
	ld	h, a
	ret

.globl _longjmp

_longjmp:
	pop	af
	pop	iy
	pop	de

	; Ensure that return value is non-zero.
	ld	a, e
	or	a, d
	jr	NZ, jump
	inc	de
jump:

	; Restore frame pointer.
	ld	hl, 4(iy)
	push	hl
	pop	ix

	; Adjust stack pointer.
	ld	hl, 2(iy)
	ld	sp, hl
	pop	hl

	; Move return value into hl.
	ex	de, hl

	; Jump.
	ld	bc, 0(iy)
	push	bc
	ret
