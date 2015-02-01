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

;;
;; Constant values
;;
.equ firmware_RST_jp, 0x38  ;; Memory address were a jump (jp) to the firmware code is stored.
.equ GA_port_byte,    0x7F  ;; 8-bit Port of the Gate Array

;;
;; Global symbols
;;

;;================== gfw_mode_rom_status ===================================
;; Store the last selection of MODE, INT.GENERATOR and ROM status 
;;  Default: 0x9C = (10011100) == (GGGIRRnn)
;;  GGG=Command for video mode and ROM selection (100)
;;  I=Interrupt Generation Enabled (1)
;;  RR=Reading from Lower and Upper ROM Disabled (11) (a 0 value means ROM enabled)
;;  nn=Video Mode 1 (01)
.globl gfw_mode_rom_status 
gfw_mode_rom_status: .db #0x9D


;
;########################################################################
;### FUNCTION: _cpct_disableFirmware                                  ###
;########################################################################
;###  Disables the firmware modifying the interrupt vector at 0x38.   ###
;### Normally, firmware routines are called and executed at each      ###
;### interrupt and the ROM entry point is stored at 0x38.             ###
;### This function substitutes the 2 bytes located at 0x38 by 0xC9FB, ###
;### (FB = EI, C9 = RET), which basically does nothing at each        ###
;### interruption.                                                    ###
;########################################################################
;### INPUTS (none)                                                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values:HL                                    ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  84 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying                   ###
;### cpc_disableFirmware from cpcrslib by Raul Simarro.               ###
;########################################################################
;
.globl _cpct_disableFirmware
_cpct_disableFirmware::
   DI                         ;; [ 4c] Disable interrupts
   LD HL,(firmware_RST_jp)    ;; [16c] Obtain firmware ROM code pointer and store it for restoring it later
   LD (firmware_address),HL   ;; [16c]

   IM 1                       ;; [ 8c] Set Interrupt Mode 1 (CPU will jump to &0038 when a interrupt occurs)
   LD HL,#0xC9FB              ;; [10c] FB C9 (take into account little endian) => EI : RET

   LD (firmware_RST_jp), HL   ;; [16c] Setup new "interrupt handler" and enable interrupts again
   EI                         ;; [ 4c] 

   RET                        ;; [10c]

;; Reserve two bytes (a word) to store the address where ROM routines start
;; This will be used by functions _cpct_disableFirmware and _cpct_reenableFirmware
firmware_address: .DW 0

