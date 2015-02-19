;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;#####################################################################
;### MODULE: Sprites                                               ###
;#####################################################################
;### This module contains several functions and routines to manage ###
;### sprites and video memory in an Amstrad CPC environment.       ###
;#####################################################################
;
.module cpct_sprites

;
;########################################################################
;### FUNCTION: cpct_drawSprite2x8_aligned                             ###
;########################################################################
;### Copies a 2x8-bytes sprite from its original memory storage       ###
;### position to a video memory position (taking into account video   ###
;### memory distribution). This function asumes that the destination  ###
;### in the video memory will be the starting line of a character.    ###
;### (First byte line of video memory, where characters from the 25   ###
;###  lines start. First 2000 bytes, C000 to C7D0, in bank 4)         ###
;### It also asumes that the sprite is a solid entity and all of its  ###
;### bytes are stored consecutively: they are copied as they are.     ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Source Sprite Pointer (16-byte vector with pixel data)   ###
;###  * (2B) Destiny aligned video memory start location              ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;###  MEMORY:  26 bytes                                               ###
;###  TIME:   566 cycles (141.50 us)                                  ### 
;########################################################################
;
.globl _cpct_drawSprite2x8_aligned
_cpct_drawSprite2x8_aligned::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                  ;; [10] AF = Return Address
   POP  HL                  ;; [10] HL = Source address
   POP  DE                  ;; [10] DE = Destination address
   PUSH DE                  ;; [11] Leave the stack as it was
   PUSH HL                  ;; [11]
   PUSH AF                  ;; [11]

   ;; Copy 8 lines of 2 bytes width (2x8 = 16 bytes)
   LD   A, D                ;; [ 4] Save D into A for fastly doing +800h increment of DE
   LD   BC, #16             ;; [10] BC = 16, countdown of the number of bytes remaining
   JP   dsa28_first_line    ;; [10] First line does not need to do math to start transfering data. 

dsa28_next_line:
   ;; This 4 lines do "DE += 800h - 2h" to move DE pointer to the next pixel line at video memory
   DEC  E                   ;; [ 4] E -= 2 (We only decrement E because D is saved into A, 
   DEC  E                   ;; [ 4]         hence, it does not matter if E is 00 and becomes FF, cause we do not have to worry about D)
   ADD  #8                  ;; [ 7] D += 8 (To add 800h to DE, we increment previous value of D by 8, and move it into D)
   LD   D, A                ;; [ 4]

dsa28_first_line:
   LDI                      ;; [16] Copy 2 bytes for (HL) to (DE) and decrement BC 
   LDI                      ;; [16]
   JP   PE, dsa28_next_line ;; [10] While BC!=0, continue copying bytes (When BC=0 LDI resets P/V, otherwise, P/V is set)

   RET                      ;; [10]

;
;########################################################################
;### FUNCTION: cpct_drawSprite2x8Fast_aligned                         ###
;########################################################################
;### Does the same as cpct_drawSprite2x8_aligned, but using an unro-  ###
;### lled version of the loop, which is ~27% faster. The only draw-   ###
;### back is that this version requires more memory space to store    ###
;### the code (+173%).                                                ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Source Sprite Pointer (16-byte vector with pixel data)   ###
;###  * (2B) Destiny aligned video memory start location              ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;###  MEMORY:  71 bytes                                               ###
;###  TIME:   449 cycles (113.25 us)                                  ###
;########################################################################
;
.globl _cpct_drawSprite2x8Fast_aligned
_cpct_drawSprite2x8Fast_aligned::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                  ;; [10] AF = Return Address
   POP  HL                  ;; [10] HL = Source address
   POP  DE                  ;; [10] DE = Destination address
   PUSH DE                  ;; [11] Leave the stack as it was
   PUSH HL                  ;; [11]
   PUSH AF                  ;; [11]

   ;; Copy 8 lines of 2 bytes width (2x8 = 16 bytes)
   ;;  (Unrolled version of the loop)
   LD   A, D                ;; [ 4] First, save DE into A and B, 
   LD   B, E                ;; [ 4]   to ease the 800h increment step
   LD   C, #17              ;; [ 7] Ensure that 16 LDIs do not change value of B (as they will decrement BC)

   ;; Sprite Line 1
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 2
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 3
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 4
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 5
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 6
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 7
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]
   ADD  #8                  ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A                ;; [ 4]
   LD   E, B                ;; [ 4]

   ;; Sprite Line 8
   LDI                      ;; [16] Copy line (2 bytes)
   LDI                      ;; [16]

   RET                      ;; [10]

