;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2023 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;
.module cpct_keyboard

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_getKeypressedAsASCII
;;
;;    Assuming there is only one Key currently pressed, it returns the ASCII value
;; associated to the pressed key. For the keys that do not have a corresponding
;; ASCII value, this function associates one unique value greater than 127, to
;; be able to differenciate those keys.
;;    Returns 0 if no key is currently pressed.
;;
;; C Definition:
;;    <u8> <cpct_getKeypressedAsASCII> ();
;;
;; Assembly call:
;;    > call cpct_getKeypressedAsASCII_asm
;;
;; Return value:
;;    <u8> - ASCII value of the first key found to be pressed in the 
;; <cpct_keyboardStatusBuffer>, or 0 in case no key is pressed.
;;
;; Assembly return:
;;    A - Register holding the return value
;;
;; Known limitations:
;;    * This function may not work properly if more than one key is pressed at
;; the same time. Particularly, this function does not take into account the
;; special keys (SHIFT, BLOQ-SHIFT, CONTROL, ETC) and their combinations, as it
;; treats all keys individually, expecting only one to be pressed at a time.
;;    * Not all keys have unique values. F0-9 numbers have the same ASCII values
;; as their numbers in the normal keyboard. Similar to FDot and dot values. Please
;; refer to all concrete ASCII values below if you need to know them exactly.
;;
;; Details:
;;    This function reads the <cpct_keyboardStatusBuffer> and looks for the first
;; key that its pressed (marked there as a bit set to 0). If it does not found
;; any key pressed, it returns a 0.
;;
;;    When it founds a key pressed, it calculates the offset of that key in the
;; <cpct_keyID_to_ASCII_table>. This table mimicks the 10 lines x 8 keys of the
;; Amstrad CPC's keyboard matrix, which is the same the <cpct_keyboardStatusBuffer>
;; mimicks bit-per-bit. However, this table is made of 1-byte entries, each one
;; representing the associated ASCII value for the same key it mimicks.
;;
;;    With that offset, the function retrieves the ASCII value from the 
;; <cpct_keyID_to_ASCII_table> and returns it to the caller. 
;;
;; The <cpct_keyID_to_ASCII_table> along with its concrete ASCII values for each
;; one of the 80 Amstrad CPC's keys is as follows:
;; 
;; (start code)
;;             Key 01             Key 02            Key 04           Key 08             Key 10        Key 20         Key 40        Key 80
;;         ___________________________________________________________________________________________________________________________________
;; Line 00 |    244 'ô'     |     246 'ö'     |     245 'õ'    |      57 '9'      |     54 '6'    |    51 '3'  |    165 '¥'    |    46 '.'   | ASCII VALUES
;; Line 01 |    247 '÷'     |     232 'è'     |      55 '7'    |      56 '8'      |     53 '5'    |    49 '1'  |     50 '2'    |    48 '0'   | and
;; Line 02 |    238 'î'     |      91 '['     |     140 '?'    |      93 ']'      |     52 '4'    |   208 'Ð'  |     92 '\'    |   233 'é'   | CHARACTERS
;; Line 03 |     94 '^'     |      45 '-'     |      64 '@'    |      80 'P'      |     59 ';'    |    58 ':'  |     47 '/'    |    46 '.'   |
;; Line 04 |     48 '0'     |      57 '9'     |      79 'O'    |      73 'I'      |     76 'L'    |    75 'K'  |     77 'M'    |    44 ','   |
;; Line 05 |     56 '8'     |      55 '7'     |      85 'U'    |      89 'Y'      |     72 'H'    |    74 'J'  |     78 'N'    |    32 ' '   |
;; Line 06 |     54 '6'     |      53 '5'     |      82 'R'    |      84 'T'      |     71 'G'    |    70 'F'  |     66 'B'    |    86 'V'   |
;; Line 07 |     52 '4'     |      51 '3'     |      69 'E'    |      87 'W'      |     83 'S'    |    68 'D'  |     67 'C'    |    88 'X'   |
;; Line 08 |     49 '1'     |      50 '2'     |     224 'à'    |      81 'Q'      |    197 'Å'    |    65 'A'  |    196 'Ä'    |    90 'Z'   |
;; Line 09 |    240 'ð'     |     241 'ñ'     |     242 'ò'    |     243 'ó'      |    203 'Ë'    |   206 'Î'  |    207 'Ï'    |   199 'Ç'   |
;;         -----------------------------------------------------------------------------------------------------------------------------------
 ;;        ___________________________________________________________________________________________________________________________________
