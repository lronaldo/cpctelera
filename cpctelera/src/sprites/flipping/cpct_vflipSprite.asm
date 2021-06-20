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
;; Function: cpct_vflipSprite
;;
;;   Flips a sprite vertically in-place, modifying it.
;;
;; C definition:
;;   void <cpct_vflipSprite> (<u8> width, <u8> height, void* spbl, void* sprite) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (1B  C) width  - Width of the sprite in *bytes* (*NOT* in pixels!). Must be >= 1.
;;  (1B  B) height - Height of the sprite in pixels / bytes (both are the same). Must be >= 1.
;;  (2B DE) spbl   - Pointer to the bottom-left byte of the sprite
;;  (2B HL) sprite - Pointer to the sprite array (top-left byte)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_vflipSprite_asm
;;
;; Parameter Restrictions:
;;  * *width*  [1-255] must be the width of the sprite *in bytes* (Not! in pixels).
;;  * *height* [2-255] must be the height of the sprite in pixels.
;;  * *spbl*   must be a pointer to the bottom-left byte of the *sprite*.
;;  * *sprite* must be a pointer to an array containing sprite's pixels data.
;;
;; Known limitations:
;;  * This function will not work from ROM, as it uses self-modifying code.
;;  * This function does not do any kind of boundary check. If you give it 
;; incorrect values for *width*, *height*, *spbl* or *sprite* it might 
;; potentially alter the contents of memory locations beyond *sprite* boundaries. 
;; This could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that your values are correct.
;;  * This function assumes that *sprite* data is stored consecutive into
;; memory. Optimized or special ways to store *sprite* should not use this
;; function, as results may be unpredictable.
;;
;; Details:
;;    This function flips a *sprite* vertically in-place, modifying *sprite*
;; memory. It does it by interchanging top and bottom rows one by one. As this 
;; function does only interchange sprite rows, it is valid for any video 
;; mode: pixel data will stay the same. It is also valid for sprites with
;; interlaced mask. Such sprites will have 2x*width* bytes per row. Therefore,
;; if you call this function with 2x*width* as *width* parameter, it will
;; perform a valid vertical flipping for sprites with interlaced mask.
;;
;;    Next example shows how to flip an sprite vertically,
;; (start code)
;;    //
;;    // Flip the sprite of an entity vertically
;;    //
;;    void verticallyFlipEntity(TEntity* e) {
;;       // Calculate a pointer to bottom-left row of entity's sprite 
;;       // The flipping function requires this pointer
;;       u8* sblp = cpctm_spriteBottomLeftPtr(e->sprite, e->width, e->height);
;;
;;       // Flip the sprite vertically
;;       cpct_vflipSprite(e->width, e->height, sblp, e->sprite);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, AF', BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 37 bytes
;;  ASM-bindings - 33 bytes
;;
;; Time Measures:
;; (start code) 
;;  Case       |      microSecs (us)       |         CPU Cycles          |
;; -----------------------------------------------------------------------
;;  Any        |     19 + (12 + 18W)HH     |     76 + (48 + 72W)HH       |
;; -----------------------------------------------------------------------
;;  W=2,H=16   |           403             |           1612              |
;;  W=5,H=32   |          1651             |           6604              |
;; -----------------------------------------------------------------------
;;  Asm saving |           -15             |            -60              |
;; -----------------------------------------------------------------------
;; (end code)
;;   *W* = *width*, *H* = *height*, *HH* = int(*height*/2)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.include "macros/cpct_maths.h.s"

   ;; Store width in its placeholder. This will enable it
   ;; to be restored at every row iteration
   ld     a, c       ;; [1] A = sprite width
   ld (wrestore), a  ;; [5] Store sprite width in its restore-placeholder
   
   ;; Calculate the number of rows that need to be interchanged (B)
   srl    b          ;; [2] B /= 2. Half the height of the sprite is the number of rows to interchange

   jr    byte_loop   ;; [3] Jump over next-row-loop for the first iteration

row_loop:
   ;; Set up everything for next row to be processed
   ;; HL already points to next row
   ;; DE must be moved to previous row, and width must be restored
wrestore=.+1
   ld    a, #00         ;; [2] A = sprite width (#00 is a placeholder that gets modified)
   ld    c, a           ;; [1] C = sprite width 
   add   a              ;; [1] A = 2*A (We need to take 2 times the width from DE to make it point to previous row)
   sub_de_a             ;; [7] DE -= 2*width (Make DE point to previous row)

   ;; Interchange rows pointed by HL and DE. 
   ;; HL Points to the upper part of the sprite, whereas DE starts from the bottom
byte_loop:
      ld     a, (hl)    ;; [2] / 
      ex    af, af'     ;; [1] \ A' = top-row byte
      ld     a, (de)    ;; [2] A = bottom-row byte
      ld  (hl), a       ;; [2] Store bottom-row byte into the top-row
      ex    af, af'     ;; [1] /
      ld  (de), a       ;; [2] \ Store top-row byte into the bottom-row
      inc   hl          ;; [2]  / Increment Both row pointers 
      inc   de          ;; [2]  \ to point to the next bytes

   dec   c              ;; [1]   --C (1 less byte into the width of this row)
   jr   nz, byte_loop   ;; [2/3] If C!=0, continue with next byte of this row

   ;; Finished processing present row. Decrement B (height counter) and
   ;; continue with next row, in case
   djnz row_loop        ;; [3/4] --B. If B!=0, continue with next row

ret                     ;; [3]  Return to the caller