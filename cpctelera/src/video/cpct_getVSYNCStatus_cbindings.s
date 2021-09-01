;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2001? Kevin Thacker / Arnoldemu (http://www.cpctech.org.uk/)
;;  Copyright (C) 2021  Nestornillo (https://github.com/nestornillo)
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
.module cpct_video

.include "video/videomode.h.s"

;;
;; C bindings for <cpct_getVSYNCStatus>
;;
_cpct_getVSYNCStatus::     ;; C entry point
   
   ;; Include common code between C and ASM
   .include "cpct_getVSYNCStatus.asm"
   
   ;; Put return value into L
   and   #0b00000001       ;; [2] A &= 1 // Leave only bit 0 (VSYNC status)
   ld    l, a              ;; [1] L = VSYNC Status (Return value). 1=active, 0=inactive
   ret                     ;; [3] Return to caller
   
