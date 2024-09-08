;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;
;;  Created by Nesu 2022. Optimized by Joaquín Ferrero & Nesu in 2023
;;  Copyright (C) 2021 Joaquín Ferrero (https://github.com/joaquinferrero)
;;  Copyright (C) 2021 Nestornillo (https://github.com/nestornillo)
;;  Copyright (C) 2024 raulgarfer (https://github.com/raulgarfer)
;;  Copyright (C) 2024 cpcitor ( https://github.com/cpcitor)
;;  Copyright (C) 2024 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU Lesser General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU Lesser General Public License for more details.
;;
;;  You should have received a copy of the GNU Lesser General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
.module cpct_divideby10
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_divideby10
;;
;; Returns a byte that is the result of dividing an integer number by 10.
;;
;; C Definition:
;;    <u8> <cpct_divideby10> (u8 dividend) __z88dk_fastcall;
;;
;; Assembly call:
;;    > call cpct_divideby10_asm
;;
;; Input Parameters (1 Byte):
;;    (1B A ) dividend  - [0-255] Number to be divided Byte
;;
;; Return Value:
;;    (A) u8 - Result of the operation.
;;
;; Parameter Restrictions:
;;    * Number must be among 0 and 255. 
;;
;; Known limitations:
;;
;; Details:
;; This function returns the integer result of dividing a given number
;; by 10 (decimal). The function is exact for integer division
;; for all integers within the byte range [0-255]. The function does not return
;; a remainder, nor does it use decimals in its operations. 
;; If the input number is greater than 127, function does a calculation
;; to make corrections to avoid possible failures.
;
;; This is a simplified explanation, the code actually groups partial results 
;; together so as to minimize the number of shift-right operations,
;; and it happens that the result is more precise that way.
;;
;; We could compute something different, like:
;; ( M + M>>1 + M>>3 ) >> 3
;; This would be smaller because of the lost bits to the right.
;;
; A number divided by 10 is equal to that number divided by 2 and multiplied
;; by 1/5 (0.20). For example, if we consider N = 20, then:
;; 1) 20/2 * 1/5 
;; 2) 10/1 * 1/5 
;; 3) 10/5 
;; 4) 2
;; So, 20/10 = 2.  
;; This operation is close to 13/64; if we multiply 1 by 13 and 5
;; by 13, the result is 13/65, close to 13/64. The approximation used by
;; the routine works well up to 127, but then it 'shifts' from
;; the actual result of A/10. It is necessary to adjust for higher values.
;; N/10 = N/2 * 1/5, which is close to N/2 * 13/64.
;; The approximation 13/64 can be refactored to:
;; 1) (1   + 4   +  8) / 64, which is equal to
;; 2) (1/8 + 4/8 + 8/8 ) / 8
;; 3) (1/8 + 1/2 + 1/1 ) / 8
;; Previously, we omitted N/2; let's insert it now:
;; 4) N/2 * 1 / 5 is similar to
;; 5) N/2 * (1/8  + 1/2 + 1/1) / 8, which refactors to
;; 6)       (N/16 + N/4 + N/2) / 8
;; Taking the example of number 125, perform the operations:
;; 1) (125/16 + 125/4 + 125/2) / 8
;; 2) (   7   +   31  +   62  ) / 8
;; 3) (           99          ) / 8
;; 4)             12
;;
;; Destroyed Register values:
;;    AF, BC
;;
;; Required memory:
;;    C-bindings - 4 bytes
;;  ASM-bindings - 1 byte
;;
;; Time Measures: 
;; (start code)
;;     Case   | microSecs (us) | CPU Cycles
;; -----------------------------------------
;;     Any    |      33        |     132
;; -----------------------------------------
;; Asm saving |     -12        |     -48
;; -----------------------------------------
;; (end code)
;;
	ld bc,#0xC0FE ; [3]  C0 used as a threshold (-64),
	              ; FE will be used to unset bit 0 twice later
	srl a         ; [2]  A = N/2

;; At this point we've divided N by two, let's call M=N/2. That was the easy part.
;; Now we'd like to compute M/5.
;; 1/5 = 0.2
;; What if we tried to multiply instead?
;; 13/64 = 0.203125 which is a little bigger than 0.2 .
;; So we consider M*13/64 as a guide and we'll make it exact with some nudge.

;; We'll do something that looks like
;; M*13/64 = (M<<3 + M<<2 + M) >> 6

;; Remember, this is too big because 13/64 > 1/5 .

;; Also, M*13 will overflow a byte as soon as M reaches 20, so we
;; don't want to do this on a Z80.
;; Remember, it's explained before.
;; We could compute something different, like:
;; ( M + M>>1 + M>>3 ) >> 3
;; This would be smaller because of the lost bits to the right.
;; Could this be exact? No, it is too small.

;; Actually, since Z80 does not have multi-bit shift, we prefer to
;; shift and combine, as many times as needed.
;; Also it happens that the result is exact up to N=128.
;; Even better, it is enough to subtract 2 to N for 128<=N<256 to get
;; it exact there too.

;; The combined effect of the next two instructions is to
;; decrement A if bigger than 64 in only 2 bytes 2µs
	add b   ; [1]  Corrupt A, also set Carry flag if b>=64 (eqv N>=128 or M>=64 )
	sbc b   ; [1]  Restore A, minus one if N>=128.
;; The instructions above can be skipped if the input range is
;; known to be restricted to 0<=N<128

;; let X=N/2 if N<=127, or N/2-1 if N>=128
	ld b,a   ; [1] A = B = X

;; Next trick: quickly shift right.
;; `srl a` consumes 2 bytes and 2µs.
;; `rra` with a clear Carry does the same as `srl a` in only 1 byte 1µs.
;; A nice trick is to prefix two `rra` instructions with one
;; instruction that make both behave like `srl a` but cheaper.
;; The combined effect of the next 3 instructions is to shift A
;; by 2 bits to the right (divide by 4) in 3µs, 3 bytes.
	and c   ; [1]  clear bit 0 and Carry,
;; to allow use of two `rra` to perform two right shifts
	rra     ; [1]  A = X >> 1, Carry clear
	rra     ; [1]  A = X >> 2, Carry might be set
	add b   ; [1]  A = (X >> 2) + X,
	        ; also clear Carry so that next `rra` is actually a right shift
	rra     ; [1]  A = ((X >> 2) + X) >> 1
	add b   ; [1]  A = (((X >> 2) + X) >> 1) + X,
	        ; also clear Carry so that next `rra` is actually a right shift

;; Let's call the value Y = (((X >> 2) + X) >> 1) + X
;; Now we only need 3 right shifts to compute Y >> 3.
;; Since Carry is cleared, the first one is straightforward.
	rra     ; [1]  A = Y >> 1, also Carry might be set.

;; the combined effect of the next 3 instructions is to shift A
;; by 2 bits to the right (divide by 4) in 3µs, 3 bytes
	and c   ; [1]  clear bit 0 and Carry,
	        ; to allow use of two `rra` to perform two right shifts
	rra     ; [1]  A = Y >> 2, Carry clear
	rra     ; [1]  A = Y >> 3, Carry might be set