;
;########################################################################
;### FUNCTION: _cpct_reenableFirmware                                 ###
;########################################################################
;###   Restores the ROM address where the firmware routines start.    ###
;### Beware: cpct_disableFirmware has to be called before calling     ###
;### this routine; otherwise, it would put a 0 in 0x38 and a reset    ###
;### will be performed at the next interrupt.                         ###
;########################################################################
;### INPUTS (none)                                                    ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: HL                                   ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  50 cycles                                                       ###
;########################################################################
;### CREDITS:                                                         ###
;###  This function was coded copying and modifying                   ###
;### cpc_disableFirmware from cpcrslib by Raul Simarro.               ###
;########################################################################
;
.globl _cpct_reenableFirmware
_cpct_reenableFirmware::
   DI                         ;; [ 4c] Disable interrupts

   LD HL, (firmware_address)  ;; [16c] Restore previously saved pointer to ROM code
   LD (#0x38), HL             ;; [16c]

   EI                         ;; [ 4c] Reenable interrupts and return
   RET                        ;; [10c]

;
;########################################################################
;## FUNCTIONs: _cpct_enableLowerROM, _cpct_disableLowerROM,           ###
;##            _cpct_enableUpperROM, _cpct_disableUpperROM            ###
;########################################################################
;### This 4 functions enable/disable low or upper ROMs. By default,   ###
;### cpctelera sets both ROMS as disabled at the first event of video ###
;### mode change. Enabling one of them means changing the way the cpu ###
;### gets information from memory: on the places where ROM is enabled,###
;### cpu gets values from ROM whenever it tries to read from memory.  ###
;### If ROM is disabled, these memory reads get values from RAM. ROMs ###
;### are mapped in this address space:                                ###
;###  - Lower ROM: 0000h - 3FFFh                                      ###
;###  - Upper ROM: C000h - FFFFh                                      ###
;### CPU Requests to write to memory are always mapped to RAM, so     ###
;### there is no need to worry about that. Also, Gate Array always    ###
;### gets video memory values from RAM (it never reads from ROM), so  ###
;### enabling Upper ROM does not have any impact on the screen.       ###
;### WARNING 1: If the execution of your program if going through some###
;### of these 2 ROM spaces and you enable ROM, cpu will be unable to  ###
;### get machine code of your program, as it will start reading from  ###
;### ROM instead of RAM (where your program is placed). This will re- ###
;### sult in unexpected behaviour.                                    ###
;### WARNING 2: Enabling Lower ROM reenables Firmware JMP in interrupt###
;### mode one, as it reestablishes the values at 0x0038 mem.location. ###
;### If you have disabled the firmware by putting FB:C9 (EI:RET) at   ###
;### 0x0038 (as _cpct_disableFirmware does) you may experience unde-  ###
;### sired behaviour when next interrupts come. The most tipical is   ###
;### a return to MODE 1, as firmware may be unaware of your currently ###
;### selected MODE and will try to reestablish its own MODE. Take it  ###
;### into account and disable interrupts if you need.                 ###
;########################################################################
;### INPUTS (~)                                                       ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  86/96 cycles (22.5/24 us)                                       ###
;########################################################################
;
.globl _cpct_enableLowerROM
_cpct_enableLowerROM::
   LD  HL, #0xFBE6           ;; [10] HL = Machine Code. E6 FB = AND #0b11111011 = Reset Bit 3 (Enable Lower ROM, 0 = enabled)
   JP  mrs_modifyROMstatus   ;; [10] Jump to ROM-Modification Code

.globl _cpct_disableLowerROM
_cpct_disableLowerROM::
   LD  HL, #0x04F6           ;; [10] HL = Machine Code. F6 04 = OR #0b00000100 = Set Bit 3 (Disable Lower ROM, 0 = enabled)
   JP  mrs_modifyROMstatus   ;; [10] Jump to ROM-Modification Code

.globl _cpct_enableUpperROM
_cpct_enableUpperROM::
   LD  HL, #0xF7E6           ;; [10] HL = Machine Code. E6 F7 = OR #0b11110111 = Reset Bit 4 (Enable Upper ROM, 0 = enabled)
   JP  mrs_modifyROMstatus   ;; [10] Jump to ROM-Modification Code

.globl _cpct_disableUpperROM
_cpct_disableUpperROM::
   LD  HL, #0x08F6           ;; [10] HL = Machine Code. F6 08 = OR #0b00001000 = Set Bit 4 (Disable Upper ROM, 0 = enabled)
   ;JP  mrs_modifyROMstatus  ;; [10] Jump to ROM-Modification Code

mrs_modifyROMstatus::
   LD  (mrs_operation), HL   ;; [16] Modify Machine Code that makes the operation (AND/OR) to set/reset ROM bits
   LD   HL, #gfw_mode_rom_status ;; [10] HL points to present MODE, INT.GEN and ROM selection byte.
   LD   A,  (HL)             ;; [ 7] A = mode_rom_status (present value)
mrs_operation:
   AND  #0b11111011          ;; [ 7] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   LD   B,  #GA_port_byte    ;; [ 7] B = Gate Array Port (0x7F)
   OUT (C), A                ;; [12] GA Command: Set Video Mode and ROM status (100)

   LD (HL), A                ;; [ 7] Save new Mode and ROM status for later use if required

   RET                       ;; [10] Return


