;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 Arnaud Bouche (@Arnaud6128)
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
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Variables: cpct_color_old, cpct_color_new
;;
;;    Contains the colors to be used by theses functions:
;;     * cpct_setReplaceColors
;;     * cpct_spriteColorizeM0
;;     * cpct_spriteColorizeM1
;;     * cpct_drawSpriteColorizeM0
;;     * cpct_drawSpriteColorizeM1
;;
;; C Definition:
;;    <u8> <cpct_color_old>;
;;    <u8> <cpct_color_new>;
;;
;; Known limitations:
;;    * These color are used for Mode0 (0 to 15) and Mode1 (0 to 3)
;;    * You may assing a value to this seed direcly from C or ASM. If directly accessed 
;; from ASM, do not forget to put an underscore in front (_cpct_color_old and _cpct_color_new).
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_color_old:: .db #00
_cpct_color_new:: .db #00