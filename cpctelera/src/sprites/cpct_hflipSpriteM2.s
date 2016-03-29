;; >> GPL info
;; function: cpct_mirrorSpriteM2
;;   Mirrors a mode 2 encoded sprite left-to-right and viceversa
;;
;; C definition:
;;   void <cpct_mirrorSpriteM2> (<u8>*sprite, <u8> width, <u8> height);
;;
;; Input parameters:
;;    de - sprite
;;    hl - alto / ancho
;;
;; Only for mode 2 (changes byte reversing routine)
;;
;; Measures:
;;   Space:
;;   Time:    24H + (55WW + 24W)H + 14 us 
;;
;; Use example:
;; (start code)
;;   // Draws the main character sprite always looking to the 
;;   // appropriate side (right or left), reversing it whenever required
;;   void drawCharacter(u8 lookingat) {
;;
;;      // Check if we have to reverse character sprite or not
;;      if(lookingAt != wasLookingAt) {
;;         cpct_mirrorSpriteM2(characterSprite, 4, 8);
;;         wasLookingAt = lookingAt;
;;      }
;;
;;      // Draw main character's sprite
;;      cpct_drawSprite(characterSprite, pmem, 4, 8);
;;   }
;; (end code)
;;

.include /cpct_asmMacros.s/

;; Parameter retrieval
   pop  hl     ;; [3] HL = return address
   pop  de     ;; [3] DE = Sprite start address pointer
   ex (sp), hl ;; [6] HL = height / width, while leaving return address in the
               ;; ... stack, as this function uses __z88dk_callee convention

;; This loop is repeated for every vertical row of the sprite
;;   Set HL to point to the end of present sprite row, as DE
;;   already points to the start of it.
;;
nextrow:
   push hl      ;; [4] Save width/height into the stack for later use
   ld    b, l   ;; [1] b = width (to count how many bytes are to be reversed by inner loop)
   ld    h, #0  ;; [2] hl = l, setting hl = width to use it for math purposes 
   add  hl, de  ;; [3] hl = de + hl (start of row pointer + width, so hl points to start of next sprite row)
   push hl      ;; [4] Save pointer to the start of next sprite row
   dec  hl      ;; [2] Make hl point to the last byte of the present sprite row

   jr  first    ;; [3] Jump to start processing present sprite row

;; This loop is repeated for every horizontal byte in the same sprite row.
;; It reverses each byte's pair of colours, and it also reverses byte order
;; inside the same row. So, virtual layout scheme is as follows:
;; --------------------------------------------------------------------------
;; byte order    |      x        y               y         x
;; pixel values  | [01234567][89ABCDEF] --> [FEDCBA98][76543210]
;; --------------------------------------------------------------------------
;; byte layout   |                 
;; (Mode 2,8 px) | [ 01234567 ] --> [ 76543210 ] << Pixel bits (pixels 0 to 7)
;;
nextbyte:
   ld (de), a     ;; [2] Save last reversed byte into destination
   inc  hl        ;; [2] Start-of-the-row pointer advances to next byte to be reversed 
   dec  de        ;; [2] End-of-the-row pointer moves 1 byte backwards to the next byte to be reversed

;; DE points to the start of the byte row and increasing, whereas 
;; HL points to the end of the byte row and decreasing. 
;;    This inner part of the code reverses byte order inside the row
;;    and pixel order inside each byte
;;
first:
   ;;
   ;; This part reverses (DE) byte and places it at (HL) location, 
   ;; taking a byte from the start of the row and placing it at the end
   ;;
   ex  de, hl     ;; [1] DE <-> HL to use HL to refer to the byte going to be reversed now
   ld   a, (hl)   ;; [2]  A=next byte to be reversed
   ld   c, a      ;; [1]  A=C=pixels to be reversed, as required by revert macro
   _revert_a      ;; [16] Revert the bits of A (C gets modified)

   dec  b         ;; [1] B-- (One less byte to be reversed)
   jr   z, end    ;; [2/3] If B=0, this was the last byte to be reversed, son got to end

   ;;
   ;; This part reverses (DE) byte and places it at (HL) location too,
   ;; but taking bytes from the end of the row and placing them at the start
   ;;
   ex  de, hl     ;; [1] DE <-> HL to use HL to refer to the byte going to be reversed now
   ld   c, (hl)   ;; [2] C=Next byte to be reversed
   ld (hl), a     ;; [2] Save previously reverted byte
   ld   a, c      ;; [1] A=C=pixels to be reversed, as required by revert macro
   _revert_a      ;; [16] Revert the bits of A (C gets modified)

   djnz nextbyte  ;; [3/4] B--, if B!=0, continue reversing next byte

;; Finished reversing present byte row from the sprite
;; 
end:
   ld (de), a     ;; [2] Save last reversed byte into its final destination

   pop  de        ;; [3] DE points to the start of next sprite byte row (saved previously)
   pop  hl        ;; [3] HL contains (H: height, L: width of the sprite)

   dec   h        ;; [1] H--, One less sprite row to finish
   jr   nz, nextrow ;; [2/3] If H!=0, process next sprite row, as there are more

   ret            ;; [3] All sprite rows reversed. Return.
