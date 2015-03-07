;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 - 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

;
;########################################################################
;### FUNCTION: _cpct_scanKeyboardFast                                 ###
;########################################################################
;### This function reads the status of keyboard and joysticks and     ###
;### stores it in the 10 bytes reserverd as "keyboardStatusBuffer"    ###
;### It uses an unrolled version of its main loop to gain 147 cycles. ###
;########################################################################
;### INPUTS (-)                                                       ###
;########################################################################
;### OUTPUTS (10B)                                                    ###
;###   -> KeyboardStatusBuffer full with pressed/not pressed info     ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  561 cycles (140,25 us)                                          ###
;########################################################################
;### CREDITS:                                                         ###
;###    This fragment of code is based on a scanKeyboard code issued  ###
;### by CPCWiki.                                                      ###
;### http://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning    ###
;########################################################################
;
;; Keyboard Status Buffer defined in an external file
.globl cpct_keyboardStatusBuffer

.globl _cpct_scanKeyboardFast
_cpct_scanKeyboardFast:: 

    DI                              ;; [ 4] Disable interrupts

    LD  HL,#cpct_keyboardStatusBuffer ;; [10] HL Points to the start of the keyboardBuffer, where scanned data will be stored

    ;; Configure PPI: Select Register 14 (the one connected with keyboard status) and set it for reading
    ;;
    LD  BC,  #0xF782                ;; [10] Configure PPI 8255: Set Both Port A and Port C as Output. 
    OUT (C), C                      ;; [12] 82 = 1000 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=0)=> Port A=Output, 
                                    ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output
    LD  BC,  #0xF40E                ;; [10] Write (0Eh = 14) on PPI 8255 Port A (F4h): the register we want to select on AY-3-8912  
    LD  E,   B                      ;; [ 4] Save F4h into E to use it later in the loop
    OUT (C), C                      ;; [12] 

    LD  BC,  #0xF6C0                ;; [10] Write (C0h = 11 000000b) on PPI Port C (F6h): operation > select register 
    LD  D,   B                      ;; [ 4] Save F6h into D to use it later in the loop
    OUT (C), C                      ;; [12]
    .DW #0x71ED                     ;; [12] OUT (C), 0 => Write 0 on PPI's Port C to put PSG's in inactive mode (required in between different operations)

    LD  BC,  #0xF792                ;; [10] Configure PPI 8255: Set Port A = Input, Port C = Output. 
    OUT (C), C                      ;; [12] 92h= 1001 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=1)=> Port A=Input, 
                                    ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output
    ;; Read Loop (Unrolled version): We read the 10-bytes that define the pressed/not pressed status
    ;;
    LD  A,   #0x40                  ;; [ 7] A refers to the next keyboard line to be read (40h to 49h)

    ;; Read line 40h
    LD  B,   D                      ;; [ 4] B = F6h => Write the value of A to PPI's Port C to select next Matrix Line
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4] B = F4h => Read from PPI's Port A: Pressed/Not Pressed Values from PSG
    INI                             ;; [16] The read value is written to (HL), then HL<-HL+1 and B<-B-1
    INC A                           ;; [ 4] Loop: Increment A => Next Matrix Line. 

    ;; Read line 41h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]

    ;; Read line 42h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]

    ;; Read line 43h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]

    ;; Read line 44h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]
 
    ;; Read line 45h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]

    ;; Read line 46h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]

    ;; Read line 47h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]

    ;; Read line 48h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]
    INC A                           ;; [ 4]

    ;; Read line 49h
    LD  B,   D                      ;; [ 4]
    OUT (C), A                      ;; [12] 
    LD  B,   E                      ;; [ 4]
    INI                             ;; [16]

    ;; Restore PPI status to Port A=Output, Port C=Output
    ;;
    LD  BC,  #0xF782                ;; [10] Put again PPI in Output/Output mode for Ports A/C.
    OUT (C), C                      ;; [12]

    EI                              ;; [ 4] Reenable interrupts

    RET                             ;; [10] Return
