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
.module cpct_videomode

.include /videomode.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: void cpct_waitVSYNC()
;;
;; This function waits for the VSYNC signal from the CRTC. This
;; signal means that the monitor has finished drawing the last frame
;; and it is returning to the top left of the screen to start draw-
;; ing the next one. This is useful to synchronize routines to the
;; 50Hz drawing display.
;; 
;; This function reads bytes from PPI Port B, whose bits mean...
;; (start code)
;; BIT  NAME     DESCRIPTION
;; ---------------------------------------------------------------- 
;; 7    CAS.IN   Cassette data input
;; 6    RN.BUSY  Parallel/Printer port signal, "0"=ready,"1"= Not R.
;; 5    /EXP     Expansion Port /EXP pin
;; 4    LK4      Screen Refresh Rate ("1"=50Hz, "0"=60Hz)
;; 3    LK3      3bit Distributor ID. Usually set to 4=Awa,
;; 2    LK2        5=Schneider, or 7=Amstrad, see LK-selectable
;; 1    LK1        Brand Names for details.
;; 0    CRTC     Vertical Sync ("1"=VSYNC active, "0"=inactive)
;; (end)
;;
;; Destroyed Register values: 
;;   AF, BC
;;
;; Required memory:
;;   8 bytes
;;
;; Time Measures:
;; (start code)
;; Case  | Cycles    | microSecs (us)
;; ----------------------------------
;; Best  | 43        |  10,75
;; Worst | 17 + 26*L |  xx,xx
;; (end code)
;;
;; NOTE:
;;  As this is an active wait loop, it does not actually mind
;;  at all the time needed to process. It will vary depending
;;  on how much time has passed since the last VSYNC.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_waitVSYNC::
   ld  b, #PPI_PORT_B;; [ 7] B = F5h ==> B has the address of PPI Port B, where we get information from VSYNC

wvs_wait:
   in  a,(c)         ;; [12] A = Status register got from PPI port B
   rra               ;; [ 4] Move bit 0 of A to Carry (bit 0 contains VSYNC status)
   jr  nc, wvs_wait  ;; [10] No Carry means No VSYNC, so loop While No Carry

   ret               ;; [10] Carry Set, VSYNC Active, Return