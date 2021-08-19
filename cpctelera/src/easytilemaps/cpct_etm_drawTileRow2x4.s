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
;; Function: cpct_etm_drawTileRow2x4
;;
;;    Draws a given number of consecutive 2x4-bytes tiles of a tilemap as a row.
;;
;; C Definition:
;;    void <cpct_etm_drawTileRow2x4> (<u8> *numtiles*, void* *pvideomem*, 
;;                                    const void* *ptilemap_row*) __z88dk_callee;
;;
;; Input Parameters (5 bytes):
;;    (1B   B) numtiles     - Number of tiles from the tilemap to draw in a row
;;    (2B DE') pvideomem    - Pointer to the video memory byte where to draw the tile row
;;    (2B  HL) ptilemap_row - Pointer to the first tile from the tilemap that we want to draw
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawTileRow2x4_asm
;;    This function requires parameters divided into the 2 sets of registers of the Z80. 2
;; parameters must be on the alternate register set, while 1 has to be on the main register
;; set. An example call to this function will be:
;; (start code)
;;   ld hl, #ptilemap_row             ;; HL points to the first tile from the tilemap we want to draw
;;   exx                              ;; Change to the alternate register set
;;   ld  b, #row_width                ;; B' holds the number of tiles to draw from the tilemap
;;   ld de, #pvideomem                ;; DE points to the place on video memory where we are going to draw
;;   exx                              ;; Return to the main register set (Very important to do this!)
;;   call cpct_etm_drawTileRow2x4_asm ;; Finally, call the function
;; (end code)
;;
;; Parameter Restrictions:
;;    * *ptilemap_row* and *pvideomem* could be any 16-bits value, representing the memory 
;; addresses where the first tile to be drawn is and the location on the video memory where to
;; draw the row, respectively. This does not do any check on these parameters and, if they
;; are badly provided, the result is undefined (most likely a crash or rubbish on the screen).
;;    * *ptilemap_row* should point to a tile index inside a tilemap (a 2D matrix of 8-bit tile
;; indexes). The tile pointed and next *numtiles* consecutive ones should all be valid tile 
;; indexes from the tilemap. Otherwise, undefined results may happen (most likely rubbish or
;; incoherent tiles on the screen).
;;    * *pvideomem* must point to a valid pixel line (0 to 4) for from video memory or
;; from a backbuffer. To know which pixel lines are considered 0 to 4 you may have a look
;; at <cpct_drawSprite> documentation.
;;    * *numtiles* is expected to be the number of tiles to draw. There is no restriction 
;; on the number and, again, the function does not do any kind of check. Providing a number
;; of tiles greater than the tiles available in the row or greater than the space for tiles
;; available on the screen will usually lead to graphical glitches (displaced tiles), rubbish
;; on the screen and, occasionally a crash.
;;
;; Known limitations:
;;     * This function does not do any kind of check on the parameters. It is up to the
;; programmer to provide correct values for them.
;;     * This function *will not properly work* if <cpct_etm_setTileset2x4> has not been
;; called previously, as the tileset pointer will not have been set.
;;     * This function only draws 8-bytes tiles of size 2x4 (in bytes). It uses 
;; <cpct_drawTileAligned2x4_f> (assembly bindings), so this function will be included in the 
;; code and called.
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * Under hardware scroll conditions, tile drawing will fail if asked to draw
;; near 0x?7FF or 0x?FFF addresses (at the end of each one of the 8 pixel lines), as 
;; next screen byte at that locations is -0x7FF and not +1 bytes away.
;;
;; Details:
;;    This function draws a screen row of tiles coming from a tilemap definition. The function
;; walks through the tilemap retrieving tile indexes one by one, getting pointers to
;; the tiles from the default tileset, using tile indexes, and calling <cpct_drawTileAligned2x4_f> 
;; for each tile.
;;
;;    The function *needs* the tileset to have been previously set, which can be done by
;; calling <cpct_etm_setTileset2x4>. If this has not been done when <cpct_etm_drawTileRow2x4> is
;; called, then NULL (0x0000) is used as pointer to the tileset, yielding unexpected results.
;;
;; Destroyed Register values: 
;;      AF,   B,  HL
;;      AF', BC', DE',  HL'
;;
;; Required memory:
;;      33 bytes (+ 33 bytes from <cpct_drawTileAligned2x4_f>)
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles  
;; ------------------------------------------
;;    Any      |  21 + 103W     | 84 + 412W
;; ------------------------------------------
;; ASM Saving  |     -19        |    -76
;; ------------------------------------------
;; (end code)
;;    W - Map width (number of horizontal tiles)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Declare tile drawing function we are going to use
.globl cpct_drawTileAligned2x4_f_asm

;; C bindings for <cpct_etm_drawTileRow2x4>
;;   --> Included here not to duplicate dtr_restore_ptileset
;;
;;    19 microSecs, 7 bytes
;;
_cpct_etm_drawTileRow2x4::   
   ;; Recover parameters from the stack
   pop hl           ;; [3] HL = Return Address
   dec sp           ;; [2] As B is 1-byte value, we move SP to get B and something irrelevant in C
   pop bc           ;; [3] B = Number of tiles in this row
   exx              ;; [1] Change to alternate Register Set
   pop de           ;; [3] DE' = Pointer to video memory where to draw the tiles

   exx              ;; [1] Return to normal register set
   ex (sp), hl      ;; [6] HL = Pointer to the start of the tilemap row
                    ;; ... also putting again Return Address where SP is located now
                    ;; ... as this function is using __z88dk_callee convention

cpct_etm_drawTileRow2x4_asm:: ;; Assembly entry point
drawtiles_width:
   ld    a, (hl)   ;; [2] A = tilenum (tile index in the tileset)
  
   exx             ;; [1] Change to Alternative Register Set

dtr_restore_ptileset:: ;; set_tileset must be called before using this function
   ld   bc, #0000      ;; [3] BC' points again to the tileset (#0000 is a placeholder for ptileset)

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

   inc  de              ;; [2] DE' += 2 (DE' += tilewidth, so that DE' points to the place in 
   inc  de              ;; [2]   video memory where next till will be drawn

   exx                  ;; [1] Back to normal Register Set
   inc  hl              ;; [2] Point to next item in the tilemap
   djnz drawtiles_width ;; [2/3] IF A!=0, continue with next tile from this line

   ret                  ;; [3] Return
