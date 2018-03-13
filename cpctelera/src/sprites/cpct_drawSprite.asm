;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawSprite
;;
;;    Copies a sprite from an array to video memory (or to a screen buffer).
;;
;; C Definition:
;;    void <cpct_drawSprite> (void* *sprite*, void* *memory*, <u8> *width*, <u8> *height*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B HL) sprite - Source Sprite Pointer (array with pixel data)
;;  (2B DE) memory - Destination video memory pointer
;;  (1B C ) width  - Sprite Width in *bytes* [1-63] (Beware, *not* in pixels!)
;;  (1B B ) height - Sprite Height in bytes (>0)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSprite_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*.
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 
;; (<cpct_px2byteM1>) as for mode 2 is linear (1 bit = 1 pixel).
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;  * *width* must be the width of the sprite *in bytes*, and must be in the range [1-63].
;; A sprite width outside the range [1-63] will probably make the program hang or crash, 
;; due to the optimization technique used. Always remember that the width must be 
;; expressed in bytes and *not* in pixels. The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;
;; Known limitations:
;;     * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;     * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
;;
;; Details:
;;    This function copies a generic WxH bytes sprite from memory to a 
;; video-memory location (either present video-memory or software / hardware  
;; backbuffer). The original sprite must be stored as an array (i.e. with 
;; all of its pixels stored as consecutive bytes in memory). It only works 
;; for solid, rectangular sprites, with 1-63 bytes width
;;
;;    This function will just copy bytes, not taking care of colours or 
;; transparencies. If you wanted to copy a sprite without erasing the background
;; just check for masked sprites and <cpct_drawMaskedSprite>.
;;
;;    Copying a sprite to video memory is a complex operation due to the 
;; particular distribution of screen pixels in CPC's video memory. At power on,
;; video memory starts at address 0xC000 (it can be changed by BASIC's scroll,
;; or using functions <cpct_setVideoMemoryPage> and <cpct_setVideoMemoryOffset>).
;; This means that the byte at 0xC000 contains first pixels colour values, the ones
;; at the top-left corner of the screen (2 first pixels in mode 0, 4 in mode 1 and 
;; 8 in mode 2). Byte at 0xC001 contains next pixel values to the right, etc. 
;; However, this configuration is not always linear. First 80 bytes encode the 
;; first screen pixel line (line 0), next 80 bytes encode pixel line 8, next 
;; 80 encode pixel line 16, and so on. Pixel line 1 start right next to pixel
;; line 200 (the last one on screen), then goes pixel line 9, and so on. 
;; 
;; This particular distribution was thought to be used in 'characters' when it 
;; was conceived. As a character has 8x8 pixels, pixel lines have a distribution
;; in jumps of 8. This means that the screen has 25 character lines, each one
;; with 8 pixel lines. This distribution is shown at table 1, depicting memory 
;; locations where every pixel line starts, related to their character lines. 
;; (start code)
;; | Character   |  Pixel |  Pixel |  Pixel |  Pixel |  Pixel |  Pixel |  Pixel |  Pixel |
;; |   Line      | Line 0 | Line 1 | Line 2 | Line 3 | Line 4 | Line 5 | Line 6 | Line 7 |
;; ---------------------------------------------------------------------------------------
;; |      1      | 0xC000 | 0xC800 | 0xD000 | 0xD800 | 0xE000 | 0xE800 | 0xF000 | 0xF800 |
;; |      2      | 0xC050 | 0xC850 | 0xD050 | 0xD850 | 0xE050 | 0xE850 | 0xF050 | 0xF850 |
;; |      3      | 0xC0A0 | 0xC8A0 | 0xD0A0 | 0xD8A0 | 0xE0A0 | 0xE8A0 | 0xF0A0 | 0xF8A0 |
;; |      4      | 0xC0F0 | 0xC8F0 | 0xD0F0 | 0xD8F0 | 0xE0F0 | 0xE8F0 | 0xF0F0 | 0xF8F0 |
;; |      5      | 0xC140 | 0xC940 | 0xD140 | 0xD940 | 0xE140 | 0xE940 | 0xF140 | 0xF940 |
;; |      6      | 0xC190 | 0xC990 | 0xD190 | 0xD990 | 0xE190 | 0xE990 | 0xF190 | 0xF990 |
;; |      7      | 0xC1E0 | 0xC9E0 | 0xD1E0 | 0xD9E0 | 0xE1E0 | 0xE9E0 | 0xF1E0 | 0xF9E0 |
;; |      8      | 0xC230 | 0xCA30 | 0xD230 | 0xDA30 | 0xE230 | 0xEA30 | 0xF230 | 0xFA30 |
;; |      9      | 0xC280 | 0xCA80 | 0xD280 | 0xDA80 | 0xE280 | 0xEA80 | 0xF280 | 0xFA80 |
;; |     10      | 0xC2D0 | 0xCAD0 | 0xD2D0 | 0xDAD0 | 0xE2D0 | 0xEAD0 | 0xF2D0 | 0xFAD0 |
;; |     11      | 0xC320 | 0xCB20 | 0xD320 | 0xDB20 | 0xE320 | 0xEB20 | 0xF320 | 0xFB20 |
;; |     12      | 0xC370 | 0xCB70 | 0xD370 | 0xDB70 | 0xE370 | 0xEB70 | 0xF370 | 0xFB70 |
;; |     13      | 0xC3C0 | 0xCBC0 | 0xD3C0 | 0xDBC0 | 0xE3C0 | 0xEBC0 | 0xF3C0 | 0xFBC0 |
;; |     14      | 0xC410 | 0xCC10 | 0xD410 | 0xDC10 | 0xE410 | 0xEC10 | 0xF410 | 0xFC10 |
;; |     15      | 0xC460 | 0xCC60 | 0xD460 | 0xDC60 | 0xE460 | 0xEC60 | 0xF460 | 0xFC60 |
;; |     16      | 0xC4B0 | 0xCCB0 | 0xD4B0 | 0xDCB0 | 0xE4B0 | 0xECB0 | 0xF4B0 | 0xFCB0 |
;; |     17      | 0xC500 | 0xCD00 | 0xD500 | 0xDD00 | 0xE500 | 0xED00 | 0xF500 | 0xFD00 |
;; |     18      | 0xC550 | 0xCD50 | 0xD550 | 0xDD50 | 0xE550 | 0xED50 | 0xF550 | 0xFD50 |
;; |     19      | 0xC5A0 | 0xCDA0 | 0xD5A0 | 0xDDA0 | 0xE5A0 | 0xEDA0 | 0xF5A0 | 0xFDA0 |
;; |     20      | 0xC5F0 | 0xCDF0 | 0xD5F0 | 0xDDF0 | 0xE5F0 | 0xED50 | 0xF550 | 0xFD50 |
;; |     21      | 0xC640 | 0xCE40 | 0xD640 | 0xDE40 | 0xE640 | 0xEE40 | 0xF640 | 0xFE40 |
;; |     22      | 0xC690 | 0xCE90 | 0xD690 | 0xDE90 | 0xE690 | 0xEE90 | 0xF690 | 0xFE90 |
;; |     23      | 0xC6E0 | 0xCEE0 | 0xD6E0 | 0xDEE0 | 0xE6E0 | 0xEEE0 | 0xF6E0 | 0xFEE0 |
;; |     24      | 0xC730 | 0xCF30 | 0xD730 | 0xDF30 | 0xE730 | 0xEF30 | 0xF730 | 0xFF30 |
;; |     25      | 0xC780 | 0xCF80 | 0xD780 | 0xDF80 | 0xE780 | 0xEF80 | 0xF780 | 0xFF80 |
;; | spare start | 0xC7D0 | 0xCFD0 | 0xD7D0 | 0xDFD0 | 0xE7D0 | 0xEFD0 | 0xF7D0 | 0xFFD0 |
;; | spare end   | 0xC7FF | 0xCFFF | 0xD7FF | 0xDFFF | 0xE7FF | 0xEFFF | 0xF7FF | 0xFFFF |
;; ---------------------------------------------------------------------------------------
;;           Table 1 - Video memory starting locations for all pixel lines 
;; (end)
;;    *Note on how to interpret Table 1*: Table 1 contains starting video memory locations 
;; for all 200 pixel lines on the screen (with default configuration). To know where does 
;; a particular pixel line start, please read Table 1 left-to-right, top-to-bottom. So, 
;; ROW 1 at Table 1 contains the memory start locations for the first 8 pixel lines on 
;; screen (0 to 7), ROW 2 refers to pixel lines 8 to 15, ROW 3 has pixel lines 16 to 23, 
;; and so on.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;     C-bindings - 165 bytes
;;   ASM-bindings - 160 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |   microSecs (us)       |        CPU Cycles
;; ----------------------------------------------------------------
;;  Best      | 20 + (21 + 5W)H + 9HH  | 80 + (84 + 20W)H + 36HH
;;  Worst     |       Best + 9         |      Best + 36
;; ----------------------------------------------------------------
;;  W=2,H=16  |        525 /  534      |   2100 / 2136
;;  W=4,H=32  |       1359 / 1368      |   5436 / 5472
;; ----------------------------------------------------------------
;; Asm saving |         -16            |        -64
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = [(H-1)/8]
;;
;; Credits:
;;    This routine was inspired in the original *cpc_PutSprite* from
;; CPCRSLib by Raul Simarro.
;;
;;    Thanks to *Mochilote* / <CPCMania at http://cpcmania.com> for creating the original
;; <video memory locations table at 
;; http://www.cpcmania.com/Docs/Programming/Painting_pixels_introduction_to_video_memory.htm>.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Modify code using width to jump in drawSpriteWidth
   ld    a, #126           ;; [2] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite * 2 (2B)
   sub   c                 ;; [1]    to do as much LDIs as bytes the Sprite is wide
   sub   c                 ;; [1]
   ld (ds_drawSpriteWidth+#4), a ;; [4] Modify JR data to create the jump we need

   ld    a, b              ;; [1] A = Height (used as counter for the number of lines we have to copy)
   ex   de, hl             ;; [1] Instead of jumping over the next line, we do the inverse operation because 
                           ;; .... it is only 4 cycles and not 10, as a JP would be)

