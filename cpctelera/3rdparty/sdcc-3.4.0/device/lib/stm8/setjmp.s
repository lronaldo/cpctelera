;--------------------------------------------------------------------------
;  setjmp.s
;
;  Copyright (C) 2014, Philipp Klaus Krause
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

	.globl ___setjmp

___setjmp:
	ldw	y, (3, sp)

	; store return address
	ldw	x, (1, sp)
	ldw	(y), x

	; store stack pointer
	ldw	x, sp
	ldw	(2, y), x

	; return 0
	clrw	x

	ret

	.globl _longjmp

_longjmp:
	ldw	x, (3, sp)
	ldw	y, (5, sp)

	; Restore return address
	pushw	x
	ldw	x, (2, x)
	ldw	(1, x), y
	popw	y
	ldw	sp, x

	; Calculate return value
	popw	x
	tnzw	x
	jrne	jump
	incw	x
jump:
	; Return
	ldw	y, (y)
	jp	(y)

	ret

