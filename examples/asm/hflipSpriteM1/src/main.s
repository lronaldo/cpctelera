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
screen_W      = 80             ;; Width of the screen in bytes
palete_size   = 4              ;; Number of total palette colours
border_colour = 0x0010         ;; 0x10 (Border ID), 0x00 (Colour to set: White).
sprite_HxW    = 0x1805         ;; Height (24, 0x18) and Width (5, 0x05) of the sprite in bytes.
sprite_end_x  = 75             ;; x coordinate where sprite will bounce to the left
look_left     = 0x01           ;; Looking Left
look_right    = 0x00           ;; Looking right

;;===============================================================================
;; DATA SECTION
;;===============================================================================

.area _DATA

;; ASCII zero-terminated String to be printed at the top of the screen
;;
str_demo:: .asciz "[ MODE 1 ] Sprite Flipping Demo"
str_len      = . - str_demo  ;; Length of the string
str_colour   = 0x0001	     ;; Pen 1, Paper 0 (Black over background)

;; String location at the centre of the first character line in the screen
;; pvideomem is (0, 0) location and we have to add to it half the bytes
;; that will be left after printing the string
str_location = pvideomem + (screen_W - str_len*2)/2 

;; Sprites and palette are defined in an external file. As they are
;; defined in C language, their symbols will be preceded by an underscore.
;; We declare sprite symbols here as global, and the linker will look
;; for them on the other file.
.globl _g_spr_monsters_0
.globl _g_spr_monsters_1
.globl _g_spr_palette

