;; >> GPL info
;; function: cpct_drawSpriteMirroredM0
;;   Draws a normal sprite mirrored left-to-right to a screen or backbuffer 
;; in mode 0.
;;
;; C definition:
;;   void <cpct_drawSpriteMirroredM0> (<u8> width, <u8> height, <u8>*sprite, <u8>* pvmem);
;;
;; Input parameters:
;;    de - sprite
;;    hl - video memory pointer
;;    bc - height / width
;;
;; Only for mode 0 (changes byte reversing routine)
;;
;; Measures:
;;   Space:
;;   Time:   (20W + 23)H + 7HH + 11 us (-15 in asm)
;;
;; Use example:
;; (start code)
;; (end code)
;;

.include "macros/cpct_undocumentedOpcodes.h.s"
.include "macros/cpct_reverseBits.h.s"

_cpct_drawSpriteHFlipM0::

;; Parameter retrieval
   pop   af    ;; [3] AF = Return address
   pop   de    ;; [3] DE = Sprite start address pointer
   pop   hl    ;; [3] HL = Video memory address to draw the sprite
   pop   bc    ;; [3] BC = height / width
   push  af    ;; [4] Leave only return address in the stack, 
               ;; ... as this function uses __z88dk_callee convention
   
   push  ix    ;; [5] Save IX
   
   ld__ixl_b   ;; [2] IXL = Height

   ld     a, c          ;; [1]
   ld (widthRestore), a ;; [4]
   ld (addWidth), a     ;; [4]

   ;; HL += width - 1 (Point to last byte at the screen)
   dec    a       ;; [1]
   add    l       ;; [1]
   ld     l, a    ;; [1]
   adc    h       ;; [1]
   sub    l       ;; [1]
   ld     h, a    ;; [1]

   jr  firstbyte  ;; [3]

nextrow:
widthRestore = .+1
   ld     c, #00        ;; [2] Width restore
nextbyte:
   inc   de             ;; [2]
   dec   hl             ;; [2]
firstbyte:
   ld     a, (de)       ;; [2]
   ld     b, a          ;; [1]
   cpctm_reverse_mode_0_pixels_of_A b ;; [7] Reverses pixel order in A (Using B as temporary storage)
   ld  (hl), a          ;; [2]

   dec    c             ;; [1]
   jr    nz, nextbyte   ;; [2/3]

   dec__ixl             ;; [2]
   jr     z, return     ;; [2/3]

addWidth = .+1
   ld    bc, #0x0800    ;; [3] HL += 0x800 + width 
   add   hl, bc         ;; [3]
 
   ;; Check if HL has moved out of screen video memory boundaries
   ld     a, h          ;; [1]
   and   #0x38          ;; [2]   Check bits 13-11 of H
   jr    nz, nextbyte   ;; [2/3] if any of the 3 bits is not 0, boundaries have not been crossed

   ;; HL points outside screen video memory area (last +0x800 crossed boundaries)   
   ;; To wrap up to the start of video memory again, add up +0xC050
   ld    bc, #0xC050    ;; [3] HL += 0xC050
   add   hl, bc         ;; [3]
   jr    nextrow        ;; [3]
return:
   pop   ix             ;; [4]
   ret                  ;; [3]