;
;########################################################################
;### FUNCTION: cpct_drawSprite4x8_aligned                             ###
;########################################################################
;### Copies a 4x8-bytes sprite from its original memory storage       ###
;### position to a video memory position (taking into account video   ###
;### memory distribution). This function asumes that the destination  ###
;### in the video memory will be the starting line of a character.    ###
;### (First byte line of video memory, where characters from the 25   ###
;###  lines start. First 2000 bytes, C000 to C7D0, in bank 4)         ###
;### It also asumes that the sprite is a solid entity and all of its  ###
;### bytes are stored consecutively: they are copied as they are.     ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Source Sprite Pointer (32-byte vector with pixel data)   ###
;###  * (2B) Destiny aligned video memory start location              ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  MEMORY:  30 bytes                                               ###
;###  TIME:   927 cycles                                              ###
;########################################################################
;
.globl _cpct_drawSprite4x8_aligned
_cpct_drawSprite4x8_aligned::
   ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                 ;; [10] AF = Return Address
   POP  HL                 ;; [10] HL = Source address
   POP  DE                 ;; [10] DE = Destination address
   PUSH DE                 ;; [11] Leave the stack as it was
   PUSH HL                 ;; [11] 
   PUSH AF                 ;; [11] 

   ;; Copy 8 lines of 4 bytes width
   LD   A, #8              ;; [ 7] We have to draw 8 lines of sprite
   JP   dsa48_first_line   ;; [10] First line does not need to do math to start transfering data. 

dsa48_next_line:
   ;; Move to the start of the next line
   EX   DE, HL             ;; [ 4] Make DE point to the start of the next line of pixels in video memory, by adding BC
   ADD  HL, BC             ;; [11] (We need to interchange HL and DE because DE does not have ADD)
   EX   DE, HL             ;; [ 4]

dsa48_first_line:
   ;; Draw a sprite-line of 4 bytes
   LD   BC, #0x800         ;; [10] 800h bytes is the distance to the start of the first pixel in the next line in video memory (it will be decremented by 1 by each LDI)
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|

   ;; Repeat for all the lines
   DEC  A                  ;; [ 4] A = A - 1 (1 more line completed)
   JP   NZ,dsa48_next_line ;; [10] 

   RET                     ;; [10]

;
;########################################################################
;### FUNCTION: cpct_drawSprite4x8Fast_aligned                         ###
;########################################################################
;### Does the same as cpct_drawSprite4x8_aligned, but using an unro-  ###
;### lled version of the loop, which is (~24%) faster. The only draw- ###
;### back is that this version requires more memory space to store    ###
;### the code (+243%).                                                ###
;########################################################################
;### INPUTS (4 Bytes)                                                 ###
;###  * (2B) Source Sprite Pointer (32-byte vector with pixel data)   ###
;###  * (2B) Destiny aligned video memory start location              ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  MEMORY: 103 bytes                                               ###
;###  TIME:   705 cycles (176.25 us)                                  ###
;########################################################################
;
.globl _cpct_drawSprite4x8Fast_aligned
_cpct_drawSprite4x8Fast_aligned::
   ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                 ;; [10] AF = Return Address
   POP  HL                 ;; [10] HL = Source address
   POP  DE                 ;; [10] DE = Destination address
   PUSH DE                 ;; [11] Leave the stack as it was
   PUSH HL                 ;; [11] 
   PUSH AF                 ;; [11] 

   ;; Copy 8 lines of 4 bytes each (4x8 = 32 bytes)
   ;;  (Unrolled version of the loop)
   LD   A, D               ;; [ 4] First, save DE into A and B, 
   LD   B, E               ;; [ 4]   to ease the 800h increment step
   LD   C, #33             ;; [ 7] Ensure that 32 LDIs do not change value of B (as they will decrement BC)

   ;; Sprite Line 1
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   ADD  #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A               ;; [ 4]
   LD   E, B               ;; [ 4]

   ;; Sprite Line 2
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   ADD  #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A               ;; [ 4]
   LD   E, B               ;; [ 4]

   ;; Sprite Line 3
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   ADD  #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A               ;; [ 4]
   LD   E, B               ;; [ 4]

   ;; Sprite Line 4
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   ADD  #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A               ;; [ 4]
   LD   E, B               ;; [ 4]

   ;; Sprite Line 5
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   ADD  #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A               ;; [ 4]
   LD   E, B               ;; [ 4]

   ;; Sprite Line 6
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   ADD  #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A               ;; [ 4]
   LD   E, B               ;; [ 4]

   ;; Sprite Line 7
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   ADD  #8                 ;; [ 7] DE += 800h (Using previous A, B copy)
   LD   D, A               ;; [ 4]
   LD   E, B               ;; [ 4]

   ;; Sprite Line 8
   LDI                     ;; [16] <|Copy 4 bytes with (DE) <- (HL) and decrement BC 
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|

   RET                     ;; [10]
