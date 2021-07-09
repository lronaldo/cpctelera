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
;; Function: cpct_getVSYNCActive
;;
;;    Checks if the vertical synchronization signal (VSYNC) is active or not and returns.
;;
;; C Definition:
;;    <u8> <cpct_getVSYNCActive> ()
;;
;; Assembly call:
;;    > call cpct_getVSYNCActive_asm
;;
;; Return Value:
;;    (1B L) - 0 if VSYNC not active, 1 if VSYNC is active.
;;
;; Details:
;;    This function gets the VSYNC signal from the CRTC. It will return 0 if VSYNC is not active,
;; and 1 if it is active.
;;
;;    This function can be used within an interrupt handler as a method to know which interrupt has
;; occurred and can be used for synchronising interrupts.
;; 
;;    To detect VSYNC signal status, function reads a byte from PPI Port B. That byte contains 
;; this information:
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
;;    So, checking the bit 0 of a byte coming from PPI Port B tells us if
;; VSYNC is active or not.
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
;; Any   |       12       |      48
;; (end code)
;;
;; Credits:
;;    This function was coded based on code by Kevin Thacker.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_getVSYNCActive::     ;; C entry point
cpct_getVSYNCActive_asm::  ;; Assembly entry point
   ld  b, #PPI_PORT_B      ;; [2] B = F5h ==> B has the address of PPI Port B, where we get information from VSYNC
   in a,(c)                ;; [4] A = Status register got from PPI port B
   and #1                  ;; [2] Leave only bit 0 (VSYNC status)
   ld l,a                  ;; [1] L=0 if VSYNC is not active, L=1 if VSYNC is active

   ret                     ;; [3] Return to caller
   
