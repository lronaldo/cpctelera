;--------------------------------------------------------------------------
;  _divslong.s
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

	.globl __divslong

	.area CODE
__divslong:
	ldw x, (#0x0a, sp)
	ldw y, (#0x08, sp)
	jrpl __divslong_0
	callf __fast_long_neg
__divslong_0:
	pushw x
	pushw y
__divslong_1:
	ldw x, (#0x0a, sp)
	ldw y, (#0x08, sp)
	jrpl __divslong_2
	callf __fast_long_neg
__divslong_2:
	pushw x
	pushw y
__divslong_3:
	callf __divulong
	addw sp, #0x08
__divslong_4:
	ld a, (#0x04, sp)
	xor a, (#0x08, sp)
	jrpl __divslong_5
	callf __fast_long_neg
__divslong_5:
	retf
