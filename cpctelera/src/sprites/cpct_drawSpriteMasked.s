;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

.include /sprites.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawSpriteMasked
;;
;;    Copies a masked sprite from an array to video memory (or to a screen buffer),
;; using mask as transparency information, to prevent erasing the background.
;;
;; C Definition:
;;    void <cpct_drawSpriteMasked> (void* *sprite*, void* *memory*, <u8> *width*, <u8> *height*);
;;
;; Input Parameters (6 bytes):
;;  (2B HL) sprite - Source Sprite Pointer (array with pixel and mask data)
;;  (2B DE) memory - Destination video memory pointer
;;  (1B B ) width  - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (1B C ) height - Sprite Height in bytes (>0)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawSpriteMasked_asm
;;
;; Parameter Restrictions:
;;  * *sprite* must be an array containing sprite's pixels data in screen pixel format
;; along with mask data. Each mask byte will contain enabled bits as those that should
;; be picked from the background (transparent) and disabled bits for those that will
;; be printed from sprite colour data. Each mask data byte must precede its associated
;; colour data byte.
;; Sprite must be rectangular and all bytes in the array must be consecutive pixels, 
;; starting from top-left corner and going left-to-right, top-to-bottom down to the
;; bottom-right corner. Total amount of bytes in pixel array should be 
;; 2 x *width* x *height* (mask data doubles array size). You may check screen 
;; pixel format for mode 0 (<cpct_px2byteM0>) and mode 1 (<cpct_px2byteM1>) as 
;; for mode 2 is linear (1 bit = 1 pixel).
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy sprites to software or hardware backbuffers, and
;; not only video memory.
;;  * *width* must be the width of the sprite *in bytes*, *excluding mask data*, and 
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
;;
;; Details:
;;    This function copies a generic WxH bytes sprite from memory to a 
;; video-memory location (either present video-memory or software / hardware  
;; backbuffer). The original sprite must be stored as an array (i.e. with 
;; all of its pixels stored as consecutive bytes in memory). It works in
;; a similar way to <cpct_drawSprite>, but taking care about transparency
;; information encoded in mask bytes. For detailed information about 
;; how sprite copying works, and how video memory is formatted, take a 
;; look at <cpct_drawSprite>.
;;
;;    For this routine to work, the sprite must contain mask information: inside 
;; the sprite array, for each byte containing screen colour information, there 
;; *must* be a preceding byte with mask information. So, the format is
;; as depicted in example 1:
;; (start code)
;; Array storage format:  <-byte 1-> <-byte 2-> <-byte 3-> <-byte 4->
;;                        <- mask -> <-colour-> <- mask -> <-colour->
;; ------------------------------------------------------------------------------
;;        u8* sprite =  {    0xFF,     0x00,       0x00,     0xC2,   .... };
;; ------------------------------------------------------------------------------
;; Video memory output:   <- 1st Screen byte -> <- 2nd Screen byte -> 
;; ______________________________________________________________________________
;;         Example 1: Definition of a masked sprite and byte format
;; (end)
;;     In example 1, each "Screen byte" will become 1 byte outputted to video memory
;; resulting of the combination of 3 bytes: 1-byte mask, 1-byte sprite colour data 
;; and 1-byte previous screen colour data. The combination of these 3 bytes results
;; in sprite colour data being "blended" with previous screen colour data, adding
;; sprite pixels with background pixels (the ones over transparent pixels).
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    54 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |           Cycles                 |      microSecs (us)
;; ------------------------------------------------------------------------------------
;;  Best      |  64 + 59*W*H + 54*H + 40*[H / 8] | 16 + 14.75*W*H + 13.50*H + 10*[H/8]
;;  Worst     | 104 + 59*W*H + 54*H + 40*[H / 8] | 26 + 14.75*W*H + 13.50*H + 10*[H/8]
;; ------------------------------------------------------------------------------------
;;  W=2,H=16  |        2894 / 2943               |     723.50 /  733.50
;;  W=4,H=32  |        9422 / 9462               |    2355.50 / 2365.50
;; ------------------------------------------------------------------------------------
;; Asm saving |         -84                      |     -21.00 
;; ------------------------------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


