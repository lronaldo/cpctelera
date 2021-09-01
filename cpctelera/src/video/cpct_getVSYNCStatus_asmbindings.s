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
;; ASM bindings for <cpct_getVSYNCStatus_asm>
;;
cpct_getVSYNCStatus_asm::  ;; Assembly entry point
   
   ;; Include common code between C and ASM
   .include "cpct_getVSYNCStatus.asm"

   rra                     ;; [1] Move Bit 0 of A (VSYNC Status) to the Carry
                           ;;     So that C=VSYNC active, NC=VSYNC inactive
   ret                     ;; [3] Return to caller
   
