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

pvideomem     = 0xC000         ;; First byte of video memory
screen_H      = 200            ;; Height of the screen in pixels
screen_W      = 80             ;; Width of the screen in bytes
init_colour   = 0x0001         ;; Background = 0x00, Foreground = 0x01
palete_size   = 16             ;; Number of total palette colours
border_colour = 0x1F10         ;; 0x10 (Border ID), 0x1F (Colour to set, Pen 0 from palette). Ready to be loaded into HL
initxy_sprite = 0xC711         ;; (x, y) = (17, 200) = (0x11, 0xC8) Initial Sprite coordinates in bytes
sprite_HxW    = 0x852D         ;; Height (133, 0x85) and Width (45, 0x2D) of the sprite in bytes. To be loaded in BC   
sprite_end_y  = 20             ;; y coordinate where sprite will stop its entering animation
clip_Height   = screen_H - 133 ;; Minimum height at which to do clipping (200 - sprite_Height)
xy_str_dav    = 0x0212         ;; y = 2 (0x02), x = 18 (0x12) (ready to be loaded into BC, B=y, C=x)  
itmusiccycles = 6              ;; Number of interrupt cycles between music calls

;;===============================================================================
;; DATA SECTION
;;===============================================================================

.area _DATA

;; Interrupt status counter
iscount:  .db itmusiccycles

;; Ascii zero-terminated strings
str_happy: .asciz "Happy"
str_bday:  .asciz "Birthday"
str_dav:   .asciz "@octopusjig"

;; Sprite, Palette and music (defined in their own generated source files)
.globl _g_octopusjig
.globl _g_palette
.globl _g_music

;; Scrolling data structures for both scrolling strings (Happy / BDay)
tscroll_happy: 
   .dw   str_happy       ;; String pointer (2 bytes)
   .db   1, 20*8         ;; x, y starting coordinates (y=20th character line)
   .db  -1               ;; x velocity
   .db   screen_W-5*4-1  ;; maxX coordinate: line-size (80) - 5 chars * 4 bytes/char - 1 (to set boundary 1 byte before)

tscroll_bday: 
   .dw   str_bday        ;; String pointer (2 bytes)
   .db   47, 23*8        ;; x, y starting coordinates (y=23th character line)
   .db    1              ;; x velocity 
   .db   screen_W-8*4-1  ;; maxX coordinate: line-size (80) - 8 chars * 4 bytes/char - 1 (to set boundary 1 byte before)

;;===============================================================================
;; CODE SECTION
;;===============================================================================

.area _CODE

;; Include all CPCtelera macros, including opcode macros that 
;; we will be using later
.include "macros/allmacros.h.s"

;; Symbols with the names of the CPCtelera functions we want to use
;; must be declared globl to be recognized by the compiler. Later on,
;; linker will do its job and make the calls go to function code.
.globl cpct_disableFirmware_asm
.globl cpct_setVideoMode_asm
.globl cpct_setPalette_asm
.globl cpct_setPALColour_asm
.globl cpct_getScreenPtr_asm
.globl cpct_drawSprite_asm
.globl cpct_waitVSYNC_asm
.globl cpct_drawStringM0_asm
.globl cpct_setDrawCharM0_asm
.globl cpct_akp_musicInit_asm
.globl cpct_akp_musicPlay_asm
.globl cpct_setInterruptHandler_asm

interrupt_handler:
   ;; Update interrupt counter variable (iscount)
   ld   hl, #iscount          ;; HL points to interrupt counter variable (iscount)
   dec (hl)                   ;; --iscount
   ret  nz                    ;; Do not play music if iscount != 0 (so, return)

   ;; Play music
   ld    (hl), #itmusiccycles    ;; Restore interrupt counter variable intial value
   call  cpct_akp_musicPlay_asm  ;; Play the music

   ret                           ;; Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: init
