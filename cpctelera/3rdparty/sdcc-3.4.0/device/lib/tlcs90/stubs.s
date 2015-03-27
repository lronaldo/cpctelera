;--------------------------------------------------------------------------
;  stubs.s
;
;  Copyright (C) 2001, Michael Hope
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

; Just stubs - not copyrightable

        ;; Stubs to match between function names
        .area	_CODE

        .globl  __mullong
        .globl  __modslong
        .globl  __modulong
        .globl  __divslong
        .globl  __divulong
        .globl	__divschar_rrx_s
        .globl	__divuchar_rrx_s
        .globl	__divsuchar_rrx_s
        .globl	__divuschar_rrx_s
        .globl	__divsint_rrx_s
        .globl	__divuint_rrx_s
        .globl	__mulschar_rrx_s
        .globl	__muluchar_rrx_s
        .globl	__mulsuchar_rrx_s
        .globl	__muluschar_rrx_s
        .globl	__mulint_rrx_s
        .globl  __modschar_rrx_s
        .globl  __moduchar_rrx_s
        .globl  __modsuchar_rrx_s
        .globl  __moduschar_rrx_s
        .globl  __moduint_rrx_s
        .globl  __modsint_rrx_s

__mullong_rrx_s::
__mullong_rrf_s::
        jp      __mullong

__modslong_rrx_s::
__modslong_rrf_s::
        jp      __modslong

__modulong_rrx_s::
__modulong_rrf_s::
        jp      __modulong

__divslong_rrx_s::
__divslong_rrf_s::
        jp      __divslong

__divulong_rrx_s::
__divulong_rrf_s::
        jp      __divulong


