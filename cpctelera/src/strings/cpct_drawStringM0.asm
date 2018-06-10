;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
;; Function: cpct_drawStringM0
;;
;;    Prints a null-terminated string with ROM characters on a given byte-aligned 
;; position on the screen in Mode 0 (160x200 px, 16 colours).
;;
;; C Definition:
;;    void <cpct_drawStringM0> (void* *string*, void* *video_memory*, <u8> *fg_pen*, <u8> *bg_pen*)
;;
;; Input Parameters (5 Bytes):
;;  (2B HL) string       - Pointer to the null terminated string being drawn
;;  (2B DE) video_memory - Video memory location where the string will be drawn
;;  (1B C ) fg_pen       - Foreground colour (PEN, 0-15)
;;  (1B B ) bg_pen       - Background colour (PEN, 0-15)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawStringM0_asm
;;
;; Parameter Restrictions:
;;  * *string* must be a null terminated string. It could contain any 8-bit value as 
;; characters except 0, which will signal the end of the string. Be careful to provide
;; strings with a 0 (null) at the end of the string. Otherwise, unexpected results may
;; happen (Typically, rubbish characters printed on screen and, occasionally, memory 
;; overwrite and even hangs or crashes).
;;  * *video_memory* could theoretically be any 16-bit memory location. It will work
;; outside current screen memory boundaries, which is useful if you use any kind of
;; double buffer. However, be careful where you use it, as it does no kind of check
;; or clipping, and it could overwrite data if you select a wrong place to draw.
;;  * *fg_pen* must be in the range [0-15]. It is used to access a colour mask table and,
;; so, a value greater than 15 will return a random colour mask giving unpredictable 
;; results (typically bad character rendering, with odd colour bars).
;;  * *bg_pen* must be in the range [0-15], with identical reasons to *fg_pen*.
;;
;; Requirements and limitations:
;;  * *Do not put this function's code below 0x4000 in memory*. In order to read
;; characters from ROM, this function enables Lower ROM (which is located 0x0000-0x3FFF),
;; so CPU would read code from ROM instead of RAM in first bank, effectively shadowing
;; this piece of code. This would lead to undefined results (typically program would
;; hang or crash).
;;  * This routine does not check for boundaries. If you draw too long strings or out 
;; of the screen, unpredictable results will happen.
;;  * Screen must be configured in Mode 0 (160x200 px, 16 colours)
;;  * This function requires the CPC *firmware* to be *DISABLED*. Otherwise, random
;; crashes might happen due to side effects.
;;  * This function *disables interrupts* during main loop (character printing), and
;; re-enables them at the end.
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function receives a null-terminated string and draws it to the screen in 
;; Mode 0 (160x200, 16 colours). This function calls <cpct_drawCharM0> to draw every    
;; character. *video_memory* parameter points to the byte where the string will be
;; drawn. The first pixel of that byte will be the upper-left corner of the string.
;; As this function uses a byte-pointer to refer to the upper-left corner of the 
;; string, it can only draw string on even-pixel columns (0, 2, 4, 6...), as 
;; every byte contains 2 pixels in Mode 0.
;;
;; Destroyed Register values: 
;;    AF, BC, DE, HL
;;
;; Required memory:
;;    C bindings  - 66 (+35 setDrawCharM0_asm + 100 cpct_drawCharM0_inner_asm = 201 bytes)
;;  ASM bindings  - 62 (+35 setDrawCharM0_asm + 100 cpct_drawCharM0_inner_asm = 197 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) | CPU Cycles
;; ----------------------------------------------
;;   Best     |   143 + 854*L  |  572 + 3416*L  
;;   Worst    |   143 + 862*L  |  572 + 3448*L
;; ----------------------------------------------
;; Asm saving |      -15       |     -60
;; ----------------------------------------------
;; (end code)
;;    L = Length of the string (excluding null-terminator character)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_drawCharM0_inner_asm
.globl cpct_setDrawCharM0_asm

   ;; Save IX and IY before using them
   ld    (saveix), ix   ;; [6] Save IX
   ld    (saveiy), iy   ;; [6] Save IY

   ;; Set foreground and background colours before drawing the string
   push  de                      ;; [4]   Save Pointer to Destination in Video Memory
   push  bc                      ;; [4]   Save Pointer to the null terminated string
   call  cpct_setDrawCharM0_asm  ;; [5+50] Set Colours
   pop   iy                      ;; [5]   IY = Pointer to the null terminated string
   pop   hl                      ;; [3]   HL = Pointer to Destination in Video Memory

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld     a,(_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present value)
   and    #0b11111011               ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld     b, #GA_port_byte          ;; [2] B = Gate Array Port (0x7F)
   di                               ;; [1] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out   (c), a                     ;; [3] GA Command: Set Video Mode and ROM status (100)

   jr    firstChar                  ;; [3] Jump to first char (Saves 1 jr back every iteration)

nextChar:
   push  hl                         ;; [4] Save HL
   call  cpct_drawCharM0_inner_asm  ;; [5 + 824/832] Does the next character
   pop   hl                         ;; [3] Recover HL 

   ;; Increment Pointers
   ld    de, #4                     ;; [3] /
   add   hl, de                     ;; [3] | HL += 4 (point to next position in video memory, 8 pixels to the right)
   inc   iy                         ;; [3] IY += 1 (point to next character in the string)

firstChar:
   ld     a, (iy)                   ;; [5] A = next character from the string
   or     a                         ;; [1] Check if A = 0
   jr    nz, nextChar               ;; [2/3] if A != 0, A is next character, draw it, else end

endstring:
   ;; After finishing character printing, restore previous ROM and Interrupts status
   ld     a, (_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present saved value)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)
   ei                                ;; [1] Enable interrupts

saveix = .+2
saveiy = .+6
   ld    ix, #0000   ;; [6] Restore IX before returning (0000 is a placeholder)
   ld    iy, #0000   ;; [6] Restore IY before returning (0000 is a placeholder)
   ret               ;; [3] Return
