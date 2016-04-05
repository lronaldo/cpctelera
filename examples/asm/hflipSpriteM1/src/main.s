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
screen_H      = 200            ;; Height of the screen in pixels
screen_W      = 80             ;; Width of the screen in bytes
palete_size   = 16             ;; Number of total palette colours
border_colour = 0x1F10         ;; 0x10 (Border ID), 0x1F (Colour to set, Pen 0 from palette). Ready to be loaded into HL
sprite_HxW    = 0x0804         ;; Height (8, 0x08) and Width (4, 0x04) of the sprite in bytes.
sprite_end_x  = 76             ;; x coordinate where sprite will bounce to the left
look_left     = 0x01           ;; Looking Left
look_right    = 0x00           ;; Looking right

;;===============================================================================
;; DATA SECTION
;;===============================================================================

.area _DATA

;; Ascii zero-terminated strings
str_demo: .asciz "Mode 1 Flipping Demo"
str_colour  = 0x0003	   ;; Pen 3, Paper 0, as colours to draw the string

;; Palette (16 bytes)
g_palette: 
.db 0x1F, 0x14, 0x18, 0x05  ; (14,  0,  4,  7) || Pastel Blue, Black, Magenta, Purple
.db 0x16, 0x1C, 0x06, 0x1E  ; ( 9,  3, 10, 12) || Green, Red, Cyan, Yellow
.db 0x00, 0x0E, 0x07, 0x0F  ; (13, 15, 16, 17) || White, Orange, Pink, Pastel Magenta
.db 0x19, 0x0A, 0x03, 0x0B  ; (22, 24, 25, 26) || Pastel Green, Bright Yellow, Pastel Yellow, Bright White

;; Sample Character (12x8 px, 3x8 mode1 bytes)
g_sprite_looking_at: 
   .db look_left        ;; Stores a value to indicate the direction where the sprite is looking at
g_sprite:
   .db 0x00, 0x11, 0x22, 0x33
   .db 0x00, 0x33, 0x44, 0x31
   .db 0x00, 0x22, 0x55, 0x11
   .db 0x00, 0x33, 0x12, 0x55
   .db 0x00, 0x11, 0x22, 0x33
   .db 0x00, 0x33, 0x44, 0x31
   .db 0x00, 0x22, 0x55, 0x11
   .db 0x00, 0x33, 0x12, 0x55

;; Moving entities. 8 moving entities on the screen,
;;  each one having next structure:
;;   - x (1B): Horizontal coordinate
;;   - lookat (1B): 1 = Look left, 0 = look right 
g_mentities:
   .db   0, look_right   ;; Entity 0
   .db  10, look_right   ;; Entity 1
   .db  25, look_right   ;; Entity 2
   .db  40, look_right   ;; Entity 3
   .db  50, look_left    ;; Entity 4
   .db  35, look_left    ;; Entity 5
   .db  20, look_left    ;; Entity 6
   .db   5, look_left    ;; Entity 7

;;===============================================================================
;; CODE SECTION
;;===============================================================================

.area _CODE

;; Symbols with the names of the CPCtelera functions we want to use
;; must be declared globl to be recognized by the compiler. Later on,
;; linker will do its job and make the calls go to function code.
.globl cpct_disableFirmware_asm
.globl cpct_setVideoMode_asm
.globl cpct_setPalette_asm
.globl cpct_setPALColour_asm
.globl cpct_getScreenPtr_asm
.globl cpct_hflipSpriteM1_asm
.globl cpct_drawSprite_asm
.globl cpct_drawStringM1_f_asm
.globl cpct_getRandom_xsp40_u8_asm

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

   ;; Set Mode 1
   ld    c, #0                      ;; C = 1 (New video mode)
   call  cpct_setVideoMode_asm      ;; Set Mode 1
   
   ;; Set Palette
   ld    hl, #g_palette             ;; HL = pointer to the start of the palette array
   ld    de, #palete_size           ;; DE = Size of the palette array (num of colours)
   call  cpct_setPalette_asm        ;; Set the new palette

   ;; Change border colour
   ld    hl, #border_colour         ;; L=Border colour value, H=Palette Colour to be set (Border=16)
   call  cpct_setPALColour_asm      ;; Set the border (colour 16)

   ;; Draw upper string             
   ld    hl, #str_demo              ;; HL points to the string with the demo message
   ld    bc, #str_colour            ;; BC = fg/bg colours used to draw the string
   call  cpct_drawStringM1_f_asm    ;; Draw the string (fast method)

   ret                              ;; return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: drawEntity