ds_drawSpriteWidth_next:
   ;; NEXT LINE
   ex   de, hl             ;; [1] HL and DE are exchanged every line to do 16bit maths with DE. 
                           ;; .... This line reverses it before proceeding to copy the next line.
ds_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes
   ld   bc, #0x800  ;; [3] 0x800 bytes is the distance in memory from one pixel line to the next within every 8 pixel lines
                    ;; ... Each LDI performed will decrease this by 1, as we progress through memory copying the present line
   jr__0            ;; [3] Self modifying instruction: the '00' will be substituted by the required jump forward. 
                    ;; ... (Note: Writting JR 0 compiles but later it gives odd linking errors)
   ldi              ;; [5] <| 63 LDIs, which are able to copy up to 63 bytes each time.
   ldi              ;; [5]  | That means that each Sprite line should be 63 bytes width at most.
   ldi              ;; [5]  | The JR instruction at the start makes us ignore the LDIs we don't need 
   ldi              ;; [5] <| (jumping over them) That ensures we will be doing only as much LDIs 
   ldi              ;; [5] <| as bytes our sprite is wide.
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
   ldi              ;; [5] <|
   ldi              ;; [5] <|
   ldi              ;; [5]  |
   ldi              ;; [5]  |
 
   dec   a          ;; [1] Another line finished: we discount it from A
   ret   z          ;; [2/4] If that was the last line, we safely return

   ;; Jump destination pointer to the start of the next line in video memory
   ex   de, hl      ;; [1] DE has destination, but we have to exchange it with HL to be able to do 16bit maths
   add  hl, bc      ;; [3] We add 0x800 minus the width of the sprite (BC) to destination pointer 
   ld    b, a       ;; [1] Save A into B (B = A)
   ld    a, h       ;; [1] We check if we have crossed video memory boundaries (which will happen every 8 lines). 
                    ;; .... If that happens, bits 13,12 and 11 of destination pointer will be 0
   and   #0x38      ;; [2] leave out only bits 13,12 and 11 from new memory address (00xxx000 00000000)
   ld    a, b       ;; [1] Restore A from B (A = B)
   jp   nz, ds_drawSpriteWidth_next ;; [3] If any bit from {13,12,11} is not 0, we are still inside 
                                    ;; .... video memory boundaries, so proceed with next line

   ;; Every 8 lines, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means our next line is 16K-0x50 bytes back
   ;; which is the same as advancing 48K+0x50 = 0xC050 bytes, as memory is 64K 
   ;; and our 16bit pointers cycle over it
   ld   bc, #0xC050           ;; [3] We advance destination pointer to next line
   add  hl, bc                ;; [3]  HL += 0xC050
   jp ds_drawSpriteWidth_next ;; [3] Continue copying
