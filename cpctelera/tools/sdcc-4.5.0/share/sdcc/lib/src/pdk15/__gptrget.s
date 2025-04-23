;--------------------------------------------------------------------------
;  __gptrget.s - read from pointer
;
;  Copyright (c) 2019, Philipp Klaus Krause
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
;  might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

.module __gptrget
.area CODE

__gptrget::
	sub	a, #0x80
	t1sn.io	f, c
	goto	code

	; Pointer to RAM
	idxm	a, p
	ret

	; Pointer to ROM
code:
	; Put pointer on stack
	xch	a, p
	push	af	; Put lower byte of pointer on stack.
	mov.io	a, sp
	add	a, #-1
	xch	a, p
	idxm	p, a	; Put upper byte of pointer on stack.

	; Jump to it. ret there will return from the call to __gprtget
	ret

