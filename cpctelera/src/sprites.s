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
  POP  AF			;; [10c] AF = Return Address
  POP  HL			;; [10c] HL = Source address
  POP  DE			;; [10c] DE = Destination address
  PUSH DE			;; [11c] Leave the stack as it was
  PUSH HL			;; [11c] 
  PUSH AF			;; [11c] 
  
  ;; Copy 8 lines of 2 bytes width
  LD   BC, #16			;; [10c] 16 bytes is the total we have to transfer (2x8)
  JP   dsa28_first_line		;; [10c] First line does not need to do math to start transfering data. 

dsa28_next_line:
  LD   A, D			;; [ 4c] Add 800h and Subtract 2h to DE (Jump to next visual line in the video memory)
  ADD  A, #8			;; [ 7c]
  LD   D, A			;; [ 4c]
  DEC  DE			;; [ 6c]
  DEC  DE			;; [ 6c]

dsa28_first_line:
  LDI				;; [16c] Copy 2 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
  LDI				;; [16c]
  XOR  A			;; [ 4c] A = 0
  XOR  C			;; [ 4c] Check if C = 0 using XOR (as A is already 0)
  JP   NZ,dsa28_next_line 	;; [10c] 
  
  ;; RETURN
  RET				;; [10c]

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
  POP  AF			;; [10c] AF = Return Address
  POP  HL			;; [10c] HL = Source address
  POP  DE			;; [10c] DE = Destination address
  PUSH DE			;; [11c] Leave the stack as it was
  PUSH HL			;; [11c] 
  PUSH AF			;; [11c] 
  
  ;; Copy 8 lines of 4 bytes width
  LD   A, #8			;; [ 7c] We have to draw 8 lines of sprite
  JP   dsa48_first_line		;; [10c] First line does not need to do math to start transfering data. 
  
dsa48_next_line:
  ;; Move to the start of the next line
  EX   DE, HL			;; [ 4c] Make DE point to the start of the next line of pixels in video memory, by adding BC
  ADD  HL, BC			;; [11c] (We need to interchange HL and DE because DE does not have ADD)
  EX   DE, HL			;; [ 4c]

dsa48_first_line:
  ;; Draw a sprite-line of 4 bytes
  LD   BC, #0x800		;; [10c] 800h bytes is the distance to the start of the first pixel in the next line in video memory (it will be decremented by 1 by each LDI)
  LDI				;; [16c] <|Copy 4 bytes with (DE) <- (HL) and decrement BC (distance is 1 byte less as we progress up)
  LDI				;; [16c]  |
  LDI				;; [16c]  |
  LDI				;; [16c] <|
  
  ;; Repeat for all the lines
  DEC  A			;; [ 4c] A = A - 1 (1 more line completed)
  JP   NZ,dsa48_next_line 	;; [10c] 
  
  ;; RETURN
  RET				;; [10c]

  
;
;########################################################################
;### FUNCTION: _cpct_setVideoMode                                     ###
;########################################################################
;### This function establishes the video mode for the Amstrad CPC.    ###
;### Video modes available are:                                       ###
;###  0: 160x200, 16 colours                                          ###
;###  1: 320x200,  4 colours                                          ###
;###  2: 640x200,  2 colours                                          ###
;###  3: 160x200,  4 colours (undocumented)                           ###
;### Important: Do not use this method when the firmware is up and    ###
;###    running, as it modifies Upper and Lower ROM paging.           ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (1B) Video mode to set (only lowest 2 bits will be used)      ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  67 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying cpc_setMode from  ###
;### cpcrslib by Raul Simarro.                                        ###
;########################################################################
;
.globl _cpct_setVideoMode
_cpct_setVideoMode::
        LD  HL, #2		;; [10c] Load A with the parameter (video mode)
        ADD HL, SP		;; [11c]
        LD   A, (HL)		;; [ 7c]

        OR   A, #140 		;; [ 7c] 140 = @10001100 => Mode  and  rom  selection  (and Gate Array function)
        LD  BC, #0x7F00		;; [10c] 7F00h           => Port of the Gate Array Chip
        OUT (C),A		;; [12c] Send request to Port
        
        RET			;; [10c] Return

        
;
;########################################################################
;### FUNCTION: _cpct_disableFirmware                                  ###
;########################################################################
;###  Disables the firmware modifying the interrupt vector at 0x38.   ###
;### Normally, firmware routines are called and executed at each      ###
;### interrupt and the ROM entry point is stored at 0x38.             ###
;### This function substitutes the 2 bytes located at 0x38 by 0xC9FB, ###
;### (FB = EI, C9 = RET), which basically does nothing at each        ###
;### interruption.                                                    ###
;########################################################################
;### INPUTS (none)                                                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values:HL                                    ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  84 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying                   ###
;### cpc_disableFirmware from cpcrslib by Raul Simarro.               ###
;########################################################################
;        
.globl _cpct_disableFirmware
_cpct_disableFirmware::
	DI			    ;; [ 4c] Disable interrupts
	LD HL,(#0x38)               ;; [16c] Obtain firmware ROM code pointer and store it for restoring it later
	LD (firmware_call_add),HL   ;; [16c]

        IM 1			    ;; [ 8c] Set Interrupt Mode 1 (CPU will jump to &0038 when a interrupt occrs)
	LD HL,#0xC9FB		    ;; [10c] FB C9 (take into account little endian) => EI : RET

        LD (#0x38), HL		    ;; [16c] Setup new "interrupt handler" and enable interrupts again
	EI                          ;; [ 4c] 

	RET                         ;; [10c]

firmware_call_add: .DW 0

;
;########################################################################
;### FUNCTION: _cpct_reenableFirmware                                 ###
;########################################################################
;###   Restores the ROM address where the firmware routines start.    ###
;### Beware: cpct_disableFirmware has to be called before calling     ###
;### this routine; otherwise, it would put a 0 in 0x38 and a reset    ###
;### will be performed at the next interrupt.                         ###
;########################################################################
;### INPUTS (none)                                                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: HL                                   ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  50 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying                   ###
;### cpc_disableFirmware from cpcrslib by Raul Simarro.               ###
;########################################################################
;        
.globl 	_cpct_reenableFirmware
_cpct_reenableFirmware::
	DI                          ;; [ 4c] Disable interrupts

	LD HL, (firmware_call_add)  ;; [16c] Restore previously saved pointer to ROM code
	LD (#0x38), HL		    ;; [16c]

	EI                          ;; [ 4c] Reenable interrupts and return
	RET                         ;; [10c]
