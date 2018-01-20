;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud BOUCHE
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
;; Function: cpct_getScreenToSprite
;;
;;    Copies and convert screen area to sprite
;;
;; C Definition:
;;    void <cpct_getScreenToSprite> (void* *memory*, void* *sprite*, <u8> *width*, <u8> *height*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B DE) memory - Source Screen Address (Video memory location)
;;  (2B HL) sprite - Destination Sprite Address (Sprite data array)
;;  (1B C ) width  - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (1B B ) height - Sprite Height in bytes (>0)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_getScreenToSprite_asm
;;
;; Parameter Restrictions:
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy software or hardware backbuffers, and
;; not only video memory.
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;;  * *width* must be the width of the screen to capture *in bytes*, and 
;; must be 1 or more. Using 0 as *width* parameter for this function could potentially 
;; make the program hang or crash. Always remember that the *width* must be 
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
;;    * This function does not do any kind of boundary check or clipping. If you 
;; get data beyond your video memory or screen buffer the sprite will also contains 
;; not video data.
;;
;; Details:
;;    This function copies a video-memory location (either present video-memory or software / hardware  
;; backbuffer) to a generic WxH bytes sprite from memory to . The destination sprite must be stored as an array (i.e. with 
;; all of its pixels stored as consecutive bytes in memory). 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 50 bytes
;;  ASM-bindints - 44 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |    microSecs (us)        |    CPU Cycles
;; ----------------------------------------------------------------
;;  Best      |     16 + (27 + 12W)H     |   84 + (88 + 72W)H 
;; ----------------------------------------------------------------
;;  W=2,H=16  |          832             |      3328
;;  W=4,H=32  |         2416             |      9964
;; ----------------------------------------------------------------
;; Asm saving |          -16             |       -64
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   push ix         ;; [5] Save IX regiter before using it as temporal var
   ld__ixl_c       ;; [3] Save Sprite Width into IXL for later use

dms_sprite_height_loop:
   push de         ;; [4] Save DE for later use (jump to next screen line)

dms_sprite_width_loop:
   ld    a, (de)   ;; [2] Get Screen byte into A
   ld   (hl), a    ;; [2] Put A (Screen) byte into Sprite Memory
   inc  de         ;; [2] Next Screen Memory byte
   inc  hl         ;; [2] Next Sprite byte

   dec   c         ;; [1] C holds sprite width, we decrease it to count pixels in this line.
   jr   nz,dms_sprite_width_loop;; [2/3] While not 0, we are still painting this sprite line 
                                ;;      - When 0, we have to jump to next pixel line

   pop  de         ;; [3] Recover DE from stack. We use it to calculate start of next pixel line on screen

   dec   b         ;; [1] B holds sprite height. We decrease it to count another pixel line finished
   jr    z,dms_sprite_copy_ended;; [2/3] If 0, we have finished the last sprite line.
                                ;;      - If not 0, we have to move pointers to the next pixel line

   ld__c_ixl       ;; [3] Restore Sprite Width into C

   ld    a, d      ;; [1] Start of next pixel line normally is 0x0800 bytes away.
   add   #0x08     ;; [2]    so we add it to DE (just by adding 0x08 to D)
   ld    d, a      ;; [1]
   and   #0x38     ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jr   nz, dms_sprite_height_loop ;; [2/3]  by checking the 4 bits that identify present memory line. 
                                   ;; ....  If 0, we have crossed boundaries

dms_sprite_8bit_boundary_crossed:
   ld    a, e      ;; [1] DE = DE + 0xC050h
   add   #0x50     ;; [2] -- Relocate DE pointer to the start of the next pixel line:
   ld    e, a      ;; [1] -- DE is moved forward 3 memory banks plus 50 bytes (4000h * 3) 
   ld    a, d      ;; [1] -- which effectively is the same as moving it 1 bank backwards and then
   adc   #0xC0     ;; [2] -- 50 bytes forwards (which is what we want to move it to the next pixel line)
   ld    d, a      ;; [1] -- Calculations are made with 8 bit maths as it is faster than other alternatives here

   jr  dms_sprite_height_loop ;; [3] Jump to continue with next pixel line

dms_sprite_copy_ended:
   pop  ix         ;; [5] Restore IX before returning
   ret             ;; [3] Return to caller
