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

;; Note: SDCC requires at least _DATA and _CODE areas to be declared.

;;==============================================================================
;; Start of _DATA area
;;==============================================================================
.area _DATA

;;----------------------------------------------------
;; Definition of DATA structures and constants
;;----------------------------------------------------

;; Useful constants
CPCT_VMEM_START = 0xC000

;;
;; Macro: struct Player
;;    Macro that creates a new initialized instance of Player Struct
;; 
;; Parameters:
;;    instanceName: name of the variable that will be created as an instance of Player struct
;;    st:           status of the player
;;    px:           X location of the Player (bytes)
;;    py:           Y location of the Player (pixels)
;;    wi:           Width of the Player Sprite (bytes)
;;    he:           Height of the Player Sprite (bytes)
;;    sprite:       Pointer to the player sprite
;;
.macro definePlayer instanceName, st, px, py, wi, he, sprite
   ;; Struct data
   instanceName:
      instanceName'_status: .db st     ;; Status of the Player
      instanceName'_x_pos:  .db px     ;; X location of the Player (bytes)
      instanceName'_y_pos:  .db py     ;; Y location of the Player (pixels)
      instanceName'_width:  .db wi     ;; Width of the Player Sprite (bytes)
      instanceName'_height: .db he     ;; Height of the Player Sprite (bytes)
      instanceName'_sprite: .dw sprite ;; Pointer to the player sprite
.endm

;;
;; Macro: struct Player offsets
;;
;;    Macro that generates offsets for accessing different elements of 
;; the Player struct (distances from the start of the struct to each
;; struct member). Requires a already defined instance as an example for
;; calculating offsets.
;;
;; Parameters:
;;    stname:        Name of the structure
;;    instanceName:  Name of the instance that will be used to calculate offsets
;;
.macro definePlayerOffsets stname, instanceName
   ;; Offset constants
   stname'_status_off = instanceName'_status - instanceName ;; status offset
   stname'_x_pos_off  = instanceName'_x_pos  - instanceName ;; X offset
   stname'_y_pos_off  = instanceName'_y_pos  - instanceName ;; Y offset
   stname'_width_off  = instanceName'_width  - instanceName ;; Width offset
   stname'_height_off = instanceName'_height - instanceName ;; Height offset
   stname'_sprite_off = instanceName'_sprite - instanceName ;; Sprite offset
.endm

;;------------------------------------------------------------
;; Definition of data elements
;;------------------------------------------------------------

;; RYU and KEN Sprites (Generated and compiled as C files)
.globl _g_sprite_ryu
.globl _g_sprite_ken

;; Definition of Players
definePlayer ryu, 0, 58, 60, 21, 81, _g_sprite_ryu
definePlayer ken, 0,  0, 60, 21, 81, _g_sprite_ken
definePlayerOffsets player, ryu 

;;==============================================================================
;; Start of _CODE area
;;==============================================================================
.area _CODE

;; Symbols with the names of the CPCtelera functions we want to use
;; must be declared globl to be recognized by the compiler. Later on,
;; linker will do its job and make the calls go to function code.
.globl cpct_disableFirmware_asm
.globl cpct_getScreenPtr_asm
.globl cpct_drawSprite_asm
.globl cpct_setVideoMode_asm

;;-----------------------------------------------
;; Draw a player
;;    IX = player struct pointer
;;-----------------------------------------------
drawPlayer:
   ;; Get Screen Pointer
   ld  de, #CPCT_VMEM_START      ;; DE = Pointer to video memory start
   ld  c, player_x_pos_off(ix)   ;; C  = Player X Position
   ld  b, player_y_pos_off(ix)   ;; B  = Player Y Position
   call cpct_getScreenPtr_asm    ;; Get Screen Pointer
   ;; Return value: HL = Screen Pointer to (X, Y) byte

   ;; Draw Sprite
   ex  de, hl                          ;; DE = Pointer to Video Memory (X,Y) location
   ld   h, player_sprite_off + 1(ix)   ;; | HL = Pointer to Player Sprite
   ld   l, player_sprite_off + 0(ix)   ;; |
   ld   c, player_width_off (ix)       ;; C = Player Width (bytes)
   ld   b, player_height_off(ix)       ;; B = Player Height (pixels)
   call cpct_drawSprite_asm            ;; Draw the sprite

   ret

;;-----------------------------------------------
;; MAIN function. This is the entry point of the application.
;;    _main:: global symbol is required for correctly compiling and linking
;;-----------------------------------------------
_main::

   ;; Initialize CPC
   call cpct_disableFirmware_asm ;; Disable Firmware
   ld  c, #0                     ;; C = 0 (Mode 0)
   call cpct_setVideoMode_asm    ;; Set Mode 0

   ;; Draw RYU and KEN
   ld  ix, #ryu                  ;; IX = Pointer to Ryu structure
   call drawPlayer               ;; Draw RYU Player
   ld  ix, #ken                  ;; IX = Pointer to Ken structure
   call drawPlayer               ;; Draw Ken Player

   ;; Infinite waiting loop
forever:
   jp forever