;; Line 00 | Key_CursorUp   | Key_CursorRight | Key_CursorDown | Key_F9           | Key_F6        | Key_F3     | Key_Enter     | Key_FDot    | cpct_keyID symbols
;; Line 01 | Key_CursorLeft | Key_Copy        | Key_F7         | Key_F8           | Key_F5        | Key_F1     | Key_F2        | Key_F0      |
;; Line 02 | Key_Clr        | Key_OpenBracket | Key_Return     | Key_CloseBracket | Key_F4        | Key_Shift  | Key_BackSlash | Key_Control |
;; Line 03 | Key_Caret      | Key_Hyphen      | Key_At         | Key_P            | Key_SemiColon | Key_Colon  | Key_Slash     | Key_Dot     |
;; Line 04 | Key_0          | Key_9           | Key_O          | Key_I            | Key_L         | Key_K      | Key_M         | Key_Comma   |
;; Line 05 | Key_8          | Key_7           | Key_U          | Key_Y            | Key_H         | Key_J      | Key_N         | Key_Space   |
;; Line 06 | Key_6          | Key_5           | Key_R          | Key_T            | Key_G         | Key_F      | Key_B         | Key_V       |
;;         | Joy1_Up        | Joy1_Down       | Joy1_Left      | Joy1 Right       | Joy1_Fire1    | Joy1_Fire2 | Joy1_Fire3    |             |
;; Line 07 | Key_4          | Key_3           | Key_E          | Key_W            | Key_S         | Key_D      | Key_C         | Key_X       |
;; Line 08 | Key_1          | Key_2           | Key_Esc        | Key_Q            | Key_Tab       | Key_A      | Key_CapsLock  | Key_Z       |
;; Line 09 | Joy0_Up        | Joy0_Down       | Joy0_Left      | Joy0_Right       | Joy0_Fire1    | Joy0_Fire2 | Joy0_Fire3    | Key_Del     |
;;         -----------------------------------------------------------------------------------------------------------------------------------
;; (end code)
;; *Note*: Non-standar ASCII characters (>127) in this table will not match Amstrad CPC's characters.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;       117 bytes (including 80 bytes from the <cpct_keyID_to_ASCII_table>)
;;
;; Time Measures:
;;    Time measures depend on the key pressed. As the keys are ordered by lines,
;; detecting the keypress of a key in the 10th line is much slower than detecting
;; that on the first line. Best and Worst Cases refer to detecting those keys
;; (the very first in the key order, and the last).
;; (start code)
;;   Case      | microSecs (us) | CPU Cycles 
;; -------------------------------------------
;; Best        |       39       |    156
;; -------------------------------------------
;; Average     |      106       |    424
;; -------------------------------------------
;; Worst       |      173       |    692
;; -------------------------------------------
;; Not-found   |      118       |    472
;; -------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Keyboard Status Buffer defined in an external file
.globl _cpct_keyboardStatusBuffer

   ld  de, #_cpct_keyboardStatusBuffer ;; [3] DE Points to the start of the keyboard status buffer
   ld   b, #10                         ;; [2] B=10 (Counter of Keyboard Lines to be checked)

