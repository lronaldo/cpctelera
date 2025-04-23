;--------------------------------------------------------------------------
;  _modulong.s
;
;  Copyright (C) 2014, Ben Shi
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

	.globl __modulong

	.area CODE
__modulong:
	sub	sp, #3
;	device/lib/_modulong.c: 342: unsigned char count = 0;
	clr	(0x03, sp)
;	device/lib/_modulong.c: 344: while (!MSB_SET(b))
	clr	(0x01, sp)
__modulong_00103:
	ld	a, (0x0b, sp)
	sll	a
	clr	a
	rlc	a
	tnz	a
	jrne	__modulong_00117
;	device/lib/_modulong.c: 346: b <<= 1;
	ldw	y, (0x0d, sp)
	ldw	x, (0x0b, sp)
	sllw	y
	rlcw	x
	ldw	(0x0d, sp), y
	ldw	(0x0b, sp), x
;	device/lib/_modulong.c: 347: if (b > a)
	ldw	x, (0x09, sp)
	cpw	x, (0x0d, sp)
	ld	a, (0x08, sp)
	sbc	a, (0x0c, sp)
	ld	a, (0x07, sp)
	sbc	a, (0x0b, sp)
	jrnc	__modulong_00102
;	device/lib/_modulong.c: 349: b >>=1;
	ldw	y, (0x0d, sp)
	ldw	x, (0x0b, sp)
	srlw	x
	rrcw	y
	ldw	(0x0d, sp), y
	ldw	(0x0b, sp), x
;	device/lib/_modulong.c: 350: break;
	jra	__modulong_00117
__modulong_00102:
;	device/lib/_modulong.c: 352: count++;
	inc	(0x01, sp)
	ld	a, (0x01, sp)
	ld	(0x03, sp), a
	jra	__modulong_00103
;	device/lib/_modulong.c: 354: do
__modulong_00117:
	ld	a, (0x03, sp)
	ld	(0x02, sp), a
__modulong_00108:
;	device/lib/_modulong.c: 356: if (a >= b)
	ldw	x, (0x09, sp)
	subw	x, (0x0d, sp)
	ld	a, (0x08, sp)
	sbc	a, (0x0c, sp)
	ld	yl, a
	ld	a, (0x07, sp)
	sbc	a, (0x0b, sp)
	jrc	__modulong_00107
;	device/lib/_modulong.c: 357: a -= b;
	ldw	(0x09, sp), x
	ld	yh, a
	ldw	(0x07, sp), y
__modulong_00107:
;	device/lib/_modulong.c: 358: b >>= 1;
	ldw	y, (0x0d, sp)
	ldw	x, (0x0b, sp)
	srlw	x
	rrcw	y
	ldw	(0x0d, sp), y
	ldw	(0x0b, sp), x
;	device/lib/_modulong.c: 360: while (count--);
	ld	a, (0x02, sp)
	dec	(0x02, sp)
	tnz	a
	jrne	__modulong_00108
;	device/lib/_modulong.c: 362: return a;
	ldw	x, (0x09, sp)
	ldw	y, (0x07, sp)
	addw	sp, #3
	retf
