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
.module cpct_strings

;;
;; Include constants and general values
;;
.include /strings.s/
.globl _cpct_drawCharM1_f_asm

;
;########################################################################
;## FUNCTION: _cpct_drawROMStringM1_fast                              ###
;########################################################################
;### This function receives a null terminated string and draws it to  ###
;### the screen in mode 1, using _cpct_drawROMCharM1_fast to draw each###
;### character.                                                       ###
;### * Some IMPORTANT things to take into account:                    ###
;###  -- This routine does not check for boundaries. If you draw too  ###
;###     long strings or out of the screen, unpredictable resuls will ###
;###     happen.
;###  -- Do not put this function's code below 4000h in memory. In    ###
;###     order to read from ROM, this function enables Lower ROM      ###
;###     (which is located 0000h-3FFFh), so CPU would read from ROM   ###
;###     instead of RAM in first bank, effectively shadowing this     ###
;###     piece of code, and producing undefined results (tipically,   ###
;###     program would hang or crash).                                ###
;###  -- This function works well for drawing on double buffers loca- ###
;###     ted at whichever memory bank, except 0000h (4000h-FFFFh)     ###
;###  -- This function disables interrupts during main loop (charac-  ###
;###     ter printing). It reenables them at the end.                 ###
;###  -- Do not pass numbers greater that 3 as color parameters, as   ###
;###     they are used as indexes in a color table, and results may   ###
;###     be unpredictable                                             ###
;########################################################################
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B HL) Pointer to the null terminated string being drawn     ### 
;###  * (2B DE) Video memory location where the char will be printed  ### 
;###  * (1B C) Foreground color (PEN, 0-3)                            ###
;###  * (1B B) Background color (PEN, 0-3)                            ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES (Way 2 for parameter retrieval from stack)              ###
;### MEMORY:  34 bytes (+351 bytes drawROMCharM1_fast)                ###
;### TIME:                                                            ###
;###  Best case  = 145 + (108+1878)*length cycles                     ###
;###  Worst case = 145 + (108+2596)*length cycles                     ###
;### EXAMPLES:                                                        ###
;###  10 character string = [20005-27187]cycles [5001,25- 6796,75]us  ###
;###  20 character string = [39865-54225]cycles [9966,25-13556,25]us  ###
;########################################################################
;

_cpct_drawStringM1_f::
   ;; Get parameters form stack
.if let_disable_interrupts_for_function_parameters
   ;; Way 1: Pop + Restoring SP. Faster, but consumes 7 bytes more, and requires disabling interrupts
   ld (drsm1f_restoreSP+1), sp         ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   di                                  ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                             ;; [10] AF = Return Address
   pop  hl                             ;; [10] HL = Pointer to the null terminated string
   pop  de                             ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                             ;; [10] BC = Colors (B=Background color, C=Foreground color) 
drsm1f_restoreSP:
   ld sp, #0                           ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   ei                                  ;; [ 4] Enable interrupts again
.else 
   ;; Way 2: Pop + Push. Just 8 cycles more, but does not require disabling interrupts
   pop  af                             ;; [10] AF = Return Address
   pop  hl                             ;; [10] HL = Pointer to the null terminated string
   pop  de                             ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                             ;; [10] BC = Colors (B=Background color, C=Foreground color) 
   push bc                             ;; [11] Restore Stack status pushing values again
   push de                             ;; [11] (Interrupt safe way, 8 cycles more)
   push hl                             ;; [11]
   push af                             ;; [11]
.endif

   ld (drsm1f_values+1), bc            ;; [20] Save BC as LD direct value to be read later for saving color values (Foreground and Background)
   jp drsm1f_firstChar                 ;; [10] Jump to first char

drsm1f_nextChar:
   push hl                             ;; [11] Save HL and DE to the stack befor calling draw char
   push de                             ;; [11]
   call _cpct_drawCharM1_f_asm         ;; [17] Draw next char
   pop  de                             ;; [10] Recover HL and DE from the stack
   pop  hl                             ;; [10]

drsm1f_values:
   ld   bc, #00                        ;; [10] Restore BC value (Foreground and Background colors)
   inc  de                             ;; [ 6] DE += 2 (point to next position in video memory, 8 pixels to the right)
   inc  de                             ;; [ 6]
   inc  hl                             ;; [ 6] HL += 1 (point to next character in the string)

drsm1f_firstChar:
   ld  a, (hl)                         ;; [ 7] A = next character from the string
   or  a                               ;; [ 4] Check if A = 0
   jp  nz, drsm1f_nextChar             ;; [10] if A != 0, A is next character, draw it, else end

drsm1f_endString:
   ret                                 ;; [10] Return