;;    Sets CPC to its initial status
;; DESTROYS:
;;    AF, BC, DE, HL
;;
init:
   ;; Disable Firmware
   call  cpct_disableFirmware_asm   ;; Disable firmware

   ;; Set Mode 0
   ld    c, #0                      ;; C = 0 (New video mode)
   call  cpct_setVideoMode_asm      ;; Set Mode 0
   
   ;; Set Palette
   ld    hl, #_g_palette            ;; HL = pointer to the start of the palette array
   ld    de, #palete_size           ;; DE = Size of the palette array (num of colours)
   call  cpct_setPalette_asm        ;; Set the new palette

   ;; Change border colour
   ld    hl, #border_colour         ;; L=Border colour value, H=Palette Colour to be set (Border=16)
   call  cpct_setPALColour_asm      ;; Set the border (colour 16)

   ;; Initialize music
   ld    de, #_g_music              ;; DE points to the start of the song to be initialized
   call  cpct_akp_musicInit_asm     ;; Initalize arkos tracker player with the song pointed by DE

   ;; 
   ld    hl, #interrupt_handler       ;; HL points to the interrupt handler routine
   call  cpct_setInterruptHandler_asm ;; Set the new interrupt handler routine

   ret                              ;; return


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: drawSpriteClipped
;;    Draw sprite with down clippling
;; INPUT:
;;    B: y pixel coordinate where to draw the sprite
;;    C: x pixel coordinate where to draw the sprite
;; DESTROYS:
;;    AF, BC, DE, HL
;;
drawSpriteClipped:
   ;; Calculate the memory location where the sprite will be drawn
   push  bc                      ;; save x,y coordinates passed as parameters
   ld    de, #pvideomem          ;; DE points to the start of video memory
   call  cpct_getScreenPtr_asm   ;; Return pointer to byte located at (x,y) (C, B) in HL
   ex    de, hl                  ;; DE = pointer to video memory location to draw the sprite
   pop   af                      ;; A = y coordinate

   ;; Check if clipping is needed
   ld    bc, #sprite_HxW         ;; WidthxHeight of the sprite in bytes
   cp    #clip_Height            ;; Compare A with clipping height: y coordinate where clipping starts
   jr     c, no_clip             ;; If Carry, (A < 160), no need to do clipping

   ;; Perform clippling (A = vertical size of the sprite to be drawn)
   sub   #screen_H               ;; A2 =  A - screen_Height
   neg                           ;; A3 = -A2 = screen_Height - A
   ld     b, a                   ;; B = Reduced sprite height (clipped) to be drawn

no_clip:
   ;; Draw the sprite 
   ;; - DE already points to video mem, and BC contains WidthxHeight)
   ld    hl, #_g_octopusjig      ;; HL points to the sprite
   call  cpct_drawSprite_asm     ;; Draw the sprite on the screen

   ret                           ;; Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: moveSprite
;;    Moves the sprite from down the screen to its final location in a
;; simple, linear animation.
;; DESTROYS:
;;    AF, BC, DE, HL
;;
moveSprite:
   ;; Draw Sprite After VSYNC
   call  cpct_waitVSYNC_asm   ;; Wait for VSYNC

   y_coord = .+2              ;; Location in memory of the value for the Y coordinate
   ld    bc, #initxy_sprite   ;; BC takes XY coordinates (value loaded is modified dynamically)
   call  drawSpriteClipped    ;; Draw the sprite

   ;; Move Sprite Up
   ld    hl, #y_coord         ;; HL points to y_coord in memory
   ld     a, (hl)             ;; A = y (Vertical coordinate)
   cp    #sprite_end_y        ;; Check against y coordinate where sprite has to end its animation (end_y)
   ret   z                    ;; If y==end_y, end of the animation (no more coordinate update)

   dec   (hl)                 ;; Move sprite 1 pixel UP (y--)

   jr    moveSprite           ;; Loop again


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: redrawString
;;    Draws a string with a new colour value each time, on a given location
;; INPUT:
;;    IY: Pointer to the string
;;    B: y pixel coordinate where to draw the string
;;    C: x pixel coordinate where to draw the string
;; DESTROYS:
;;    AF, BC, DE, HL
;;
redrawString:
   ;; Calculate screen location where to draw the string
   ;; BC Already have screen coordinates for the string to be drawn
   ld    de, #pvideomem          ;; DE points to the start of video memory
   call  cpct_getScreenPtr_asm   ;; Return pointer to byte located at (x,y) (C, B) in HL
   push  hl                      ;; Returns HL = Pointer to video memory (Required by drawStringM0)
                                 ;; We save it for later use

   ;; Set colours to be used by DrawChar/DrawStringM0 functions
   fg_colour = .+1               ;; fg_colour = location in memory of the Foreground colour value
   ld    hl, #init_colour        ;; HL = fg/bg colours (value modified dynamically)
   call  cpct_setDrawCharM0_asm  ;; Set colours before using DrawStringM0

   ;; Draw the string (IY points to the string)
   pop   hl                      ;; HL Points to video memory location where the string will be drawn
   call  cpct_drawStringM0_asm   ;; Draw the string

   ;; Increment colour for next call
   ld     a, (fg_colour)         ;; A = Foreground colour
   inc    a                      ;; A++
   cp     #palete_size           ;; Check against number of palette colours used
   jr     c, dont_reset_a        ;; If Carry (A < palette_size), do nothing
   ld     a, #1                  ;; Else, set A=1 again

