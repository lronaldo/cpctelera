/*-------------------------------------------------------------------------
;  __sdcc_atomic_maybe_rollback.c - C run-time: rollback for restartable
;  sequence implementation of C11 atomics
;
;  Copyright (c) 2024, Philipp Klaus Krause
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
;------------------------------------------------------------------------*/

#ifdef __SDCC_MODEL_FLAT24

static void dummy(void) __naked
{
	__asm
	.area HOME    (CODE)

; This relies on the restartable implementations being aligned properly.

___sdcc_atomic_maybe_rollback::
	push ar0
	mov  r0, SP
	dec  r0
	push psw
	cjne @r0, #(sdcc_atomic_exchange_rollback_start >> 16), 4$
	dec  r0
	cjne @r0, #(sdcc_atomic_exchange_rollback_start >> 8), 4$
	dec  r0
	cjne @r0, #<sdcc_atomic_exchange_rollback_start, 0$
0$:
	jc   4$
	cjne @r0, #sdcc_atomic_exchange_rollback_end, 1$
1$:
	jnc  4$
	; we now know the interrupted routine was somewhere among the
	; restartable implementations of atomic functions.
	push acc
	mov  a, @r0
	anl  a, #0x07
	cjne a, #6, 2$
2$:
	jnc  3$
	; we actually need to restart.
	mov  a, @r0
	anl  a, #0xf8
	mov  @r0, a
3$:	; inner skip
	pop acc
4$:	; outer skip
	pop psw
	pop ar0
	reti

	__endasm;
}

#endif

