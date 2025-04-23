;--------------------------------------------------------------------------
;  _divulong.s
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

	.globl __divulong

	.area CODE
__divulong:
	sub	sp, #6
;	device/lib/_divulong.c: 333: unsigned long reste = 0L;
	clrw	x
	ldw	(0x05, sp), x
	ldw	(0x03, sp), x
;	device/lib/_divulong.c: 334: unsigned char count = 32;
	ld	a, #0x20
	ld	(0x02, sp), a
;	device/lib/_divulong.c: 337: do
__divulong_00105:
;	device/lib/_divulong.c: 340: c = MSB_SET(x);
	ld	a, (0x0a, sp)
	sll	a
	clr	a
	rlc	a
	ld	(0x01, sp), a
;	device/lib/_divulong.c: 341: x <<= 1;
	ldw	y, (0x0c, sp)
	ldw	x, (0x0a, sp)
	sllw	y
	rlcw	x
	ldw	(0x0c, sp), y
	ldw	(0x0a, sp), x
;	device/lib/_divulong.c: 342: reste <<= 1;
	ldw	y, (0x05, sp)
	ldw	x, (0x03, sp)
	sllw	y
	rlcw	x
	ldw	(0x05, sp), y
	ldw	(0x03, sp), x
;	device/lib/_divulong.c: 343: if (c)
	tnz	(0x01, sp)
	jreq	__divulong_00102
;	device/lib/_divulong.c: 344: reste |= 1L;
	ld	a, (0x06, sp)
	or	a, #0x01
	ld	(0x06, sp), a
__divulong_00102:
;	device/lib/_divulong.c: 346: if (reste >= y)
	ldw	x, (0x05, sp)
	subw	x, (0x10, sp)
	ld	a, (0x04, sp)
	sbc	a, (0x0f, sp)
	ld	yl, a
	ld	a, (0x03, sp)
	sbc	a, (0x0e, sp)
	jrc	__divulong_00106
;	device/lib/_divulong.c: 348: reste -= y;
	ldw	(0x05, sp), x
	ld	yh, a
	ldw	(0x03, sp), y
;	device/lib/_divulong.c: 350: x |= 1L;
	ld	a, (0x0d, sp)
	or	a, #0x01
	ld	(0x0d, sp), a
__divulong_00106:
;	device/lib/_divulong.c: 353: while (--count);
	dec	(0x02, sp)
	jrne	__divulong_00105
;	device/lib/_divulong.c: 354: return x;
	ldw	x, (0x0c, sp)
	ldw	y, (0x0a, sp)
	addw	sp, #6
	retf
