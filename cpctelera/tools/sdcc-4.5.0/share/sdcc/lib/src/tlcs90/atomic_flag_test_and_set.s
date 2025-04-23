;--------------------------------------------------------------------------
;  atomic_flag_test_and_set.s
;
;  Copyright (C) 2021, Philipp Klaus Krause
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

	.globl _atomic_flag_test_and_set

_atomic_flag_test_and_set:
	srl	(hl)
	ld	l, #1
	sbc	hl, #0
	ld	a, l
	ret
	
; Previous implementation (which might be needed again in the future, see below)
;	xor	a, a
;	tset	0, (hl)
;	ret	Z
;	inc	a
;	ret
; This implementation used 0 as clear state, 1 as set state,
; which seems closer to the spirit of atomic_flag. The current
; implementation fulfills the letter of C standards up to C23.
; At the time the previous implementation was written, there were
; proposals for the C standard to allow default-initialization of atomic_flag
; to the clear state, or allow assignments to and from atomic_flag
; (with clear being 0, and set being 1). While these proposals were not
; approved by SC22 WG14 (at least for now), they would have substantially
; complicated an implementation where set is internally represented as 0.
; SC22 WG14 might still go the way of one of these proposals in the future.
; The additional burden on implementations would not be considered undue:
; e.g. implementations where floating-point zero is not represented as
; all-zero bytes already have to bear it for floating-point types.
; On by far platforms today, default-initialization of atomic_flag to clear
; just works, so the is also a risk of developers expecting it to work.