;
;########################################################################
;### FUNCTION: cpct_drawSprite                                        ###
;########################################################################
;### This function copies a generic NxM bytes sprite from memory to a ###
;### video-memory location (either present video-memory or hardware   ###
;### backbuffer. The original sprite must be stored as an array (i.e. ###
;### with all of its pixels stored as consecutive bytes in memory) It ###
;### only works for solid, rectangular sprites, with 1-63 bytes width ###
;########################################################################
;### INPUTS (6 Bytes)                                                 ###
;###  * (2B HL) Source Sprite Pointer (array with pixel data)         ###
;###  * (2B DE) Destination video memory pointer                      ###
;###  * (1B B) Sprite Height in bytes                                 ###
;###  * (1B C) Sprite Width in bytes (Max.63)                         ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 191 bytes                                                ###
;### TIME:                  (w=width, h=height)                       ###
;###  - Best Case:  76 + (79 + 16w)h + 31(|h/8| - 1) cycles;          ###
;###  - Worst Case: Best Case + 31 cycles                             ###
;###  ** EXAMPLES **                                                  ###
;###   - 2x16 bytes sprite = 1883 / 1914 cycles ( 470.75 /  478.5 us) ###
;###   - 4x32 bytes sprite = 4745 / 4776 cycles (1186.25 / 1194.0 us) ###
;########################################################################
;
.globl _cpct_drawSprite
_cpct_drawSprite::
   ;; GET Parameters from the stack (Pop is fastest way)
   LD (ds_restoreSP+1), SP    ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   DI                         ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP  AF                    ;; [10] AF = Return Address
   POP  HL                    ;; [10] HL = Source Address (Sprite data array)
   POP  DE                    ;; [10] DE = Destination address (Video memory location)
   POP  BC                    ;; [10] BC = Height/Width (B = Height, C = Width)
ds_restoreSP:
   LD SP, #0                  ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   EI                         ;; [ 4] Enable interrupts again

   ;; Modify code using width to jump in drawSpriteWidth
   LD  A, #126                   ;; [ 7] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   SUB C                         ;; [ 4]    to do as much LDIs as bytes the Sprite is wide
   SUB C                         ;; [ 4]
   LD (ds_drawSpriteWidth+#4), A ;; [13] Modify JR data to create the jump we need

   LD   A, B                 ;; [ 4] A = Height (used as counter for the number of lines we have to copy)
   EX   DE, HL               ;; [ 4] Instead of jumping over the next line, we do the inverse operation because it is only 4 cycles and not 10, as a JP would be)

ds_drawSpriteWidth_next:
   ;; NEXT LINE
   EX   DE, HL             ;; [ 4] HL and DE are exchanged every line to do 16bit math with DE. This line reverses it before proceeding to copy the next line.

