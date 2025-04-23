;--------------------------------------------------------------------------
;  crt0.s -Generic crt0.s for a GBZ80.
;
;  Copyright (C) 2000, Michael Hope
;  Copyright (C) 2021 - 2022, Sebastian 'basxto' Riedel (sdcc@basxto.de)
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

        .globl  _main

        .area   _HEADER (ABS)
        ;; Reset vector
        .org    0x00
        jp      init
        ;; Used by regression tests
        .org    0x08
        reti
        .org    0x10
        reti
        .org    0x18
        reti
        .org    0x20
        reti
        .org    0x28
        reti
        .org    0x30
        reti
        ;; 0xFF (rst 0x38) is the default value of empty memory
        .org    0x38
        jp      _exit

        ;; Interrupt vector
        .org    0x40
        reti
        .org    0x48
        reti
        .org    0x50
        reti
        .org    0x58
        reti
        .org    0x60
        reti

        .org    0x100
        jr      init

        .org    0x150
init:
        di
        ;; Set stack pointer directly above top of Work RAM.
        ld      sp,#0xe000

        call	___sdcc_external_startup

        ;; Initialise global variables. Skip if __sdcc_external_startup returned
        ;; non-zero value. Note: calling convention version 1 only.
        or      a, a
        call    Z, gsinit

        ;; Use _main instead of main to bypass sdcc's intelligence
        call    _main
        jp      _exit

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
__clock::
        ld      a,#2
        rst     0x08
        ret

_exit::
        ;; Exit - special code to the emulator
        ld      a,#0
        rst     0x08
1$:
        halt
        jr      1$

        .area   _GSINIT
gsinit::
        ; Default-initialized global variables.
        ld      hl, #s__DATA
        ld      bc, #l__DATA + 0x0101
        xor     a, a
        jr      loop_implicit_compare
loop_implicit:
        ld	(hl+), a
loop_implicit_compare:
        dec     c
        jr      NZ, loop_implicit
        dec     b
        jr      NZ, loop_implicit
zeroed_data:
        ; Explicitly initialized global variables.
        ld	de, #s__INITIALIZED
        ld	hl, #s__INITIALIZER
        ld	bc, #l__INITIALIZER + 0x0101
        jr      loop_explicit_compare
loop_explicit:
        ld	a, (hl+)
        ld	(de), a
        inc	de
loop_explicit_compare:
        dec     c
        jr      NZ, loop_explicit
        dec     b
        jr      NZ, loop_explicit

gsinit_next:

        .area   _GSFINAL
        ret

