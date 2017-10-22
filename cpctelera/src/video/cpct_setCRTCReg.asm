;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;#####################################################################
;### MODULE: SetVideoMode                                          ###
;#####################################################################
;### Routines to establish and control video modes                 ###
;#####################################################################
;
.module cpct_video

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Function: cpct_setCRTCReg
;;
;;   Sets a new value for a given CRTC register
;;
;; C Definition:
;;    void <cpct_setCRTCReg> (<u8> *regnum*, <u8> *newval*) __z88dk_callee
;;
;; Input Parameters (2 Bytes):
;;    (1B B) newval - New value to be set for the register
;;    (1B C) regnum - Number of the register to be set 
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setCRTCReg_asm
;;
;; Parameter Restrictions:
;;    - *regnum* has to be a regiter number from 0 to 31
;;    - *newval* can be any 8-bits value. Its valid range depends on the selected 
;;             register. Each register has a different set of valid values.
;;
;; Known issues:
;;   * Using values out of range have unpredicted behaviour and can even 
;; potentially cause damage to real Amstrad CPC monitors. Please, use with care.
;;
;; Details:
;;    This function sets a new value for a given CRTC register. To do so, two 
;; commands have to be sent to two different ports of the CRTC: 
;;    - Select Register (Port 0xBC) 
;;    - Write Register (Port 0xBD)
;;
;;    So, this function just gets the two parameters given by the user and sends
;; them to these two ports in sequence, in order to set a new value for the 
;; given register.
;;
;;    It is important to note that the function does not perform any kind of check.
;; You must be careful not to select inappropriate registers or write out-of-range
;; values to any register. In fact, as CRTC registers control the way the image
;; is produced by the monitor, there is always a risk of damaging a real monitor
;; due to inappropriate values. Use this feature with care.
;;
;; Destroyed Register values:
;;    AF, BC
;;
;; Required memory:
;;     C-bindings - 14 bytes
;;   ASM-bindings - 11 bytes
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles
;; ------------------------------------------
;;     Any     |      27        |    108
;; ------------------------------------------
;;  ASM-Saving |     -10        |    -40
;; ------------------------------------------
;; (end code)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

   ;; Select CRTC the register to be modified
   ld     a, b                ;; [1] A = Value to be set (Save for later use)
   ld     b, #CRTC_SELECTREG  ;; [2] B = 0xBC CRTC Select Register, C = register number to be selected
   out  (c), c                ;; [4] Select register
   ;; Set the new value for the CRTC Register
   ld     c, a                ;; [1] C = Value to be set (which was previously saved into A)
   ld     b, #CRTC_SETVAL     ;; [2] B = 0xBD CRTC Set Register Value
   out  (c), c                ;; [4] Set the value

   ret                        ;; [3] Return to caller