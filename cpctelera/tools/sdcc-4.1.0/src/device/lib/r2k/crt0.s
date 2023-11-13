;--------------------------------------------------------------------------
;  crt0.s - Generic crt0.s for a Rabbit 2000
;	derived from "Generic crt0.s for a Z80"
;
;  Copyright (C) 2000, Michael Hope
;  Modified for Rabbit by Leland Morrison 2011
;  Copyright (C) 2020, Philipp Klaus Krause
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

	.module crt0
	.globl	_main
	.globl	__sdcc_external_startup

GCSR		.equ	0x00 ; Global control / status register
MMIDR		.equ	0x10
STACKSEG	.equ	0x11
SEGSIZE		.equ	0x13
MB0CR		.equ	0x14 ; Memory Bank 0 Control Register
MB1CR		.equ	0x15 ; Memory Bank 1 Control Register
MB2CR		.equ	0x16 ; Memory Bank 2 Control Register
MB3CR		.equ	0x17 ; Memory Bank 3 Control Register

	.area	_HEADER (ABS)

	; Reset vector - assuming smode0 and smode1 input pins are grounded
	.org 	0

	; setup internal interrupts
	ld	a, #1
	ld	iir, a

	; Configure physical address space.
	; Leave MB0CR Flash at default slow at /OE0, /CS0
	; Assume slow RAM at /CS1, /OE1, /WE1
	ld	a, #0x05
	ioi
	ld	(MB2CR), a;

	; Configure logical address space. 32 KB root segment followed by 8 KB data segment, 16 KB stack segement, 8 KB xpc segment.
	; By default, SDCC will use the root segment for code and constant data, stack segment for data (including stack). data segment and xpc segement are then unused.
	ld	a, #0xa8	; 16 KB stack segment at 0xa000, 8 KB data segment at 0x8000
	ioi
	ld	(SEGSIZE), a

	; Configure mapping to physical address space.
	ld	a, #0x76
	ioi
	ld	(STACKSEG), a	; stack segment base at 0x76000 + 0xa000 = 0x80000

	; Set stack pointer directly above top of stack segment
	ld	sp, #0xe000

	call __sdcc_external_startup

	; Initialise global variables
	call	gsinit

	call	_main
	jp	_exit

	; Periodic Interrupt
	.org	0x100
	push	af
	ioi
	ld	a, (GCSR) ; clear interrupt
	pop	af
	reti

	; Secondary Watchdog - Rabbit 3000A only
	.org	0x100
	reti

	; rst 0x10
	.org	0x120
	ret

	; rst 0x18
	.org	0x130
	ret

	; rst 0x20
	.org	0x140
	ret

	; rst 0x28
	.org	0x150
	ret

	; Syscall instruction - Rabbit 3000A only
	.org	0x160
	ret

	; rst 0x38
	.org	0x170
	ret

	; Slave Port
	.org	0x180
	reti

	; Timer A
	.org	0x1a0
	reti

	; Timer B
	.org	0x1b0
	reti

	; Serial Port A
	.org	0x1c0
	reti

	; Serial Port B
	.org	0x1d0
	reti

	; Serial Port C
	.org	0x1e0
	reti

	; Serial Port D
	.org	0x1f0
	reti

	.org	0x200

	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL

	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP

	.area   _CODE
_exit::
	;; Exit - special code to the emulator
	ld	a,#0
	rst     #0x28
1$:
	;halt		; opcode for halt used for 'altd' on rabbit processors
	jr	1$

	.area   _GSINIT
gsinit::
	ld	bc, #l__DATA
	ld	a, b
	or	a, c
	jr	Z, zeroed_data
	ld	hl,	#s__DATA
	ld	(hl), #0x00
	dec	bc
	ld	a, b
	or	a, c
	jr	Z, zeroed_data
	ld	e, l
	ld	d, h
	inc	de
zero_loop:
	ldi	; Work around new ldir wait state bug.
	jp	LO, zero_loop

zeroed_data:

	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
copy_loop:
	ldi	; Work around new ldir wait state bug.
	jp	LO, copy_loop
gsinit_next:

	.area   _GSFINAL
	ret

