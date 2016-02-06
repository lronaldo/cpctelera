;--------------------------------------------------------------------------
;  uchar2fps16x16.S - convert signed char to fixed16x16
;
;  Copyright (C) 2004, Vangelis Rokas <vrokas AT otenet.gr>
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

	radix dec

;--------------------------------------------------------
; public variables in this module
;--------------------------------------------------------
	global ___uchar2fps16x16

;--------------------------------------------------------
;	Equates to used internal registers
;--------------------------------------------------------
STATUS	equ	0xfd8
WREG	equ	0xfe8
FSR0L	equ	0xfe9
FSR1L	equ	0xfe1
FSR2L	equ	0xfd9
POSTDEC1	equ	0xfe5
PREINC1	equ	0xfe4
PLUSW1	equ	0xfe3
PRODL	equ	0xff3
PRODH	equ	0xff4


S_uchar2fps16x16____uchar2fps16x16	code
___uchar2fps16x16:

	movlw	0x01
	movff	PLUSW1, PRODH

	clrf	WREG
	clrf	PRODL
	clrf	FSR0L

	bcf	FSR0L, 7

@1:
	return

	end
