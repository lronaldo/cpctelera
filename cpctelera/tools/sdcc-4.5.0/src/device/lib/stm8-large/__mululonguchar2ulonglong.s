;--------------------------------------------------------------------------
;  __mululonguchar2ulonglong.s
;
;  Copyright (c) 2023, Philipp Klaus Krause
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

.globl ___mululonguchar2ulonglong

.area CODE

___mululonguchar2ulonglong:
	; Create temporary destination.
	clrw	x
	push	a
	pushw	x
	pushw	x

	ld	a, (15, sp)

	ldw	x, (13, sp)
	mul	x, a
	ldw	(4, sp), x

	ldw	x, (12, sp)
	mul	x, a
	addw	x, (3, sp)
	ldw	(3, sp), x

	ldw	x, (11, sp)
	mul	x, a
	addw	x, (2, sp)
	ldw	(2, sp), x

	ldw	x, (10, sp)
	mul	x, a
	addw	x, (1, sp)

	ldw	y, (9, sp)
	ldw	(3, y), x
	clrw	x
	exgw	x, y
	ldw	(1, x), y
	clr	(x)
	ldw	y, (3, sp)
	ldw	(5, x), y
	ld a, (5, sp)
	ld	(7, x), a

	addw	sp, #5

	retf