_cpct_drawSpriteMasked::
   ;; GET Parameters from the stack 
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 4 bytes more, and requires disabling interrupts
   ld (dms_restoreSP+1), sp   ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   di                         ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                    ;; [10] AF = Return Address
   pop  hl                    ;; [10] HL = Source Address (Sprite data array)
   pop  de                    ;; [10] DE = Destination address (Video memory location)
   pop  bc                    ;; [10] BC = Height/Width (B = Height, C = Width)
dms_restoreSP:
   ld   sp, #0                ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   ei                         ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 6 cycles more, but does not require disabling interrupts
   pop  af                    ;; [10] AF = Return Address
   pop  hl                    ;; [10] HL = Source Address (Sprite data array)
   pop  de                    ;; [10] DE = Destination address (Video memory location)
   pop  bc                    ;; [10] BC = Height/Width (B = Height, C = Width)
   push bc                    ;; [11] Restore Stack status pushing values again
   push de                    ;; [11] (Interrupt safe way, 6 cycles more)
   push hl                    ;; [11]
   push af                    ;; [11]
.endif

cpct_drawSpriteMasked_asm::   ;; Assembly entry point

   push ix                    ;; [15] Save IX regiter before using it as temporal var
   .DW  #0x69DD ; ld ixl, c   ;; [ 8] Save Sprite Width into IXL for later use

dms_sprite_height_loop:
   push de                    ;; [11] Save DE for later use (jump to next screen line)

dms_sprite_width_loop:
   ld    a, (de)              ;; [ 7] Get next background byte into A
   and (hl)                   ;; [ 7] Erase background part that is to be overwritten (Mask step 1)
   inc  hl                    ;; [ 6] HL += 1 => Point HL to Sprite Colour information
   or (hl)                    ;; [ 7] Add up background and sprite information in one byte (Mask step 2)
   ld (de), a                 ;; [ 6] Save modified background + sprite data information into memory
   inc  de                    ;; [ 6] Next bytes (sprite and memory)
   inc  hl                    ;; [ 6] 

   dec   c                    ;; [ 4] C holds sprite width, we decrease it to count pixels in this line.
   jp   nz,dms_sprite_width_loop;; [10] While not 0, we are still painting this sprite line 
                              ;;      - When 0, we have to jump to next pixel line

   pop  de                    ;; [10] Recover DE from stack. We use it to calculate start of next pixel line on screen

   dec   b                    ;; [ 4] B holds sprite height. We decrease it to count another pixel line finished
   jp    z, dms_sprite_copy_ended;; [10] If 0, we have finished the last sprite line.
                              ;;      - If not 0, we have to move pointers to the next pixel line

   .DW #0x4DDD ; ld c, ixl    ;; [ 8] Restore Sprite Width into C

   ld    a, d                 ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   add   #0x08                ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   ld    d, a                 ;; [ 4]
   and   #0x38                ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   jp   nz, dms_sprite_height_loop ;; [10]  by checking the 4 bits that identify present memory line. 
                                   ;; ....  If 0, we have crossed boundaries

dms_sprite_8bit_boundary_crossed:
   ld    a, e                 ;; [ 4] DE = DE + 0xC050h
   add   #0x50                ;; [ 7] -- Relocate DE pointer to the start of the next pixel line:
   ld    e, a                 ;; [ 4] -- DE is moved forward 3 memory banks plus 50 bytes (4000h * 3) 
   ld    a, d                 ;; [ 4] -- which effectively is the same as moving it 1 bank backwards and then
   adc   #0xC0                ;; [ 7] -- 50 bytes forwards (which is what we want to move it to the next pixel line)
   ld    d, a                 ;; [ 4] -- Calculations are made with 8 bit maths as it is faster than other alternatives here

   jp  dms_sprite_height_loop ;; [10] Jump to continue with next pixel line

dms_sprite_copy_ended:
   pop  ix                    ;; [14] Restore IX before returning
   ret                        ;; [11] Return to caller
