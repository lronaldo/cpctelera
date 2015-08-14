;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;### MODULE: SetVideoMode                                          ###
;#####################################################################
;### Routines to establish and control video modes                 ###
;#####################################################################
;
.module cpct_video

.include /videomode.s/
;;
;; C bindings for <cpct_setPALColour>
;;
;;   9 us, 2 bytes
;;
_cpct_setPALColour::
   ;; Get parameters from the stack, with __z88dk_callee convention
   pop   hl                ;; [3] HL = Return Address
   ex  (sp), hl            ;; [6] H = hw_ink, L = pen, Return address left at the top of the stack

.include /cpct_setPALColour.asm/