keylines_loop:
   ld     a, (de)          ;; [2] A=Key status of Current Keyboard Line (8 bits which mean 1=not pressed, 0=pressed)
   inc    a                ;; [1] ++A (If all keys are not-pressed, A=0xFF, so ++A = 0)
   jr    nz, key_pressed   ;; [2/3] So, if (++A != 0), at least 1 key is pressed in this Keyboard Line
   inc   de                ;; [2] DE++ (Next Keyboard Line)
   djnz  keylines_loop     ;; [3/4] If (--B != 0) Check next Keyboard Line

no_key_is_pressed:
   ld     l, a             ;; [1] A=0 already, L=0 for C-return value
   ret                     ;; [3] return 0 (no key is pressed)

;;
;; Table for converting <cpct_keyID> to ASCII,
;;
;;    The table is inserted in the middle of this function because otherwise it
;; would be treated as code by the CPU, as everything after and before the function
;; is exectuted due to how the bindings of this function are done.
;;
_cpct_keyID_to_ASCII_table::
   .db 244, 246, 245, 57, 54, 51, 165, 46       ;; Keyboard Line 0
   .db 247, 232, 55, 56, 53, 49, 50, 48          ;; Keyboard Line 1
   .db 238, 91, 140, 93, 52, 208, 92, 233       ;; Keyboard Line 2
   .db 94, 45, 64, 80, 59, 58, 47, 46           ;; Keyboard Line 3
   .db 48, 57, 79, 73, 76, 75, 77, 44           ;; Keyboard Line 4
   .db 56, 55, 85, 89, 72, 74, 78, 32           ;; Keyboard Line 5
   .db 54, 53, 82, 84, 71, 70, 66, 86           ;; Keyboard Line 6
   .db 52, 51, 69, 87, 83, 68, 67, 88           ;; Keyboard Line 7
   .db 49, 50, 224, 81, 197, 65, 196, 90        ;; Keyboard Line 8
   .db 240, 241, 242, 243, 203, 206, 207, 199   ;; Keyboard Line 9

key_pressed:
   dec    a                      ;; [1] ++A: Some Key is pressed, Add 1 to A, as we previously subtracted 1 to check
   ld     e, a                   ;; [1] E = A (Save A for later use)
   ld    hl, #_cpct_keyID_to_ASCII_table ;; [3] HL Points to the key_to_ASCII_table
   ld     a, #10                 ;; [2] A = 10     (A = Keyboard Lines Total)
   sub    b                      ;; [1] A = 10 - B (A = Current Keyboard Line where a key has been found to be pressed)
   add    a                      ;; [1] /
   add    a                      ;; [1] | A = 8*(10 - B)
   add    a                      ;; [1] \ A = Offset from start of key_to_ASCII_table to start of the Keyboard line inside it, containing the 8 ASCII values
   ;; Previous operation should never produce carry
   ld     c, a    ;; [1] / C = Offset from start of key_to_ASCII_table
   dec    c       ;; [1] \     minus 1 (as the loop starts by incrementing C, we do this to adjust)
   ld     a, e    ;; [1] A = 8 bits representing the 8 keys in the current keyboard line, starting from bit 0,
                  ;;         with 1=key not pressed, 0=keypressed   
count_keys_in_line:
   inc    c                      ;; [1] ++C, counting 1 more key in this line that is not pressed
   rrca                          ;; [1] Move all bits in the line to the right, so that bit 0 moves to carry and we can test it for being pressed or not
   jr     c, count_keys_in_line  ;; [2/3] If C=1, key is not pressed, continue checking next key and adding 1 to the count of not pressed
   ;; Now C = Offset to the ASCII of the Key pressed
   ;; Add it to HL to get the address of the ASCII value and return it
   ld     b, #0                  ;; [2] B=0, So that BC = C (Offset)
   add   hl, bc                  ;; [3] HL += BC (HL = key_to_ASCII_value + Offset)

return:
   ;; Return depending on C-ASM bindings



