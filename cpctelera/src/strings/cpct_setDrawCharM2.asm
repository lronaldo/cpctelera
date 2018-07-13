;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_strings

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setDrawCharM2
;;
;;    Sets foreground and background colours that will be used by <cpct_drawCharM2_inner>
;; when called. <cpct_drawCharM2_inner> is used by both <cpct_drawCharM2> and <cpct_drawStringM2>.
;;
;; C Definition:
;;    void <cpct_setDrawCharM2> (<u8> *fg_pen*, <u8> *bg_pen*) __z88dk_callee
;;
;; Input Parameters (2 Bytes):
;;  (1B L )  fg_pen       - Foreground palette colour index (Similar to BASIC's PEN, 0-1)
;;  (1B H )  bg_pen       - Background palette colour index (PEN, 0-1)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setDrawCharM2_asm
;;
;; Parameter Restrictions:
;;  * *fg_pen* will be interpreted as [0-1]. Values greater than 1 will be interpreted
;; as 1 if they are odd, and as 0 if they are even.
;;  * *bg_pen* will be interpreted as [0-1]. Values greater than 1 will be interpreted
;; as 1 if they are odd, and as 0 if they are even.
;;
;; Requirements and limitations:
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function sets the internal values of <cpct_drawCharM2_inner_asm>, so that next calls
;; to <cpct_drawCharM2> or <cpct_drawStringM2> will use these new values. Concretely, these
;; values are *fg_pen* (foreground colour) and *bg_pen* (background colour). This function
;; receives 2 colour values in the range [0-1] and transforms them into the 4 possible 
;; combinations. Namely
;; (start code)
;;    -----------
;;    | FG / FG | > 8x8 pixels shown as foreground colour (a square)
;;    | FG / BG | > Character displayed in foreground colour
;;    | BG / FG | > Character displayed in background colour (inverse video)
;;    | BG / BG | > 8x8 pixels shown as background colour (a square)
;;    -----------
;; (end code)
;;
;;    Selected combination is transformed into the concrete bytes of code that perform this drawing
;; and inserted into <cpct_drawCharM2_inner_asm>, in the address named <cpct_charm2imc> (drawCharM2 
;; inner modifiable code). This modification remains constant unless a new call to <cpct_setDrawCharM2> 
;; changes it. This lets the user change colours once and use them many times for subsequent calls 
;; to draw functions.
;;
;;    The appropriate use of this function is to call it each time a new pair of colours is
;; required for following character drawings to be made either with <cpct_drawCharM2> or with
;; <cpct_drawStringM2>. If the same colours are to be used for many drawings, a single call to 
;; <cpct_setDrawCharM2> followed by many drawing calls would be the best practice. 
;;
;;    You may found use examples consulting documentation for <cpct_drawCharM2> and <cpct_drawStringM2>.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    ASM bindings  - 30 bytes (+38 from <cpct_drawCharM2_inner_asm> = 68 bytes)
;;      C bindings  - 32 bytes (+38 from <cpct_drawCharM2_inner_asm> = 70 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles
;; -------------------------------------------
;;   Any      |      39        |     156
;; -------------------------------------------
;; Asm saving |      -9        |     -36
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Global symbols
;;
.globl cpct_charm2imc  ;; charM2 inner modifiable code section address

   ;; Calculate offset inside the MC table where the machine code to be copied starts
   ld     a, l          ;; [1] | Make the 6th bit of A hold the fg_pen colour selected
   rlca                 ;; [1] \
   add    h             ;; [1] 7th bit of A = bg_pen colour selected
   rlca                 ;; [1] Multiply result by 2, because each option takes 2 bytes
   and   #0x07          ;; [2] Remove upper bits of A to ensure parameter errors do not make the program crash

   ld    hl, #mc_table  ;; [3] HL Points to the start of the Machine Code Table
   add_hl_a             ;; [5] Add offset to the start of mc_table to point to the start of machine code to be copied
                        ;; Now HL points to the machine code bytes to be copied

   ld    de, #cpct_charm2imc  ;; [3] DE = Pointer to the start drawM2 inner modifiable code section

   ;; Copy Selected Machine Code to cpct_drawCharM2_inner_asm
   ldi                  ;; [5] Copy first byte
   ldi                  ;; [5] Copy second byte

   ;; Everything set up. Now return
   ret                  ;; [3] Return

;; Machine Code table with the Z80 code for each one of the 4 alternatives, 2 bytes per alternative
mc_table:
.db #0x3E, #0x00  ;; Option 00: BG/BG >> LD A, #00
.db #0x1A, #0x2F  ;; Option 01: BG/FG >> LD A, (DE) : CPL
.db #0x1A, #0x00  ;; Option 10: FG/BG >> LD A, (DE) : NOP
.db #0x3E, #0xFF  ;; Option 11: FG/FG >> LD A, #FF