dont_reset_a:
   ld    (fg_colour), a          ;; Save Foreground colour for next use

   ret                           ;; Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: doOneScrollStep
;;    Scrolls a complete charater line 1 byte to the left or to the right 
;; (2 mode 0 pixels)
;; INPUT: 
;;    HL: Pointer to the start of the first screen line to be scrolled (it
;; must be a pixel-0 screen line of a character)
;;     A: -1 to scroll the line to the left, other value to scroll to the right
;; DESTROYS:
;;    AF, BC, DE, HL
;;
inc_e  = 0x1C   ;; Opcode for incrementing e
inc_l  = 0x2C   ;; Opcode for incrementing l
_ldir_ = 0xB0ED ;; Opcode for LDIR
_lddr_ = 0xB8ED ;; Opcode for LDDR

doOneScrollStep:
   ;;
   ;; Set up the copy
   ;;
   
   ;; Check type of copy (-1=scroll left, scroll right otherwise)
   inc    a                ;; A++ (To check if A is -1 or not)
   jr     z, scr_left      ;; If A=-1, scroll left

scr_right:                 ;; A is not -1, scroll right
   ld     a, #inc_e        ;; Increment e on scrolling right (so DE = HL + 1)
   ld    bc, #screen_W-2   ;; | Add 78 bytes to point to ...
   add   hl, bc            ;; | ... the byte before the last byte of the line 
   ld    bc, #_lddr_       ;; Copy instruction to use: decrementing copy (HL=>DE, DE--, HL--, BC--)
   jr    start_copy        ;; Start the copy

scr_left:
   ld     a, #inc_l        ;; | Increment l on scrolling left (so HL = DE + 1)
   ld    bc, #_ldir_       ;; Copy instruction to use: incrementing copy (HL=>DE, DE++, HL++, BC--)

start_copy:
   ;; Modify loop instruction that differs between movements and start
   ld   (scr_pcopy), bc    ;; | Modify internal loop code with instructions...
   ld   (scr_pincr), a     ;; | ... required for this type of copy
   
   ;;
   ;; Do the copy
   ;;
   ld     a, #8            ;; 8 pixel lines to be scrolled
