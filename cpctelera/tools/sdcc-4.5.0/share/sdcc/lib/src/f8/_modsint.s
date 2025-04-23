;--------------------------------------------------------------------------
;  _modsint.s
;
;  Copyright (C) 2023-2024, Philipp Klaus Krause
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

	.globl __modsint
	.globl __divuint

	.area CODE

; _modsint (int x, int y)
__modsint:
	ld	xl, yh
	tst	xl
	jrnn	x_nonnegative
	negw	y
x_nonnegative:

	ldw	z, (2, sp)
	jrnn	y_nonnegative
	negw	z
y_nonnegative:

	push	xl
	pushw	z
	call	#__divuint
	ldw	y, z
	tst	(2, sp)
	jrnn	return_nonnegative
	negw	y
return_nonnegative:
	addw	sp, #3

	ret

