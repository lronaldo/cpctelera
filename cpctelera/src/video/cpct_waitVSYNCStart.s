;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_waitVSYNCStart
;;
;;    Waits until the start of the vertical synchronization signal (VSYNC) and returns.
;;
;; C Definition:
;;    void <cpct_waitVSYNCStart> ()
;;
;; Assembly call:
;;    > call cpct_waitVSYNCStart_asm
;;
;; Details:
;;    This function implements a wait loop that will return only when the start
;; of the VSYNC signal from the CRTC is detected. This signal means that the monitor 
;; has finished drawing the last frame and it is returning to the top left of the screen 
;; to start drawing the next one. This is useful to synchronize routines with the
;; 50Hz drawing display.
;; 
;;    To detect VSYNC signal status, function reads bytes from PPI Port B. 
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
;;    So, checking the bit 0 of a byte coming from PPI Port B tells us if VSYNC
;; is active or not. That signal is active during a small period of time, but 
;; for synchronization purposes it is normally very important to detect exactly
;; when VSYNC signal starts. In order to achieve this, this function first waits
;; for VSYNC being inactive, and then waits for VSYNC being active. That ensures
;; that the function will return at the start of the VSYNC signal.
;;
;;    This function is optimized for size (instead of speed) as it is an active
;; wait, and does not make sense making it faster. It will stop when VSYNC start is
;; detected no matter how fast the code is.
;;
;; Destroyed Register values: 
;;   AF, BC
;;
;; Required memory:
;;   13 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | microSecs (us) | CPU Cycles
;; -------------------------------------
;; Best  |         19     |       76
;; -------------------------------------
;; Worst |      19984     |    79936
;; -------------------------------------
;; (end code)
;; 
;; NOTE:
;;  As this function is an active wait, it does not actually mind
;;  at all the time needed to process. It will vary depending on
;;  how much time has passed since the last VSYNC start.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_waitVSYNCStart::         ;; C entry point
cpct_waitVSYNCStart_asm::      ;; Assembly entry point
   ld  b, #PPI_PORT_B          ;; [2] B = F5h ==> B has the address of PPI Port B, where we get information from VSYNC

wait_for_vs_inactive:
   in  a,(c)                   ;; [4] A = Status register got from PPI port B
   rra                         ;; [1] Move bit 0 of A to Carry (bit 0 contains VSYNC status)
   jr  c, wait_for_vs_inactive ;; [2/3]  Carry means VSYNC is active, so loop While Carry
   
wait_for_vs_active: 
   in  a,(c)                   ;; [4] A = Status register got from PPI port B
   rra                         ;; [1] Move bit 0 of A to Carry (bit 0 contains VSYNC status)
   jr  nc, wait_for_vs_active  ;; [2/3] No Carry means No VSYNC, so loop While No Carry

   ret                         ;; [3] Start of VSYNC, Return
