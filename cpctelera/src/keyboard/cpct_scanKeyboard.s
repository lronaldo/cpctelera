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
.module cpct_keyboard

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_scanKeyboard
;;
;;    Reads the status of keyboard and joysticks and stores it in the 
;; 10 bytes reserved as <cpct_keyboardStatusBuffer>
;;
;; C Definition:
;;    void *cpct_scanKeyboard* ()
;;
;; Output results (10 bytes):
;;    <cpct_keyboardStatusBuffer> filled up with pressed / not pressed info
;; about all the 80 available Amstrad CPC's keys / buttons.
;;
;; Known limitations:
;;    This function disables interrupts while does keyboard scanning. Interrupts
;; are enabled at the end, when scanning has finished.
;;
;; Details:
;;    This function reads the pressed / not pressed status of the entire set
;; of 80 keys / buttons from the Amstrad CPC and writes this status in 
;; <cpct_keyboardStatusBuffer>. <cpct_keyboardStatusBuffer> is a 10-bytes 
;; buffer (80 bits) that holds 1 bit for each key / button of the Amstrad CPC,
;; meaning 0 = pressed, and 1 = not pressed. This codification is the same
;; as the one returned by the AY-3-8912 chip, which reads the keyboard when
;; Programmable Peripheral Interface (PPI) chip demands it. For more details
;; on how all this process works, check <Keyboard> topic.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    17 bytes
;;
;; Time Measures:
;; (start code)
;; Case | Cycles | microSecs (us)
;; -------------------------------
;; Any  |  712   |   178
;; -------------------------------
;; (end code)
;;
;; Credits:                                                       
;;    This fragment of code is based on a scanKeyboard code issued by CPCWiki.                                                    
;; http://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning. This version
;; of the code, however, is ~33% faster than CPCWiki's.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Keyboard Status Buffer defined in an external file
.globl cpct_keyboardStatusBuffer

_cpct_scanKeyboard:: 

   ld   hl, #cpct_keyboardStatusBuffer ;; [10] HL Points to the start of the keyboardBuffer, where scanned data will be stored

   di                       ;; [ 4] Disable interrupts

   ;; Configure PPI: Select Register 14 (the one connected with keyboard status) and set it for reading
   ;;
   ld   bc, #0xF782         ;; [10] Configure PPI 8255: Set Both Port A and Port C as Output. 
   out (c), c               ;; [12] 82 = 1000 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=0)=> Port A=Output, 
                            ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output

   ld   bc, #0xF40E         ;; [10] Write (0Eh = 14) on PPI 8255 Port A (F4h): the register we want to select on AY-3-8912  
   ld    e, b               ;; [ 4] Save F4h into E to use it later in the loop
   out (c), c               ;; [12] 

   ld   bc, #0xF6C0         ;; [10] Write (C0h = 11 000000b) on PPI Port C (F6h): operation > select register 
   ld    d, b               ;; [ 4] Save F6h into D to use it later in the loop
   out (c), c               ;; [12]
   .DW #0x71ED ; out (c), 0 ;; [12] OUT (C), 0 => Write 0 on PPI's Port C to put PSG's in inactive mode (required in between different operations)

   ld   bc, #0xF792         ;; [10] Configure PPI 8255: Set Port A = Input, Port C = Output. 
   out (c), c               ;; [12] 92h= 1001 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=1)=> Port A=Input, 
                            ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output

   ;; Read Loop: We read the 10-bytes that define the pressed/not pressed status
   ;;
   ld    a, #0x40           ;; [ 7] A refers to the next keyboard line to be read (40h to 49h)
   ld    c, #0x4a           ;; [ 7] 4a is used to compare A and know when we have read all the Matrix Lines

rfks_nextKeyboardLine:
   ld    b, d               ;; [ 4] B = F6h => Write the value of A to PPI's Port C to select next Matrix Line
   out (c), a               ;; [12]

   ld    b, e               ;; [ 4] B = F4h => Read from PPI's Port A: Pressed/Not Pressed Values from PSG
   ini                      ;; [16] The read value is written to (HL), then HL<-HL+1 and B<-B-1

   inc   a                  ;; [ 4] Loop: Increment A => Next Matrix Line. 
   cp    c                  ;; [ 4] Check if we have arrived to line 4a, which is the end 
   jp    c, rfks_nextKeyboardLine ;; [10] Repeat loop if we are not done.

   ;; Restore PPI status to Port A=Output, Port C=Output
   ;;
   ld   bc, #0xF782         ;; [10] Put again PPI in Output/Output mode for Ports A/C.
   out (c), c               ;; [12]

   ei                       ;; [ 4] Reenable interrupts

   ret                      ;; [10] Return
