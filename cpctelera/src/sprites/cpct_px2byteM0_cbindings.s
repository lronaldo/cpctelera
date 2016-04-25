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

;;
;; C bindings for <cpct_px2byteM0>
;;
;;   9 us, 2 bytes
;;
_cpct_px2byteM0::
   ;; Recover parameters from the stack
   pop hl           ;; [3] HL = Return Address
   ex (sp), hl      ;; [6] HL => L = Pixel, H = Pixel 1, 1st and 2nd 8-bit parameter
                    ;; ... also putting again Return Address where SP is located now
                    ;; ... as this function is using __z88dk_callee convention
.include /cpct_px2byteM0.asm/