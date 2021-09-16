;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_video
   
.include /videomode.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_waitVSYNC
;;
;;    Waits until CRTC produces vertical synchronization signal (VSYNC) and returns.
;;
;; C Definition:
;;    void <cpct_waitVSYNC> ()
;;
;; Assembly call:
;;    > call cpct_waitVSYNC_asm
;;
;; Details:
;;    This function implements a wait loop that exists only when VSYNC signal 
;; from the CRTC is detected. This signal means that the monitor has finished 
;; drawing the last frame and it is returning to the top left of the screen 
;; to start drawing the next one. This is useful to synchronize routines with the
;; 50Hz drawing display.
;; 
;; To detect VSYNC signal status, function reads bytes from PPI Port B. 
;; Every byte read from the port has this information:
;; (start code)
;; BIT  NAME     DESCRIPTION
;; ---------------------------------------------------------------- 
;; 7    CAS.IN   Cassette data input
;; 6    RN.BUSY  Parallel/Printer port signal, "0"=ready,"1"= Not Ready
;; 5    /EXP     Expansion Port /EXP pin
;; 4    LK4      Screen Refresh Rate ("1"=50Hz, "0"=60Hz)
;; 3    LK3      3bit Distributor ID. Usually set to 4=Awa,
;; 2    LK2        5=Schneider, or 7=Amstrad, see LK-selectable
;; 1    LK1        Brand Names for details.
;; 0    CRTC     Vertical Sync ("1"=VSYNC active, "0"=inactive)
;; (end)
;; So, checking the bit 0 of a byte coming from PPI Port B tells us if
;; VSYNC is active or not.
;; This function is optimized for size (instead of speed) as it is a wait loop
;; and does not make sense making it faster. It will stop when VSYNC is detected
;; no matter how fast the loop is.
;;
;; Destroyed Register values: 
;;   AF, BC
;;
;; Required memory:
;;   8 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | microSecs (us) | CPU Cycles
;; ----------------------------------------
;; Best  |      12        |      48
;; ----------------------------------------
;; Any   |    4 + 8*L     |   16 + 32*L
;; ----------------------------------------
;; (end code)
;;    L=Number of times loop is executed
;;
;; NOTE:
;;  As this is an active wait loop, it does not actually mind
;;  at all the time needed to process. It will vary depending
;;  on how much time has passed since the last VSYNC.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_waitVSYNC::
cpct_waitVSYNC_asm::	;; Assembly entry point
   ld  b, #PPI_PORT_B;; [2] B = F5h ==> B has the address of PPI Port B, where we get information from VSYNC

wvs_wait:
   in  a,(c)         ;; [4] A = Status register got from PPI port B
   rra               ;; [1] Move bit 0 of A to Carry (bit 0 contains VSYNC status)
   jr  nc, wvs_wait  ;; [2/3] No Carry means No VSYNC, so loop While No Carry

   ret               ;; [3] Carry Set, VSYNC Active, Return