;; Monster Sprites (20x24 pixels each)
;;  (This sprites are a modification from Mini Knight Expansion 1, by Master484
;;   got from OpenGameArt: http://opengameart.org/content/mini-knight-expansion-1-0
;;   with Public Domain License CC0: http://creativecommons.org/publicdomain/zero/1.0/)
;;
;;   Each sprite in this structure is encoded as follows (3 Bytes per sprite):
;;     1 Byte  - Direction towards the sprite is looking (1=Left, 0=Right)
;;     2 Bytes - Pointer to the sprite
;; 
g_sprites::
   ;; Sprite 0 (monster 0)
   .db look_right        ;; Direction towards the sprite is looking at
   .dw _g_spr_monsters_0 ;; Pointer to the sprite
   ;; Sprite 1 (monster 0)
   .db look_right        ;; Direction towards the sprite is looking at
   .dw _g_spr_monsters_1 ;; Pointer to the sprite

;; Moving entities. 8 moving entities on the screen,
;;  each one having next structure (2 Bytes per entity):
;;   - 1 Byte - X Horizontal coordinate
;;   - 1 Byte - look_at value (1=Left,0=Right)
;;
g_mentities::
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
;.globl cpct_hflipSpriteM1_r_asm ;; Alternative ROM-friendly version
.globl cpct_drawSprite_asm
.globl cpct_drawStringM1_f_asm
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
   ld    hl, #_g_spr_palette        ;; HL = pointer to the start of the palette array
   ld    de, #palete_size           ;; DE = Size of the palette array (num of colours)
   call  cpct_setPalette_asm        ;; Set the new palette

   ;; Change border colour
   ld    hl, #border_colour         ;; L=Border colour value, H=Palette Colour to be set (Border=16)
   call  cpct_setPALColour_asm      ;; Set the border (colour 16)

   ;; Draw upper string             
   ld    hl, #str_demo              ;; HL points to the string with the demo message
   ld    bc, #str_colour            ;; BC = fg/bg colours used to draw the string
   ld    de, #str_location          ;; DE points to the place in video memory where the string will be drawn
   call  cpct_drawStringM1_f_asm    ;; Draw the string (fast method)

   ret                              ;; return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: drawEntity
;;    Draws an entity on the screen 
;; INPUT:
;;    B: y pixel coordinate where to draw the sprite
;;    C: x pixel coordinate where to draw the sprite
;;    D: Current entity "looking_at"
;;   HL: Pointer to sprite structure
;; DESTROYS:
;;    AF, BC, DE, HL
;;
drawEntity::
   ;; Check if sprite has to be flipped or not
   ld    a, (hl)                 ;; A = direction where the entity is currently looking at
   cp    d                       ;; Check against where the sprite is looking

   inc  hl                       ;; | DE = Pointer to the sprite
   ld    e, (hl)                 ;; |
   inc  hl                       ;; |
   ld    d, (hl)                 ;; |
   push de                       ;; Save pointer to the sprite in the stack for later use

   ;; This Conditional jump uses result from previous "CP E" instruction
   jr    z, looking_good         ;; If Z, sprite is looking to the right direction, nothing to do

   ;; Flip the sprite because it is looking opposite
   push  bc                      ;; save x, y coordinates passed as parameters
   xor   #0x01                   ;; Switch looking direction (0->1, or 1->0)
   dec   hl                      ;; | HL -= 2, to make it point again to the sprite looking_at value
   dec   hl                      ;; |
   ld  (hl),a                    ;; Save new looking_at direction

   ;; Flip the sprite
   ld    bc, #sprite_HxW         ;; B = Sprite Height, C = Width
   ex    de, hl                  ;; HL points to the sprite (DE was pointing to it)
   call  cpct_hflipSpriteM1_asm   ;; Flip the sprite

   ;; Sprite could also be flipped using ROM-friendly version, using this code
   ;; (.globl cpct_hflipSpriteM1_r_asm must be added)
   ;ld    hl, #sprite_HxW         ;; H = Sprite Height, L = Width
   ;call  cpct_hflipSpriteM1_r_asm;; Flip the sprite

   pop   bc                      ;; Recover coordinates to draw the sprite

looking_good:
   ;; Calculate the memory location where the sprite will be drawn
   ld    de, #pvideomem          ;; DE points to the start of video memory
   call  cpct_getScreenPtr_asm   ;; Return pointer to byte located at (x, y) (C, B) in HL
   ex    de, hl                  ;; DE = pointer to video memory location to draw the sprite

   ;; Draw the sprite 
   ;; - DE already points to video memory location where sprite will be drawn
   pop   hl                      ;; HL points to the sprite (Recover from the stack)
   ld    bc, #sprite_HxW         ;; BC = Sprite WidthxHeight
   call  cpct_drawSprite_asm     ;; Draw the sprite on the screen

   ret                           ;; Return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; FUNC: moveNextEntity
;;    Picks up the next entity and moves it 1 step to its looking_at direction 
;; DESTROYS:
;;    AF, BC, DE, HL
;;
lastMovedEntity: .db 7  ;; Holds the ID of the last entity that has been moved
moveNextEntity::
   ld    hl, #lastMovedEntity    ;; HL points to the ID value of the last entity moved
   ld     a, (hl)                ;; A = ID of the last entity moved
   inc    a                      ;; A++ (A = Next entity to be moved)
   and #0x07                     ;; A %= 8 (If A > 7 then A = 0)
   ld   (hl), a                  ;; Store A as last moved entity (Update variable value)

   ;; Select the sprite type for this entity (odd entities = Sprite 1, even = Sprite 0)
   ld     e, a                   ;; E = A (Save A value for later use)
   ld    hl, #g_sprites          ;; HL points to the vector of sprite types
   rrca                          ;; Move Least Significant bit of A to the Carry to check if its is ODD or Even
   jr    nc, even                ;; If Carry=0, LSB of A was 0, so A was even: HL already points to sprite 0

   ;; Odd sprite type: HL must point to sprite 1, 3 bytes away
   ld    bc, #3                  ;; BC = 3
   add   hl, bc                  ;; HL += 3 (HL now points to sprite 1)

even:
   push  hl                      ;; Save HL pointing to the sprite type for later use

   ;; Point to the Entity selected 
   sla    e                      ;; E *= 2 ( Entity Index (0-7) * 2)
   ld     d, #0                  ;; D = 0 so that DE = E = Entity Index * 2
   ld    hl, #g_mentities        ;; HL points to the start of entities vector
   add   hl, de                  ;; HL += DE, HL points to the concrete entity to be updated

   ;; Update entity information
   ld     c, (hl)                ;; C = X coordinate of the entity
   inc   hl                      ;; HL Points to the looking_at value for this entity
   ld     d, (hl)                ;; D = Looking at value of the entity
   ld     a, d                   ;; | Check if the entity is looking right
   cp    #look_right             ;; | 
   jr     z, ent_look_right      ;; If Z, B was looking right

   ;; Entity looking left
   dec    c                      ;; Move entity to the left 1 byte
   jr     nz, location_updated   ;; If C != 0, we haven't reached left limit

   ;; left limit reached
   dec    d                      ;; D = 0 (Look right)
   ld  (hl), d                   ;; Make entity look right
   jr     location_updated       ;; Finished moving, update location and continue

ent_look_right:
   ;; Entity looking right
   inc    c                      ;; Move entity to the right 1 byte
   ld     a, #sprite_end_x       ;; | Check against sprite end x
   cp     c                      ;; |
   jr    nz, location_updated    ;; If B != sprite_end_x, we haven't reached right limit

   ;; Right limit reached
   inc    d                      ;; D = 1 (looking left)
   ld  (hl), d                   ;; Make entity look left

location_updated:
   dec   hl                      ;; Make HL point again to Entity Location
   ld  (hl), c                   ;; Update entity location

   ;; Calculate Y location for the entity, given its ID, using this formulat
   ;; Y = 24*EntityID + 8
   xor    a                      ;; A = 0 and clear Carry flag
   ld     a, e                   ;; A =  2*EntityID (0-7 * 2)
   rla                           ;; A =  4*EntityID
   rla                           ;; A =  8*EntityID
   ld     e, a                   ;; E =  8*EntityID
   rla                           ;; A = 16*EntityID
   add    e                      ;; A = 16*EntityID + 8*EntityID = 24*EntityID
   add   #8                      ;; A = 24*EntityID + 8
   ld     b, a                   ;; B = A (Y location of the entity)

   ;; Draw entity (HL=Sprite structure, D=Entity looking_at, B=Y Coordinate, C=X Coordinate)
   pop   hl                      ;; HL points to the sprite structure
   call drawEntity               ;; Draw the updated entity

   ret                           ;; Nothing more to do, return.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;
_main:: 
   call  initialize           ;; Initialize the CPC

loop:
   call  cpct_waitVSYNC_asm ;; Synchronize with VSYNC

   ;; Move 4 entities in a ROW
   call  moveNextEntity     ;; Moves next Entity
   call  moveNextEntity     ;; Moves next Entity
   call  moveNextEntity     ;; Moves next Entity
   call  moveNextEntity     ;; Moves next Entity

   jr    loop               ;; Repeat forever
