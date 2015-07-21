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
;;    Draws a complete tilemap of type <cpct_TEasyTilemap>
;;
;; C Definition:
;;    void <cpct_etm_drawFullTilemap> (void* *ptilemap*) __z88dk_fastcall;
;;
;; Input Parameters (2 bytes):
;;    (2B HL) ptilemap - Pointer to the <cpct_TEasyTilemap> structure defining the tilemap.
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawFullTilemap_asm
;;
;; Parameter Restrictions:
;;    * *ptilemap* could be any 16-bits value, representing the memory 
;; address where the <cpct_TEasyTilemap> structure is stored. This function expects
;; the parameter to point to a complete structure like this: if the structure is not
;; well populated or the pointer points to other thing, the result is undefined (most
;; likely a crash or rubbish on the screen).
;;
;; Known limitations:
;;     * This function does not do any kind of checking over the tilemap, its
;; contents or size. If you give a wrong pointer, your tilemap has different
;; dimensions than required or has less / more tiles than will be used later,
;; rubbish can appear on the screen.
;;     * Inside the structure, the pointer to the video memory *must* point to a
;; pixel line 0 or a pixel line 4 from any character line on the screen or back buffer.
;; To know where pixel lines 0/4 are located you may have a look at <cpct_drawSprite> 
;; documentation.
;;     * This function only draws 8-bytes tiles of size 2x4 (in bytes).
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * Under hardware scroll conditions, tile drawing will fail if asked to draw
;; near 0x?7FF or 0x?FFF addresses (at the end of each one of the 8 pixel lines), as 
;; next screen byte at that locations is -0x7FF and not +1 bytes away.
;;
;; Details:
;;    This function draws a complete tilemap on the screen or on a backbuffer. The function
;; receives the tilemap as a pointer to a <cpct_TEasyTilemap> structure. This structure
;; contains a tilemap definition (matrix of tile indexes), a tileset definition (array of
;; pointers to tiles), a pointer to video memory or backbuffer (the location to draw) and 
;; the height and width of the tilemap in tiles. With this complete information, the
;; function traverses the tilemap matrix and draws tiles one by one. 
;;
;;    Each tile from the tileset must be a 8-bytes, 2x4-sized tile. This function calls
;; <cpct_drawTileAligned2x4_f> for drawing each one of the tiles. 
;;
;; Destroyed Register values: 
;;      AF,  BC,  DE,  HL
;;      AF', BC', DE', HL'
;;
;; Required memory:
;;      94 bytes (+ 33 bytes from <cpct_drawTileAligned2x4_f>)
;;
;; Time Measures:
;; (start code)
;;    Case     |      microSecs (us)            |          CPU Cycles              |
;; ---------------------------------------------------------------------------------
;;    Any      | 70 + (10 + 104W)H + 9HO + 16HE | 280 + (40 + 416W)H + 36HO + 64HE |
;; ---------------------------------------------------------------------------------
;;  H=30, W=30 |       94.336 (4,68 VSYNCs)     |           377.344                |
;;  Start=Pix0 |               0,09 secs        |                                  |
;; ---------------------------------------------------------------------------------
;;  H=40, W=40 |      167.354 (8,38 VSYNCs)     |           669.416                |
;;  Start=Pix4 |               0,17 secs        |                                  |
;; ---------------------------------------------------------------------------------
;; (end code)
;; W  = Map width (number of horizontal tiles)
;; H  = Map height (number of vertical tiles)
;; HO = Number of odd tile rows.
;; HE = Number of even tile rows.
;; For HO and HE, the first row is considered odd if it starts at a pixel line 4,
;; and it is considered even otherwise. The last row of the tilemap is never taken 
;; into account (either for HO or HE).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Declare tile drawing function we are going to use
.globl cpct_drawTileAligned2x4_f_asm

;;
;; Macros for using IXL and IXH in a comfortable way
;; 
.macro ld__ixh_d
   .dw #0x62DD    ;; ld ixh, d
.endm
.macro ld__ixl_e
   .dw #0x6BDD    ;; ld ixl, e
.endm

