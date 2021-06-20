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

;;//////////////////////////////////////////////////////////////////////
;;//////////////////////////////////////////////////////////////////////
;; File: Macros (asm)
;;//////////////////////////////////////////////////////////////////////
;;//////////////////////////////////////////////////////////////////////

;;//////////////////////////////////////////////////////////////////////
;; Group: Video memory manipulation
;;//////////////////////////////////////////////////////////////////////

;;
;; Constant: CPCT_VMEM_START_ASM
;;
;;    The address where screen video memory starts by default in the Amstrad CPC.
;;
;;    This address is exactly 0xC000, and this macro represents this number but
;; automatically converted to <u8>* (Pointer to unsigned byte). You can use this
;; macro for any function requiring the start of video memory, like 
;; <cpct_getScreenPtr>.
;;
CPCT_VMEM_START_ASM = 0xC000

;;
;; Constants: Video Memory Pages
;;
;; Useful constants defining some typical Video Memory Pages to be used as 
;; parameters for <cpct_setVideoMemoryPage>
;;
;; cpct_pageCO - Video Memory Page 0xC0 (0xC0··)
;; cpct_page8O - Video Memory Page 0x80 (0x80··)
;; cpct_page4O - Video Memory Page 0x40 (0x40··)
;; cpct_page0O - Video Memory Page 0x00 (0x00··)
;;
cpct_pageC0_asm = 0x30
cpct_page80_asm = 0x20
cpct_page40_asm = 0x10
cpct_page00_asm = 0x00

;;
;; Macro: cpctm_memPage6_asm
;;
;;    Macro that encodes a video memory page in the 6 Least Significant bits (LSb)
;; of a byte, required as parameter for <cpct_setVideoMemoryPage>. It loads resulting
;; value into a given 8-bits register.
;;
;; ASM Definition:
;; .macro <cpct_memPage6_asm> *REG8*, *PAGE*
;;
;; Parameters (1 byte):
;; (__) REG8 - 8bits register where result will be loaded
;; (1B) PAGE - Video memory page wanted 
;;
;; Known issues:
;;   * This macro can only be used from assembler code. It is not accessible from 
;; C scope. For C programs, please refer to <cpct_memPage6>
;;   * This macro will work *only* with constant values, as its value needs to
;; be calculated in compilation time. If fed with variable values, it will give 
;; an assembler error.
;;
;; Destroyed Registers:
;;    REG8
;;
;; Size of generated code:
;;    2 bytes 
;;
;; Time Measures:
;;    * 2 microseconds
;;    * 8 CPU Cycles
;;
;; Details:
;;  This is just a macro that shifts *PAGE* 2 bits to the right, to leave it
;; with just 6 significant bits. For more information, check functions
;; <cpct_setVideoMemoryPage> and <cpct_setVideoMemoryOffset>.
;;
.macro cpctm_memPage6_asm REG8, PAGE 
   ld REG8, #PAGE / 4      ;; [2] REG8 = PAGE/4
.endm

;;
;; Macro: cpctm_screenPtr_asm
;;
;;    Macro that calculates the video memory location (byte pointer) of a 
;; given pair of coordinates (*X*, *Y*). Value resulting from calculation 
;; will be loaded into a 16-bits register.
;;
;; ASM Definition:
;;    .macro <cpctm_screenPtr_asm> *REG16*, *VMEM*, *X*, *Y*
;;
;; Parameters:
;;    (__) REG16 - 16-bits register where the resulting value will be loaded
;;    (2B) VMEM  - Start of video memory buffer where (*X*, *Y*) coordinates will be calculated
;;    (1B) X     - X Coordinate of the video memory location *in bytes* (*BEWARE! NOT in pixels!*)
;;    (1B) Y     - Y Coordinate of the video memory location in pixels / bytes (they are same amount)
;;
;; Parameter Restrictions:
;;    * *REG16* has to be a 16-bits register that can perform ld REG16, #value.
;;    * *VMEM* will normally be the start of the video memory buffer where you want to 
;; draw something. It could theoretically be any 16-bits value. 
;;    * *X* must be in the range [0-79] for normal screen sizes (modes 0,1,2). Screen is
;; always 80 bytes wide in these modes and this function is byte-aligned, so you have to 
;; give it a byte coordinate (*NOT a pixel one!*).
;;    * *Y* must be in the range [0-199] for normal screen sizes (modes 0,1,2). Screen is 
;; always 200 pixels high in these modes. Pixels and bytes always coincide in vertical
;; resolution, so this coordinate is the same in bytes that in pixels.
;;    * If you give incorrect values to this function, the returned pointer could
;; point anywhere in memory. This function will not cause any damage by itself, 
;; but you may destroy important parts of your memory if you use its result to 
;; write to memory, and you gave incorrect parameters by mistake. Take always
;; care.
;;
;; Known issues:
;;   * This macro can only be used from assembler code. It is not accessible from 
;; C scope. For C programs, please refer to <cpct_getScreenPtr>
;;   * This macro will work *only* with constant values, as calculations need to be 
;; performed at assembler time.
;;
;; Destroyed Registers:
;;    REG16
;;
;; Size of generated code:
;;    3 bytes 
;;
;; Time Measures:
;;    * 3 microseconds
;;    * 12 CPU Cycles
;;
;; Details:
;;    This macro does the same calculation than the function <cpct_getScreenPtr>. However,
;; as it is a macro, if all 3 parameters (*VMEM*, *X*, *Y*) are constants, the calculation
;; will be done at compile-time. This will free the binary from code or data, just putting in
;; the result of this calculation (2 bytes with the resulting address). It is highly 
;; recommended to use this macro instead of the function <cpct_getScreenPtr> when values
;; involved are all constant. 
;;
;; Recommendations:
;;    All constant values - Use this macro <cpctm_screenPtr_asm>
;;    Any variable value  - Use the function <cpct_getScreenPtr>
;;
.macro cpctm_screenPtr_asm REG16, VMEM, X, Y 
   ld REG16, #VMEM + 80 * (Y / 8) + 2048 * (Y & 7) + X   ;; [3] REG16 = screenPtr
