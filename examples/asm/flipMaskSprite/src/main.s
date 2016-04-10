;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;===============================================================================
;; DEFINED CONSTANTS
;;===============================================================================

pvideomem     = 0xC000         ;; First byte of video memory
palete_size   = 4              ;; Number of total palette colours
border_colour = 0x0010         ;; 0x10 (Border ID), 0x00 (Colour to set: White).
tile_HxW      = 0x3214         ;; Height (50 pixels or bytes,  0x32) 
                               ;; Width  (80 pixels, 20 bytes, 0x14) 1 byte = 4 pixels
sprite_HxW    = 0x9119         ;; Height (145 pixels or bytes,  0x91) 
sprite_Width  = 0x19           ;; Width  (100 pixels, 25 bytes, 0x19) 1 byte = 4 pixels

;;===============================================================================
;; DATA SECTION
;;===============================================================================

.area _DATA

;; Sprites and palette are defined in an external file. As they are
;; defined in C language, their symbols will be preceded by an underscore.
;; We declare sprite symbols here as global, and the linker will look
;; for them on the other file.
.globl _g_tileset
.globl _g_palette
.globl _g_spr_knight

;; Location offsets for background tiles
;;    There are 16 background tiles, each one taking 80x50 pixels. As tiles
;; will always be at the same place with respect to the origin og the background
;; (coordinate (0,0), top-left corner of the background), we can pre-calculate
;; their offset in bytes with respect to the origin. Next array contains the
;; pre-calculated 16 offsets, which will let easily draw the background by
;; taking each tile and drawing it at origin + offset.
bg_tile_offsets:
;; COLUMN |   0   |  80   |  160  |  240  |   ROW
;;--------------------------------------------------
       .dw 0x0000, 0x0014, 0x0028, 0x003C  ;;   0
       .dw 0x11E0, 0x11F4, 0x1208, 0x121C  ;;  50
       .dw 0x23C0, 0x23D4, 0x23E8, 0x23FC  ;; 100
       .dw 0x35A0, 0x35B4, 0x35C8, 0x35DC  ;; 150
;;--------------------------------------------------


;;===============================================================================
;; CODE SECTION
;;===============================================================================

.area _CODE

;; Include macros to easily manage undocumented opcodes
.include "macros/cpct_undocumentedOpcodes.s"

;; Symbols with the names of the CPCtelera functions we want to use
;; must be declared globl to be recognized by the compiler. Later on,
;; linker will do its job and make the calls go to function code.
.globl cpct_disableFirmware_asm
.globl cpct_setVideoMode_asm
.globl cpct_setPalette_asm
.globl cpct_setPALColour_asm
;.globl cpct_getScreenPtr_asm
;.globl cpct_hflipSpriteM1_asm
;.globl cpct_hflipSpriteM1_r_asm ;; Alternative ROM-friendly version
.globl cpct_drawSprite_asm
;.globl cpct_drawStringM1_f_asm
.globl cpct_waitVSYNC_asm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: initialize
;;    Sets CPC to its initial status
;; DESTROYS:
;;    AF, BC, DE, HL
;;
initialize::
   ;; Disable Firmware
   call  cpct_disableFirmware_asm   ;; Disable firmware

   ;; Set Mode 1
   ld    c, #1                      ;; C = 1 (New video mode)
   call  cpct_setVideoMode_asm      ;; Set Mode 1
   
   ;; Set Palette
   ld    hl, #_g_palette            ;; HL = pointer to the start of the palette array
   ld    de, #palete_size           ;; DE = Size of the palette array (num of colours)
   call  cpct_setPalette_asm        ;; Set the new palette

   ;; Change border colour
   ld    hl, #border_colour         ;; L=Border colour value, H=Palette Colour to be set (Border=16)
   call  cpct_setPALColour_asm      ;; Set the border (colour 16)

