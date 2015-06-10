;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_tilemaps

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawTileRowAligned4x8
;;
;;    Copies a row of tiles to a given location in video memory (or screen 
;; backbuffer), assuming that location to be copied is Pixel Line 0 of a 
;; character line. 
;;
;; C Definition:
;;    void <cpct_drawTileRowAligned4x8> (void* pvideomem, void* tileset, void* tilearray, <u8> arraysize);
;;
;; Input Parameters (7 bytes):
;;  (2B DE) pvideomem  - Pointer (aligned) to the first byte in video memory where the sprite will be copied.
;;  (2B HL) ptileset   - Pointer to the start to the array containing the set of tiles (their screen-pixel definitions)
;;  (2B BC) ptilearray - Source Sprite Pointer (32-byte array with 8-bit pixel data)
;;  (1B  A) size       - Size of the tilearray (number of tiles that will be printed in the row)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawTileRowAligned4x8_asm
;;
;; Parameter Restrictions:
;;    * *pvideomem* must be a pointer to the first byte in video memory (or screen
;; buffer) where the sprite will be drawn. This location *must be aligned*, 
;; meaning that it must be a Pixel Line 0 of a screen character line. To Know
;; more about pixel lines and character lines on screen, take a look at
;; <cpct_drawSprite>. If *memory* points to a not aligned byte (one pertaining
;; to a Non-0 Pixel Line of a character line), this function will overwrite 
;; random parts of the memory, with unexpected results (typically, bad drawing 
;; results, erratic program behaviour, hangs and crashes).
;;    * *ptileset*  ****************
;;    * *ptilearray*  ****************
;;    * *size*  ****************
;;
;; Known limitations:
;;     * This function does not do any kind of boundary check or clipping. If you 
;; try to draw tiles on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;     * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned tiles. This means that the sprite cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;
;; Details:
;;    Copies a complete row of tiles from a *tilearray* and a *tileset* to video 
;; memory (or screen backbuffer). *tilearray* contains a list with the indices of
;; the tiles from the *tileset* that will be drawn to screen. Each tile is a 
;; 4x8-byte tile (an array with 32 screen pixel format bytes). This function is 
;; tagged *aligned*, meaning that the destination byte must be *character aligned*. 
;; Being character aligned means that the 8 lines of each tile have to 
;; coincide with the 8 lines of a character line in video memory (or 
;; in the screen buffer). For more details about video memory character
;; and pixel lines check table 1 at <cpct_drawSprite>.
;;
;;    As the 8 lines of each tile must go to a character line on video 
;; memory (or screen buffer), *pvideomem* destination pointer must point to
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
;; (xx000xxx). Any address not having all these 3 bits set to 0 does not
;; refer to a Pixel Line 0 and is not considered to be aligned.
;;
;;    This function will just copy bytes, not taking care of colours or 
;; transparencies. 
;;
;; Destroyed Register values: 
;;    AF, AF', BC, DE, HL
;;
;; Required memory:
;;     bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | Cycles | microSecs (us)
;; ---------------------------------
;;    Any     |        |  
;; ---------------------------------
;; Asm saving |        |  
;; ---------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  (2B DE) pvideomem  - Pointer (aligned) to the first byte in video memory where the sprite will be copied.
;;  (2B HL) ptileset   - Pointer to the start to the array containing the set of tiles (their screen-pixel definitions)
;;  (2B BC) ptilearray - Source Sprite Pointer (32-byte array with 8-bit pixel data)

_cpct_drawTileRowAligned4x8::
   ;; GET Parameters from the stack (145 cycles)
   ld   hl, #8             ;; 10 (6+2) 
   add  hl, sp             ;; 11
   pop  af                 ;; [10] IX = Return Address
   ex   af, af'            ;; 4
   ld    a, (hl)           ;; 7
   pop  de                 ;; [10] DE = Destination Adress (video memory)
   pop  hl                 ;; [10] HL = Pointer to the tileset (array with tile definitions)
   pop  bc                 ;; [10] HL = Pointer to the tileset (array with tile definitions)
   push bc                 ;; [11] Leave the stack as it was
   push hl                 ;; [11] 
   push de                 ;; [11] 
   ex   af, af'            ;; 4
   push af                 ;; [11] 
   ex   af, af'            ;; 4

cpct_drawTileRowAligned4x8_asm::  ;; Assembly entry point

   ;; Copy 8 lines of 4 bytes width
   ld    a, #8             ;; [ 7] We have to draw 8 lines of sprite
   jp dsa48_first_line     ;; [10] First line does not need to do maths to start transferring data. 

dsa48_next_line:
   ;; Move to the start of the next line
   ex   de, hl             ;; [ 4] Make DE point to the start of the next line of pixels in video memory, by adding BC
   add  hl, bc             ;; [11] (We need to interchange HL and DE because DE does not have ADD)
   ex   de, hl             ;; [ 4]

dsa48_first_line:
   ;; Draw a sprite-line of 4 bytes
   ld   bc, #0x800         ;; [10] 800h bytes is the distance to the start of the first pixel in the next line in 
                           ;; .... video memory (it will be decremented by 1 by each LDI)
   ldi                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   ldi                     ;; [16]  | (distance is 1 byte less as we progress up)
   ldi                     ;; [16]  |
   ldi                     ;; [16] <|

   ;; Repeat for all the lines
   dec   a                 ;; [ 4] A = A - 1 (1 more line completed)
   jp   nz, dsa48_next_line;; [10] Continue to next line if A != 0 (A = Lines left)

   ret                     ;; [10]
