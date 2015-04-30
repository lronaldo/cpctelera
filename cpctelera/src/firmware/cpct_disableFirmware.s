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
;#####################################################################
;### MODULE: Firmware and ROM routines                             ###
;#####################################################################
;### Routines to disable CPC Firmware and reenable it when needed, ###
;### and managing Upper and Lower ROMs.                            ###
;#####################################################################
;
.module cpct_firmware

.include /firmware.s/

;; 1-byte storage for MODE, INT. GENERATOR and ROM status (with default value)
_cpct_mode_rom_status:: .db #0x9D

;; 2-byte storage for firmware address
_cpct_firmware_address:: .dw 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_disableFirmware
;;
;;    Disables Amstrad CPC firmware, preventing it from being executed at every
;; CPU interrupt.
;;
;; C Definition:
;;    void <cpct_disableFirmware> ()
;;
;; Details:
;;    Disables the firmware modifying the interrupt vector at memory location 
;; 0x38. Normally, firmware routines are called and executed at every interrupt
;; and the ROM entry point is stored at 0x38 (a 16-bit address where the 
;; firmware code starts). This function substitutes the 2 bytes located at 
;; 0x38 by 0xC9FB, (0xFB = EI, 0xC9 = RET), which basically does nothing 
;; at each interruption (restores interrupts and returns).
;;
;;    Before inserting 0xC9FB at 0x38, the 2 bytes lying there at saved in
;; <cpct_firmware_address> variable. This permits restoring firmware function
;; copying this bytes again to 0x38, as <cpct_reenableFirmware> does.
;;
;;    Disabling the firmware is useful for several reasons:
;;    - Firmware code gets executed 6 times per frame (1 at each interrupt) when 
;; is active. If you turn it off, you win CPU clock cycles for your program, 
;; because firmware will not execute anymore.
;;    - Most of CPCtelera's functions talk directly to hardware, no to firmware.
;; They are faster, but they do not change firmware variables. As a result, 
;; firmware can revert changes made by CPCtelera's functions when active. For 
;; instance, if you change to video mode 0 using CPCtelera's functions, firmware
;; will see that the video mode is different from what it should be (attending 
;; to its own variables) and will change it again to 1. This happens with video
;; modes and palette values mainly.
;;    - Also, firmware uses part of the RAM to store its variables (from 
;; 0xA6FC to 0xBFFF). If you accidentaly overwrite anything there with firmware
;; being active, unpredictable things will happen, even hanging the computer.
;; Moreover, disabling firmware lets you use this part of the RAM for your 
;; own uses.
;;
;; Destroyed Register values: 
;;    HL
;;
;; Required memory:
;;    20 bytes
;;
;; Time Measures:
;; (start code)
;; Case | Cycles | microSecs (us)
;; -------------------------------
;; Any  |   84   |   21
;; -------------------------------
;; (end code)
;;
;; Credits:                                                       
;;    This function was coded copying and modifying cpc_disableFirmware 
;; from CPCRSLib by Raul Simarro.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_disableFirmware::
   di                         ;; [ 4] Disable interrupts
   ld   hl, (firmware_RST_jp) ;; [16] Obtain firmware ROM code pointer and store it for restoring it later
   ld (_cpct_firmware_address),hl ;; [16]

   im    1                    ;; [ 8] Set Interrupt Mode 1 (CPU will jump to 0x38 when a interrupt occurs)
   ld   hl, #0xC9FB           ;; [10] FB C9 (take into account little endian) => EI : RET

   ld (firmware_RST_jp), hl   ;; [16] Setup new "interrupt handler" and enable interrupts again
   ei                         ;; [ 4] 

   ret                        ;; [10] Return