;   ;; Draw upper string             
;   ld    hl, #str_demo              ;; HL points to the string with the demo message
;   ld    bc, #str_colour            ;; BC = fg/bg colours used to draw the string
;   ld    de, #str_location          ;; DE points to the place in video memory where the string will be drawn
;   call  cpct_drawStringM1_f_asm    ;; Draw the string (fast method)

   ret                              ;; return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: drawFullBackground
;;    Draws all background starting at a given location in the screen
;; INPUT:
;;    DE: Pointer to the place in video memory where background is to be drawn
;; DESTROYS:
;;    AF, BC, DE, HL
;;
drawFullBackground::
   ld    hl, #bg_tile_offsets    ;; HL points to the start of the memory offsets for tiles
   ld    bc, #_g_tileset         ;; BC points to the start of the tileset
   ld__ixl #16                   ;; IXL will act as counter for the number of tiles

next_tile:
   push  de       ;; Save DE (Pointer to the origin of the background (0,0) coordinates)

   ;; Make DE Point to the place where the next tile is to be drawn, that is
   ;;   DE += (HL), as DE points to the origin (0,0) of the background and HL points
   ;; to the Offset to be added to point to the place where the tile should be drawn
   ld     a, e    ;; | E += (HL) as HL points to the Least Significant Byte of
   add  (hl)      ;; |  the offset to be added to DE (remember that Z80 is little endian)
   ld     e, a    ;; |
   
   inc   hl       ;; HL++, HL points now to the Most Significant Byte of the offset value

   ld     a, d    ;; | D += (HL) + Carry, as HL points to the MSB of the offset and
   adc  (hl)      ;; |   Carry contains the carry of the last E += (HL) operation.
   ld     d, a    ;; |

   ;; Make HL point to the offset for the next tile to be drawn, then save it
   inc   hl       ;; HL++, so HL points to the LSB of the offset for the next tile to be drawn
   push  hl       ;; Save HL in the stack to recover it for next loop iteration

   ;; Now that DE points to the place in video memory where the tile should be drawn,
   ;; make HL point to the sprite (the tile) that should be drawn there. Get that 
   ;; pointer from (BC), that points to the next element in the _g_tileset array, that is,
   ;; the next sprite (tile) to be drawn
   ld     a, (bc) ;; A = LSB from the pointer to the next tile to be drawn
   ld     l, a    ;; L = A = LSB
   inc   bc       ;; BC++, so that BC points to the MSB of the next tile to be drawn
   ld     a, (bc) ;; A = MSB from the pointer to the next tile to be drawn
   ld     h, a    ;; H = A = MSB (Now HL Points to the next tile to be drawn)

   ;; Make BC point to the pointer to the next sprite (tile) to be drawn and save it
   inc   bc       ;; BC++, so that it points to the LSB of the next sprite (tile) in the _g_tileset
   push  bc       ;; Save BC in the stack to recover it for next loop iteration

   ;; Draw the tile.
   ;; HL already points to the sprite
   ;; DE already points to the memory location where to draw it
   ld    bc, #tile_HxW           ;; BC = Sprite WidthxHeight
   call  cpct_drawSprite_asm     ;; Draw the sprite on the screen

   ;; Recover saved values for next iteration from the stack
   pop   bc       ;; BC points to the pointer to the next sprite (tile) to be drawn
   pop   hl       ;; HL points to the offset with respect to (0,0) where next tile should be drawn
   pop   de       ;; DE points to the origin (0,0) in video memory where background is being drawn

   dec__ixl         ;; IXL--, one less tile to be drawn
   jr nz, next_tile ;; If IXL!=0, there are still some tiles to be drawn, so continue

   ret            ;; Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main:: 
   call  initialize           ;; Initialize the CPC

   ld    de, #pvideomem       ;; DE points to the start of video memory, where Background should be drawn
   call  drawFullBackground   ;; Draw the background

loop:
   jr    loop                 ;; Repeat forever
