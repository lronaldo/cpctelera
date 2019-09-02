;--------------------------------------------------------------------------
;  __setjmp.s
;
;  Copyright (C) 2019, Philipp Klaus Krause
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

        .area DATA
___setjmp_PARM_1::
        .ds 2
_longjmp_PARM_1::
	.ds 2
_longjmp_PARM_2::
	.ds 2

	.area   CODE

___setjmp::
	mov	a, sp
	add	a, #-1
	mov	p, a
	idxm	a, p
	push	af
	dec	p
	idxm	a, p

	xch	a, p
	mov	a, ___setjmp_PARM_1+0
	xch	a, p

	idxm	p, a
	inc	p
	pop	af
	idxm	p, a

	inc	p
	mov	a, sp
	idxm	p, a
	clear	p
	ret	#0

_longjmp::
	mov	a, _longjmp_PARM_1+0
	add	a, #2
	mov	p, a
	idxm	a, p
	add	a, #-2
	mov	sp, a

	dec	p
	dec	p
	idxm	a, p
	push	af
	inc	p
	idxm	a, p

	xch	a, p
	mov	a, sp
	add	a, #-1
	xch	a, p
	idxm	p, a

	mov	a, _longjmp_PARM_2+1
	mov	p, a
	mov	a, _longjmp_PARM_2+0

	ceqsn	a, p
	ret
	ceqsn	a, #0
	ret
	ret	#1

