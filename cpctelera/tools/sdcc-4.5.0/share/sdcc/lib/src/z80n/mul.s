;--------------------------------------------------------------------------
;  mul.s
;
;  Copyright (C) 2000, Michael Hope
;  Copyright (C) 2021, Philipp Klaus Krause
;  Copyright (C) 2024, Peter Ped Helcmanovsky
;  Copyright (C) 2024, Janko Stamenović
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

        .zxn

.area   _CODE

.globl	__mulint

__mulint:
__mul16_HL_DE_to_DE::
        ;; unsigned _mulint (unsigned m, unsigned n) __sdcccall(1);
        ;;
        ;; 16-bit multiplication
        ;;
        ;; Entry conditions
        ;; hl = multiplicand
        ;; de = multiplier
        ;;
        ;; Exit conditions
        ;; de = less significant word of product
        ;;
        ;; Registers used: AF, C,DE,HL
        ;
        ; Goal:  DE :=  H_0 L_0  *  D_0 E_0
        ; (_0 inital content of the registers,
        ;  16-bit register pairs contain
        ;  16-bit values to be multiplied.)
        ;
        ; see also: mul_16_16_16_AE (CC0)
        ld c, e
        ld e, l
        mlt de   ;
        ld a, e  ; A =  LOWBYTE( D_O*L_0 )
        ld e, c
        ld d, h
        mlt de   ;
        add a, e ; A += LOWBYTE( E_0*H_0 )
        ld e, c
        ld d, l
        mlt de   ; DE = E_0*L_0
        add a, d ; A += HIGHBYTE( E_0*L_0 )
        ld d, a
        ; total: 64T to here
        ; D = the upper product byte, summed
        ; E = LOW( E_0*L_0 )
        ret

