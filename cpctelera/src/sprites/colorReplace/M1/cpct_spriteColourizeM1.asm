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
;; Function: cpct_spriteColourizeM1
;;
;;    Replaces a colour pattern (Find Pattern) by a new one (Insert Pattern) in 
;; an array/sprite. Typical use is to replace pixels of a given PEN Colour (OldPen)
;; into pixels of a new PEN Colour (NewPen).
;;
;; C Definition:
;;    void <cpct_spriteColourizeM1> (<u16> *rplcPat*, <u16> *size*, void* *sprite*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;  (1B DE) rplcPat - Replace Pattern => 1st byte(D)=Pattern to Find, 2nd byte(E)=Pattern to insert instead
;;  (2B BC) size    - Size of the Array/Sprite in bytes (if it is a sprite, width*height)
;;  (2B HL) sprite  - Array/Sprite Pointer (array of bytes with Mode 1 pixel data)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_spriteColourizeM1_asm
;;
;; Parameter Restrictions:
;;  * *rplcPat* ([0000-FFFF], 16bits, unsigned) should contain 2 8-bit colour pixel 
;; patterns in Mode 1 screen pixel format (see below for an explanation of patterns).
;; Any value is valid, but values different of what you actually want will probably
;; result in estrange colours in the final array/sprite.
;;  * *size*    ([0001-65535], 16bits, unsigned) must be the exact size in bytes of the
;; given array/sprite. A value of *0* will be treated as 65536, probably resulting on all 
;; memory being overwritten and your program crashing.
;;  * *sprite*  ([0000-FFFF], 16bits, pointer) must be a pointer to the start of an array 
;; containing sprite's pixels data in Mode 1 screen pixel format. Total amount of bytes
;; in the array should be equal to *size*.
;;
;; Details:
;;    This function replaces pixels of a given colour pattern (Find Pattern *FindPat*)
;; by pixels of another colour pattern  (Insert Pattern *InsrPat*) inside an array/sprite. 
;; Typically, these colour patterns consists of a single Pen Colour repeated on all 
;; 4 pixels of a byte in Mode 1 screen pixel format. In this common case, the function 
;; changes all pixels of the Pen Colour in *FindPat* (*OldPen*) into the Pen Colour in 
;; *InsrPat* (*NewPen*). Summarized, it changes pixels of desired colours into desired
;; new colours inside an array/sprite.
;;
;;    The parameter *rplcPat* (Replace Pattern) should be composed of both *FindPath* 
;; and *InsrPat* as follows,
;; (start code) 
;; |       rplcPat       | 16-bits
;; -----------------------
;; | FindPat  | InsrPat  | 8-bits | 8-bits
;; -----------------------
;; | 10101010 | 01010101 | 8-bits | 8-bits (exact bits shown are just an example)
;; -----------------------
;; (end code)
;; therefore, *rplcPat* is a 16-bit value with its highest 8-bits equal to *FindPat* and
;; its lowest 8-bits equal to *InsrPat*. To easily obtain these patterns for desired
;; pen colours (giving an *OldPen* and a *NewPen* to be found and replaced) you may
;; want to use the functions <cpct_pen2pixelPatternM1> and <cpct_pens2pixelPatternPairM1>
;; or its counterpart macros for compile-time constant calculations
;; <CPCTM_PEN2PIXELPATTERN_M1> and <CPCTM_PENS2PIXELPATTERNPAIR_M1>.
;;
;;    To perform the replacing, the function uses *FindPat* to identify the pixels that
;; will be changed in colour. It performs an XOR operation between *FindPat* and each 
;; byte in the array/sprite. As a result of these operation, pixels coinciding with
;; *FindPath* will be zeroed (their two bits will turn into 0). Afterwards, all zeroed
;; pixels (those selected by *FindPat*) will be replaced with their corresponding values
;; in *InsrPath*. In some sense, we could say that *FindPat* acts as a selector mask, 
;; selecting the pixels to be changed, and *InsrPat* holds the new values to be inserted.
;;
;;    Let's see this operation with an example. Imagine we have a sprite with an stripped
;; pattern like this,
;; (start code)
;; Addr |   Bytes     |               | Bit Encoding for | 
;; --------------------               |    Mode 1 RBRY   |
;; C000 | RCRY | RCRY |   (R)ed       -------------------- Rr = 11 (3, Red   )
;; C800 | RCRY | RCRY |   (C)yan      | 1 0 1 1  1 1 1 0 | Cc = 10 (2, Cyan  )
;; D000 | RCRY | RCRY |   (Y)ellow    | r c r y  R C R Y | Yy = 01 (1, Yellow)
;;      | .... | .... |               --------------------
;; (end code)
;; we will see 8 vertical lines of (Red-Cyan-Red-Yellow)*2 colours. Each byte in video 
;; memory encoding this Pattern of 4-pixels will be represented by the binary number 
;; 10111110 (0xBE hexadecimal). Lets suppose we wanted to change the second Red column
;; of pixels (the one between Cyan and Yellow) by a Blue value. Default pen colours are
;; Red = Pen 3 (11 in binary) and Blue = Pen 0 (00 in binary). We can construct a
;; *FindPat* to select the Red pixels in the 3rd column of each byte (not affecting 
;; red pixels in 1st column) like this,
;; (start code)
;;  | Bit Encoding for | 
;;  |    Mode 1 BBRB   |
;;  -------------------- Rr = 11 (3, Red )
;;  | 0 0 1 0  0 0 1 0 | Bb = 00 (0, Blue)
;;  | b b r b  B B R B | (0x22 Hexadecimal) 
;;  --------------------
;; (end code)
;; this *FindPat* will be valid for selecting blue pixels in columns 1,2 and 4, and 
;; red pixels in column 3. As our original pattern does not have blue colours, it will
;; only detect pixels in column 3. Now, to replace those pixels with Blue, we could 
;; use a *InsrPat* with all 4 pixels blue, like this one,
;; (start code)
;;  | Bit Encoding for | 
;;  |    Mode 1 BBBB   |
;;  -------------------- 
;;  | 0 0 0 0  0 0 0 0 | Bb = 00 (0, Blue)
;;  | b b b b  B B B B | (0x00 hexadecimal)
;;  --------------------
;; (end code)
;; this will ensure that pixels in columns 1,2 and 4 will be left untouched either the
;; case (if we found a Blue and replace by another Blue, they will be left equal) and
;; only red pixels in the 3rd column will be changed into Blue. Pixels of colours other
;; than Red in 3rd column won't be selected and, therefore, will remain unchanged.
;;
;;   Following is a code sample doing the exact operation we have described of replacing
;; only red pixels in 3rd column by blue pixels,
;; (start code)
;;    // This function replaces all Red pixels (Pen 3) found in the 3rd 
;;    // column of each byte (group of 4-pixels) by Blue (Pen 0)
;;    void replaceRedsIn3rdColumnWithBlue(u8* sprite, u16 size) {
;;       // Replace Pattern (*rplcPat*) should find red pixels (Pen 3)
;;       // in 3rd column (*FindPat* = 0b00100010 = 0x22) and insert
;;       // Blue pixels (Pen 0, *InsrPat* = 0b00000000 = 0x00)
;;       u16 const rplcPat = 0x2200; // *FindPat*=0x22, *InsrPat*=0x00
;;       
;;       // Perform pixel colour replace in the sprite
;;       cpct_spriteColourizeM1(rplcPat, size, sprite);
;;    }
;; (end code)
;;
;;   Typically, this function will be used to change all pixels of a given colour (*OldPen*)
;; into another colour (*NewPen*). That is easily performed using corresponding *FindPat* 
;; and *InsrPat* with all pixels set to *OldPen* and *NewPen* respectively. This can be
;; easily done by using <cpct_pens2pixelPatternPairM1> function, with takes *OldPen* and
;; *NewPen* as parameters and returns the 16-bits *rplcPat* containing *FindPat* and
;; *InsrPat*. Following example shows how to do it,
;; Example,
;; (start code) 
;;    // Alien Struct Declaration
;;    typedef struct {
;;       u8 skinColour;
;;       u8 width, height;
;;       u8* sprite;
;;       // ....
;;    } Alien_t;
;;    
;;    //....
;;
;;    // Changes the skin colour of an alien by changing all the pixels in its
;;    // sprite from its old skinColour to te given NewColour
;;    void setAliensSkinColour(Alien_t* alien, u8 const newColour) {
;;       // Calculate Replace Pattern to change Alien Skin Colour from
;;       // its current value to the new desired colour (newColour)
;;       u8 const rplcPat = cpct_pens2pixelPatternPairM1(alien->skinColour, newColour);
;;       // Now, change skin colour of our Alien
;;       u8 const size    = alien->width * alien->height;
;;       cpct_spriteColourizeM1(rplcPat, size, alien->sprite);
;;     
;;       // Acknowledge the change in colour in our Alien Data Structure
;;       alien->skinColour = newColour;
;;    }
;; (end code)
;;
;; Known limitations:
;;    * This function does not check for parameter boundaries or their validity.
;; Incorrect our out-of-bounds values will cause undefined behaviour, potentially
;; overwritting memory outside the array/sprite.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL, IXl
;;
;; Required memory:
;;     C-bindings - 30 bytes
;;   ASM-bindings - 26 bytes
;;
;; Time Measures:
;; (start code)
;; |----------------------------------------------------------
;; |  Case       |    microSecs (us)  |      CPU Cycles      |
;; |----------------------------------------------------------
;; | Any (i.e.)  | 23 + 22(S) + 3(SS) |  92 + 88(S) + 12(SS) |
;; |----------------------------------------------------------
;; | S=16 (2x 8) |        378         |         1512         |
;; | S=32 (4x 8) |        730         |         2920         |
;; | S=64 (4x16) |       1434         |         5736         |
;; | S=128(8x16) |       2842         |        11368         |
;; | S=256(8x32) |       5661         |        22644         |
;; | S=512(8x64) |      11296         |        45184         |
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

.mdelete cpctm_generate_spriteColourizeM1
.macro cpctm_generate_spriteColourizeM1 _NumIncsHL
   ;; Calculante E = (FindPat ^ InsrPat). This will be used at the end of the routine
   ;; to insert InsrPat in the byte by XORing again, as the final operation will
   ;; be performed only on the bits of the SelectedPixels (those coinciding with
   ;; FindPat). This final operation will then be (1) XOR against FindPat (Zeroing bits,
   ;; because they are equal) and (2) XOR againts InsrPat (Inserting InsrPat bits, 
   ;; because its an XOR against zeros). That way, we will perform 2 operations on 1.
   ld     a, e    ;; [1] / 
   xor    d       ;; [1] | E = (InsrPat ^ FindPat)
   ld     e, a    ;; [1] \ 
   ld__ixl_d      ;; [2] IXL = D (FindPat) Save for later use
   ;; BC holds the counter, but will be decreased by bytes (--C till 0, then --B).
   ;; Because of this, B needs to be at least 1, to become zero on first decrease
   ;; (Effectively, B + 1 will count B times, as Zero is not counted)
   inc    b       ;; [1] ++B  

   ;; Loop for all the bytes of the Array/Sprite
   ;; Then Modify pixels of each byte using Patterns
loop:
   ;; First, perfom an XOR between the FindPat and the SpriteByte to modify
   ;; This XOR will convert to zero all bits of pixels coinciding with FindPat
   ;; But will left at least a bit with a 1 in the others (not coinciding with FindPat)
   ld__a_ixl      ;; [2] / *HL = SpriteByte, IXl = FindPat
   xor   (hl)     ;; [2] | A = D = (SpriteByte ^ FindPat)
   ld     d, a    ;; [1] \   => SelectedPixels=00, OtherPixels!=00 
   ;; Now we want to create a MASK byte with 0's in all the bits corresponding to
   ;; selected pixels (coinciding with FindPat) and 1's in all other bits. To do this,
   ;; as each pixels has 2 bits, we just need to OR both of them. Pixels selected have
   ;; both of them equal to 0, and non-selected have at least a 1. We rotate the
   ;; byte to align both pair of bits in each pixel, then we or them together.
   rrca           ;; [1] /    RoR = Rotate Right
   rrca           ;; [1] | 
   rrca           ;; [1] | A = RoR(SpriteByte ^ FindPat, 4) | (SpriteByte ^ FindPat)
   rrca           ;; [1] |   => MASK (SelectedPixels==00, OtherPixels==11)
   or     d       ;; [1] \
   ;; We revert the MASK (producing ~MASK), making selected pixels be 11, and Others 00
   cpl            ;; [1] A = ~MASK (SelectedPixels==11, OtherPixels==00)

   ;; Now we are ready to insert InsrPath. First we multiply (AND) our negated mask (~MASK)
   ;; with E=(InsrPath ^ FindPat). This does the effect of Masquerading (InsrPat ^ FindPat), 
   ;; leaving 0's on all pixels that do not have to be modified, and leaving the others 
   ;; untouched. Those others will be XORed with the original SpriteByte, producing 2 
   ;; operations (SpriteByte ^ FindPat) ^ InsrPat, but only on the bits of SelectedPixels
   ;; (because of the previous Masquerading operation). Those 2 operations are (1) Zeroing
   ;; bits, (2) inserting InsrPat (because we XOR against zeros). Result is FindPat removed
   ;; and InsrPat inserted, but only on the bits of the SelectedPixels (as others are 0's 
   ;; after Masquerading).
   and    e       ;; [1] A = ~MASK & (InsrPat ^ FindPat)
   xor   (hl)     ;; [2] A = (~MASK & (InsrPat ^ FindPat)) ^ SpriteByte
   
   ld    (hl), a  ;; [2] Save modified byte
   
   ;; Make HL point to next byte in the array and decrement BC, repeating while not 0
   ;; HL will be incremented 1 in consecutive arrays, 2 or more if there are interleaved
   ;; mask bytes. This is configurable by the macro parameter _NumIncsHL
   .rept _NumIncsHL
      inc   hl       ;; [2] ++HL
   .endm
   dec    c       ;; [1] --C
   jr    nz, loop ;; [2/3] if (C > 0) then BC != 0, continue with the loop
   djnz  loop     ;; [3/4] if (--B > 0) then BC != 0, continue with the loop
   
   ret            ;; [3] Finished colourizing the sprite
.endm