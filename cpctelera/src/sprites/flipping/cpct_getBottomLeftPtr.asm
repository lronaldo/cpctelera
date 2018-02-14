;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_getBottomLeftPtr
;;
;;    Gets a pointer to the bottom-left byte of a sprite in video memory, knowing
;; its top-left byte and height.
;;
;; C Definition:
;;    void <cpct_getBottomLeftPtr> (void* *memory*, <u16> *height*) __z88dk_callee;
;;
;; Input Parameters (3 bytes):
;;  (2B DE) memory - Video memory pointer to the top-left corner of a sprite
;;  (1B C ) height - Sprite Height
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_getBottomLeftPtr
;;
;; Parameter Restrictions:
;;  * *memory* must be a pointer to top-left corner of a sprite in video memory.
;; It could be any place in memory, inside or outside current video memory. 
;; It will be equally treated as video  memory (taking into account CPC's video 
;; memory disposition). 
;;  * *height* (1-256) must be the height of the sprite. A value of 0 will be 
;; interpreted as 256. A value of 1 will return *memory* pointer unchanged.
;;
;; Known limitations:
;; <TODO>
;;
;; Details:
;;    <TODO>
;;
;; Destroyed Register values: 
;;    AF', AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings - 40 bytes
;;   ASM-bindings - 36 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;  Best      |          57            |     228
;;  Worst     |          64            |        256
;; ----------------------------------------------------------------
;; Asm saving |         -13            |        -52
;; ----------------------------------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; A' = Bits 14-15 of DE memory address (Memory 16K bank number 0-3)
;; We save this information into A' to check it later against the final
;; calculated address and know if we have gone outside our memory bank.
ld     a, #0b11000000   ;; [2] Bits to save
and    d                ;; [1] A = D & 0xC0 = Bits 14-15 of DE (Memory 16K Bank 00-11)
ex    af, af'           ;; [1] Save A into A'

;; We need height-1 (We will call it He1) because we 
;; want to point to the bottom-left of the sprite (last line)
;; and not to the line below the sprite itself
dec    c          ;; [1] C-- (C = Height-1 = He1)

;; As memory is made of 8-line characters, and we need to jump 0x800
;; from line to line, we need to know how many of these lines we will need
;; to jump. That number is (He1 % 8): the rest after jumping the number
;; of blocks of 8-lines (that would be (He1 / 8))
;; So we perform the operation DE += 0x800 * (He1 % 8)
;;
ld     a, #0b00000111   ;; [2] 3 latest bits to perform modulo 8 operation
and    c                ;; [1] A = C % 8 = He1 % 8
;; Multiplying it by 0x800 is multiplying it by 8, and then by 0x100,
;; what is just adding result to d, as if it has 8 trailing zeros
rlca              ;; [1] / 
rlca              ;; [1] | 
rlca              ;; [1] \ A = A << 8 = 8 ( He1 % 8 )
add    d          ;; [1] /
ld     d, a       ;; [1] \ DE = DE + A * 0x100 = 8 * 0x100 * (He1 % 8) = 0x800 * (He1 % 8)

;; Then we also need to add the number of 8-line characters we need to 
;; jump multiplied by 0x50, which is the distance in memory from the start 
;; of an 8-line to the next. The number of 8-line characters is the 
;; integer part of (height / 8). Therefor, we perform this operation
;;    HL = DE + 0x50 * int(He1 / 8)      [ Note that 0x50 is 80 in decimal ]
;;
;; Shifting C 3 times right would give us int(He1 / 8). However, as we need to
;; multiply that quantity by 8, it is easier to calculate 8 * int(He1 / 8)
;; that would be shifting C 3 times right, then 3 times left. That is the same
;; as making 0 the latest three bits.
ld     a, #0b11111000   ;; [2] Make 0 the latest 3 bits
and    c                ;; [1] To get A = 8 * int(He1 / 8)
ld     h, #0x00         ;; [2] / Put it into HL
ld     l, a             ;; [1] \ HL =  8 * int(He1 / 8)
;; And know we multiply up to 80 * int(He1 / 8)
add   hl, hl      ;; [3] HL = 16 * int(He1 / 8)    // 8 * 2
ld     b, h       ;; [1] /
ld     c, l       ;; [1] \ BC = 16 * int(He1 / 8)
add   hl, hl      ;; [3] HL = 32 * int(He1 / 8)    // 16 * 2
add   hl, hl      ;; [3] HL = 64 * int(He1 / 8)    // 32 * 3
add   hl, bc      ;; [3] HL = 80 * int(He1 / 8)    // 64 + 16

;; We finally add it all to HL
add   hl, de      ;; [3] HL = HL + DE = 0x50 * int(He1 / 8) + 0x800 * (He1 % 8)

;; Now we use the bits we saved into A' to compare it against bits 14-15 
;; of HL. If both are the same, that means the final address is in the same
;; 16K memory bank, and no correction is needed. 
ex    af, af'     ;; [1] A = A'     (Restore saved memory 16K bank)
xor    h          ;; [1] A ^= H     (XOR will make bits 14-15 = 0, ONLY IF both are equal in A and H)
and   #0b11000000 ;; [2] A &= 0xC0  (Make 0 all bits except 14-15)
ret    z          ;; [2/4] If result is Zero, HL is at the same memory bank, then we return.
   
;; If Ret Z failed, it means that HL points to a different 16K memory bank than 
;; the initial pointer we received into DE. Therefore, our calculations have made
;; our address jump to the next bank. That means we need to correct. Correction 
;; includes adding 0x50 (to jump one more 8-lines block ahead) and also jump
;; 3 16K memory banks ahead to perform a full cycle around the 4 16K banks of memory.
;; That will place our final pointer in the same memory bank as it started, but
;; correctly advanced 1 more 8-lines character ahead
ld    bc, #0xC050 ;; [3] BC = 0xC000 + 0x50 (Size of 3 16K Banks + 0x50 for the 8-lines block)
add   hl, bc      ;; [3] HL = HL + BC = HL + 0xC000 + 0x50
ret               ;; [3] Final address is ready, return.


