;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;;===============================================================================
;; DEFINED CONSTANTS
;;===============================================================================

pvideomem      = 0xC000  ;; First byte of video memory
pbackbuffer    = 0x8000  ;; First byte of the hardware backbuffer
palete_size    = 4       ;; Number of total palette colours
border_colour  = 0x0010  ;; 0x10 (Border ID), 0x00 (Colour to set: White).
screen_Width   = 0x50    ;; Width of the screen in bytes (80 bytes, 0x50)
tile_HxW       = 0x3214  ;; Height (50 pixels or bytes,  0x32) 
                         ;; Width  (80 pixels, 20 bytes, 0x14) 1 byte = 4 pixels
knight_WxH     = 0x9119  ;; Height (145 pixels or bytes,  0x91) 
knight_Width   = 0x19    ;; Width  (100 pixels, 25 bytes, 0x19) 1 byte = 4 pixels
knight_offset  = 0x39E0  ;; Offset for location (0,55) with respect to screen (0,0)

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
.include "macros/cpct_undocumentedOpcodes.h.s"

;; Symbols with the names of the CPCtelera functions we want to use
;; must be declared globl to be recognized by the compiler. Later on,
;; linker will do its job and make the calls go to function code.
.globl cpct_disableFirmware_asm
.globl cpct_setVideoMode_asm
.globl cpct_setPalette_asm
.globl cpct_setPALColour_asm
.globl cpct_drawSprite_asm
.globl cpct_drawSpriteMasked_asm
.globl cpct_hflipSpriteMaskedM1_asm
.globl cpct_setVideoMemoryPage_asm

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

   ret                              ;; return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: drawBackgroundTiles
;;    Draws as many background tiles as the number in IXL, picking their offsets
;; from the offset vector provided in HL, and the pointers to the tiles from 
;; the tile vector provided in BC. All tiles will be drawn one by one, in order.
;; 
;; INPUT:
;;    HL: Pointer to the offsets vector (to place tiles in video memory)
;;    DE: Pointer to the place in video memory where background is to be drawn
;;    BC: Pointer to the tiles that will be drawn
;;    IXL:Number of tiles to draw
;;    
;; DESTROYS:
;;    AF, BC, HL, IXL
;;
drawBackgroundTiles::

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
;; FUNC: switch_screen_buffer
;;    Switches between front buffer and backbuffer.
;;
;; DESTROYS:
;;    AF, BC, HL
;;
screen_buffer: .db >pbackbuffer  ;; This variable holds the upper byte of the memory address of the screen backbuffer 
                                 ;; It changes every time buffers are switched, so it always contains backbuffer address.
switch_screen_buffer::
   ;; Check which one of the buffers is actually tagged as backbuffer (0xC000 or 0x8000)
   ld   hl, #screen_buffer    ;; HL points to the variable holding actual backbuffer address Most Significant Byte 
   ld    a, (hl)              ;; A = backbuffer address MSB (0xC0 or 0x80)
   cp #0xC0                   ;; Check if it is 0xC00
   jr    z, to_back_buffer    ;; If it is 0xC000, set it to 0x8000

to_front_buffer:
   ;; Actual backbuffer is 0x8000. Switch to 0xC000
   ld (hl), #>pvideomem               ;; Save 0xC0 as new backbuffer address MSB
   ld    l, #0x20                     ;; | Then show new frontbuffer (0x8000) 
   call  cpct_setVideoMemoryPage_asm  ;; | ... in the screen
   
   ret                        ;; And Return

to_back_buffer:
   ;; Actual backbuffer is 0xC000. Switch to 0x8000
   ld (hl), #>pbackbuffer             ;; Save 0x80 as new backbuffer address MSB
   ld    l, #0x30                     ;; | Then show new frontbuffer (0x8000) 
   call  cpct_setVideoMemoryPage_asm  ;; | ... in the screen

   ret                        ;; And Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: redrawKnight
;;    Erases previous location of the Knight by repainting tiles 4-15 (3 down
;; lines of the screen), then draws the knight again over clear background
;; 
;; INPUTS:
;;    DE: Pointer to the start of video memory buffer where the knight will be drawn
;; 
;; DESTROYS:
;;    AF, BC, DE, HL
;;
knight_x:      .db 00   ;; Column where the knight is actually located
knight_dir:    .db 00   ;; Direction towards the knight is looking at (0: right, 1: left)

redrawKnight::
   omitted = 4*2        ;; To draw tiles 4-15 we must omit 4 of them. As each pointer takes 2 bytes, 
                        ;; ... we need to advance 4*2 bytes in the array to reach tile 4.

   ;; Erase previous sprite drawing 3 down rows of tiles
   ld__ixl #12                           ;; IXL=12, as we want to paint 12 tiles (4-15)
   ld    hl, #bg_tile_offsets + omitted  ;; HL points to the offset of tile 4, the first one to be drawn
   ld    bc, #_g_tileset + omitted       ;; BC points to the sprite of tile 4
   call  drawBackgroundTiles             ;; Draw the 12 tiles of the 3 down rows to erase previous sprite

   ;; Calculate location of the knight at the screen
   ;; (DE already points to the start of video memory buffer)
   ld    hl, #knight_offset         ;; HL holds the offset of the location (0,Knight_Y) with respect to the start of video memory
   add   hl, de                     ;; HL += DE. HL know points to (0,Y) location in the video memory buffer
   ld     a, (knight_x)             ;; A = Knight_X (Column where the knight is located)
   add    l                         ;; | HL += A  (HL += Knight_X)
   ld     l, a                      ;; |    To make HL point to (X,Y) location in the video memory buffer
   adc    h                         ;; |
   sub    l                         ;; |
   ld     h, a                      ;; |
   ex    de, hl                     ;; DE Points to (X,Y) location in the video memory buffer, where Knight will be drawn
   ld    hl, #_g_spr_knight         ;; HL Points to the sprite of the knight with interlaced mask
   ld    bc, #knight_WxH            ;; BC Holds dimensions of the knight (HxW)
   call  cpct_drawSpriteMasked_asm  ;; Draw the sprite of the knight in the video memory buffer

   ret         ;; Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: moveKnight
