;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2001? Kevin Thacker / Arnoldemu (http://www.cpctech.org.uk/)
;;  Copyright (C) 2021  Nestornillo (https://github.com/nestornillo)
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
   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_getVSYNCStatus
;;
;;    Check if the vertical synchronization signal (VSYNC) is active or not.
;;
;; C Definition:
;;    <u8> <cpct_getVSYNCStatus> ()
;;
;; Assembly call:
;;    > call cpct_getVSYNCStatus_asm
;;
;; Return Value (C-bindings):
;;    (1B) - VSYNC Status: 1 active, 0 inactive
;;    ASM details; Registers A, L contain return value. Flags; NZ active, Z inactive.
;;
;; Return Value (ASM-bindings):
;;    Flag C - VSYNC Status: C active, NC inactive
;;
;; Details:
;;    This function reads the status of VSYNC signal from the CRTC. Returns 1 or 0
;; (true or false) depending on whether VSYNC is active or not.
;;
;;    This function can be used for raster synchronization purposes. For instance, within 
;; an interrupt handler it could be used to know if last interrupt occurred during VSYNC or
;; not, which could be useful to number interrupts and synchronize them.
;; 
;;    To detect VSYNC signal status, this function reads a byte from PPI Port B. That byte 
;; contains the following information,
;;
;; (start code)
;; BIT  NAME     DESCRIPTION
;; ---------------------------------------------------------------- 
;; 7    CAS.IN   Cassette data input
;; 6    RN.BUSY  Parallel/Printer port signal, "0"=ready,"1"=Not Ready
;; 5    /EXP     Expansion Port /EXP pin
;; 4    LK4      Screen Refresh Rate ("1"=50Hz, "0"=60Hz)
;; 3    LK3      3bit Distributor ID. Usually set to 4=Awa,
;; 2    LK2        5=Schneider, or 7=Amstrad, see LK-selectable
;; 1    LK1        Brand Names for details.
;; 0    CRTC     Vertical Sync ("1"=VSYNC active, "0"=inactive)
;; (end)
;;
;;    So, by checking bit 0 of that byte, the function knows whether VSYNC is active or not.
;;
;; Destroyed Register values: 
;;    C-bindings   - AF, BC, L
;;    ASM-bindings - AF, BC
;;
;; Required memory:
;;    C-bindings   - 8 bytes
;;    ASM-bindings - 6 bytes
;;
;; Time Measures:
;; (start code)
;;     Case    | microSecs (us) | CPU Cycles |
;; -------------------------------------------
;;     Any     |       12       |      48    |
;; -------------------------------------------
;;  Asm Saving |       -2       |      -8    |
;; -------------------------------------------
;; (end code)
;;
;; Credits:
;;    This code is based on the original one by Kevin Thacker.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ld     b, #PPI_PORT_B   ;; [2] B = F5h ==> B has the address of PPI Port B, where we get information from VSYNC
   in     a, (c)           ;; [4] A = Status register got from PPI port B

   ;; Return depends on C/ASM bindings
