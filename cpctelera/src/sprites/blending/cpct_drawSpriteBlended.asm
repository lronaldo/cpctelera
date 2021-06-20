;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawSpriteBlended
;;
;;    Draws sprites blending them with current contents of screen video memory.
;; It uses XOR by default, but many other blending modes are available.
;;
;; C Definition:
;;    void <cpct_drawSpriteBlended> (void* *memory*, <u8> *height*, <u8> *width*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B DE) memory - Destination video memory pointer
;;  (1B C ) height - Sprite Height in bytes (>0)
;;  (1B B ) width  - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (2B HL) sprite - Source Sprite Pointer (array with pixel data)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteBlended_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be *width* x *height*
;; You may check screen pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 (<cpct_px2byteM1>)
;; as for mode 2 is linear (1 bit = 1 pixel).
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; There is no practical upper limit to this value. Height of a sprite in
;; bytes and pixels is the same value, as bytes only group consecutive pixels in
;; the horizontal space.
;;  * *width* must be the width of the sprite *in bytes* and must be 1 or more. 
;; Using 0 as *width* parameter for this function could potentially make the program 
;; hang or crash. Always remember that the *width* must be expressed in bytes 
;; and *not* in pixels. The correspondence is:
;;    mode 0      - 1 byte = 2 pixels
;;    modes 1 / 3 - 1 byte = 4 pixels
;;    mode 2      - 1 byte = 8 pixels
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; layout). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;
;; Known limitations:
;;  * This function could be used from ROM, but it will only perform a XOR
;; blending operation. Blending operation will not be possible to change 
;; on ROM, as it requires to change a byte of the code. That can only be
;; performed when this function is on RAM.
;;  * This function does not do any kind of boundary check or clipping. If you 
;; try to draw sprites on the frontier of your video memory or screen buffer 
;; if might potentially overwrite memory locations beyond boundaries. This 
;; could cause your program to behave erratically, hang or crash. Always 
;; take the necessary steps to guarantee that you are drawing inside screen
;; or buffer boundaries.
;;  * As this function receives a byte-pointer to memory, it can only 
;; draw byte-sized and byte-aligned sprites. This means that the box cannot
;; start on non-byte aligned pixels (like odd-pixels, for instance) and 
;; their sizes must be a multiple of a byte (2 in mode 0, 4 in mode 1 and
;; 8 in mode 2).
;;    * Although this function can be used under hardware-scrolling conditions,
;; it does not take into account video memory wrap-around (0x?7FF or 0x?FFF 
;; addresses, the end of character pixel lines).It  will produce a "step" 
;; in the middle of sprites when drawing near wrap-around.
;;
;; Blending modes:
;;    This function performs, *by default, a XOR* blending operation. However,
;; This function can perform many blending operations. Consult all available
;; blending modes at <CPCT_BlendMode> documentation.
;;
;;    Standard modes included: XOR (default), OR, AND, ADD, ADC, SUB, SBC, LDI and NOT.
;;
;; Details:
;;    This function blends a generic WxH bytes sprite from memory to a 
;; video-memory location (either present video-memory or software / hardware  
;; backbuffer). It performs similar to <cpct_drawSprite> but instead
;; of copying bytes and overwrite video-memory, it blends them with
;; present contents of video-memory destination (background).
;;
;;    The process is as follows. The function gets 1 byte from the destination
;; video-memory and 1 byte from the sprite. It then performs a blending 
;; operation between both bytes and stores the result in the same place where
;; it got the byte from video memory. The process is repeated for every byte
;; from the sprite.
;;
;;    The blending operation that this function performs is selectable. By 
;; default, this operation will be a XOR between both bytes. To select a
;; different operation mode, the function <cpct_setBlendMode>
;; should be called. 
;;
;; Use examples:
;;    This function may be used directly to draw a sprite in video-memory:
;; (start code)
;;    // This function will draw the sprite of the main Character
;;    void drawCharacterSprite(u8 x, u8 y) {
;;       // First, calculate video-memory location of the (x,y)
;;       // coordinates where the character is located
;;       u8 *pmem = cpct_getScreenPtr(CPCT_VMEM_START, x, y);
;;
;;       // Then, draw the sprite, blending it with the background
;;       cpct_drawSpriteBlended(pmem, 24, 8, sprite);
;;    }
;; (end code)
;;    Previous function *drawCharacterSprite* could also be used to erase
;; the sprite, leaving video-memory as it was before drawing the sprite, 
;; if the selected blending mode is XOR. Calling the function again with
;; same coordinates will do the trick, as M XOR S XOR S = M (Doing a double
;; XOR operation to any (M)emory byte against any (S)prite byte will leave
;; M untouched).
;;
;;    This function could even be used as a normal drawSprite function without
;; *width* limits (as <cpct_drawSprite> cannot draw sprites wider than 63 bytes).
;; This can be performed using the LDI mode:
;; (start code)
;;    // Draw the user interface of the application, that is composed
;;    // of several sprites 
;;    void drawUserInterface() {
;;       // First, select LDI as blending mode for cpct_drawSpriteBlended
;;       cpct_setBlendMode(CPCT_BLEND_LDI);
;;
;;       // Now print all sprites of the UI at their pre-calculated locations
;;       // Note: assume *LOCATION macros are just absolute video-memory addresses
;;       cpct_drawSpriteBlended(                 VMEM, 80, 200, background);
;;       cpct_drawSpriteBlended(  SCOREBOARD_LOCATION, 80,  20, score_board);
;;       cpct_drawSpriteBlended( LEFT_COLUMN_LOCATION, 10, 100, left_column);
;;       cpct_drawSpriteBlended(RIGHT_COLUMN_LOCATION, 10, 100, right_column);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 43 bytes
;;  ASM-bindings - 39 bytes
;;
;; Time Measures:
;; (start code)
;;  Case         |      microSecs (us)       |         CPU Cycles          |
;; -------------------------------------------------------------------------
;;  Best         | 32 + 21H + (12+B)WH + 10J | 128 + 84H + (48+4B)WH + 40J |
;;  Worst        |         Best + 10         |         Best + 40           |
;; -------------------------------------------------------------------------
;;  W=2,H=16,B=2 |        826 /  836         |        3304 /  3344         |
;;  W=4,H=32,B=2 |       2526 / 2536         |       10104 / 10144         |
;; -------------------------------------------------------------------------
;;  Asm saving   |          -15              |            -60              |
;; -------------------------------------------------------------------------
;; (end code)
;;   *W* = *width* in bytes, *H* = *height* in bytes
;;
;;   *B* = Blend operation nanoseconds, *WH* = W * H, *J* = [(H-1)/8]
;;
;;   Standard Blend operations take 2 nanoseconds except NOP and LDI, which take 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   push ix         ;; [5] Save IX regiter before using it as temporal var
   ld__ixl_b       ;; [3] Save Sprite Width into IXL for later use

   ;; Copy and blend all the rows of the sprite
height_loop:
   push de         ;; [4] Save DE for later use (jump to next screen line)

   ;; Copy and blend 1 complete sprite row
width_loop:
   ld     a,(de)   ;; [2] Get next background byte into A

  ;; Blending Function. This is a 1-byte Z80 operation that should blend
  ;; the byte in A (background) with the byte from the sprite at (HL)
  ;; This function is modified by <cpct_setBlendMode>.
_cpct_dsb_blendFunction::
   xor (hl)        ;; [2] Blend background with Sprite || This byte will be modified to change blend function

   ld  (de), a     ;; [2] Save background blended with sprite
   inc   de        ;; [2] | Point to next byte 
   inc   hl        ;; [2] |  (sprite and memory)

   djnz  width_loop;; [3/4] B holds sprite width. Decrease it and loop until B=0

   pop  de         ;; [3] Recover DE from stack. We use it to calculate start of next pixel line on screen

   dec   c         ;; [1] C holds sprite height. We decrease it to count another pixel line finished
   jr    z,copy_ended;; [2/3] If 0, we have finished the last sprite line.
                                ;;      - If not 0, we have to move pointers to the next pixel line

   ld__b_ixl       ;; [3] Restore Sprite Width into B

   ld    a, d      ;; [1] Start of next pixel line normally is 0x0800 bytes away.
   add   #0x08     ;; [2]    so we add it to DE (just by adding 0x08 to D)
   ld    d, a      ;; [1]
   and   #0x38     ;; [2] We check if we have crossed memory boundary (every 8 pixel lines)
   jr   nz,height_loop  ;; [2/3]  by checking the 4 bits that identify present memory line. (......11 1.......)
                        ;; ....  If 0, we have crossed boundaries

_8bit_boundary_crossed:
   ld    a, e      ;; [1] DE = DE + 0xC050h
   add   #0x50     ;; [2] -- Relocate DE pointer to the start of the next pixel line:
   ld    e, a      ;; [1] -- DE is moved forward 3 memory banks plus 50 bytes (4000h * 3) 
   ld    a, d      ;; [1] -- which effectively is the same as moving it 1 bank backwards and then
   adc   #0xC0     ;; [2] -- 50 bytes forwards (which is what we want to move it to the next pixel line)
   ld    d, a      ;; [1] -- Calculations are made with 8 bit maths as it is faster than other alternatives here

   jr  height_loop ;; [3] Jump to continue with next pixel line

copy_ended:
   pop  ix         ;; [5] Restore IX before returning
   ret             ;; [3] Return to caller