.endm

;;
;; Macro: cpctm_screenPtrSym_asm
;;
;;    Macro that calculates the video memory location (byte pointer) of a 
;; given pair of coordinates (*X*, *Y*). Value resulting from calculation 
;; will be assigned to the given ASZ80 local symbol.
;;
;; ASM Definition:
;;    .macro <cpctm_screenPtr_asm> *SYM*, *VMEM*, *X*, *Y*
;;
;; Parameters:
;;    (__) SYM   - ASZ80 local symbol to assign the result from the calculation to
;;    (2B) VMEM  - Start of video memory buffer where (*X*, *Y*) coordinates will be calculated
;;    (1B) X     - X Coordinate of the video memory location *in bytes* (*BEWARE! NOT in pixels!*)
;;    (1B) Y     - Y Coordinate of the video memory location in pixels / bytes (they are same amount)
;;
;; Parameter Restrictions:
;;    * *SYM* need to be a valid symbol according to ASZ80 rules for symbols
;;    * *VMEM* will normally be the start of the video memory buffer where you want to 
;; draw something. It could theoretically be any 16-bits value. 
;;    * *X* must be in the range [0-79] for normal screen sizes (modes 0,1,2). Screen is
;; always 80 bytes wide in these modes and this function is byte-aligned, so you have to 
;; give it a byte coordinate (*NOT a pixel one!*).
;;    * *Y* must be in the range [0-199] for normal screen sizes (modes 0,1,2). Screen is 
;; always 200 pixels high in these modes. Pixels and bytes always coincide in vertical
;; resolution, so this coordinate is the same in bytes that in pixels.
;;    * If you give incorrect values to this function, the returned pointer could
;; point anywhere in memory. This function will not cause any damage by itself, 
;; but you may destroy important parts of your memory if you use its result to 
;; write to memory, and you gave incorrect parameters by mistake. Take always
;; care.
;;
;; Known issues:
;;   * This macro can only be used from assembler code. It is not accessible from 
;; C scope. For C programs, please refer to <cpct_getScreenPtr>
;;   * This macro will work *only* with constant values, as calculations need to be 
;; performed at assembler time.
;;
;; Destroyed Registers:
;;    none
;;
;; Size of generated code:
;;    none (symbols are compile-time, do not generate code)
;;
;; Time Measures:
;;    - not applicable -
;;
;; Details:
;;    This macro does the same calculation than the function <cpct_getScreenPtr>. However,
;; as it is a macro, and as its parameters (*VMEM*, *X*, *Y*) must be constants, the calculation
;; will be performed at compile-time. This will free the binary from code or data, just putting in
;; the result of this calculation (2 bytes with the resulting address). It is highly 
;; recommended to use this macro instead of the function <cpct_getScreenPtr> when values
;; involved are all constant. 
;;
;; Recommendations:
;;    All constant values - Use this macro <cpctm_screenPtrSym_asm> or <cpctm_screenPtr_asm> 
;;    Any variable value  - Use the function <cpct_getScreenPtr>
;;
.macro cpctm_screenPtrSym_asm SYM, VMEM, X, Y 
   SYM = #VMEM + 80 * (Y / 8) + 2048 * (Y & 7) + X 
.endm

