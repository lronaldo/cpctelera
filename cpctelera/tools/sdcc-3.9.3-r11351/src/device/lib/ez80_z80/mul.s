;--------------------------------------------------------------------------
;  mulchar.s
;
;  Copyright (C) 2017, Philipp Klaus Krause
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

.hd64
.area   _CODE

; 16 x 16 -> 16 multiplication.

.globl	__mulint

__mulint:
        pop     af
        pop     bc
        pop     de
        push    de
        push    bc
        push    af

	;; 16-bit multiplication
	;;
	;; Entry conditions
	;; bc = multiplicand
	;; de = multiplier
	;;
	;; Exit conditions
	;; hl = less significant word of product
	;;
	;; Register used: AF,BC,DE,HL

__mul16::

	; Swap lower bytes while also copying them into hl
	ld	l, c
	ld	h, e
	ld	e, l
	ld	c, h

	mlt	bc
	mlt	de
	mlt	hl

	ld	a, c
	add	a, e
	add	a, h
	ld	h, a

        ret

