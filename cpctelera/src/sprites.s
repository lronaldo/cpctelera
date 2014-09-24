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
;### MEASURED TIME                                                    ###
;###  93 + 8 * 50 + 7 * 27 = 682 cycles                               ###
;########################################################################
;
.globl _cpct_drawSprite2x8_aligned
_cpct_drawSprite2x8_aligned::
  ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                  ;; [10c] AF = Return Address
   POP  HL                  ;; [10c] HL = Source address
   POP  DE                  ;; [10c] DE = Destination address
   PUSH DE                  ;; [11c] Leave the stack as it was
   PUSH HL                  ;; [11c] 
   PUSH AF                  ;; [11c] 

   ;; Copy 8 lines of 2 bytes width
   LD   BC, #16             ;; [10c] 16 bytes is the total we have to transfer (2x8)
   JP   dsa28_first_line    ;; [10c] First line does not need to do math to start transfering data. 

dsa28_next_line:
   LD   A, D                ;; [ 4c] Add 800h and Subtract 2h to DE (Jump to next visual line in the video memory)
   ADD  A, #8               ;; [ 7c]
   LD   D, A                ;; [ 4c]
   DEC  DE                  ;; [ 6c]
   DEC  DE                  ;; [ 6c]

dsa28_first_line:
   LDI                      ;; [16c] Copy 2 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
   LDI                      ;; [16c]
   XOR  A                   ;; [ 4c] A = 0
   XOR  C                   ;; [ 4c] Check if C = 0 using XOR (as A is already 0)
   JP   NZ,dsa28_next_line  ;; [10c] 

   RET                      ;; [10c]

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
;###  90 + 8 * 88 + 7 * 19 = 927 cycles                               ###
;########################################################################
;
.globl _cpct_drawSprite4x8_aligned
_cpct_drawSprite4x8_aligned::
   ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                 ;; [10c] AF = Return Address
   POP  HL                 ;; [10c] HL = Source address
   POP  DE                 ;; [10c] DE = Destination address
   PUSH DE                 ;; [11c] Leave the stack as it was
   PUSH HL                 ;; [11c] 
   PUSH AF                 ;; [11c] 

   ;; Copy 8 lines of 4 bytes width
   LD   A, #8              ;; [ 7c] We have to draw 8 lines of sprite
   JP   dsa48_first_line   ;; [10c] First line does not need to do math to start transfering data. 

dsa48_next_line:
   ;; Move to the start of the next line
   EX   DE, HL             ;; [ 4c] Make DE point to the start of the next line of pixels in video memory, by adding BC
   ADD  HL, BC             ;; [11c] (We need to interchange HL and DE because DE does not have ADD)
   EX   DE, HL             ;; [ 4c]

dsa48_first_line:
   ;; Draw a sprite-line of 4 bytes
   LD   BC, #0x800         ;; [10c] 800h bytes is the distance to the start of the first pixel in the next line in video memory (it will be decremented by 1 by each LDI)
   LDI                     ;; [16c] <|Copy 4 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|

   ;; Repeat for all the lines
   DEC  A                  ;; [ 4c] A = A - 1 (1 more line completed)
   JP   NZ,dsa48_next_line ;; [10c] 

   RET                     ;; [10c]

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
;###  * (2B) Source Sprite Pointer (array with pixel data)            ###
;###  * (2B) Destination video memory pointer                         ###
;###  * (1B) Sprite Width in bytes (Max.63)                           ###
;###  * (1B) Sprite Height in bytes                                   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;### [82 + (79 + 16w)h + 31c] cycles; w=width,h=height,c=8linecrosses ###
;### Examples:                                                        ###
;### 2x16 byte sprite = 1920/1889 cycles                              ###
;### 4x32 byte sprite = 4782/4751 cycles                              ###
;########################################################################
;
.globl _cpct_drawSprite
_cpct_drawSprite::
   ;; GET Parameters from the stack (Push+Pop is faster than referencing with IX)
   POP  AF                 ;; [10c] AF = Return Address
   POP  HL                 ;; [10c] HL = Source address
   POP  DE                 ;; [10c] DE = Destination address
   POP  BC                 ;; [10c] BC = Height/Width
   PUSH BC                 ;; [11c] Leave the stack as it was
   PUSH DE                 ;; [11c] 
   PUSH HL                 ;; [11c] 
   PUSH AF                 ;; [11c] 

   ;; B = Width, C = Height

   ;; Modify code using width to jump in drawSpriteWidth
   LD  A, #126                   ;; [ 7c] We need to jump 126 bytes (63 LDIs*2 bytes) minus the width of the sprite*2 (2B)
   SUB C                         ;; [ 4c]    to do as much LDIs as bytes the Sprite is wide
   SUB C                         ;; [ 4c]
   LD (ds_drawSpriteWidth+#4), A ;; [13c] Modify JR data to create the jump we need

   LD   A, B                 ;; [ 4c] A = Height (used as counter for the number of lines we have to copy)
   EX   DE, HL               ;; [ 4c] Instead of jumping over the next line, we do the inverse operation because it is only 4 cycles and not 10, as a JP would be)

ds_drawSpriteWidth_next:
   ;; NEXT LINE
   EX   DE, HL             ;; [ 4c] HL and DE are exchanged every line to do 16bit math with DE. This line reverses it before proceeding to copy the next line.

