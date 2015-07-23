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
;; Function: cpct_etm_drawTileRow
;;
;;    Draws a row of tiles from a tilemap.
;;
;; C Definition:
;;    void <cpct_etm_drawTileRow> (u8 *numtiles*, const void* *pvideomem*, 
;;                                 const void* *ptilemap*) __z88dk_callee;
;;
;; Input Parameters (5 bytes):
;;    (1B  A) numtiles  - Number of tiles from this row to draw
;;    (2B DE) pvideomem - Pointer to the video memory byte where to draw the tile row
;;    (2B HL) ptilemap  - Pointer to the tilemap byte where the definition of the row starts
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawTileRow_asm
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
;;
;; Required memory:
;;      94 bytes (+ 33 bytes from <cpct_drawTileAligned2x4_f>)
;;
;; Time Measures:
;; (start code)
;;    Case     |      microSecs (us)            |          CPU Cycles              |
;; ---------------------------------------------------------------------------------
;;    Any      | 
;; ---------------------------------------------------------------------------------
;; ---------------------------------------------------------------------------------
;; (end code)
;; W  = Map width (number of horizontal tiles)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Declare tile drawing function we are going to use
.globl cpct_drawTileAligned2x4_f_asm

drawtiles_width:
   ld    a, (hl)   ;; [2] A = tilenum (tile index in the tileset)
  
restore_ptileset:  ;; set_tileset must be called before using this function
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

   inc  hl                  ;; [2] Point to next item in the tilemap
   djnz drawtiles_width     ;; [3/4] IF B!=0, continue with next tile from this line

   ret                      ;; [3] Return