_cpct_etm_drawFullTilemap::
cpct_etm_drawFullTilemap_asm::
   ;; Parameter is passed in HL (Pointer to TEasyTilemap Structure)
   push ix             ;; [5] Save IX to restore it later
   ex   de, hl         ;; [1] DE = HL 
   ld__ixh_d           ;; [2] IX = DE (IX Points to TEasyTilemap)
   ld__ixl_e           ;; [2]

   ld    h, 1(ix)      ;; [5] HL = Pointer to the tilemap (ptilemap)
   ld    l, 0(ix)      ;; [5] 
   ld    a, 6(ix)      ;; [5] A = Map width
   ld (set_HLp_nextRow+1), a ;; [4] Save Map_width into the placeholder to restore B at each height loop
   ld    c, 7(ix)      ;; [5] C = Map height
   ld    d, 5(ix)      ;; [5] DE = Pointer to video memory (place to draw)
   ld    e, 4(ix)      ;; [5] 
   exx                 ;; [1] Change to Alternative Register Set
   ld    h, 3(ix)      ;; [5] HL' = Pointer to the tileset (ptileset)
   ld    l, 2(ix)      ;; [5] 
   ld (restore_ptileset+1), hl ;; [5] Save tileset pointer for fast restoring into BC'
   exx                 ;; [1] Back to normal register set

   jr  set_HLp_nextRow ;; [3] Start drawing the first row of tiles

drawtiles_height:
   ld    a, d          ;; [1] DE += 0x2000 (0x800 x 4) to jump 4 pixel lines from here
   add  #0x20          ;; [2] 
   ld    d, a          ;; [1]
   and  #0x38          ;; [2] Check bits 11,12,13 of D to know if we have jumped to a 
                       ;;     ... new character line (pixel line 0, all three bits = 0)
   jr nz, set_HLp_nextRow ;; [2/3] Not 0 => Pixel Line 4 => No need to adjust

   ;; We are jumping to a new pixel line 0, 
   ;; so we have to jump to the next character line (adding 0xC050)
   ld    a, e          ;; [1] DE += 0xC050
   add  #0x50          ;; [2] ... jump 4 lines and, as we will overflow video memory 
   ld    e, a          ;; [1] ... add 0xC050 to move to next character line
   ld    a, d          ;; [1]  
   adc  #0xC0          ;; [2]
   ld    d, a          ;; [1] 

set_HLp_nextRow:
   ld    b, #00    ;; [2] B = map_width (it must be restored for each new row to be drawn, #00 is a placeholder)

   ;; HL' = DE (HL' is the pointer to video memory which 
   ;; ... we are changing, so put the result in there
   push  de        ;; [4] Save DE (Pointer to next video memory line)
   exx             ;; [1] Change to alternate register set
   pop   de        ;; [3] DE' = DE, now pointing to next video memory line 
   exx             ;; [1] Back to normal register set

drawtiles_width:
   ld    a, (hl)   ;; [2] A = tilenum (tile index in the tileset)

   exx             ;; [1] Change to Alternative Register Set
   
restore_ptileset:
   ld   bc, #0000  ;; [3] BC' points again to the tileset (#0000 is a placeholder for ptileset)

   ;; Calculate HL' = BC' + 2A to point to a pointer to definition of the tilenum tile
   ;;    as BC' points to the start of the tileset pointer vector, and A is the index
   ld    l, a      ;; [1] <| HL' = A
   ld    h, #0     ;; [2] <|
   add  hl, hl     ;; [3] HL' = 2A
   add  hl, bc     ;; [3] HL' = BC' + 2A

   ;; Now make DE' = tilenum tile definition, which is pointed by (HL')
   ld    a, (hl)   ;; [2] A = LSB of the tile pointer
   inc  hl         ;; [2]
   ld    h, (hl)   ;; [2] H = HSB of the tile pointer
   ld    l, a      ;; [1] HL = tile pointer

   push de                             ;; [4] Save DE'
   call cpct_drawTileAligned2x4_f_asm  ;; [5+59] Draw the tile
   pop  de                             ;; [3] Restore DE'

   inc  de                  ;; [2] DE' += 2 (DE' += tilewidth, so that DE' points to the place in 
   inc  de                  ;; [2]   video memory where next till will be drawn

   exx                      ;; [1] Back to normal Register Set
   inc  hl                  ;; [2] Point to next item in the tilemap
   djnz drawtiles_width     ;; [3/4] IF B!=0, continue with next tile from this line

   dec  c                   ;; [1] 1 less tile row to draw
   jr  nz, drawtiles_height ;; [2/3] If there still are some tile rows to draw (C!=0), continue

   ;; Drawing end
   pop  ix         ;; [5] Restore IX before returning
   ret             ;; [3] Return