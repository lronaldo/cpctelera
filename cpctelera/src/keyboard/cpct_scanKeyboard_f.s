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
;; Function: cpct_scanKeyboard_f
;;
;;    Reads the status of keyboard and joysticks and stores it in the 
;; 10 bytes reserved as <cpct_keyboardStatusBuffer>. This function is ~25% faster
;; than standard <cpct_scanKeyboard>.
;;
;; C Definition:
;;    void <cpct_scanKeyboard_f> ()
;;
;; Output results (10 bytes):
;;    <cpct_keyboardStatusBuffer> filled up with pressed / not pressed info
;; about all the 80 available Amstrad CPC's keys / buttons.
;;
;; Assembly call:
;;    > call cpct_scanKeyboard_f_asm
;;
;; Known limitations:
;;    This function disables interrupts while does keyboard scanning. Interrupts
;; are enabled at the end, when scanning has finished.
;;
;; Details:
;;    This function does the same as <cpct_scanKeyboard> but using an unrolled
;; loop to gain 42 microseconds (~25% performance increase, with a cost of 57 bytes
;; more in memory requirements for code, ~116% increase).
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
;;    106 bytes
;;
;; Time Measures:
;; (start code)
;; Case | microSecs (us) | CPU Cycles
;; ------------------------------------
;; Any  |     170        |    680
;; ------------------------------------
;; (end code)
;;
;; Credits:                                                       
;;    This fragment of code is based on a <scanKeyboard code issued by CPCWiki at
;; http://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning>. This version
;; of the code is, however, ~60% faster than CPCWiki's.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Keyboard Status Buffer defined in an external file
.globl _cpct_keyboardStatusBuffer

_cpct_scanKeyboard_f:: 
cpct_scanKeyboard_f_asm::   ;; Assembly entry point

   di                       ;; [1] Disable interrupts

   ;; Configure PPI: Select Register 14 (the one connected with keyboard status) and set it for reading
   ;;
   ld  bc,  #0xF782         ;; [3] Configure PPI 8255: Set Both Port A and Port C as Output. 
   out (c), c               ;; [4] 82 = 1000 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          
                            ;;                       (B4=0)=> Port A=Output,  (B3=0)=> Port Cu=Output, 
                            ;;                       (B2=0)=> Group B, Mode 0,(B1=1)=> Port B=Input, (B0=0)=> Port Cl=Output
   ld  bc,  #0xF40E         ;; [3] Write (0Eh = 14) on PPI 8255 Port A (F4h): the register we want to select on AY-3-8912
   ld  e, b                 ;; [1] Save F4h into E to use it later in the loop
   out (c), c               ;; [4]

   ld  bc,  #0xF6C0         ;; [3] Write (C0h = 11 000000b) on PPI Port C (F6h): operation > select register 
   ld  d, b                 ;; [1] Save F6h into D to use it later in the loop
   out (c), c               ;; [4]
   .dw #0x71ED ; out (c), 0 ;; [4] out (C), 0 => Write 0 on PPI's Port C to put PSG's in inactive mode 
                            ;; .... (required in between different operations)
   ld  bc,  #0xF792         ;; [3] Configure PPI 8255: Set Port A = Input, Port C = Output. 
   out (c), c               ;; [4] 92h= 1001 0010 : (B7=1)=> I/O Mode,        (B6-5=00)=> Mode 1,          
                            ;;                       (B4=1)=> Port A=Input,    (B3=0)=> Port Cu=Output, 
                            ;;                       (B2=0)=> Group B, Mode 0, (B1=1)=> Port B=Input, (B0=0)=> Port Cl=Output
   ;; Read Loop (Unrolled version): We read the 10-bytes that define the pressed/not pressed status
   ;;
   ld    a, #0x40           ;; [2] A refers to the next keyboard line to be read (40h to 49h)
   ld   hl, #_cpct_keyboardStatusBuffer ;; [3] HL Points to the start of the keyboardBuffer, 
                                        ;; ... where scanned data will be stored

   ;; Read line 40h
   ld    b, d               ;; [1] B = F6h => Write the value of A to PPI's Port C to select next Matrix Line
   out (c), a               ;; [4] 
   ld    b, e               ;; [1] B = F4h => Read from PPI's Port A: Pressed/Not Pressed Values from PSG
   ini                      ;; [5] The read value is written to (HL), then HL<-HL+1 and B<-B-1
   inc   a                  ;; [1] Loop: Increment A => Next Matrix Line. 

   ;; Read line 41h
   ld    b, d               ;; [1] Same for line 41h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]

   ;; Read line 42h
   ld    b, d               ;; [1] Same for line 42h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]

   ;; Read line 43h
   ld    b, d               ;; [1] Same for line 43h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]

   ;; Read line 44h
   ld    b, d               ;; [1] Same for line 44h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]
 
   ;; Read line 45h
   ld    b, d               ;; [1] Same for line 45h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]

   ;; Read line 46h
   ld    b, d               ;; [1] Same for line 46h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]

   ;; Read line 47h
   ld    b, d               ;; [1] Same for line 47h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]

   ;; Read line 48h
   ld    b, d               ;; [1] Same for line 48h
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]
   inc   a                  ;; [1]

   ;; Read line 49h
   ld    b, d               ;; [1] Same for line 49h (Except we don't have to increase a anymore)
   out (c), a               ;; [4] 
   ld    b, e               ;; [1]
   ini                      ;; [5]

   ;; Restore PPI status to Port A=Output, Port C=Output
   ;;
   ld  bc,  #0xF782         ;; [3] Put again PPI in Output/Output mode for Ports A/C.
   out (c), c               ;; [4]

   ei                       ;; [1] Re-enable interrupts

   ret                      ;; [3] Return
