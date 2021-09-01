;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2001? Kevin Thacker / Arnoldemu (http://www.cpctech.org.uk/)
;;  Copyright (C) 2021  Nestornillo (https://github.com/nestornillo)
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Every byte read from the port has this information,
;;
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
;;
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
;; Best  |         18     |       72
;; -------------------------------------
;; Worst |      19984     |    79936
;; -------------------------------------
;; (end code)
;; 
;; NOTE:
;;  The function will take a time between Best and Worst cases depending on
;;  how far appart the raster is from VSYNC active region when called. Moreover,
;;  if CRTC is modified and VSYNC is shortened, the worst case could be expanded.
;;  However, this is usually unimportant, as the use of this function is 
;;  exactly to wait until the start of VSYNC for syncrhonization purposes and
;;  normally it should not frecuently be called. <cpct_waitVSYNC> may be used
;;  instead for normal waiting, or even synchronized interrupts.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; C/Assembly entry points
_cpct_waitVSYNCStart::        
cpct_waitVSYNCStart_asm::     
   
   ld     b, #PPI_PORT_B   ;; [2] B = F5h ==> B has the address of PPI Port B,
                           ;;     where we get information from VSYNC

;; First we look at VSYNC to see if it is active. If it is, we wait while
;; VSYNC continues to be active, so as to be sure that we are outside
;; VSYNC active region before continuing
while_active:
   in     a, (c)           ;; [4]   A = Status register got from PPI port B
   rra                     ;; [1]   Move bit 0 of A (VSYNC Status) to Carry
   jr     c, while_active  ;; [2/3] Loop While Carry On (VSYNC is active)

;; When execution reaches this point, we are out of VSYNC active region
;; Now we shall wait until we detect VSYNC active again, and that will
;; effectively be the start point of VSYNC active region.
while_inactive: 
   in     a, (c)           ;; [4]   A = Status register got from PPI port B
   rra                     ;; [1]   Move bit 0 of A (VSYNC Status) to Carry
   ret    c                ;; [2/4] If (Carry == 1) VSYNC is active, return immediately
   jr while_inactive       ;; [3] VSYNC is still inactive, wait for it to activate