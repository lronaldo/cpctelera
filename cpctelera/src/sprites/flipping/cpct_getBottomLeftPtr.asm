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
;;  * *height* (1-256) must be the height of the sprite in bytes. Height of a sprite in
;; bytes and pixels is the same value.
;;
;; Known limitations:
;;	<TODO>
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
;;  Best      |          57            | 		228
;;  Worst     |          64            |      	256
;; ----------------------------------------------------------------
;; Asm saving |         -13            |        -52
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = [(H-1)/8]
;;
;; Credits:
;;    This routine was inspired in the original *cpc_PutSprite* from
;; CPCRSLib by Raul Simarro.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Save Memory Bank
ld     a, #0xC0   ;; [2]
and    d          ;; [1] A = Bits 14-15 of DE (Memory 16K Bank 00-11)
ex    af, af'     ;; [1] Save A into A'

;; He1 = Height - 1
;; DE += 0x800 * (He1 % 8)
dec    c          ;; [1] C-- (C = Height-1 = He1)
ld     a, #0x07   ;; [2]
and    c          ;; [1] A = He1 % 8
rlca              ;; [1] |
rlca              ;; [1] |
rlca              ;; [1] | A = 8 ( He1 % 8 )

add    d          ;; [1] |
ld     d, a       ;; [1] | DE += 0x100 * 8 (He1 % 8)

;; HL = 80 * int(He1 / 8)
;; (note 80 = 0x50)
ld     a, #0xF8   ;; [2]
and    c          ;; [1] A = 8 * int(He1 / 8)
ld     h, #0x00   ;; [2]
ld     l, a       ;; [1] HL =  8 * int(He1 / 8)
add   hl, hl      ;; [3] HL = 16 * int(He1 / 8)
ld     b, h       ;; [1]
ld     c, l       ;; [1] BC = 16 * int(He1 / 8)
add   hl, hl      ;; [3] HL = 32 * int(He1 / 8)
add   hl, hl      ;; [3] HL = 64 * int(He1 / 8)
add   hl, bc      ;; [3] HL = 80 * int(He1 / 8)

;; DE = 0x800 * (He1 % 8) + 0x50 * int(He1 / 8)
add   hl, de      ;; [3]

;; End of page?
ex    af, af'     ;; [1] Restore A (Saved memory 16K bank)
xor    h          ;; [1]
and   #0xC0       ;; [2] Is HL pointing to the same 16K bank as DE at the start?
ret    z          ;; [2/4] Yes, return. No, cycle.

;; Cycle and jump to next row
ld    bc, #0xC050 ;; [3] Make memory cycle and add 0x50 to jump to the next row
add   hl, bc      ;; [3]
ret               ;; [3] Return


