;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_drawMaskedSprite

;
;########################################################################
;### FUNCTION: cpct_drawMaskedSprite                                  ###
;########################################################################
;### This function copies a generic NxM bytes sprite from memory to a ###
;### video-memory location (either present video-memory or hardware   ###
;### backbuffer). The original sprite must be stored as an array (i.e.###
;### with all of its pixels stored as consecutive bytes in memory).   ###
;### Moreover, the sprite must contain mask information in the format ###
;### [BCBC...BC], being C a byte with color information and B a byte  ###
;### with mask information related to next C-byte.                    ###
;########################################################################
;### INPUTS (6 Bytes)                                                 ###
;###  * (2B HL) Source Sprite Pointer (array with pixel and mask data)###
;###  * (2B DE) Destination video memory pointer                      ###
;###  * (1B B) Sprite Height in bytes                                 ###
;###  * (1B C) Sprite Width in bytes (without counting mask bytes)    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 59 bytes                                                 ###
;### TIME:                  (W=width, H=height)                       ###
;###  - Best  Case: 152 + 59(W)(H) + 54(H-1) + 40(|H/8|-1)            ###
;###  - Worst Case: (Best Case) + 40                                  ###
;###  ** EXAMPLES **                                                  ###
;###   - 2x16 bytes sprite = 2888 / 2928 cycles ( 722 /  732 us)      ###
;###   - 4x32 bytes sprite = 9416 / 9456 cycles (2354 / 2364 us)      ###
;########################################################################
;
.globl _cpct_drawMaskedSprite
_cpct_drawMaskedSprite::
   ;; GET Parameters from the stack (Pop is fastest way)
   LD (dms_restoreSP+1), SP   ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                         ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                    ;; [10] AF = Return Address
   POP  HL                    ;; [10] HL = Source Address (Sprite data array)
   POP  DE                    ;; [10] DE = Destination address (Video memory location)
   POP  BC                    ;; [10] BC = Height/Width (B = Height, C = Width)
dms_restoreSP:
   LD SP, #0                  ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                         ;; [ 4] Enable interrupts again

   PUSH IX                    ;; [15] Save IX regiter before using it as temporal var
   .DW #0x69DD                ;; [ 8] LD IXL, C ; Save Sprite Width into IXL for later use

dms_sprite_height_loop:
   PUSH DE                    ;; [11] Save DE for later use (jump to next screen line)

dms_sprite_width_loop:
   LD A, (DE)                 ;; [ 7] Get next background byte into A
   AND (HL)                   ;; [ 7] Erase background part that is to be overwritten (Mask step 1)
   INC HL                     ;; [ 6] HL += 1 => Point HL to Sprite Colour information
   OR (HL)                    ;; [ 7] Add up background and sprite information in one byte (Mask step 2)
   LD (DE), A                 ;; [ 6] Save modified background + sprite data information into memory
   INC DE                     ;; [ 6] Next bytes (sprite and memory)
   INC HL                     ;; [ 6] 

   DEC C                      ;; [ 4] C holds sprite width, we decrease it to count pixels in this line.
   JP NZ,dms_sprite_width_loop;; [10] While not 0, we are still painting this sprite line 
                              ;;      - When 0, we have to jump to next pixel line

   POP DE                     ;; [10] Recover DE from stack. We use it to calculate start of next pixel line on screen

   DEC B                      ;; [ 4] B holds sprite height. We decrease it to count another pixel line finished
   JP Z, dms_sprite_copy_ended;; [10] If 0, we have finished the last sprite line.
                              ;;      - If not 0, we have to move pointers to the next pixel line

   .DW #0x4DDD                ;; [ 8] LD C, IXL ; Restore Sprite Width into C

   LD  A, D                   ;; [ 4] Start of next pixel line normally is 0x0800 bytes away.
   ADD #0x08                  ;; [ 7]    so we add it to DE (just by adding 0x08 to D)
   LD  D, A                   ;; [ 4]
   AND #0x38                  ;; [ 7] We check if we have crossed memory boundary (every 8 pixel lines)
   JP NZ, dms_sprite_height_loop ;;   by checking the 4 bits that identify present memory line. If 0, we have crossed boundaries
dms_sprite_8bit_boundary_crossed:
   LD  A, E                   ;; [ 4] DE = DE + C050h
   ADD #0x50                  ;; [ 7]   -- Relocate DE pointer to the start of the next pixel line:
   LD  E, A                   ;; [ 4]   -- DE is moved forward 3 memory banks plus 50 bytes (4000h * 3) 
   LD  A, D                   ;; [ 4]   -- which effectively is the same as moving it 1 bank backwards and then
   ADC #0xC0                  ;; [ 7]   -- 50 bytes forwards (which is what we want to move it to the next pixel line)
   LD  D, A                   ;; [ 4]   -- Calculations are made with 8 bit arithmetic as it is faster than other alternaives here

   JP  dms_sprite_height_loop ;; [10] Jump to continue with next pixel line

dms_sprite_copy_ended:
   POP IX                     ;; [14] Restore IX before returning
   RET                        ;; [11] Return to caller