ds_drawSpriteWidth:
   ;; Draw a sprite-line of n bytes
   LD BC, #0x800           ;; [10c] 0x800 bytes is the distance in memory from one pixel line to the next within every 8 pixel lines. Each LDI performed will decrease this by 1, as we progress through memory copying the present line
   .DW #0x0018  ;; JR 0    ;; [12c] Self modifying instruction: the '00' will be substituted by the required jump forward. (Note: Writting JR 0 compiles but later it gives odd linking errors)
   LDI                     ;; [16c] <| 80 LDIs, which are able to copy up to 80 bytes each time.
   LDI                     ;; [16c]  | That means that each Sprite line should be 80 bytes width at most.
   LDI                     ;; [16c]  | The JR instruction at the start makes us ingnore the LDIs we dont need (jumping over them)
   LDI                     ;; [16c] <| That ensures we will be doing only as much LDIs as bytes our sprit is wide
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |
   LDI                     ;; [16c] <|   
   LDI                     ;; [16c] <|
   LDI                     ;; [16c]  |
   LDI                     ;; [16c]  |

   DEC A                   ;; [ 4c] Another line finished: we discount it from A
   RET Z                   ;; [11c/5c] If that was the last line, we safely return

   ;; Jump destination pointer to the start of the next line in video memory
   EX  DE, HL              ;; [ 4c] DE has destination, but we have to exchange it with HL to be able to do 16bit math
   ADD HL, BC              ;; [11c] We add 0x800 minus the width of the sprite (BC) to destination pointer 
   LD  B, A                ;; [ 4c] Save A into B (B = A)
   LD  A, H                ;; [ 4c] We check if we have crossed video memory boundaries (which will happen every 8 lines). If that happens, bits 13,12 and 11 of destination pointer will be 0
   AND #0x38               ;; [ 7c] leave out only bits 13,12 and 11
   LD  A, B                ;; [ 4c] Restore A from B (A = B)
   JP NZ, ds_drawSpriteWidth_next ;; [10c] If that does not leave as with 0, we are still inside video memory boundaries, so proceed with next line

   ;; Every 8 lines, we cross the 16K video memory boundaries and have to
   ;; reposition destination pointer. That means our next line is 16K-0x50 bytes back
   ;; which is the same as advancing 48K+0x50 = 0xC050 bytes, as memory is 64K 
   ;; and our 16bit pointers cycle over it
   LD  BC, #0xC050         ;; [10c] B850h = C050h - 800h
   ADD HL, BC              ;; [11c] We advance destination pointer to next line
   JP ds_drawSpriteWidth_next ;; [10c] and then continue copying

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
;###  * (1B) New starting page for Video Memory (Only 6 LSb are used) ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: F, BC, DE                            ###
;########################################################################
;### MEASURED TIME: 90 cycles                                         ###
;########################################################################
;
.globl _cpct_setVideoMemoryPage
_cpct_setVideoMemoryPage::
   POP BC            ;; [10c] Get Call Return Address
   POP DE            ;; [10c] Get Parameter (A = New starting page...)
   PUSH DE           ;; [11c] Restore Stack status
   PUSH BC           ;; [11c]

   ;; Select R12 Register from the CRTC and Write there the selected Video Memory Page
   LD  BC, #0xBC0C   ;; [10c] 0xBC = Port for selecting CRTC Register, 0x0C = Selecting R12
   OUT (C), C        ;; [12c] Select the R12 Register (0x0C to port 0xBC)
   INC B             ;; [ 4c] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   OUT (C), E        ;; [12c] Write Selected Video Memory Page to R12 (A to port 0xBD)

   RET               ;; [10c] Return

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
;###  * (1B) New starting offset for Video Memory                     ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: F, BC, DE                            ###
;########################################################################
;### MEASURED TIME: 90 cycles                                         ###
;########################################################################
;
.globl _cpct_setVideoMemoryOffset
_cpct_setVideoMemoryOffset::
   POP BC            ;; [10c] Get Call Return Address
   POP DE            ;; [10c] Get Parameter (A = New starting offset...)
   PUSH DE           ;; [11c] Restore Stack status
   PUSH BC           ;; [11c]

   ;; Select R13 Register from the CRTC and Write there the selected Video Memory Offset
   LD  BC, #0xBC0D   ;; [10c] 0xBC = Port for selecting CRTC Register, 0x0D = Selecting R13
   OUT (C), C        ;; [12c] Select the R13 Register (0x0D to port 0xBC)
   INC B             ;; [ 4c] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   OUT (C), E        ;; [12c] Write Selected Video Memory Offset to R13 (A to port 0xBD)

   RET               ;; [10c] Return 
 
 
 
 ;; This piece of code could be useful later
 
  ;; Get the current pixel-row of the screen where we are painting (there are 8 pixel rows)
   ;; Pixel row are bits 11-13 of the address pointer (14-15 identify the page 0000/4000/8000/C000)
   ;;LD  A, D                ;; [ 4c]
   ;;RRCA                    ;; [ 4c]
   ;;RRCA                    ;; [ 4c]
   ;;RRCA                    ;; [ 4c]
   ;;NEG                     ;; [ 8c] Two's Complement
   ;;AND #0x07               ;; [ 7c] 07 = 00000111

   ;;LD  B, A                ;; [ 4c]
   ;;LD  A, C                ;; [ 4c]
   ;;SUB B                   ;; [ 4c]
   ;;.DW #0x67DD             ;; [ 8c] LD IXh, A (Save A = C - A)