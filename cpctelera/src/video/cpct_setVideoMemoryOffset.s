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
.module cpct_video

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setVideoMemoryOffset
;;
;;    Sets the 8 Least Significant bits (the offset) of the memory address where
;; video memory starts.
;;
;; C Definition:
;;    void <cpct_setVideoMemoryOffset> (<u8> *offset*)
;;
;; Input Parameters (4 bytes):
;;    (1B A) offset - New starting offset for Video Memory (8 Least Significant bits)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setVideoMemoryOffset_asm
;;
;; Parameter Restrictions:
;;    * *offset* could be any 8-bit value, and should contain video memory 
;; offset value. Its 8-bits will become the 8 Least Significant bits (LSb) of
;; the programmable Video Memory Start Address (VMSA), as the 2 least significant 
;; bits of the address are always set to 0 (see figure 1). For instance, an
;; *offset* value of 10101011 will set the VMSA to xxxxxx1010101100, being
;; xxxxxx the previous value of VMSA page. Consult <cpct_setVideoMemoryPage>
;; for more details on this.
;;
;; Details:
;;    Changes part of the address where video memory starts (Video Memory Start
;; Address, VMSA). Video memory starts at an address that can be defined in 
;; binary as follows:
;; (start code)
;;  --------------------------------------------------------
;;             VMSA = [ ppppppoooooooo00 ]
;;  --------------------------------------------------------
;;  Figure 1 - Video Memory Start Address (VMSA) composition
;; (end)
;;    where you have 6 bits that define the Page (p-bits), and 8 bits that 
;; define the *offset* (o-bits). The 2 Least Significant Bits (LSb) are always
;; set to 0.
;;
;;    This function changes the 8 bits that define the *offset* (the o-bits). 
;; These 8 bits are controlled by Register R13 from the CRTC. If you wanted
;; to change the 6 Most Significant bits (the page), you should use
;; <cpct_setVideoMemoryPage>
;;                                                                 
;;    Changing the *offset* effectively changes the place where your video 
;; memory starts in your system RAM, immediately changing what is displayed 
;; on screen. This could be used to produce scrolling effects or to make 
;; a fine grained control of double / triple buffers by hardware.
;;                                                                  
;;    To better explain this, lets show an example,
;;
;;    1 - You want your VMSA at 0xC0A0, instead of 0xC000
;;    2 - 0xC0A0 corresponds to page = 0x30, offset = 0xA0 (check Figure 1)
;;    3 - Page 0x30 is set by default (memory address starts at 0xC000 at power on)
;;    4 - You only have to call cpct_setVideoMemoryOffset(0xA0)
;; 
;;    Another example, requiring page and offset to be set, could be the next
;; one. Imagine you have a double buffer starting at memory address 0x801A and
;; you wanted to make it visible on screen. Your code to do that could be like
;; this:
;; (start code)
;;    ...
;;    cpct_setVideoMemoryPage(cpct_page80);  // Set page   0x80
;;    cpct_setVideoMemoryOffset(0x1A);       // Set offset 0x1A
;;    ...
;; (end)
;;    When creating hardware back buffers, always take into account that video
;; memory is never considered linear by Gate Array. This means that, wherever 
;; you put your VMSA, it will be considered the start of a Video Memory made 
;; of 8-pixel-line characters. Consult <cpct_drawSprite> for more details on
;; how video memory is distributed at its default place (it will be distributed
;; similarly at every place it were located).
;;
;; Destroyed Register values: 
;;    AF, BC, HL
;;
;; Required memory:
;;    14 bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | Cycles | microSecs (us)
;; ------------------------------------
;;     Any    |   76   |   19.00
;; ------------------------------------
;; Asm saving |  -28   |   -7.00
;; ------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_cpct_setVideoMemoryOffset::
   ;; Get the parameter from the stack
   ld   hl, #2       ;; [10] HL = SP + 2 (Place where parameter is)
   add  hl, sp       ;; [11]
   ld    a, (hl)     ;; [ 7] A = First Parameter (Video Memory Offset to Establish)

cpct_setVideoMemoryOffset_asm::     ;; Assembly entry point

   ;; Select R13 Register from the CRTC and Write there the selected Video Memory Offset
   ld   bc, #0xBC0D  ;; [10] 0xBC = Port for selecting CRTC Register, 0x0D = Selecting R13
   out (c), c        ;; [12] Select the R13 Register (0x0D to port 0xBC)
   inc   b           ;; [ 4] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   out (c), a        ;; [12] Write Selected Video Memory Offset to R13 (A to port 0xBD)

   ret               ;; [10] Return 
