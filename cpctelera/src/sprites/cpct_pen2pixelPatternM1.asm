;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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
;; Function: cpct_pen2pixelPatternM1
;;
;;    Returns 1 byte in Mode 1 screen pixel format containing a pattern with
;; all 4 pixels in the same pen colour as given by the argument *PEN*.
;;
;; C Definition:
;;    <u8> <cpct_pen2pixelPatternM1> (<u16> *PEN*)
;;
;; Assembly Call (Input parameters on Registers):
;;    > call cpct_pen2pixelPatternM1_asm
;;
;; Input Parameters (2 bytes):
;;    (2B HL) PEN - Pen colour number 
;;
;; Return value:
;;    A 4-pixel-byte pattern in mode 1 screen pixel format, with its 
;; 4 pixels coloured same as *PEN* parameter.
;;
;;    C-bindings   - 1 byte (in L)
;;    ASM-bindings - 1 byte (in A)
;;
;; Parameter Restrictions:
;;    * *PEN* ([0-3], 16 bits, unsigned). Although parameter is 16-bits-wide, 
;; values outside the [0-3] range will produce undefined behaviour. It is 16-bits 
;; only for optimization purposes.
;; 
;; Known limitations:
;;    * Parameter boundaries are not checked. Providing parameters out of range 
;; will result in undefined behaviour.
;;    * This function is to be used with variable parameters. For constant 
;; parameters it is better to use the macro version <CPCTM_PEN2PIXELPATTERN_M1> 
;; to save CPU time and bytes.
;;
;; Details:
;;    This function receives 1 pen colour as parameter (Mode 1, [0-3]) and uses 
;; it to create a byte in mode 1 screen pixel format, with its 4 pixels of the
;; pen colour provided
;;    
;;    In mode 1, only 4 diferent colours can be displayed at the same time due
;; to the codification of video memory (the screen pixel format). Each byte of
;; video memory codifies 4 pixels, which leaves 2 bits for each pixel, 4 possible
;; combinations. The codification of these 4 pixels inside the byte is not 
;; linear, but as follows,
;; (start code)
;;    Memory represenation
;;    BIT   |   7 6 5 4 3 2 1 0    
;;    BYTE  | [ a b c d A B C D ]      >> Each letter represents 1-bit
;;
;;    Screen representation
;;    Pixel |   0     1     2     3
;;    PEN   | [ Aa ][ Bb ][ Cc ][ Dd ] >> Each pixel takes the colour of its 2 bits
;; (end code)
;;
;;    This function has only 4 possible valid return values, for each one of
;; its 4 valid input parameters,
;; (start code)
;;    ---------------------------
;;    |     |      PATTERN     |
;;    | PEN |   Binary  | Hex  |
;;    --------------------------
;;    |   0 | 0000 0000 | 0x00 |
;;    |   1 | 1111 0000 | 0xF0 |
;;    |   2 | 0000 1111 | 0x0F |
;;    |   3 | 1111 1111 | 0xFF |
;;    ---------------------------
;; (end code)
;;
;;    The output of this function is useful for drawing to screen or as input to
;; other CPCtelera functions that require pixel patterns, like <cpct_spriteColourizeM1>.
;; Next example shows how it can be used to draw an horizontal line in mode 1 with
;; all pixels of the same colour (pen),
;; (start code)
;;    // Draws an horizontal line at given location 
;;    void drawHorizontalLineM1(u8* pvideomem, u16 pen) {
;;       // Get a 4-pixels-byte with all pixels of pen colour
;;       u8 pixels = cpct_pen2pixelPatternM1(pen); 
;;       // Draw 320 pixels by setting 80 consecutive bytes
;;       // of video memory. Each byte's 4-pixels of the same colour (pen)
;;       cpct_memset(pvideomem, pixels, 80);
;;    }
;; (end code)
;;
;;    The following example shows how to use this codification to replace pixels of
;; pen colour *OldPen*, by pixels of pen colour *NewPen* inside an array/sprite. 
;; Alternative, you may also want to use <cpct_pens2pixelPatternPairM1> for better 
;; performance and clarity,
;; (start code)
;;    // Entity struct declaration
;;    typedef struct {
;;       u8* sprite;
;;       u8 width, height;
;;       // .....
;;    } Entity_t;
;;    
;;    // ......
;;    
;;    // This function replaces all pixels of a given colour (OldPen) inside the
;;    // sprite of an entity by another given colour (NewPen)
;;    void replaceSpriteColour(Entity_t* e, u16 OldPen, u16 NewPen) {
;;       // Get 4-pixels-byte patterns for OldPen and NewPen
;;       u8  const old_px  = cpct_pen2pixelPatternM1(OldPen);
;;       u8  const new_px  = cpct_pen2pixelPatternM1(NewPen);
;;       // Create 16-bit Replace Pattern required as parameter
;;       //  > Higher 8-bits = old_px, Lower 8-bits = new_px
;;       u16 const rplcPat = (old_px << 8) | new_px;   // Equivalent to 256*old_px + new_px
;;       // Calculate complete size of the sprite array
;;       u8  const size    = e->width * e->height;
;;       // Replace colour OldPen with colour NewPen in 
;;       // all the pixels of the sprite of entity e
;;       cpct_spriteColourizeM1(rplcPat, size, e->sprite);
;;    }
;; (end code)
;;
;; Destroyed Register values:
;;    C-bindings   -  F, HL, BC
;;    ASM-bindings - AF, HL, BC
;;
;; Required memory:
;;    6 bytes
;;
;; Time Measures:
;; (start code)
;;  --------------------------------------------
;;  |    Case    | microSecs (us) | CPU Cycles |
;;  --------------------------------------------
;;  | Any        |       11       |    44      |
;;  --------------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_pen2fourPixelM1_table      ;; Conversion table from PEN to 4-pixels-byte screen pixel format

   ld    bc, #cpct_pen2fourPixelM1_table  ;; [3] BC Points to the start of the Replace Colours Pattern Conversion Array
   add   hl, bc                           ;; [3] HL += BC // HL now points to the Colour Pattern for the PEN given in BC

   ;; Return managed at binding code
