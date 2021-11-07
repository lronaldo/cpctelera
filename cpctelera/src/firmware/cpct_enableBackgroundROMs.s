;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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

.module cpct_firmware

.include /firmware.s/
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_enableBackgroundROMs
;;
;;    Enables all background ROMs.
;;
;; C Definition:
;;    void <cpct_enableBackgroundROMs> ()
;;
;; Assembly call:
;;    > call cpct_enableBackgroundROMs_asm
;;
;; Requirements:
;;    This function requires the CPC *firmware* to be *ENABLED*, as it uses
;; firmware routines to work. 
;;
;; Details:
;;    This function enables all background ROMs (i.e.: disc controller). When a
;; program is executed using Basic's *RUN* command, background ROMs are disabled.
;; In order to read data from disc, background ROMs must be enabled again. This
;; function achieves this by calling to firmware routines that reset the program
;; status (mc_start program) and enable background ROMs (kl_rom_walk).
;;
;;    Those firmware routines reset the stack of the system, so it is very
;; important that cpct_enableBackgroundROMs is *called at the start of* '*main*'. 
;; And, also important, '*main*' should *not have local variables*, as they 
;; are stored in the stack. Be aware that some ROMs display messages on screen when
;; initialized, so you might need to clear the screen after calling this function.
;;
;; Known limitations:
;;    * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    30 bytes
;;
;; Time Measures:
;; Next table shows only time used by CPCtelera's code in cpct_enableBackgroundROMs. It does not
;; take into account time used by firmware functions mc_start_program and kl_rom_walk.
;; (start code)
;; Case | microSecs (us) | CPU Cycles
;; --------------------------------------
;; Any  |      43        |     172
;; --------------------------------------
;; (end code)
;;
;; Credits:                                                       
;;    This function was created based on code from Kevin Thacker.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_enableBackgroundROMs::
cpct_enableBackgroundROMs_asm::

   ;; Save Return Address
   pop hl                         ;; [3] HL = Return Address
   ld (return_to_program+1), hl   ;; [5] Save Return Adress in the code, as stack will be destroyed in firmware calls

   ;; store the drive number the loader was run from
   ld hl,(#firmware_disc_mem_ptr) ;; [5] Read memory location of disc memory area. First byte is the current drive
   ld a,(hl)                      ;; [2] Read current drive
   ld (restore_drive+1),a         ;; [4] Store current drive

   ;; Disable and reenable all background ROMs
   ld c,#0xFF                     ;; [2] Disable all roms
   ld hl, #continue_program       ;; [3] Execution address for mc_start_program
   jp firmware_mc_start_program   ;; [3] Run a foreground program
   continue_program:
   call firmware_kl_rom_walk      ;; [5] Enable all background ROMs

   ;; When AMSDOS is enabled, the drive reverts back to drive 0!
   ;; This will restore the drive number to the drive the program was run from
   ld hl,(#firmware_disc_mem_ptr) ;; [5] Read memory location of current drive
   restore_drive:
   ld (hl),#0x00                  ;; [3] Restore drive number to the drive the program was run from

   ;; Return to the caller of cpct_enableBackgroundROMs
   return_to_program:
   jp 0x0000                      ;; [3] Return to caller
