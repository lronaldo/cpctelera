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
.module cpct_sprites

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_setVideoMemoryPage
;;
;;    Sets the 6 most significant bits (the page) of the memory address where
;; video memory starts.
;;
;; C Definition:
;;    void <cpct_setVideoMemoryPage> (<u8> *page_6LSb*);
;;
;; Input Parameters (4 bytes):
;;    (1B A) page_6LSb - New starting page for Video Memory (Only 6 Least Significant bits are used)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_setVideoMemoryPage_asm
;;
;; Parameter Restrictions:
;;    * *page_6LSb* should contain video memory page value encoded in its 6 
;; Least Significant bits (LSb). This parameter must be in the range [0-63] as 
;; only its 6 LSb must be used. These 6 bits will become the 6 Most significant 
;; bits (MSb) of the video memory start address (its page). So, for instance, 
;; if *page_6LSb* = 00101000, then video memory will start at address 
;; 101000xxxxxxxx00, being xxxxxx = the offset (consult 
;; <cpct_setVideoMemoryOffset> for more details).
;;
;; Details:
;;    Changes the place where the video memory starts. Concretely, it  
;; changes de Video Memory Page, which corresponds to the 6 Most significant 
;; bits (MSb) of the memory address where video memory starts. By doing this, 
;; you are effectively changing what part of the 64K of RAM will be used as 
;; Video Memory and, therefore, what is to be displayed on screen. With this 
;; you can affectively implement and triple buffer using hardware instead 
;; of software (i.e. copying bytes). 
;;                                                                  
;;    This routine achieves this by changing Register R12 from CRTC. This 
;; register is the responsible for holding the 6 MSb of the place where 
;; Video Memory starts (its page). However, this register stores this 6 bits 
;; as its 6 LSb (--bbbbbb). We have to take this into account to correctly 
;; set the page we want.
;;                                                                  
;;    To better explain this, lets show an example,
;;
;;    1 - You want your Video Memory to start at 0x8000, instead of 0xC000
;;    2 - Then, you should set Video Memory Page to 0x80 (10000000)
;;    3 - You call this routine with 0x20 as parameter (00100000)
;;    4 - Note that 0x20 (00100000) is 0x80 (10000000) shifted twice to the 
;; right. Your 6 MSb are to be passed as the 6 LSb of the page parameter.
;;
;;    In order to make this process easier, you may use predefined constants
;; to set the most typical video memory pages:
;; (start code)
;;           Page | Memory Start | constant to use | value
;;          ------------------------------------------------
;;           0xC0 |    0xC000    |  cpct_pageC0    |  0x30
;;           0x80 |    0x8000    |  cpct_page80    |  0x20
;;           0x40 |    0x4000    |  cpct_page40    |  0x10
;;           0x00 |    0x0000    |  cpct_page00    |  0x00
;;          ------------------------------------------------
;;     Table 1. Useful constants for setting video memory pages
;; (end)
;;    If you wanted to set a more specific video memory page, you can calculate
;; it by hand, or use <cpct_memPage6> macro. With this macro, you specify 
;; the page you want, and you get the *page_6LSb* parameter you should pass
;; to this function. For instance, imagine you want to use 0x84 as your 
;; video memory page (and 0x8400 will be the starting point of video memory),
;; you could do it this way:
;; (start code)
;;    ...
;;    i8 page = cpct_memPage6(0x84);   // Transform page into 6LSb parameter
;;    cpct_setVideoMemoryPage(page);   // ... and use it to set the Video Memory page
;;    ...
;; (end)
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

_cpct_setVideoMemoryPage::
   ;; Get the parameter from the stack
   ld   hl, #2       ;; [10] HL = SP + 2 (Place where parameter is)
   add  hl, sp       ;; [11]
   ld    a, (hl)     ;; [ 7] A = First Parameter (Video Memory Page to Set)

cpct_setVideoMemoryPage_asm::  ;; Assembly entry point

   ;; Select R12 Register from the CRTC and Write there the selected Video Memory Page
   ld   bc, #0xBC0C  ;; [10] 0xBC = Port for selecting CRTC Register, 0x0C = Selecting R12
   out (c), c        ;; [12] Select the R12 Register (0x0C to port 0xBC)
   inc   b           ;; [ 4] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   out (c), a        ;; [12] Write Selected Video Memory Page to R12 (A to port 0xBD)

   ret               ;; [10] Return
