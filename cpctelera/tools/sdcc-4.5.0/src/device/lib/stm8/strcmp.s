;--------------------------------------------------------------------------
;  strcmp.s
;
;  Copyright (C) 2016-2021, Philipp Klaus Krause
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

	.globl _strcmp

	.area CODE

; int strcmp (const char *s1, const char *s2)
_strcmp:

	exgw	x, y
	ldw	x, (3, sp)

loop:
	ld	a, (y)
	jreq	null
	cp	a, (x)
	jrne	diff

	ld	a, (1, y)
	jreq	null_1
	cp	a, (1, x)
	jrne	diff

	ld	a, (2, y)
	jreq	null_2
	cp	a, (2, x)
	jrne	diff

	addw	y, #3
	addw	x, #3

	jra	loop

null_2:
	incw	x
null_1:
	incw	x
null:
	tnz	(x)
	jrne	less
	clrw	x
	jra	end

diff:
	jrult less
	ldw	x, #1
	jra	end

less:
	ldw	x, #-1

end:
	popw	y
	addw	sp, #2
	jp	(y)