loop_scroll:
   push  hl                ;; Save current hl value

   ld     d, h             ;; | DE points to the first byte...
   ld     e, l             ;; | and HL to the next one
   scr_pincr = .           ;; << Memory address of the increment instruction
   inc    l                ;; | (this instruction is modified dynamically: inc l / inc e)

   ld    bc, #screen_W-1   ;; 79 bytes to copy (all bytes in 1 line minus the first one)
   scr_pcopy = .           ;; << Memory address of the copy instruction
   ldir                    ;; do the copy  (This instruction is modified dynamically: ldir / lddr)

   pop   hl                ;; Restore HL value, for calculations

   dec    a                ;; A-- (1 line less to be copied)
   ret    z                ;; If A==0, scroll has ended

   ;; Point to the next line to be scrolled inside this same character (HL += 0x800)
   ld     b, a             ;; B = A (B acts as a backup of A)
   ld     a, h             ;; | HL += 0x800
   add   #8                ;; |
   ld     h, a             ;; |
   ld     a, b             ;; A = B (Restore A value from its backup)

   jr    loop_scroll       ;; Next line

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: moveString
;;    Moves a string 1 byte to the left or to the right, bouncing on the
;; boundaries and then redrawing the string with a new colour
;; INPUT:
;;    IX: Pointer to a 6-bytes structure containing these fields,
;;       2 bytes > Pointer to string to be drawn
;;       1 byte  > x coordinate where the string is
;;       1 byte  > y coordinate where the string is
;;       1 byte  > current movement velocity (-1 or +1)
;;       1 byte  > max x coordinate (where it should bounce on the right side)
;; MODIFIES:
;;       byte 2: x coordinate 
;;       byte 4: movement velocity 
;; DESTROYS:
;;    AF, AF', BC, DE, HL
;;
moveString:
   ;; Update x location
   ld     d, 4(ix)               ;; D  = x velocity (VelX)
   ld     a, 2(ix)               ;; A  = x coordinate
   add    d                      ;; A2 = A + D = x + VelX

   ;; Set B=y coordinate, as this value will be used by both next branches
   ld     b, 3(ix)               ;; B = y coordinate of the string

   ;; Check whether string is touching a boundary or not
   jr     z, boundary_hit        ;; If x == 0, left boundary (1) has been exceeded
   cp  5(ix)                     ;; Check against maxX value
   jr     c, do_string_movement  ;; If x < maxX, right boundary has not been hit

boundary_hit:
   ;; Save in C the x coordinate previous to the actual one (boundary exceeded)
   sub    d                      ;; A = A2 - D = x + VelX - VelX = x
   ld     c, a                   ;; C = x

   ;; Invert velocity (to start scrolling to the other side)
   xor    a                      ;; A = 0
   sub    d                      ;; A = -D = -VelX
   ld 4(ix), a                   ;; Update VelX value
   
   ;; Redraw string with a new colour value
   ;; (BC already contains y and x coordinates)
   ld     a, 0(ix)               ;; | IY = Pointer to the string
   ld__iyl_a                     ;; |
   ld     a, 1(ix)               ;; |
   ld__iyh_a                     ;; |
   call  redrawString            ;; Redraws the string

   ret                           ;; Nothing more to do, return.

do_string_movement:
   ;; Update string's x location
   ld 2(ix), a                   ;; Update x location value

   ;; Save VelX into a'
   ld     a, d                   ;; A = D = VelX
   ex    af, af'                 ;; A' = VelX

   ;; Calculate memory location for y line
   ;; (B already contains y coordinate)
   ld     c, #0                  ;; C = 0 (x coordinate = 0 to get the start of the y line)
   ld    de, #pvideomem          ;; DE points to the start of video memory
   call  cpct_getScreenPtr_asm   ;; Return pointer to byte located at (x,y) (C, B) in HL
   ;; HL now points to the start of the first pixel line where the
   ;; string is located (to be able to scroll it)

   ;; Scroll the string
   ;; (HL already points to the start of the first pixel line to be scrolled)
   ex    af, af'                 ;; A = VelX (recover value from A')
   call  doOneScrollStep         ;; Do scroll to the side determined by VelX

   ret                           ;; Return


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main:: 
   call  init                 ;; Initialize the CPC
   call  moveSprite           ;; Do the complete sprite animation and return when finished

   ;; Draw octopusjig string
   ld    iy, #str_dav         ;; IY points to @octopusjig string
   ld    bc, #xy_str_dav      ;; BC = y, x coordinates where to draw @octopusjig string
   call  redrawString         ;; Draw the string with the initial colour

loop:
   ;; Scroll Happy String
   ld    ix, #tscroll_happy   ;; IX points to the structure with scroll information about "Happy"
   call  cpct_waitVSYNC_asm   ;; Wait for VSYNC before proceeding
   call  moveString           ;; Move "Happy" String

   ;; Scroll BDay String
   ld    ix, #tscroll_bday    ;; IX points to the structure with scroll information about "Birthday"
   call  cpct_waitVSYNC_asm   ;; Wait for VSYNC before proceeding
   call  moveString           ;; Move "Birthday" String

   jr    loop                 ;; Repeat forever
