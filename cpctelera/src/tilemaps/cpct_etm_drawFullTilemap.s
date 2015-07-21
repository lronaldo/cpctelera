;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_tilemaps

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_etm_drawFullTilemap
;;
;;    Draws a complete tilemap of type TEasyTilemap
;;
;; C Definition:
;;    void <cpct_etm_drawFullTilemap> (void* *ptilemap*) __z88dk_fastcall;
;;
;; Input Parameters (2 bytes):
;;  (2B HL) ptilemap - Pointer to the TEasyTilemap structure defining the tilemap
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawFullTilemap
;;
;; Parameter Restrictions:
;;    * *ptilemap* could be any 16-bits value, representing the memory 
;; address where the tilemap structure is stored.
;;
;; Known limitations:
;;     * This function does not do any kind of checking over the tilemap, its
;; contents or size. If you give a wrong pointer, your tilemap has different
;; dimensions than required or has less / more tiles than will be used later,
;; rubbish can appear on the screen.
;;
;; Details:
;;
;;
;; Destroyed Register values: 
;;    C Call   - AF, HL
;;    ASM Call - none
;;
;; Required memory:
;;      bytes
;;
;; Time Measures:
;; (start code)
;;    Case    | Cycles | microSecs (us)
;; ---------------------------------
;;    Any     |    
;; ---------------------------------
;; Asm saving |   
;; ---------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Declare tile drawing function we are going to use
.globl cpct_drawTileAligned2x4_f_asm

_cpct_etm_drawFullTilemap::
   ;; Parameter is passed in HL (Pointer to TEasyTilemap Structure)
   push ix     ;; [5] Save IX to restore it later
   ex  de, hl  ;; [1] DE = HL 
   ld  ixh, d  ;; [2] IX = DE (IX Points to TEasyTilemap)
   ld  ixl, e  ;; [2]

   ld  l, 0(ix);; [5] HL Points to tilemap
   ld  h, 1(ix);; [5]
   ld  b, 7(ix);; [5] B = Map width
   ld  c, 8(ix);; [5] C = Map height
   exx         ;; [1] Change to Alternative Register Set
   ld  l, 2(ix);; [5] HL' points to the tileset (ptileset)
   ld  h, 3(ix);; [5]
   ld (restore_ptileset+1), hl ;; [5] Save tileset pointer for fastly restoring into bc
   ld  c, l    ;; [1] BC' points to the tileset (ptileset)
   ld  b, h    ;; [1]
   ld  l, 4(ix);; [5] HL' points to video memory (place to draw)
   ld  h, 5(ix);; [5]
   exx         ;; [1] Back to normal register set
   jr  drawtiles_width ;; [3]

drawtiles_height:
   ;; Next line  HL' > Next line
   ld    a, d   ;; [1] Check bits 11,12,13 of D to know if DE is pointing
   and  #0x38   ;; [2] to a pixel line 0 or a pixel line 4
   jr nz, case2 ;; [2/3] Not 0 => Pixel Line 4 => Case 2

case1:          ;; This is a pixel line 0, jump to pixel line 4
   ld    a, d   ;; [1] DE += 0x2000 (0x800 x 4)
   add  #0x20   ;; [2] ... to jump 4 pixel lines
   ld    d, a   ;; [1]
   jr set_HLp_nextRow ;; [3] Continue drawing next row

case2:          ;; This is a pixel line 4, jump to next pixel line 0
   ld    a, e   ;; [1] DE += 0xE050 (0x2000 + 0xC050)
   add  #0x50   ;; [2] ... jump 4 lines and, as we will overflow video memory 
   ld    e, a   ;; [1] ... add 0xC050 to move to next character line
   ld    a, d   ;; [1]  
   adc  #0xE0   ;; [2]
   ld    d, a   ;; [1]  

set_HLp_nextRow:
   ;; HL' = DE (HL' is the pointer to video memory which 
   ;; ... we are changing, so put the result in there
   push  de     ;; [4] Save DE (Pointer to next video memory line)
   exx          ;; [1] Change to alternate register set
   pop   hl     ;; [3] HL' = DE, now pointing to next video memory line 
   exx          ;; [1] Back to normal register set

drawtiles_width:
   ld    a, (hl)   ;; [2] DA = tilenum

   exx             ;; [1] Change to Alternative Register Set
   ld    d, #0     ;; [2]<| D'A = 8*D'A = 8*tilenum
;; beware previous carry
   cp    a         ;; [1]---Reset carry flag
   rla             ;; [1] |     8    = size of a tile
   rl    d         ;; [2] |     8*D'A = size of D'A tiles (from 0 to tilenum)
   rla             ;; [1] |     this is the offset from the start of the tileset
   rl    d         ;; [2] |     where tilenum starts.
   rla             ;; [1] |
   rl    d         ;; [2]<|

   add   c         ;; [1]<|DE' = D'A + BC' = ptileset + 8*tilenum
   ld    e, c      ;; [1] |     We add tilenum offset to the start of tileset, so
   ld    a, d      ;; [1] | ... DE' points to the start of the tile definition (tilenum)
   adc   b         ;; [1] |
   ld    d, b      ;; [1]<|

   push hl         ;; [4] Save HL'
   call cpct_drawTileAligned2x4_f_asm  ;; [5+59] Draw the tile
restore_ptileset:
   ld   bc, #0000  ;; [3] BC' points again to the tileset (#0000 is a placeholder)
   pop  hl         ;; [4] Restore HL'
   inc  hl         ;; [2] HL' += 2 (Point to the location where to draw next tile)
   inc  hl         ;; [2] 
   exx             ;; [1] Back to normal Register Set
   djnz drawtiles_width ;; [3/4] IF B!=0, continue with next tile from this line

   dec  c                   ;; [1] 
   jr  nz, drawtiles_height ;; [2/3]

drawing_end:
   pop  ix         ;; [5] Restore IX before returning
   ret             ;; [3] Return