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
.module cpct_keyboard

;; bndry directive does not work when linking previously compiled files
;.bndry 16
;;   16-byte aligned in memory to let functions use 8-bit maths for pointing
;;   (alignment not working on user linking)

_cpct_keyboardStatusBuffer:: .ds 10

;;
;; Assembly entry points for functions
;;
.globl cpct_isKeyPressed_asm
.globl cpct_scanKeyboard_asm
.globl cpct_scanKeyboard_f_asm

;;
;; Assembly constant definitions for keyboard mapping
;;

;; Matrix Line 0x00
.equ Key_CursorUp     ,#0x0100  ;; Bit 0 (01h) => | 0000 0001 |
.equ Key_CursorRight  ,#0x0200  ;; Bit 1 (02h) => | 0000 0010 |
.equ Key_CursorDown   ,#0x0400  ;; Bit 2 (04h) => | 0000 0100 |
.equ Key_F9           ,#0x0800  ;; Bit 3 (08h) => | 0000 1000 |
.equ Key_F6           ,#0x1000  ;; Bit 4 (10h) => | 0001 0000 |
.equ Key_F3           ,#0x2000  ;; Bit 5 (20h) => | 0010 0000 |
.equ Key_Enter        ,#0x4000  ;; Bit 6 (40h) => | 0100 0000 |
.equ Key_FDot         ,#0x8000  ;; Bit 7 (80h) => | 1000 0000 |
;; Matrix Line 0x01
.equ Key_CursorLeft   ,#0x0101
.equ Key_Copy         ,#0x0201
.equ Key_F7           ,#0x0401
.equ Key_F8           ,#0x0801
.equ Key_F5           ,#0x1001
.equ Key_F1           ,#0x2001
.equ Key_F2           ,#0x4001
.equ Key_F0           ,#0x8001
;; Matrix Line 0x02
.equ Key_Clr          ,#0x0102
.equ Key_OpenBracket  ,#0x0202
.equ Key_Return       ,#0x0402
.equ Key_CloseBracket ,#0x0802
.equ Key_F4           ,#0x1002
.equ Key_Shift        ,#0x2002
.equ Key_BackSlash    ,#0x4002
.equ Key_Control      ,#0x8002
;; Matrix Line 0x03
.equ Key_Caret        ,#0x0103
.equ Key_Hyphen       ,#0x0203
.equ Key_At           ,#0x0403
.equ Key_P            ,#0x0803
.equ Key_SemiColon    ,#0x1003
.equ Key_Colon        ,#0x2003
.equ Key_Slash        ,#0x4003
.equ Key_Dot          ,#0x8003
;; Matrix Line 0x04
.equ Key_0            ,#0x0104
.equ Key_9            ,#0x0204
.equ Key_O            ,#0x0404
.equ Key_I            ,#0x0804
.equ Key_L            ,#0x1004
.equ Key_K            ,#0x2004
.equ Key_M            ,#0x4004
.equ Key_Comma        ,#0x8004
;; Matrix Line 0x05
.equ Key_8            ,#0x0105
.equ Key_7            ,#0x0205
.equ Key_U            ,#0x0405
.equ Key_Y            ,#0x0805
.equ Key_H            ,#0x1005
.equ Key_J            ,#0x2005
.equ Key_N            ,#0x4005
.equ Key_Space        ,#0x8005
;; Matrix Line 0x06
.equ Key_6            ,#0x0106
.equ Joy1_Up          ,#0x0106
.equ Key_5            ,#0x0206
.equ Joy1_Down        ,#0x0206
.equ Key_R            ,#0x0406
.equ Joy1_Left        ,#0x0406
.equ Key_T            ,#0x0806
.equ Joy1_Right       ,#0x0806
.equ Key_G            ,#0x1006
.equ Joy1_Fire2       ,#0x1006
.equ Key_F            ,#0x2006
.equ Joy1_Fire1       ,#0x2006
.equ Key_B            ,#0x4006
.equ Joy1_Fire3       ,#0x4006
.equ Key_V            ,#0x8006
;; Matrix Line 0x07
.equ Key_4            ,#0x0107
.equ Key_3            ,#0x0207
.equ Key_E            ,#0x0407
.equ Key_W            ,#0x0807
.equ Key_S            ,#0x1007
.equ Key_D            ,#0x2007
.equ Key_C            ,#0x4007
.equ Key_X            ,#0x8007
;; Matrix Line 0x08
.equ Key_1            ,#0x0108
.equ Key_2            ,#0x0208
.equ Key_Esc          ,#0x0408
.equ Key_Q            ,#0x0808
.equ Key_Tab          ,#0x1008
.equ Key_A            ,#0x2008
.equ Key_CapsLock     ,#0x4008
.equ Key_Z            ,#0x8008
;; Matrix Line 0x09
.equ Joy0_Up          ,#0x0109
.equ Joy0_Down        ,#0x0209
.equ Joy0_Left        ,#0x0409
.equ Joy0_Right       ,#0x0809
.equ Joy0_Fire1       ,#0x1009
.equ Joy0_Fire2       ,#0x2009
.equ Joy0_Fire3       ,#0x4009
.equ Key_Del          ,#0x8009