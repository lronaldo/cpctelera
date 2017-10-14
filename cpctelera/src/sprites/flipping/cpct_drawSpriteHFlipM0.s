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

.include "macros/cpct_reverseBits.h.s"

;; Parameter retrieval
   pop  hl     ;; [3] HL = return address
   pop  bc     ;; [3] BC = height / width
   pop  de     ;; [3] DE = Sprite start address pointer
   ex (sp), hl ;; [6] HL = Video memory address to draw the sprite, while leaving return address ...
               ;; ... in the stack, as this function uses __z88dk_callee convention
   
   jr firstrow	;; [3] Jump to start processing the first row of the sprite
   
;; This loop is repeated for every vertical row of the sprite
;;   Set HL to point to the end of present sprite row, as DE
;;   already points to the start of it.
;;
nextrow:
   ;; Next screen row, add up 0x800
   ld   a, h        ;; [1] |
   add #0x08        ;; [2] | HL += 0x800 (Make HL point to next row in ...
   ld   h, a        ;; [1] | ... screen video memory)
   
   ;; Check if HL has moved out of screen video memory boundaries
   and #0x38        ;; [2]   Check bits 13-11 of H
   jr  nz, firstrow ;; [2/3] if any of the 3 bits is not 0, boundaries have not been crossed

   ;; HL points outside screen video memory area (last +0x800 crossed boundaries)   
   ;; To wrap up to the start of video memory again, add up +0xC050
   ld   a, e        ;; [1] |
   add  #0x50       ;; [2] |  HL += 0xC050
   ld   e, a        ;; [1] |  (Move 0xC000, which is 3 memory banks, and...
   ld   a, d        ;; [1] |  ... 0x050, which is 1 character row, or 8 pixel ...
   adc  #0xC0       ;; [2] |  ... rows)
   ld   d, a        ;; [1] |

firstrow:
   push bc         ;; [4] Save width/height into the stack for later use

   ;; Make HL point to the end of the first screen video memory row + 1...
   ;; ... to simplify locating DE at each new row with an sbc command
   ld    b, #0     ;; [2] BC= C, setting bc = width to use it for math purposes 
   add  hl, bc     ;; [3] HL += C (as BC = C)

   jr   first      ;; [3] Start processing first byte from this row

nextbyte:
   inc  de         ;; [2] DE++ (Points to next byte from the sprite)
   dec  hl         ;; [2] HL-- (Poinst to previous screen memory location)

first:
   ld   a, (de)    ;; [2] A=byte to be reversed	
   ld   b, a       ;; [1] B=A (Copy to be used later for mixing bits)

   cpctm_reverse_mode_0_pixels_of_A b ;; [7] Reverses pixel order in A (Using B as temporary storage)
   
   ld (hl), a      ;; [2] Copy reversed byte to screen video memory
   
   dec  c          ;; [1] C-- (One less byte to be reversed)
   jr   nz, nextbyte ;; [2/3] If C!=0, process next byte, as there are more in this line

;; Finished reversing present byte row from the sprite
;; 
end:
   pop  bc        ;; [3] Recover BC (B:height, C: width)

   djnz nextrow   ;; [3/4] B--, if B!=0, there are next rows waiting to be processed

   ret            ;; [3] All sprite rows reversed. Return.
