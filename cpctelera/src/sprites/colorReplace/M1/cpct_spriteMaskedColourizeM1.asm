;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud Bouche (@Arnaud6128)
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_spriteMaskedColourizeM1
;;
;;    Replaces a colour pattern (Find Pattern) by a new one (Insert Pattern) in 
;; an array/sprite with interleaved mask bytes. Typical use is to replace pixels 
;; of a given PEN Colour (OldPen) into pixels of a new PEN Colour (NewPen).
;;
;; C Definition:
;;    void <cpct_spriteMaskedColourizeM1> (<u16> *rplcPat*, <u16> *size*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (1B DE) rplcPat - Replace Pattern => 1st byte(D)=Pattern to Find, 2nd byte(E)=Pattern to insert instead
;;  (2B BC) size    - Size of the Array/Sprite in bytes (if it is a sprite, width*height)
;;  (2B HL) sprite  - Array/Sprite Pointer (array of bytes with Mode 1 pixel data)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_spriteMaskedColourizeM1_asm
;;
;; Parameter Restrictions:
;;  * *rplcPat* ([0000-FFFF], 16bits, unsigned) should contain 2 8-bit colour pixel 
;; patterns in Mode 1 screen pixel format (see below for an explanation of patterns).
;; Any value is valid, but values different of what you actually want will probably
;; result in estrange colours in the final array/sprite.
;;  * *size*    ([0001-32768], 16bits, unsigned) must be the exact size in bytes of the
;; given array/sprite. A value of *0* will be treated as 65536, probably resulting on all 
;; memory being overwritten and your program crashing.
;;  * *sprite*  ([0000-FFFF], 16bits, pointer) must be a pointer to the start of an array 
;; containing sprite's pixels data in Mode 1 screen pixel format. Total amount of bytes
;; in the array should be equal to *size*.
;;
;; Details:
;;    This function does exactly the same operation as <cpct_spriteColourizeM1> but 
;; taking into account that the given array/sprite contains interleaved mask bytes.
;; The interleaved mask bytes must be in the following format,
;; (start code)
;; Array storage format:  <-byte 1-> <-byte 2-> <-byte 3-> <-byte 4->
;;                        <- mask -> <-colour-> <- mask -> <-colour->
;; ------------------------------------------------------------------------------
;;      void* sprite =  {    0xFF,     0x00,       0x00,     0xC2,   .... };
;; ------------------------------------------------------------------------------
;; Video memory output:   <- 1st Screen byte -> <- 2nd Screen byte -> 
;; ______________________________________________________________________________
;;         Example 1: Definition of a masked sprite and byte format
;; (end)
;; first byte of the array/sprite must be the mask byte to be used with the second 
;; byte, which is the first screen pixel mode 1 format byte, with the colours of the 
;; first 4-pixels. Next bytes follow same order: mask, colour, mask, colour....
;;
;;    This function ignores mask bytes and replaces pixel colours in the colour bytes.
;; *Beware!* This means that your mask will not be modified. Therefore, if you change
;; colours of pixels outside the mask, those pixels will be ORed with the corresponding
;; background pixels, which won't be removed for them. This could led to undesired 
;; effects.
;;
;;    Read <cpct_spriteColourizeM1> for more details on how colour replacing works.
;;
;;    The following code example shows a typical use of this function,
;; (start code) 
;;    // Enemy Struct Declaration
;;    typedef struct {
;;       u8 tsPattern;        // T-Shirt Pixel Pattern
;;       u8 width, height;
;;       u8* maskedSprite;
;;       // ....
;;    } Enemy_t;
;;    
;;    //....
;;    
;;    // Changes the T-Shirt Pattern of our enemies. It
;;    // searches the pixels in the current T-Shirt Pattern
;;    // and replaces them with a new one
;;    void changeTShirt(Enemy_t* ene, u8 const newPattern) {
;;       // Create a replace pattern looking for current tspattern
;;       // and replacing it with the new pattern.
;;       //  Higher 8-bits = FindPat, Lower 8-bits = InsrPat
;;       u16 const rplcPat = ((u16)ene->tsPattern << 8) | newPattern;
;;       
;;       // Calculate size of our enemy sprite and replace T-Shirt Pattern
;;       u8  const size    = ene->width * ene->height;
;;       cpct_spriteColourizeM1(rplcPat, size, ene->maskedSprite);
;;     
;;       // Remember the new T-shirt Pattern for later changes
;;       ene->tsPattern = newPattern;
;;    }
;; (end code)
;;
;; Known limitations:
;;    * Beware! The function does not change mask bytes. Therefore, no matter what pixels
;; you change the colour of in your sprite, same pixels will be removed from background
;; each time you draw the sprite. If you change colours of pixels outside your mask
;; boundaries, those pixels will be ORed with the correspoding background ones, potentially
;; leading to undesired glitches.
;;    * This function does not check for parameter boundaries or their validity.
;; Incorrect our out-of-bounds values will cause undefined behaviour, potentially
;; overwritting memory outside the array/sprite.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL, IXl
;;
;; Required memory:
;;     C-bindings - 32 bytes
;;   ASM-bindings - 28 bytes
;;
;; Time Measures:
;; (start code)
;; |----------------------------------------------------------
;; |  Case       |    microSecs (us)  |      CPU Cycles      |
;; |----------------------------------------------------------
;; | Any (i.e.)  | 25 + 24(S) + 3(SS) | 100 + 92(S) + 12(SS) |
;; |----------------------------------------------------------
;; | S=16 (2x 8) |        412         |         1648         |
;; | S=32 (4x 8) |        796         |         3184         |
;; | S=64 (4x16) |       1564         |         6256         |
;; | S=128(8x16) |       3100         |        12400         |
;; | S=256(8x32) |       6175         |        24700         |
;; | S=512(8x64) |      12322         |        49288         |
;; |----------------------------------------------------------
;; | Asm saving  |        -15         |          -60         |
;; |----------------------------------------------------------
;; (end code)
;;    S  - Size of the array/Sprite in bytes
;;    SS - 1 + [ *S* / 256 ] · · · · ( *[x]* = Integral part of x )
;;
;; Credits:
;;    Original routine optimized by @Docent and discussed in CPCWiki :
;; http://www.cpcwiki.eu/forum/programming/cpctelera-colorize-sprite/
;;
;; Thanks to all who participated in the discussion for their help and support.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.mdelete cpctm_generate_spriteMaskedColourizeM1
.macro cpctm_generate_spriteMaskedColourizeM1 _NumIncsHL
   ;; Code for this function is exactly the same as cpct_spriteColourizeM1
   ;; but incrementing HL 2 times in each loop iteration to jump over
   ;; the interleaved mask byte
   .include /cpct_spriteColourizeM1.asm/

   ;; Generate the code with the given number of NumIncs (2)
   inc   hl                      ;; [2] Jump over the first mask byte
   cpctm_generate_spriteColourizeM1 _NumIncsHL
.endm