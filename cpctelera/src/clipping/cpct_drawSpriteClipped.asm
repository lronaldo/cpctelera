;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;  Copyright (C) 2018 Arnaud Bouche (@Arnaud6128)
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
.module game

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawSpriteClipped
;;
;;    Partially draws a sprite to video memory (or to a screen buffer)
;;
;; C Definition:
;;    void <cpct_drawSpriteClipped> (<u8> *width_to_draw*, 
;;                                   u8* *sprite*, u8* *memory*
;;                                   <u8> *width*, <u8> *height*) __z88dk_callee;
;;
;; Input Parameters (7 bytes):
;;  (1B   A) width_to_draw  - Sprite Width to draw (<= width)
;;  (2B  DE) sprite         - Source Sprite Pointer
;;  (1B   C) width          - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (1B   B) height         - Sprite Height in bytes (>0)
;;  (2B  HL) memory         - Destination video memory pointer
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteClipped_asm
;;
;; Parameter Restrictions:
;;  * *width_to_draw* must be the width of the sprite *in bytes* to draw and must be 
;; more than 1 and less than sprite width.
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the 
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*. 
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 
;; (<cpct_px2byteM1>) as for mode 2 is linear (1 bit = 1 pixel). 
;;  * *pvideomem* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;  * *width* must be the width of the sprite *in bytes* and must be 1 or more. 
;; Using 0 as *width* parameter for this function could potentially make the program hang 
;; or crash. Always remember that the *width* must be expressed in bytes and *not* in pixels. 
;; The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;
;; Known limitations:
;;    * *width*s or *height*s of 0 will be considered as 256 and will potentially 
;; make your program behave erratically or crash.
;;    * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;    * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;    * This function *cannot be run from ROM* as it uses self-modifying code.
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
;;
;; Details:
;;    This function draws a generic *width_to_draw* x *height* bytes sprite either to
;; video memory or to a back buffer.
;;    This function can be used to draw a partial sprite or subsprite from larger sprite.
;;    The sprite must be stored as an array (i.e. with all of its pixels stored 
;; as consecutive bytes in memory).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 37 bytes
;;  ASM-bindings - 31 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |      microSecs (us)      |       CPU Cycles
;; ------------------------------------------------------------------
;;  Best      |  36 + (29 + 6W)H + 9HH   | 144 + (116 + 24W)H + 36HH
;;  Worst     |       Best + 9           |      Best + 36
;; ------------------------------------------------------------------
;;  W=2,H=16  |        946 / 955         |      3784 /  3820
;;  W=4,H=32  |       2816 / 2825        |     11264 / 11300
;; ------------------------------------------------------------------
;; Asm saving |          -20             |         -80
;; ------------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = [(H-1)/8]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ld  (width_to_draw), a        ;; [4] Store Sprite width_to_draw at placeholder    
   neg                           ;; [1] Compute next pixel line Lsb (0x0800 - width_to_draw = 0x07LL)
   ld  (offset_to_next_line), a  ;; [4] Store A to Lsb offset for next line placeholder
   add  c                        ;; [1] A = Sprite Offset (Sprite Width (C) - Sprite Width_to_draw (A))
   ld  (offset_sprite), a        ;; [4] Store Sprite offset at placeholder  
   ld   a, b                     ;; [1] A = B (Sprite height)
   
;; Draw partial Sprite loop
draw_clip_loop:
   ex  de, hl                    ;; [1] HL and DE are exchanged every line to do 16-bits maths with DE        
 
;; Placeholder for the Sprite width_to_draw   
width_to_draw = .+1     
   ld   bc, #0000                ;; [3] BC = Sprite Width to draw  = 0x00XX (XX is a placeholder to be modified) 
   ldir                          ;; [6*C-1] Copy one whole line of bytes from sprite to video memory
   dec  a                        ;; [1] A-- One less iteration to complete Sprite Height
   ret  z                        ;; [2/3] If 0 we have finished the last sprite line and return to caller

;; Placeholder for the Video next line
offset_to_next_line = .+1
   ld   bc, #0x0700              ;; [3] BC = 0x07XXS (XX is a placeholder) = 0x800 bytes is the distance in memory from one pixel line to the next within every 8 pixel lines   
   ex   de, hl                   ;; [1] DE has destination, but we have to exchange it with HL to be able to do 16-bits maths
   add  hl, bc                   ;; [3] | And we add 0x800 minus the width of the sprite (BC) to destination pointer
   ex   de, hl                   ;; [1] DE has destination, but we have to exchange it with HL to be able to do 16-bits maths   

;; Placeholder for the Sprite offset
offset_sprite = .+1      
   ld   bc, #0000                ;; [3] BC = Sprite Offset = 0x00XX (XX is a placeholder to be modified) 
   add  hl, bc                   ;; [3] Add offset sprite    
   ex   de, hl                   ;; [1] DE has destination, but we have to exchange it with HL to be able to do 16-bits maths

;; Sprite 8-bits boundary crossed 	 
   ld   b, a                     ;; [1] Save A (remaing Sprite Height to draw) into B before using it
   ld   a, h                     ;; [1] We check if we have crossed video memory boundaries (which will happen every 8 lines). 
   and  #0x38                    ;; [2] leave out only bits 13,12 and 11 from new memory address (00xxx000 00000000)
   ld   a, b                     ;; [1] Recover A from B
   jp   nz, draw_clip_loop       ;; [2/3]  and checking the 4 bits that identify present memory line. 

   ld   bc, #0xC050              ;; [3] Advance destination pointer to next line
   add  hl, bc                   ;; [3] HL += BC (0xC050)
   jp   draw_clip_loop           ;; [3] Jump to continue with next pixel line
