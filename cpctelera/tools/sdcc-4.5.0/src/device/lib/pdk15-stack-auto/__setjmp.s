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

	.area   CODE

___setjmp::
	; Get return address
	mov.io	a, sp
	add	a, #-1
	mov	p, a
	idxm	a, p
	push	af
	dec	p
	idxm	a, p
	push	af

	; Get Parameter (buffer)
	dec	p
	dec	p
	idxm	a, p
	mov	p, a

	; Store return address to buffer
	pop	af
	idxm	p, a
	inc	p
	pop	af
	idxm	p, a

	; Store stack pointer to buffer
	inc	p
	mov.io	a, sp
	idxm	p, a

	; Return 0
	clear	p
	ret	#0

_longjmp::
	; Push return value onto top of old stack (adding padding bytes)
	mov.io	a, sp
	add	a, #-6
	mov	p, a
	idxm	a, p
	push	af
	inc	p
	idxm	a, p
	push	af

	; Get pointer to buffer
	inc	p
	idxm	a, p
	mov	p, a

	; Push return address onto top of old stack (adding padding bytes)
	idxm	a, p
	push	af
	inc	p
	idxm	a, p
	push	af

	; Get new stack pointer
	inc	p
	idxm	a, p
	mov	p, a

	; Store return address to new stack
	pop	af
	dec	p
	idxm	p, a
	pop	af
	dec	p
	idxm	p, a

	; Store return value to new stack
	pop	af
	inc	p
	inc	p
	idxm	p, a
	pop	af
	inc	p
	idxm	p, a

	; Switch to new stack
	inc	p
	mov	a, p
	mov.io	sp, a

	; Get return value from stack
	mov	p, a
	dec	p
	idxm	a, p
	push	af
	inc	p
	inc	p
	idxm	a, p
	dec	p
	dec	p
	idxm	p, a
	pop	af
	mov	p, a
	pop	af
	xch	a, p

	; Return 1 if return value is 0
	ceqsn	a, p
	ret
	ceqsn	a, #0
	ret
	ret	#1

