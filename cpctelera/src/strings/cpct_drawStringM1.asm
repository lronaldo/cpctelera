;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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
.module cpct_strings

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_drawStringM1
;;
;;    Prints a null-terminated string with ROM characters on a given byte-aligned 
;; position on the screen in Mode 1 (320x200px, 4 colours).
;;
;; C Definition:
;;    void <cpct_drawStringM1> (void* *string*, void* *video_memory*) __z88dk_callee;
;;
;; Input Parameters (4 Bytes):
;;  (2B IY) string       - Pointer to the null terminated string being drawn
;;  (2B HL) video_memory - Video memory location where the string will be drawn
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_drawStringM1_asm
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
;;
;; Requirements and limitations:
;;  * *Do not put this function's code below 0x4000 in memory*. In order to read
;; characters from ROM, this function enables Lower ROM (which is located 0x0000-0x3FFF),
;; so CPU would read code from ROM instead of RAM in first bank, effectively shadowing
;; this piece of code. This would lead to undefined results (typically program would
;; hang or crash).
;;  * This routine does not check for boundaries. If you draw too long strings or out 
;; of the screen, unpredictable results will happen.
;;  * Screen must be configured in Mode 1 (320x200 px, 4 colours)
;;  * This function requires the CPC *firmware* to be *DISABLED*. Otherwise, random
;; crashes might happen due to side effects.
;;  * This function *disables interrupts* during main loop (character printing), and
;; re-enables them at the end.
;;  * This function *will not work from ROM*, as it uses self-modifying code.
;;
;; Details:
;;    This function receives a null-terminated string and draws it to the screen in 
;; Mode 1 (320x200, 4 colours). To do so, it repeatedly calls <cpct_drawCharM1_inner_asm>,
;; for every character to be drawn. As foreground and background colours it uses the
;; ones previously set up by the latest call to <cpct_setDrawCharM1>. Therefore, you
;; need to call <cpct_setDrawCharM1> previous to using this function to select the
;; colours you want the text to be drawn of. However, once you set colours, they 
;; remain set with no need to call <cpct_setDrawCharM1> again.
;;
;;   *video_memory* parameter points to the byte where the string will be
;; drawn. The first pixel of that byte will be the upper-left corner of the string.
;; As this function uses a byte-pointer to refer to the upper-left corner of the 
;; string, it can only start drawing the string on every pixel columns divisible by 4 
;; (0, 4, 8, 12...), as every byte contains 4 pixels in Mode 1.
;;
;;    Usage of this function is quite straight-forward, as you can see in the 
;; following example,
;; (start code)
;;    // Just print some strings for testing
;;    void main () {
;;       u8* pvmem;  // Pointer to video memory
;;
;;       // Set video mode 0
;;       cpct_disableFirmware();
;;       cpct_setVideoMode(1);
;;
;;       // Draw some testing strings with curious colours, more or less centered
;;       pvmem = cpctm_screenPtr(CPCT_VMEM_START, 16, 88);  // Calculate video memory address
;;       cpct_setDrawCharM1(3, 1);                          // Red over yellow
;;       cpct_drawStringM1("Hello there!", pvmem);          // Draw the string
;;
;;       pvmem = cpctm_screenPtr(CPCT_VMEM_START, 20, 108); // Calculate new video memory address
;;       cpct_setDrawCharM1(2, 3);                          // Blue over red
;;       cpct_drawStringM1("Great man!",   pvmem);          // Draw the string
;;
;;       // And loop forever
;;       while(1);
;;    }
;; (end code)
;;
;; Destroyed Register values: 
;;    C bindings     - AF, BC, DE, HL
;;    ASM bindings   - AF, BC, DE, HL, IY
;;
;; Required memory:
;;    C bindings     - 48 bytes (+80 bytes <cpct_drawCharM1_inner_asm> = 128 bytes)
;;    ASM bindings   - 36 bytes (+80 bytes <cpct_drawCharM1_inner_asm> = 116 bytes)
;;
;; Time Measures:
;; (start code)
;;   Case     | microSecs (us) |  CPU Cycles
;; -------------------------------------------
;;   Best     |   62 + 486*L   |  248 + 1944*L
;;   Worst    |   62 + 494*L   |  248 + 1976*L
;; ----------------------------------------------
;; Asm saving |      -26       |      -104
;; ----------------------------------------------
;; (end code)
;;    L = Length of the string (excluding null-terminator character)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.globl cpct_drawCharM1_inner_asm

   ;; Enable Lower ROM during char copy operation, with interrupts disabled 
   ;; to prevent firmware messing things up
   ld     a,(_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present value)
   and    #0b11111011               ;; [2] bit 3 of A = 0 --> Lower ROM enabled (0 means enabled)
   ld     b, #GA_port_byte          ;; [2] B = Gate Array Port (0x7F)
   di                               ;; [1] Disable interrupts to prevent firmware from taking control while Lower ROM is enabled
   out   (c), a                     ;; [3] GA Command: Set Video Mode and ROM status (100)

   jr    firstChar                  ;; [3] Jump to first char (Saves 1 jr back every iteration)

nextChar:
   ;; Draw next character
   push  hl                         ;; [4] Save HL
   call  cpct_drawCharM1_inner_asm  ;; [5 + 458/466] Draws the next character
   pop   hl                         ;; [3] Recover HL 

   ;; Increment Pointers
   inc   hl                         ;; [2] /
   inc   hl                         ;; [2] | HL += 2 (point to next position in video memory, 8 pixels to the right)
   inc   iy                         ;; [3] IX += 1 (point to next character in the string)

firstChar:
   ld     a, (iy)                   ;; [5] A = next character from the string
   or     a                         ;; [1] Check if A = 0
   jr    nz, nextChar               ;; [2/3] if A != 0, A is next character, draw it, else end

endstring:
   ;; After finishing character drawing, restore previous ROM and Interrupts status
   ld     a, (_cpct_mode_rom_status) ;; [4] A = mode_rom_status (present saved value)
   ld     b, #GA_port_byte           ;; [2] B = Gate Array Port (0x7F)
   out   (c), a                      ;; [3] GA Command: Set Video Mode and ROM status (100)
   ei                                ;; [1] Enable interrupts

;; IY Restore and Return provided by bindings
