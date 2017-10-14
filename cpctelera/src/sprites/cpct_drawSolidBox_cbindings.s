;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_sprites

;; Macros for easy use of undocumented opcodes
.include "macros/cpct_undocumentedOpcodes.h.s"

;;
;; C bindings for <cpct_drawSolidBox>
;;
;;   29 us, 14 bytes
;;
_cpct_drawSolidBox::
   ;; GET Parameters from the stack
   ld   hl, #2      ;; [3] HL Points to SP+2 (first 2 bytes are return address)
   add  hl, sp      ;; [3]    , to use it for getting parameters from stack
   ld    e, (hl)    ;; [2] DE = First Parameter (Video memory pointer)
   inc  hl          ;; [2]
   ld    d, (hl)    ;; [2]
   inc  hl          ;; [2]  / Copy first value to video memory (upper-left corner of the box)
   ldi              ;; [5] (HL)->(DE) Move 2nd parameter (1-byte Colour Pattern) directly into 1st byte of the box in memory
   ld    c, (hl)    ;; [2] C = Third Parameter (Box Width)
   inc  hl          ;; [2]
   ld    b, (hl)    ;; [2] B = Fourth Parameter (Box Height)

   ;; Prepare HL and DE pointers for copying bytes consecutively
   ld    h, d       ;; [1] HL = DE - 1 (HL Points to the first byte of the box, the one that contains the colour pattern)
   ld    l, e       ;; [1]
   dec   hl         ;; [2]

.include /cpct_drawSolidBox.asm/