;;
;; Macro: cpctm_setCRTCReg
;;
;;    Macro that sets a new value for a given CRTC register.
;;
;; ASM Definition:
;;    .macro <cpctm_setCRTCReg> *HEXREG*, *HEXVAL*
;;
;; Parameters:
;;    (1B) HEXREG - New value to be set for the register (in hexadecimal)
;;    (1B) HEXVAL - Number of the register to be set (in hexadecimal)
;;
;; Parameter Restrictions:
;;    * *HEXREG* has to be an hexadecimal value from 00 to 1F
;;    * *HEXVAL* has to be an hexadecimal value. Its valid range will depend
;;          upon the selected register that will be modified. 
;;
;; Known issues:
;;   * This macro can *only* be used from assembler code. It is not accessible from 
;; C scope. 
;;   * This macro can only be used with *constant values*. As given values are 
;; concatenated with a number, they must also be hexadecimal numbers. If a 
;; register or other value is given, this macro will not work.
;;   * Using values out of range have unpredicted behaviour and can even 
;; potentially cause damage to real Amstrad CPC monitors. Please, use with care.
;;
;; Destroyed Registers:
;;    BC
;;
;; Size of generated code:
;;    10 bytes 
;;
;; Time Measures:
;;    * 14 microseconds
;;    * 56 CPU Cycles
;;
;; Details:
;;    This macro expands to two CRTC commands: Register selection and Register setting.
;; It selects the register given as first parameter, then sets its new value to 
;; that given as second parameter. Both given parameters must be of exactly 1 byte
;; in size and the have to be provided in hexadecimal. This is due to the way
;; that macro expansion and concatenation works. Given values will be concatenated
;; with another 8-bit hexadecimal value to form a unique 16-bits hexadecimal value.
;; Therefore, any parameter given will always be considered hexadecimal.
;;
.macro cpctm_setCRTCReg_asm HEXREG, HEXVAL
   ld    bc, #0xBC'HEXREG  ;; [3] B=0xBC CRTC Select Register, C=register number to be selected
   out  (c), c             ;; [4] Select register
   ld    bc, #0xBD'HEXVAL  ;; [3] B=0xBD CRTC Set Register, C=Value to be set
   out  (c), c             ;; [4] Set the value
.endm

;;//////////////////////////////////////////////////////////////////////
;; Group: Setting the border
;;//////////////////////////////////////////////////////////////////////

;;
;; Macro: cpctm_setBorder_asm
;;
;;   Changes the colour of the screen border.
;;
;; ASM Definition:
;;   .macro <cpctm_setBorder_asm> HWC 
;;
;; Input Parameters (1 Byte):
;;   (1B) HWC - Hardware colour value for the screen border in *hexadecimal [00-1B]*.
;;
;; Known issues:
;;   * *Beware!* *HWC* colour value must be given in *hexadecimal*, as it is
;; substituted in place, and must be in the range [00-1B].
;;   * This macro can only be used from assembler code. It is not accessible from 
;; C scope. For C programs, please refer to <cpct_setBorder>
;;   * This macro will work *only* with constant values, as calculations need to be 
;; performed at assembler time.
;;
;; Destroyed Registers:
;;    AF, B, HL
;;
;; Size of generated code:
;;    * 16 bytes 
;;     6b - generated code
;;    10b - cpct_setPALColour_asm code
;;
;; Time Measures:
;;    * 28 microseconds
;;    * 112 CPU Cycles
;;
;; Details:
;;   This is not a real function, but an assembler macro. Beware of using it along
;; with complex expressions or calculations, as it may expand in non-desired
;; ways.
;;
;;   For more information, check the real function <cpct_setPALColour>, which
;; is called when using <cpctm_setBorder_asm> (It is called using 16 as *pen*
;; argument, which identifies the border).
;;
.macro cpctm_setBorder_asm HWC
   .radix h
   cpctm_setBorder_raw_asm \HWC ;; [28] Macro that does the job, but requires a number value to be passed
   .radix d
.endm
.macro cpctm_setBorder_raw_asm HWC
   .globl cpct_setPALColour_asm
   ld   hl, #0x'HWC'10         ;; [3]  H=Hardware value of desired colour, L=Border INK (16)
   call cpct_setPALColour_asm  ;; [25] Set Palette colour of the border
.endm

;;//////////////////////////////////////////////////////////////////////
;; Group: Screen clearing
;;//////////////////////////////////////////////////////////////////////

;;
;; Macro: cpctm_clearScreen_asm
;;
;;    Macro to simplify clearing the screen.
;;
;; ASM Definition:
;;   .macro <cpctm_clearScreen_asm> COL
;;
;; Input Parameters (1 byte):
;;   (1B) COL - Colour pattern to be used for screen clearing. 
;;
;; Parameters:
;;    *COL* - Any 8-bits value or the A register are valid. Typically, a 0x00 is used 
;; to fill up all the screen with 0's (firmware colour 0). However, you may use it in 
;; combination with <cpct_px2byteM0>, <cpct_px2byteM1> or a manually created colour pattern.
;;
;; Known issues:
;;   * This macro can only be used from assembler code. It is not accessible from 
;; C scope. For C programs, please refer to <cpct_clearScreen>
;;
;; Details:
;;   Fills up all the standard screen (range [0xC000-0xFFFF]) with *COL* byte, the colour 
;; pattern given.
;;
;; Destroyed Registers:
;;    BC, DE, HL
;;
;; Size of generated code:
;;    13 bytes 
;;
;; Time Measures:
;;    98309 microseconds (*4.924 VSYNCs* on a 50Hz display).
;;    393236 CPU Cycles 
;;
.macro cpctm_clearScreen_asm COL
   ld    hl, #0xC000    ;; [3] HL Points to Start of Video Memory
   ld    de, #0xC001    ;; [3] DE Points to the next byte
   ld    bc, #0x4000    ;; [3] BC = 16384 bytes to be copied
   ld   (hl), #COL      ;; [3] First Byte = given Colour
   ldir                 ;; [98297] Perform the copy
.endm
