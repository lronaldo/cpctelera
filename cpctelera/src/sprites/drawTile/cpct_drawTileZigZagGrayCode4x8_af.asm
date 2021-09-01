;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU General Public License for more details.
;;
;;  You should have received a copy of the GNU General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawTileZigZagGrayCode4x8_af
;;
;;    Copies a 4x8-byte sprite to video memory (or screen buffer), assuming that
;; the sprite lines are gray-code ordered and zig-zagged (zgtiles), and the location 
;; to be copied is Pixel Line 0 of a character line.
;;
;; C Definition:
;;    void <cpct_drawTileZigZagGrayCode4x8_af> (void* *pvideomem*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (2B HL) sprite     - Pointer to the start of the sprite array (Sprite in Gray Code format)
;;  (2B DE) pvideomem  - Pointer (aligned) to the first byte in video memory where the sprite will be copied.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawTileZigZagGrayCode4x8_af_asm
;;
;; Parameter Restrictions:
;;    * *sprite* must be a pointer to the first byte of an array containing sprite's
;; pixels data in screen pixel format. Bytes in the sprite must be ordered as follows: 
;; lines in gray-code order, and bytes in each line zig-zagged. Zig-zagged means that 
;; even-line bytes are ordered left-to-right, whereas odd-line bytes are right-to-left.
;; Gray code Line order will be 0-1-3-2-6-7-5-4. Therefore, the first 4 bytes will 
;; contain the pixel values for sprite line 0 left-to-right, second 4 bytes will 
;; contain those of sprite line 1 right-to-left, and last 4 bytes will contain pixel
;; values for sprite line 4, right-to-left.
;; Sprite must be rectangular. Total amount of bytes in pixel array should be *32*. 
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 
;; (<cpct_px2byteM1>) as for mode 2 is linear (1 bit = 1 pixel). 
;; For clarity, if we consider the 32 bytes of the sprite, their order in the 
;; given array must be: 
;;      > BYTE: [0,1,2,3,7,6,5,4,12,13,14,15,11,10,9,8,24,25,26,27,31,30,29,28,20,21,22,23,19,18,17,16]
;;      >        ------- ------- ----------- --------- ----------- ----------- ----------- -----------
;;      > LINE:     0       1         3          2          6           7           5            4 
;;    * *memory* must be a pointer to the first byte in video memory (or screen
;; buffer) where the sprite will be drawn. This location *must be aligned*, 
;; meaning that it must be a Pixel Line 0 of a screen character line. To Know
;; more about pixel lines and character lines on screen, take a look at
;; <cpct_drawSprite>. If *memory* points to a not aligned byte (one pertaining
;; to a Non-0 Pixel Line of a character line), this function will overwrite 
;; random parts of the memory, with unexpected results (typically, bad drawing 
;; results, erratic program behaviour, hangs and crashes).
;;
;; Known limitations:
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;     * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the sprite cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It will produce a "step" 
;; in the middle of tiles when drawing near wrap-around.
;;    * This function temporarily disables interrupts while drawing, because
;; it uses SP register and Push/Pop instructions. It may cause delays or
;; loses of interrupts.
;;
;; Details:
;;    This function does the same as <cpct_drawTileAligned4x8_f>, but using
;; a Gray-Code Line Order for the screen lines (0-1-3-2-6-7-5-4), and 
;; zig-zag byte order inside lines. (left-to-right, then right-to-left). 
;; The function gets bytes from memory in pairs, using POP instruction. 
;; For that, it makes use of the stack as a way to fast read the bytes
;; to be copied. So, it must disable interrupts before starting, and 
;; re-enables them when finished.
;;
;;    Copies a 4x8-byte sprite from an array with 32 screen pixel format 
;; bytes, and Gray Code Line Order (0-1-3-2-6-7-5-4) to video memory or a 
;; screen buffer. This function is tagged *aligned*, meaning that the 
;; destination byte must be *character aligned*. Being character aligned 
;; means that the 8 lines of the sprite will coincide with the 8 lines of 
;; a character line in video memory (or in the screen buffer). For more 
;; details about video memory character and pixel lines check table 1 at 
;; <cpct_drawSprite>.
;;
;;    As the 8 lines of the sprite must go to a character line on video 
;; memory (or screen buffer), *memory* destination pointer must point to
;; a the first line (Pixel Line 0) of a character line. If hardware 
;; scrolling has not been used, all pixel lines 0 are contained inside
;; one of these 4 ranges:
;;
;;    [ 0xC000 -- 0xC7FF ] - RAM Bank 3 (Default Video Memory Bank)
;;    [ 0x8000 -- 0x87FF ] - RAM Bank 2
;;    [ 0x4000 -- 0x47FF ] - RAM Bank 1
;;    [ 0x0000 -- 0x07FF ] - RAM Bank 0
;;
;;    All of them have 3 bits in common: bits 5, 4 and 3 are always 0 
;; (--000---). Any address not having all these 3 bits set to 0 does not
;; refer to a Pixel Line 0 and is not considered to be aligned.
;;
;;    This function will just copy bytes, not taking care of colours or 
;; transparencies. If you wanted to copy a sprite without erasing the background
;; just check for masked sprites and <cpct_drawMaskedSprite>.
;;
;; Destroyed Register values: 
;;    HL, DE, F
;;
;; Interrupts:
;;    * Disables interrupts when called.
;;    * Enables interrupts before returning.
;;    * Uses SP register and POP to read from memory.
;;
;; Required memory:
;;    C-bindings - 101 bytes
;;  ASM-bindings -  98 bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | microSecs (us) | CPU Cycles
;; ------------------------------------------
;;    Any     |      191       |    728
;; ------------------------------------------
;; Asm saving |      -12       |    -48
;; ------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   di                   ;; [1] Disable interrupts before starting (we are using SP to read values)
   ld (save_sp + 1), sp ;; [6] Save actual SP to restore it in the end
   ld sp, hl            ;; [2] SP = end of sprite
   ex de, hl            ;; [1] HL = Destination in Video Memory (place where the sprite will be drawn)

   ;; Sprite Line 0. Drawing order [>>]
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   inc hl               ;; [2] HL++ -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   set 3, h             ;; [2] --000---=>--001--- (Next sprite line: 1)

   ;; Sprite Line 1. Drawing order [<<]
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte
   dec hl               ;; [2] HL-- -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte
   set 4, h             ;; [2] --001---=>--011--- (Next sprite line: 3)

   ;; Sprite Line 3. Drawing order [>>]
   pop de               ;; [3] Get 2 sprite bytes 
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   inc hl               ;; [2] HL++ -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes 
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   res 3, h             ;; [2] --011---=>--010--- (Next sprite line: 2)

   ;; Sprite Line 2. Drawing order [<<]
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte
   dec hl               ;; [2] HL-- -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte
   set 5, h             ;; [2] --010---=>--110--- (Next sprite line: 6)

   ;; Sprite Line 6. Drawing order [>>]
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   inc hl               ;; [2] HL++ -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   set 3, h             ;; [2] --110---=>--111--- (Next sprite line: 7)

   ;; Sprite Line 7. Drawing order [<<]
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte
   dec hl               ;; [2] HL-- -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte
   res 4, h             ;; [2] --111---=>--101--- (Next sprite line: 5)

   ;; Sprite Line 5. Drawing order [>>]
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   inc hl               ;; [2] HL++ -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the left byte
   inc hl               ;; [2] HL++ -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the right byte
   res 3, h             ;; [2] --101---=>--100--- (Next sprite line: 4)

   ;; Sprite Line 4. Drawing order [<<]
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte
   dec hl               ;; [2] HL-- -> next video mem location
   pop de               ;; [3] Get 2 sprite bytes
   ld (hl), e           ;; [2] 1st byte is the right byte
   dec hl               ;; [2] HL-- -> next video mem location
   ld (hl), d           ;; [2] 2nd byte is the left byte

save_sp:
   ld sp, #0000         ;; [3] Restore SP (#0000 is a placeholder)
   ei                   ;; [1] Reenable interrupts before returning
   ret                  ;; [3] Return
