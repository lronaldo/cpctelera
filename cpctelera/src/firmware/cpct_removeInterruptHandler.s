;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_firmware

.include /firmware.s/

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_removeInterruptHandler
;;
;;    Sets nothing as interrupt handler (returns every time it is called). It
;; returns previous interrupt handler for restoring purposes.
;;
;; C Definition:
;;    u16 <cpct_removeInterruptHandler> ()
;;
;; Assembly call:
;;    > call cpct_removeInterruptHandler_asm
;;
;; Return value:
;;   (u16, HL) - Previous interrupt vector code located at 0x0038.
;;
;; Details:
;;    This function sets Interrupt Mode to 1 (call 0x0038 every time a maskable 
;; interrupt happens) and then writes assembly instructions EI : RET at 0x0038.
;; This makes the CPU directly return every time an interrupt creates a jump
;; to 0x0038 (6 times per frame, 300 times per second).
;;    This function retrives previous interrupt handler before setting it. This
;; previous interrupt handler is returned. You may store it as a 16-bits value
;; and use it again to restore it later on if you wanted.
;;    Modifying the interrupt vector, this function also disables firmware, as
;; firmware will not be called again. Check <cpct_disableFirmware> if you wanted
;; to know more about the utility of disabling firmware.
;;
;; Destroyed Register values: 
;;    HL
;;
;; Required memory:
;;    16 bytes
;;
;; Time Measures:
;; (start code)
;; Case | microSecs (us) | CPU Cycles
;; -------------------------------------
;; Any  |     20         |     80
;; -------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_removeInterruptHandler::
cpct_removeInterruptHandler_asm::    ;; Assembly entry point
   di                         ;; [1] Disable interrupts
   im    1                    ;; [2] Set Interrupt Mode 1 (CPU will jump to 0x38 when a interrupt occurs)
   ld hl, (firmware_RST_jp)   ;; [3] Save previous interrupt code
   ex de, hl                  ;; [1] DE = previous interrupt code
   ld hl, #0xC9FB             ;; [3] HL = FB C9 = EI : RET (take into account little endian)
   ld (firmware_RST_jp), hl   ;; [5] Set interrupt handler to EI:RET, so that it does nothing
   ei                         ;; [1] Enable interrupts again
   ex de, hl                  ;; [1] HL = previous interrupt code (return value)

   ret                        ;; [3] Return