ds_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes
   LD BC, #0x800           ;; [10] 0x800 bytes is the distance in memory from one pixel line to the next within every 8 pixel lines. Each LDI performed will decrease this by 1, as we progress through memory copying the present line
   .DW #0x0018  ;; JR 0    ;; [12] Self modifying instruction: the '00' will be substituted by the required jump forward. (Note: Writting JR 0 compiles but later it gives odd linking errors)
   LDI                     ;; [16] <| 80 LDIs, which are able to copy up to 80 bytes each time.
   LDI                     ;; [16]  | That means that each Sprite line should be 80 bytes width at most.
   LDI                     ;; [16]  | The JR instruction at the start makes us ingnore the LDIs we dont need (jumping over them)
   LDI                     ;; [16] <| That ensures we will be doing only as much LDIs as bytes our sprit is wide
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |
   LDI                     ;; [16] <|   
   LDI                     ;; [16] <|
   LDI                     ;; [16]  |
   LDI                     ;; [16]  |

   DEC A                   ;; [ 4] Another line finished: we discount it from A
   RET Z                   ;; [11c/5] If that was the last line, we safely return

   ;; Jump destination pointer to the start of the next line in video memory
   EX  DE, HL              ;; [ 4] DE has destination, but we have to exchange it with HL to be able to do 16bit math
   ADD HL, BC              ;; [11] We add 0x800 minus the width of the sprite (BC) to destination pointer 
   LD  B, A                ;; [ 4] Save A into B (B = A)
   LD  A, H                ;; [ 4] We check if we have crossed video memory boundaries (which will happen every 8 lines). If that happens, bits 13,12 and 11 of destination pointer will be 0
   AND #0x38               ;; [ 7] leave out only bits 13,12 and 11
   LD  A, B                ;; [ 4] Restore A from B (A = B)
   JP NZ, ds_drawSpriteWidth_next ;; [10] If that does not leave as with 0, we are still inside video memory boundaries, so proceed with next line

   ;; Every 8 lines, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means our next line is 16K-0x50 bytes back
   ;; which is the same as advancing 48K+0x50 = 0xC050 bytes, as memory is 64K 
   ;; and our 16bit pointers cycle over it
   LD  BC, #0xC050         ;; [10] B850h = C050h - 800h
   ADD HL, BC              ;; [11] We advance destination pointer to next line
   JP ds_drawSpriteWidth_next ;; [10] and then continue copying
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

;
;########################################################################
;### FUNCTION: cpct_setVideoMemoryPage                                ###
;########################################################################
;### Changes the place where the video memory starts. Concretely, it  ###
;### changes de Video Memory Page, which correponds to the 6 Most     ###
;### significant bits (MSb) from the memory address where video memo- ###
;### ry starts. By doing this, you are effectively changing what part ###
;### of the RAM will be used as Video Memory and, therefore, what is  ###
;### to be displayed from now on. With this you can affectively imple-###
;### ment double and triple buffer using hardware instead of copying. ###
;###                                                                  ###
;### This routine achieves this by changing Register R12 from CRTC.   ###
;### This register is the responsible for holding the 6 MSb of the    ###
;### place where Video Memory starts (its page). However, this regis- ###
;### ter stores this 6 bits as its 6 LSb (--bbbbbb). We have to take  ###
;### this into account to correctly set the page we want.             ###
;###                                                                  ###
;### Useful example:                                                  ###
;###  1. You want your Video Memory to start at 0x8000                ###
;###    > Then, your Video Memory Page is 0x80 (10000000).            ###
;###    > You call this routine with 0x20 as parameter (00100000)     ###
;###    > Note that 0x20 (00100000) is 0x80 (10000000) shifted twice  ###
;###      to the right. Your 6 MSb are to be passed as the 6 LSb.     ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (1B A) New starting page for Video Memory (Only 6 LSb used)   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 14 bytes                                                 ###
;### TIME:   76 cycles                                                ###
;########################################################################
;
.globl _cpct_setVideoMemoryPage
_cpct_setVideoMemoryPage::
   ;; Get the parameter from the stack
   LD  HL, #2        ;; [10] HL = SP + 2 (Place where parameter is)
   ADD HL, SP        ;; [11]
   LD  A, (HL)       ;; [ 7] A = First Parameter (Video Memory Page to Set)

   ;; Select R12 Register from the CRTC and Write there the selected Video Memory Page
   LD  BC, #0xBC0C   ;; [10] 0xBC = Port for selecting CRTC Register, 0x0C = Selecting R12
   OUT (C), C        ;; [12] Select the R12 Register (0x0C to port 0xBC)
   INC B             ;; [ 4] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   OUT (C), A        ;; [12] Write Selected Video Memory Page to R12 (A to port 0xBD)

   RET               ;; [10] Return

