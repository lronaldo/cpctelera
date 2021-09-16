;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;  Copyright (C) 2021 Nestornillo (https://github.com/nestornillo)
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

;;  cpctm_createInterruptHandlerWrapper_asm
;;    Macro that creates a custom interrupt handler wrapper function.
;;    See file firmware_macros.h.s for help.

.mdelete cpct_checkReg_
.macro cpct_checkReg_
.endm

.mdelete cpct_checkReg_alt
.macro cpct_checkReg_alt
  .equ cpct_altDetected, 1
.endm

.mdelete cpct_checkReg_af
.macro cpct_checkReg_af
  .if cpct_altDetected
    .equ cpct_altAFdetected, 1
  .endif
.endm

.mdelete cpct_checkReg_bc
.macro cpct_checkReg_bc
  .if cpct_altDetected
    .equ cpct_altBCDEHLdetected, 1
  .endif
.endm

.mdelete cpct_checkReg_de
.macro cpct_checkReg_de
  .if cpct_altDetected
    .equ cpct_altBCDEHLdetected, 1
  .endif
.endm

.mdelete cpct_checkReg_hl
.macro cpct_checkReg_hl
  .if cpct_altDetected
    .equ cpct_altBCDEHLdetected, 1
  .endif
.endm

.mdelete cpct_checkReg_ix
.macro cpct_checkReg_ix
.endm

.mdelete cpct_checkReg_iy
.macro cpct_checkReg_iy
.endm

.mdelete cpct_saveReg_
.macro cpct_saveReg_
.endm

.mdelete cpct_saveReg_alt
.macro cpct_saveReg_alt
  .if cpct_altAFdetected
    ex af, af' ;; [1]
  .endif
  .if cpct_altBCDEHLdetected
    exx        ;; [1]
  .endif
.endm

.mdelete cpct_saveReg_af
.macro cpct_saveReg_af
  push af      ;; [4]
.endm

.mdelete cpct_saveReg_bc
.macro cpct_saveReg_bc
  push bc      ;; [4]
.endm

.mdelete cpct_saveReg_de
.macro cpct_saveReg_de
  push de      ;; [4]
.endm

.mdelete cpct_saveReg_hl
.macro cpct_saveReg_hl
  push hl      ;; [4]
.endm

.mdelete cpct_saveReg_ix
.macro cpct_saveReg_ix
  push ix      ;; [5]
.endm

.mdelete cpct_saveReg_iy
.macro cpct_saveReg_iy
  push iy      ;; [5]
.endm

.mdelete cpct_restoreReg_
.macro cpct_restoreReg_
.endm

.mdelete cpct_restoreReg_alt
.macro cpct_restoreReg_alt
  .if cpct_altBCDEHLdetected
    exx        ;; [1]
  .endif
  .if cpct_altAFdetected
    ex af, af' ;; [1]
  .endif
.endm

.mdelete cpct_restoreReg_af
.macro cpct_restoreReg_af
  pop af       ;; [3]
.endm

.mdelete cpct_restoreReg_bc
.macro cpct_restoreReg_bc
  pop bc       ;; [3]
.endm

.mdelete cpct_restoreReg_de
.macro cpct_restoreReg_de
  pop de       ;; [3]
.endm

.mdelete cpct_restoreReg_hl
.macro cpct_restoreReg_hl
  pop hl       ;; [3]
.endm

.mdelete cpct_restoreReg_ix
.macro cpct_restoreReg_ix
  pop ix       ;; [4]
.endm

.mdelete cpct_restoreReg_iy
.macro cpct_restoreReg_iy
  pop iy       ;; [4]
.endm

.macro cpctm_createInterruptHandlerWrapper_asm WrapperName, intHandAddress, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11
  .equ cpct_altAFdetected, 0
  .equ cpct_altBCDEHLdetected, 0
  .equ cpct_altDetected, 0
  cpct_checkReg_'R1
  cpct_checkReg_'R2
  cpct_checkReg_'R3
  cpct_checkReg_'R4
  cpct_checkReg_'R5
  cpct_checkReg_'R6
  cpct_checkReg_'R7
  cpct_checkReg_'R8
  cpct_checkReg_'R9
  cpct_checkReg_'R10
  cpct_checkReg_'R11

  WrapperName::
  _'WrapperName::

  cpct_saveReg_'R1
  cpct_saveReg_'R2
  cpct_saveReg_'R3
  cpct_saveReg_'R4
  cpct_saveReg_'R5
  cpct_saveReg_'R6
  cpct_saveReg_'R7
  cpct_saveReg_'R8
  cpct_saveReg_'R9
  cpct_saveReg_'R10
  cpct_saveReg_'R11

  call #intHandAddress ;; [5] Call Interrupt Handler

  cpct_restoreReg_'R11
  cpct_restoreReg_'R10
  cpct_restoreReg_'R9
  cpct_restoreReg_'R8
  cpct_restoreReg_'R7
  cpct_restoreReg_'R6
  cpct_restoreReg_'R5
  cpct_restoreReg_'R4
  cpct_restoreReg_'R3
  cpct_restoreReg_'R2
  cpct_restoreReg_'R1
  
  ei         ;; [1] Reenable interrupts
  reti       ;; [4] Return to main program
.endm