;;    Draws an entity on the screen 
;; INPUT:
;;    B: y pixel coordinate where to draw the sprite
;;    C: x pixel coordinate where to draw the sprite
;;    L: Current sprite "looking_at"
;; DESTROYS:
;;    AF, BC, DE, HL
;;
drawEntity:
   ;; Check if sprite has to be flipped or not
   push  bc                      ;; save x,y coordinates passed as parameters
   ld    a, (g_sprite_looking_at);; A=direction where the sprite is currently looking at
   cp    l                       ;; Check against where it should be looking
   jr    z, looking_good         ;; If Z, sprite is "looking good", nothing to do

   ;; Flip the sprite because it is looking oposite
   xor   #0x01                   ;; Switch looking direction (0->1, or 1->0)
   ld    (g_sprite_looking_at),a ;; Save new looking direction
   ld    hl, #sprite_HxW         ;; Sprite WidthxHeight (DE already points to the sprite)   
   call  cpct_hflipSpriteM1_asm  ;; Flip the sprite

looking_good:
   ;; Calculate the memory location where the sprite will be drawn
   ld    de, #pvideomem          ;; DE points to the start of video memory
   call  cpct_getScreenPtr_asm   ;; Return pointer to byte located at (x,y) (C, B) in HL
   ex    de, hl                  ;; DE = pointer to video memory location to draw the sprite

   ;; Draw the sprite 
   ;; - DE already points to video mem
   pop   bc                      ;; Recover coordinates to draw the sprite
   ld    hl, #g_sprite           ;; HL points to the sprite
   call  cpct_drawSprite_asm     ;; Draw the sprite on the screen

   ret                           ;; Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: moveRandomSprite
;;    Picks up a random sprite and moves it 1 step to its looking_at direction 
;; DESTROYS:
;;    AF, BC, DE, HL
;;
moveRandomSprite:
   ;; Get a Random number from 0 to 7 (to select a random entity)
   call cpct_getRandom_xsp40_u8_asm  ;; Get a pseudo-random 8-bits value in L
   ld     a, l                   ;; A = random number
   xor #0x03                     ;; A %= 8, to get a random number from 0 to 7

   ;; Point to the Entity selected 
   rla                           ;; A *= 2, as each element in the entities vector is sized 2 bytes
   ld     d, a                   ;; D = A*2 (Saved to be used later on)
   ld    hl, #g_mentities        ;; HL points to the start of entities vector
   add    l                      ;; | HL += A
   ld     l, a                   ;; |
   adc    h                      ;; |
   sub    l                      ;; |
   ld     h, a                   ;; | --> Now HL Points to HL[A], the concrete entity selected

   ;; Update entity information
   ld     c, (hl)                ;; C = X coordinate of the entity
   inc   hl                      ;; HL Points to the looking_at value for this entity
   ld     b, (hl)                ;; B = Looking at value of the entity
   cp    #look_right             ;; Check if the entity is looking right
   jr     z, ent_look_right      ;; If Z, B was looking right

   ;; Entity looking left
   dec    c                      ;; Move entity to the left 1 byte
   jr     nz, location_updated   ;; If C != 0, we haven't reached left limit

   ;; left limit reached
   ld     a, #look_right         ;; A = 1 (looking right)
   ld  (hl), a                   ;; Make entity look right
   jr     location_updated

ent_look_right:
   ;; Entity looking right
   inc    c                      ;; Move entity to the right 1 byte
   ld     a, #sprite_end_x       ;; | Check against sprite end x
   cp     c                      ;; |
   jr    nz, location_updated    ;; If B != sprite_end_x, we haven't reached right limit

   ;; Right limit reached
   xor    a                      ;; A = 0 (looking left)
   ld  (hl), a                   ;; Make entity look left

location_updated:
   ld  (hl), c                   ;; Update entity location

   ld     l, b                   ;; Set L=Looking at, to pass it as parameter for drawEntity

   ;; Calculate Y location for the entity, given its ID
   xor    a                      ;; A = 0 and clear Carry flag
   ld     a, d                   ;; A = EntityID * 2 (0-7 * 2)
   rla                           ;; A = EntityID * 4
   rla                           ;; A = EntityID * 8
   rla                           ;; A = EntityID * 16
   add  #24                      ;; A = 24 + EntityID * 16
   ld     b, a                   ;; B = A (Y location of the entity)

   ;; Draw entity (L=Looking At, B=Y Coordinate, C=X Coordinate)
   call drawEntity               ;; Draw the updated entity

   ret                           ;; Nothing more to do, return.


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main:: 
   call  init                 ;; Initialize the CPC

loop:
   call  moveRandomSprite     ;; Moves a Random Sprite

   jr    loop                 ;; Repeat forever