;;    Moves the knight till the end of the screen, makes it turn, returns back
;; and repeats
;; 
;; DESTROYS:
;;    AF, BC, HL
;;
moveKnight::
   ld     a, (knight_dir)     ;; A = Direction towards the knight is looking at (0: right, 1: left)
   dec    a                   ;; A-- (to check which one is the actual direction)
   ld     a, (knight_x)       ;; A = Knight_X (Present column of the knight, that must be updated)
   jr     z, move_left        ;; If Zero, then Knight_dir was 1, so it is looking to the left (jump)
                              ;; ... else it is looking to the right (continue)
move_right:
   inc    a                        ;; A++, to move knight to the right
   ld (knight_x), a                ;; Store new location of the knight
   cp #screen_Width - knight_Width ;; Check if the Knight has arrived to the right border of the screen
   jr     z, turn_around           ;; If Zero, night has arrived to the right border, jump to turn_around section
   ret                             ;; Else, nothing more to do, so return.

move_left:
   dec    a                        ;; A--, to move knight to the left
   ld (knight_x), a                ;; Store new location of the knight
   or     a                        ;; Check present value of A to know if it is 0 or not
   ret   nz                        ;; If A wasn't 0, left limit has not been reached by the knight, so return
                                   ;; Else (A=0), knight is at left limit, so continue to turn it around

turn_around:
   ld     a, (knight_dir)          ;; A=Direction towards the knight is looking at (0: right, 1:left)
   xor   #1                        ;; Change direction by altering the Least Significant Bit (0->1, 1->0)
   ld  (knight_dir), a             ;; Store new direction in knight_dir variable
   ld    bc, #knight_WxH           ;; BC=Dimensions of the knight sprite
   ld    hl, #_g_spr_knight        ;; HL=Pointer to the start of the knight sprite
   call  cpct_hflipSpriteMaskedM1_asm ;; Horizontally flip the knight sprite, along with its mask

   ret      ;; Return


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main:: 
   ;; First of all, move Stack Pointer (SP) out of the video memory buffers we want
   ;; to use. By default, it is placed at 0xC000 and stack grows backwards. As we
   ;; want to use 0x8000-0xBFFF and 0xC000-0xFFFF as video memory buffers, stack must
   ;; be moved to other place. We will place it at 0x8000, knowing that it grows backwards.
   ld    sp, #0x8000           ;; Move stack pointer to 0x8000, outside video memory buffers

   ;; Initialize the CPC (Stack initialization cannot be inside this function, 
   ;; as return address for a call is stored in the stack and changing stack
   ;; location inside the function will make us return to a random place on RET)
   call  initialize            ;; Call to CPC initialization function

   ;; Draw first tile row in the main video memory buffer (0xC000-0xFFFF)
   ;; (We don't need to draw the other 3 rows, as they will be drawn by redrawKnight function)
   ld__ixl #4                  ;; IXL will act as counter for the number of tiles
   ld    hl, #bg_tile_offsets  ;; HL points to the start of the memory offsets for tiles
   ld    bc, #_g_tileset       ;; BC points to the start of the tileset
   ld    de, #pvideomem        ;; DE points to the start of video memory, where Background should be drawn
   call  drawBackgroundTiles   ;; Draw the background

   ;; Draw first tile row in the secondary video memory buffer (0x8000-0xBFFF)
   ld__ixl #4                  ;; IXL will act as counter for the number of tiles
   ld    hl, #bg_tile_offsets  ;; HL points to the start of the memory offsets for tiles
   ld    bc, #_g_tileset       ;; BC points to the start of the tileset
   ld    de, #pbackbuffer      ;; DE points to the start of video memory, where Background should be drawn
   call  drawBackgroundTiles   ;; Draw the background

loop:
   ;; Redraw the Knight, but do it in the screen back buffer. This way, we prevent flickering
   ;; due to taking too much time drawing the knight. As it is drawn outside present video memory,
   ;; screen will not change a single bit while this drawing takes place
   ld    a, (screen_buffer)   ;; A=Most significant Byte of the video memory back buffer
   ld    d, a                 ;; | Make DE Point to video memory back buffer
   ld    e, #0                ;; |  D = MSB, E = 0, so DE = 0xC000 or 0x8000
   call  redrawKnight         ;; Draw the knight at its concrete offset respect to video memory backbuffer

   ;; After drawing the Knight in the back buffer, we switch both buffers rapidly
   ;; And the new location of the Knight will be shown in the screen, without flickering
   call  switch_screen_buffer ;; Switch buffers after drawing the knight. 

   ;; Update Knight's location for next iteration of the look
   call  moveKnight           ;; move the knight

   jr    loop                 ;; Repeat forever
