;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud BOUCHE
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_getScreenToSprite
;;
;;    Copies sprite data from screen video memory to a linear array (a sprite)
;;
;; C Definition:
;;    void <cpct_getScreenToSprite> (void* *memory*, void* *sprite*, <u8> *width*, <u8> *height*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;  (2B HL) memory - Source Screen Address (Video memory location)
;;  (2B DE) sprite - Destination Sprite Address (Sprite data array)
;;  (1B C ) width  - Sprite Width in *bytes* (>0) (Beware, *not* in pixels!)
;;  (1B B ) height - Sprite Height in bytes (>0)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_getScreenToSprite_asm
;;
;; Parameter Restrictions:
;;  * *memory* could be any place in memory, inside or outside current video memory. It
;; will be equally treated as video memory (taking into account CPC's video memory 
;; disposition). This lets you copy software or hardware backbuffers, and
;; not only video memory.
;;  * *sprite* must be a pointer to the start of a linear array that will be filled
;; up with sprite pixel data got from *memory*.
;;  * *width* must be the width of the screen to capture *in bytes*, and 
;; must be greater than 0. A 0 as *width* parameter will be considered as 65536,  
;; making this function overwrite the whole memory, making your program crash.
;;  * *height* must be the height of the sprite in bytes, and must be greater than 0. 
;; A 0 as *height* parameter will be considered as 256 (the maximum value). 
;; Height of a sprite in bytes and pixels is the same value, as bytes only group 
;; consecutive pixels in the horizontal space.
;;
;; Known limitations:
;;    * This function does not do any kind of boundary check or clipping. If you 
;; get data beyond your video memory or screen buffer the sprite will also contains 
;; not video data.
;;    * This function uses self-modifying code, so it cannot be used from ROM.
;;
;; Details:
;;    Reads screen video memory data at *memory* and copies it to a linear array (a *sprite*). 
;; The copy takes into account CPC's video memory disposition, which is comprised of character
;; lines made by 8-pixel lines each. The copy converts this disposition to linear, putting
;; each sprite line contiguous to the previous one in the resulting *sprite* array. After 
;; this copy, *sprite* can be used as any other normal sprite through sprite drawing functions
;; like <cpct_drawSprite>.
;;
;;    Next example shows how to use this function,
;; (start code)
;;    // All enemies will be of the same size
;;    #define  ENEMY_WIDTH     6
;;    #define  ENEMY_HEIGHT   10
;;
;;    // Struct Enemy
;;    //    Contains all enemy information: position, velocity, sprite and background data buffer.
;;    //    The background data buffer will contain the background screen pixel data behind the 
;;    // enemy sprite. This will be used to erase the enemy sprite restoring the background.
;;    typedef struct Enemy {
;;       u8    x, y;       // Enemy location
;;       u8    vx, vy;     // Enemy velocity
;;       u8*   sprite;     // Enemy sprite
;;       u8    background[ ENEMY_WIDTH * ENEMY_HEIGHT ]; // Background pixel data
;;    } Enemy;
;;
;;    //////////////////////////////////////////////////////////////////////////////////////////
;;    // Moves an Enemy and redraws it afterwards
;;    //
;;    void MoveAndRedrawEnemy(Enemy* enemy) {
;;       u8* pvmem; // Temporal pointer to video memory for drawing sprites (will be used later)
;; 
;;       // --- Calculations previous to moving the enemy ---
;;
;;       // Get a pointer to the start of the Enemy sprite in video memory
;;       // previous to moving. Background will need to be restored at this 
;;       // precise location to erase the Enemy before drawing it in its next location
;;       u8* pvmem_enemyBg = cpct_getScreenPtr(CPCT_VMEM_START, enemy->x, enemy->y);
;;    
;;       // Move the enemy adding velocity to position
;;       enemy->x += enemy->vx;
;;       enemy->y += enemy->vy;
;;       
;;       // Get a pointer to the Screen Video Memory location where 
;;       // Enemy will be drawn next (in its new location after movement)
;;       pvmem = cpct_getScreenPtr(CPCT_VMEM_START, enemy->x, enemy->y);
;;       
;;       // Wait for VSync and draw background and Enemy
;;       cpct_waitVSYNC();
;;       
;;       //--- ENEMY REDRAWING ---
;;        
;;       // Erase ENEMY at its previous location by drawing background over it
;;       cpct_drawSprite(enemy->background, pvmem_enemyBg, ENEMY_WIDTH, ENEMY_HEIGHT);
;;       
;;       // Before drawing ENEMY at its new location, copy the background there
;;       // to enemy->background buffer. This will let us restore it next time the ENEMY moves.
;;       cpct_getScreenToSprite(pvmem, enemy->background, ENEMY_WIDTH, ENEMY_HEIGHT);
;;       
;;       // Draw ENEMY at its new location
;;       cpct_drawSpriteMasked(enemy->sprite, pvmem, ENEMY_WIDTH, ENEMY_HEIGHT);
;;    }
;; (end code)
;; 
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C-bindings - 39 bytes
;;  ASM-bindings - 34 bytes
;;
;; Time Measures:
;; (start code)
;;  Case      |    microSecs (us)        |    CPU Cycles
;; ----------------------------------------------------------------
;;  Best      |  30 + 8HH + (19 + 6W)H   | 120 + 32HH + (76 + 24W)H
;;  Worst     |  38 + 8HH + (19 + 6W)H   | 152 + 32HH + (76 + 24W)H
;; ----------------------------------------------------------------
;;  W=2,H=16  |        534 /  542        |      2136 / 2168
;;  W=4,H=32  |       1430 / 1438        |      5720 / 5752
;; ----------------------------------------------------------------
;; Asm saving |          -16             |         -64
;; ----------------------------------------------------------------
;; (end code)
;;    W = *width* in bytes, H = *height* in bytes, HH = integer((*H*-1)/8)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ld    a, c                 ;; [1] A = sprite width
   ld    (restore_width), a   ;; [4] Modify sprite with in the code for the loop

   sub   c                    ;; [1] | A = c - c - c = -c 
   sub   c                    ;; [1] |   A contains negative width
   ld    (neg_width), a       ;; [4] Modify negative sprite width in the calculation code

   ld    a, b                 ;; [1] A = sprite height

next_line:
restore_width=.+1
   ld   bc, #0x0000  ;; [3] 0000 is a placeholder for the width of a sprite line
   ldir              ;; [6*W-1] Copy one complete sprite line

   dec   a           ;; [1]   1 less sprite line to end
   ret   z           ;; [2/4] If no lines left, return

   ;; Make HL Point to next line start
neg_width=.+1
   ld    bc, #0x0700 ;; [3] 0700 will be modified to be 07xx, with xx being negative width of the sprite
   add   hl, bc      ;; [3] HL += 0x800 - sprite width

   ;; Check for memory boundaries
   ld     b, a          ;; [1] Save the value of A into B (we need it for next loop iteration)
   ld     a, h          ;; [1] A=H (to test bits 12,13 and 14 of the new video memory address)
   and   #0x38          ;; [2] We get the 3 bits (12,13 and 14) to test if we have crossed 8-line character boundary
   ld     a, b          ;; [1] Restore A value (previously saved into B)
   jr    nz, next_line  ;; [2/3]  If bits 12,13 and 14 are 0, we are at the first line of a character 
                        ;; ....  which means we have crossed character boundaries and memory address will
                        ;; ....  be incorrect. Else, we have not, so we can safely proceed to next_line

_8line_character_boundary_crossed:
   ;; Our address has moved out of the 16K memory bank of video memory. As memory addresses
   ;; are 16-bits long (4 16K banks), in order to make it cycle to the start of the 16K bank 
   ;; we were located, we need to "jump" over the other 3 banks, which means adding 48K (0xC000).
   ;; As we also want to jump to the next character line, which is 0x50 bytes away, we need
   ;; to add 0xC050 in total.
   ld    bc, #0xC050 ;; [3] Value to be added to make HL point to next pixel line in video memory
   add   hl, bc      ;; [3] HL += 0xC050
   jr  next_line     ;; [3] Continue with next pixel line

