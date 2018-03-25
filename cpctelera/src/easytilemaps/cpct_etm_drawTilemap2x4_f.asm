;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_easytilemaps

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_etm_drawTilemap2x4_f
;;
;;    Draws a complete tilemap made of 2x4-bytes tiles.
;;
;; C Definition:
;;    void <cpct_etm_drawTilemap2x4_f> (<u8> *map_width*, <u8> *map_height*, 
;; <u8>* *pvideomem*, <u8>* *ptilemap*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;    (1B  A) map_width  - Width of the tilemap in tiles
;;    (1B  C) map_height - Height of the tilemap in tiles
;;    (2B DE) pvidemem   - Pointer to video memory location where the tilemap is to be drawn
;;    (2B HL) ptilemap   - Pointer to the start of the tilemap definition (2D tile-index matrix)
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawTilemap2x4_f_asm
;;
;; Parameter Restrictions:
;;    * *ptilemap* could be any 16-bits value, representing the memory address where 
;; the tilemap is stored. This function expects the parameter to point the start of
;; a 2D tile-index matrix (the tilemap) well populated. If the pointer points to any other 
;; thing, the result is undefined (most likely a crash or rubbish on the screen).
;;    * *pvideomem* must be the location in video memory or backbuffer where to draw
;; the tilemap. This location *must be* a pixel line 0 or pixel line 4, otherwise
;; the function may crash your program. To know what a pixel line 0/4 means, please
;; have a look at <cpct_drawSprite> documentation.
;;    * *map_width* represents the width of the tilemap in tiles. Theoretically any
;; width between 0 and 255 is possible. However, if width of the map times 4 is greater
;; than the screen width, unexpected behaviour may happen, as memory locations outside
;; video memory could be overwritten. 
;;    * *map_height* represents the height of the tilemap in tiles and is expected to
;; be in the range [1, 50], as 50*4 = 200 pixels, which is the size of a normal screen.
;; However, any size between 1 and 255 is theoretically possible. Anyway, this is not
;; recommended as it will usually lead to undefined behaviour, because memory locations
;; outside video memory could be overwritten.
;;
;; Known limitations:
;;     * This function does not do any kind of checking over the tilemap, its
;; contents or size. If you give a wrong pointer, your tilemap has different
;; dimensions than required or has less / more tiles than will be used later,
;; undefined behaviour may happen.
;;     * Internal ptileset *MUST BE SET* calling <cpct_etm_setTileset2x4> *BEFORE* 
;; calling this function. Otherwise, a NULL (0x0000) pointer will be used as tileset.
;;     * This function only draws 8-bytes tiles of size 2x4 (in bytes).
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * Under hardware scroll conditions, tile drawing will fail if asked to draw
;; near 0x?7FF or 0x?FFF addresses (at the end of each one of the 8 pixel lines), as 
;; next screen byte at that locations is -0x7FF and not +1 bytes away.
;;
;; Details:
;;    This function draws a complete tilemap on the screen or on a backbuffer. The 
;; *ptilemap* parameter is expected to point to a 2D tile-index matrix structure, 
;; having 1-byte values for each tile-index. The function traverses the tilemap 2D
;; matrix retrieving tiles one by one and drawing them on the screen or backbuffer.
;;
;;    This function uses an internal pointer to the tileset (ptileset). This internal
;; pointer *MUST BE SET BEFORE* using this function; otherwise, a NULL (0x0000) pointer
;; is used as pointer to the tileset, using information at the start of memory as
;; if it contained pointers to tiles (leading to undefined behaviour). To set the
;; ptileset pointer, the function <cpct_etm_setTileset2x4> can be used.
;;
;;    A tileset is expected to contain an array of pointers (2-byte values) to tile
;; definitions. Each pointed tile is expected to be an array of 2x4 bytes defining
;; the pixels of the tile in screen pixel format. Each one of this tiles is drawn
;; to screen or backbuffer using <cpct_drawTileAligned2x4_f> function (ASM bindings).
;;
;; Destroyed Register values: 
;;      AF,  BC,  DE,  HL
;;      AF', BC', DE', HL'
;;
;; Required memory:
;;      C-bindings - 44 bytes (+ 33 bytes from <cpct_drawTileAligned2x4_f>)
;;    ASM-bindings - 38 bytes (+ 33 bytes from <cpct_drawTileAligned2x4_f>)
;;
;; Time Measures: 
;; (start code)
;;    Case     |      microSecs (us)            |          CPU Cycles              |
;; ---------------------------------------------------------------------------------
;;    Any      | 27 + (21 + 103W)H + 9HO + 16HE | 108 + (84 + 412W)H + 36HO + 64HE |
;; ---------------------------------------------------------------------------------
;;  ASM saving |           - 17                 |            - 68                  |
;; ---------------------------------------------------------------------------------
;;  H=30, W=30 |       93.723 (4,69 VSYNCs)     |           374.892                |
;;  Start=Pix0 |               0,094 secs       |                                  |
;; ---------------------------------------------------------------------------------
;;  H=40, W=40 |      166.151 (8,32 VSYNCs)     |           664.604                |
;;  Start=Pix4 |               0,166 secs       |                                  |
;; ---------------------------------------------------------------------------------
;; (end code)
;;    W  - Map width (number of horizontal tiles)
;;    H  - Map height (number of vertical tiles)
;;    HO - Number of odd tile rows.
;;    HE - Number of even tile rows.
;; For HO and HE, the first row is considered odd if it starts at a pixel line 4,
;; and it is considered even otherwise. The last row of the tilemap is never taken 
;; into account (either for HO or HE).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Declare tile drawing function we are going to use
.globl cpct_etm_drawTileRow2x4_asm

   ld (set_HLp_nextRow+1), a ;; [4] Save Map_width into the placeholder to restore B at each height loop

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
   ld    b, #00        ;; [2] B = map_width (it must be restored for each new row to be drawn, #00 is a placeholder)

   ;; DE' = DE (DE' is the pointer to video memory which 
   ;; ... we are changing, so put the result in there
   ld   a, e           ;; [1] A = E
   exx                 ;; [1] Change to alternate register set
   ld   e, a           ;; [1] E' = A
   exx                 ;; [1] Back to normal register set
   ld   a, d           ;; [1] A = D
   exx                 ;; [1] Change to alternate register set
   ld   d, a           ;; [1] D' = A,  DE' = DE (Pointer to video memory passed to DE')
   exx                 ;; [1] Back to normal register set

   call cpct_etm_drawTileRow2x4_asm ;; [7 + 103B'] Draws the next tile Row (B' holds the width of the tilerow)

   dec  c                   ;; [1] 1 less tile row to draw
   jr  nz, drawtiles_height ;; [2/3] If there still are some tile rows to draw (C!=0), continue

   ;; Drawing ends
   ret                 ;; [3] Return