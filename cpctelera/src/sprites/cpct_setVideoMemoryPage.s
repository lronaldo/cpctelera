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

;
;########################################################################
;### FUNCTION: cpct_setVideoMemoryPage                                ###
;########################################################################
;### Changes the place where the video memory starts. Concretely, it  ###
;### changes de Video Memory Page, which correponds to the 6 Most     ###
;### significant bits (MSb) from the memory address where video memo- ###
;### ry starts. By doing this, you are effectively changing what part ###
;### of the RAM will be used as Video Memory and, therefore, what is  ###
;### to be displayed from now on. With this you can affectively imple-###
;### ment double and triple buffer using hardware instead of copying. ###
;###                                                                  ###
;### This routine achieves this by changing Register R12 from CRTC.   ###
;### This register is the responsible for holding the 6 MSb of the    ###
;### place where Video Memory starts (its page). However, this regis- ###
;### ter stores this 6 bits as its 6 LSb (--bbbbbb). We have to take  ###
;### this into account to correctly set the page we want.             ###
;###                                                                  ###
;### Useful example:                                                  ###
;###  1. You want your Video Memory to start at 0x8000                ###
;###    > Then, your Video Memory Page is 0x80 (10000000).            ###
;###    > You call this routine with 0x20 as parameter (00100000)     ###
;###    > Note that 0x20 (00100000) is 0x80 (10000000) shifted twice  ###
;###      to the right. Your 6 MSb are to be passed as the 6 LSb.     ###
;########################################################################
;### INPUTS (1 Byte)                                                  ###
;###  * (1B A) New starting page for Video Memory (Only 6 LSb used)   ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, HL                           ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY: 14 bytes                                                 ###
;### TIME:   76 cycles                                                ###
;########################################################################
;
.globl _cpct_setVideoMemoryPage
_cpct_setVideoMemoryPage::
   ;; Get the parameter from the stack
   LD  HL, #2        ;; [10] HL = SP + 2 (Place where parameter is)
   ADD HL, SP        ;; [11]
   LD  A, (HL)       ;; [ 7] A = First Parameter (Video Memory Page to Set)

   ;; Select R12 Register from the CRTC and Write there the selected Video Memory Page
   LD  BC, #0xBC0C   ;; [10] 0xBC = Port for selecting CRTC Register, 0x0C = Selecting R12
   OUT (C), C        ;; [12] Select the R12 Register (0x0C to port 0xBC)
   INC B             ;; [ 4] Change Output port to 0xBD (B = 0xBC + 1 = 0xBD)
   OUT (C), A        ;; [12] Write Selected Video Memory Page to R12 (A to port 0xBD)

   RET               ;; [10] Return