;
;########################################################################
;### FUNCTION: cpct_setVideoMemoryOffset                              ###
;########################################################################
;### Changes part of the address where video memory starts. Video me- ###
;### mory starts at an address that can be defined in binary as:      ###
;###    Start Addres = ppppppoooooooo00                               ###
;### where you have 6 bits that define the Page, and 8 bits that defi-###
;### ne the offset. The 2 Least Significant Bits (LSb) are allways 0. ###
;### This funcion changes the 8 bits that define the offset. This 8   ###
;### bits are controlled by Register R13 from the CRTC. If you wanted ###
;### to change the 6 Most Significant bits (MSb), aka the Page, you   ###
;### should call _cpct_setVideoMemoryPage.                            ###
;###                                                                  ###
;### Changing this effectively changes the place where your Video Me- ###
;### mory will be located in RAM, and it will change what is displa-  ###
;### yed. This could be used to produce scrolling effects or to make  ###
;### a fine grained control of double/triple buffers by hardware.     ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (1B A) New starting offset for Video Memory                   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: F, BC, DE                            ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 14 bytes                                                 ###
;### TIME:   76 cycles                                                ###
;########################################################################
;
.globl _cpct_setVideoMemoryOffset
_cpct_setVideoMemoryOffset::
   ;; Get the parameter from the stack
   LD  HL, #2        ;; [10] HL = SP + 2 (Place where parameter is)
   ADD HL, SP        ;; [11]
   LD  A, (HL)       ;; [ 7] A = First Parameter (Video Memory Offset to Establish)

   ;; Select R13 Register from the CRTC and Write there the selected Video Memory Offset
   LD  BC, #0xBC0D   ;; [10] 0xBC = Port for selecting CRTC Register, 0x0D = Selecting R13
   OUT (C), C        ;; [12] Select the R13 Register (0x0D to port 0xBC)
   INC B             ;; [ 4] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   OUT (C), A        ;; [12] Write Selected Video Memory Offset to R13 (A to port 0xBD)

   RET               ;; [10] Return 

;
;########################################################################
;### FUNCTION: cpct_memset                                            ###
;########################################################################
;### Sets all the bytes of an array in memory to the same given value ###
;### This is the technique it uses:                                   ###
;###  - It first sets up the first byte of the array to the value     ###
;###  - Makes HL point to the first byte and DE to the second         ###
;###  - BC has the total bytes to copy minus 1 (the first already set)###
;###  - LDIR copies first byte into second, then second into third... ###
;### WARNING: This works only for values of BC > 1. Never use this    ###
;### function with values 0 or 1 as number of bytes to copy, as it    ###
;### will have unexpected results (In fact, it could write up the en- ###
;### tire memory).                                                    ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (2B DE) Starting point in memory                              ###
;###  * (2B BC) Number of bytes to be set (should be greater than 1!) ###
;###  * (1B A) Value to be set                                        ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY:  20 bytes                                                ###
;### TIME:    113 + 21*BC cycles (28.25 + 5.25*BC us)                 ###
;########################################################################
;
.globl _cpct_memset
_cpct_memset::
   LD  HL, #0        ;; [10] HL = SP (To quickly restore it later)
   ADD HL, SP        ;; [11]
   DI                ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   POP AF            ;; [10] Get return value from stack
   POP DE            ;; [10] (1st param) DE = Starting point in memory
   POP BC            ;; [10] (2nd param) BC = Number of bytes to be set
   DEC SP            ;; [ 6] ** 3rd param is byte-size, so need to decrease SP to load it into A when doing POP (otherwise it would go to F)
   POP AF            ;; [10] (3rd param) A = Value to be set
ms_restoreSP:
   LD SP, HL         ;; [ 6] -- Restore Stack Pointer --
   EI                ;; [ 4] Enable interrupts again

   ;; Set up HL and DE for a massive copy of the Value to be set
   LD H, D           ;; [ 4] HL = DE (Starting address in memory)
   LD L, E           ;; [ 4] 
   INC DE            ;; [ 6] DE++ (Next position in memory)
   LD (HL), A        ;; [ 7] (HL) = A --> First item of the memory array = Value
   DEC BC            ;; [ 6] BC--, We already have copied the first byte with previous instruction

   LDIR              ;; [21/16] Copy the reset of the bytes, cloning previous one

   RET               ;; [10] Return 
