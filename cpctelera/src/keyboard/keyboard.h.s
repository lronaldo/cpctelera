;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2017 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_keyboard

;;
;; Constant: Key Definitions (asm)
;;
;;    Definitions of the KeyCodes required by <cpct_isKeyPressed> 
;; function for assembler programs. These are 16-bit values that define 
;; matrix line in the keyboard layout (Most Significant Byte) and bit to
;; be tested in that matrix line status for the given key (Least Significant
;; byte). Each matrix line in the keyboard returns a byte containing the
;; status of 8 keys, 1 bit each.
;;
;; CPCtelera include file:
;;    _keyboard/keyboard.h.s_
;;
;; Keycode constant names:
;; (start code)
;;  KeyCode | Constant        || KeyCode | Constant      || KeyCode |  Constant
;; -------------------------------------------------------------------------------
;;   0x0100 | Key_CursorUp    ||  0x0803 | Key_P         ||  0x4006 |  Key_B
;;          |                 ||         |               ||     ''  |  Joy1_Fire3
;;   0x0200 | Key_CursorRight ||  0x1003 | Key_SemiColon ||  0x8006 |  Key_V
;;   0x0400 | Key_CursorDown  ||  0x2003 | Key_Colon     ||  0x0107 |  Key_4
;;   0x0800 | Key_F9          ||  0x4003 | Key_Slash     ||  0x0207 |  Key_3
;;   0x1000 | Key_F6          ||  0x8003 | Key_Dot       ||  0x0407 |  Key_E
;;   0x2000 | Key_F3          ||  0x0104 | Key_0         ||  0x0807 |  Key_W
;;   0x4000 | Key_Enter       ||  0x0204 | Key_9         ||  0x1007 |  Key_S
;;   0x8000 | Key_FDot        ||  0x0404 | Key_O         ||  0x2007 |  Key_D
;;   0x0101 | Key_CursorLeft  ||  0x0804 | Key_I         ||  0x4007 |  Key_C
;;   0x0201 | Key_Copy        ||  0x1004 | Key_L         ||  0x8007 |  Key_X
;;   0x0401 | Key_F7          ||  0x2004 | Key_K         ||  0x0108 |  Key_1
;;   0x0801 | Key_F8          ||  0x4004 | Key_M         ||  0x0208 |  Key_2
;;   0x1001 | Key_F5          ||  0x8004 | Key_Comma     ||  0x0408 |  Key_Esc
;;   0x2001 | Key_F1          ||  0x0105 | Key_8         ||  0x0808 |  Key_Q
;;   0x4001 | Key_F2          ||  0x0205 | Key_7         ||  0x1008 |  Key_Tab
;;   0x8001 | Key_F0          ||  0x0405 | Key_U         ||  0x2008 |  Key_A
;;   0x0102 | Key_Clr         ||  0x0805 | Key_Y         ||  0x4008 |  Key_CapsLock
;;   0x0202 | Key_OpenBracket ||  0x1005 | Key_H         ||  0x8008 |  Key_Z
;;   0x0402 | Key_Return      ||  0x2005 | Key_J         ||  0x0109 |  Joy0_Up
;;   0x0802 | Key_CloseBracket||  0x4005 | Key_N         ||  0x0209 |  Joy0_Down
;;   0x1002 | Key_F4          ||  0x8005 | Key_Space     ||  0x0409 |  Joy0_Left
;;   0x2002 | Key_Shift       ||  0x0106 | Key_6         ||  0x0809 |  Joy0_Right
;;          |                 ||     ''  | Joy1_Up       ||         |
;;   0x4002 | Key_BackSlash   ||  0x0206 | Key_5         ||  0x1009 |  Joy0_Fire1
;;          |                 ||     ''  | Joy1_Down     ||         |
;;   0x8002 | Key_Control     ||  0x0406 | Key_R         ||  0x2009 |  Joy0_Fire2
;;          |                 ||     ''  | Joy1_Left     ||         |
;;   0x0103 | Key_Caret       ||  0x0806 | Key_T         ||  0x4009 |  Joy0_Fire3
;;          |                 ||     ''  | Joy1 Right    ||
;;   0x0203 | Key_Hyphen      ||  0x1006 | Key_G         ||  0x8009 |  Key_Del
;;          |                 ||     ''  | Joy1_Fire1    ||
;;   0x0403 | Key_At          ||  0x2006 | Key_F         ||
;;          |                 ||     ''  | Joy1_Fire2    ||
;; -------------------------------------------------------------------------------
;;  Table 1. KeyCodes defined for each possible key, ordered by KeyCode
;; (end)
;;

