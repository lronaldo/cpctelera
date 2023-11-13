;--------------------------------------------------------------------------
;  setjmp.s
;
;  Copyright (C) 2011-2014, Philipp Klaus Krause
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
	ld	hl, 0 (sp)
	ld	iy, 2 (sp)

	; Store return address.
	ld	0 (iy), hl

	; Store stack pointer.
	ld	2 (iy), sp

	; Store frame pointer.
	ld	4 (iy), ix

	; Return 0.
	sub	hl, hl
	ret

.globl _longjmp

_longjmp:
	pop	hl
	pop	iy
	pop	hl

	; Ensure that return value is non-zero.
	or	hl, hl
	jr	NZ, jump
	inc	hl
jump:

	; Restore frame pointer.
	ld	ix, 4 (iy)

	; Adjust stack pointer.
	ld	sp, 2 (iy)
	add	sp, #2

	; Jump.
	ld	bc, 0 (iy)
	jp	(bc)
