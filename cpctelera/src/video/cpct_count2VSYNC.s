;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_videomode

.include /videomode.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_count2VSYNC
;;
;;    Waits until CRTC produces vertical synchronization signal (VSYNC), counting
;; the number of active wait loop cycles done.
;;
;; C Definition:
;;    <u16> <cpct_count2VSYNC> ()
;;
;; Return Value:
;;    <u16> - Total number of iterations done of the wait loop until VSYNC was active.
;;
;; Assembly call:
;;    > call cpct_coun2VSYNC_asm
;;
;; Details:
;;    This function implements a wait loop that exists only when VSYNC signal 
;; from the CRTC is detected. It works in the same way as <cpct_waitVSYNC>, but
;; with one addition: it keeps counting the number of iterations of the waiting
;; loop. This count is returned when VSYNC is detected and the function ends.
;; For more reference about VSYNC and this waiting loop, see <cpct_waitVSYNC>.
;;
;;    The main utility of counting loop iteration is to have an stimation of
;; free cycles in a main loop. If you are planning to make a game or application
;; running at 50/25/12.5 FPS, all your CPU operations must fit into the allowed
;; time for that. A way to know if you still have time to include more operations
;; is using this function during development phase to know how much time is 
;; still available between the end of your calculations and the VSYNC.
;;
;;    The value returned by this function refers to the number of loop iteration
;; waiting for VSYNC. If you wanted to know the total amount of CPU cycles, you
;; can do this calculation,
;; (start code)
;;    availableCycles = 22 + 34 * cpct_count2VSYNC(); 
;; (end)
;;
;; Destroyed Register values: 
;;    AF, HL, BC
;;
;; Required memory:
;;    12 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | Cycles    | microSecs (us)
;; ----------------------------------
;; Best  | 56        | 14,00
;; Any   | 22 + 34*L |  5,50 + 8,50*L
;; (end code)
;;    L=Number of times loop is executed
;;
;; NOTE:
;;  As this is an active wait loop, it does not actually mind at all the time 
;;  needed to process. It will vary depending on how much time has passed since 
;;  the last VSYNC.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_count2VSYNC::
cpct_count2VSYNC_asm::		;; Assembly entry point
   ld    b, #PPI_PORT_B;; [ 7] B = F5h ==> B has the address of PPI Port B, where we get information from VSYNC
   ld   hl, #0       ;; [10] HL=0 HL will be the loop iterations counter, as it is directly used as return value in C

wvs_wait:
   inc  hl           ;; [ 6] HL++ counting new iteration of the waiting loop
   in    a,(c)       ;; [12] A = Status register got from PPI port B
   rra               ;; [ 4] Move bit 0 of A to Carry (bit 0 contains VSYNC status)
   jr   nc, wvs_wait ;; [12/7] No Carry means No VSYNC, so loop While No Carry

   ret               ;; [10] Carry Set, VSYNC Active, Return