;; Matrix Line 0x00
Key_CursorUp     = #0x0100  ;; Bit 0 (01h) => | 0000 0001 |
Key_CursorRight  = #0x0200  ;; Bit 1 (02h) => | 0000 0010 |
Key_CursorDown   = #0x0400  ;; Bit 2 (04h) => | 0000 0100 |
Key_F9           = #0x0800  ;; Bit 3 (08h) => | 0000 1000 |
Key_F6           = #0x1000  ;; Bit 4 (10h) => | 0001 0000 |
Key_F3           = #0x2000  ;; Bit 5 (20h) => | 0010 0000 |
Key_Enter        = #0x4000  ;; Bit 6 (40h) => | 0100 0000 |
Key_FDot         = #0x8000  ;; Bit 7 (80h) => | 1000 0000 |
;; Matrix Line 0x01
Key_CursorLeft   = #0x0101
Key_Copy         = #0x0201
Key_F7           = #0x0401
Key_F8           = #0x0801
Key_F5           = #0x1001
Key_F1           = #0x2001
Key_F2           = #0x4001
Key_F0           = #0x8001
;; Matrix Line 0x02
Key_Clr          = #0x0102
Key_OpenBracket  = #0x0202
Key_Return       = #0x0402
Key_CloseBracket = #0x0802
Key_F4           = #0x1002
Key_Shift        = #0x2002
Key_BackSlash    = #0x4002
Key_Control      = #0x8002
;; Matrix Line 0x03
Key_Caret        = #0x0103
Key_Hyphen       = #0x0203
Key_At           = #0x0403
Key_P            = #0x0803
Key_SemiColon    = #0x1003
Key_Colon        = #0x2003
Key_Slash        = #0x4003
Key_Dot          = #0x8003
;; Matrix Line 0x04
Key_0            = #0x0104
Key_9            = #0x0204
Key_O            = #0x0404
Key_I            = #0x0804
Key_L            = #0x1004
Key_K            = #0x2004
Key_M            = #0x4004
Key_Comma        = #0x8004
;; Matrix Line 0x05
Key_8            = #0x0105
Key_7            = #0x0205
Key_U            = #0x0405
Key_Y            = #0x0805
Key_H            = #0x1005
Key_J            = #0x2005
Key_N            = #0x4005
Key_Space        = #0x8005
;; Matrix Line 0x06
Key_6            = #0x0106
Joy1_Up          = #0x0106
Key_5            = #0x0206
Joy1_Down        = #0x0206
Key_R            = #0x0406
Joy1_Left        = #0x0406
Key_T            = #0x0806
Joy1_Right       = #0x0806
Key_G            = #0x1006
Joy1_Fire1       = #0x1006
Key_F            = #0x2006
Joy1_Fire2       = #0x2006
Key_B            = #0x4006
Joy1_Fire3       = #0x4006
Key_V            = #0x8006
;; Matrix Line 0x07
Key_4            = #0x0107
Key_3            = #0x0207
Key_E            = #0x0407
Key_W            = #0x0807
Key_S            = #0x1007
Key_D            = #0x2007
Key_C            = #0x4007
Key_X            = #0x8007
;; Matrix Line 0x08
Key_1            = #0x0108
Key_2            = #0x0208
Key_Esc          = #0x0408
Key_Q            = #0x0808
Key_Tab          = #0x1008
Key_A            = #0x2008
Key_CapsLock     = #0x4008
Key_Z            = #0x8008
;; Matrix Line 0x09
Joy0_Up          = #0x0109
Joy0_Down        = #0x0209
Joy0_Left        = #0x0409
Joy0_Right       = #0x0809
Joy0_Fire1       = #0x1009
Joy0_Fire2       = #0x2009
Joy0_Fire3       = #0x4009
Key_Del          = #0x8009