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
.module cpct_easytilemaps

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_etm_setDrawTilemap4x8_ag
;;
;;    Sets internal configuration values for <cpct_etm_drawTilemap4x8_ag>.
;;
;; C Definition:
;;    void <cpct_etm_setDrawTilemap4x8_ag> (<u8> *width*, <u8> *height*, 
;; <u16> *tilemapWidth*, const void* *tileset*) __z88dk_callee;
;;
;; Input Parameters (6 bytes):
;;    (1B  C) width        - Width in *tiles* of the view window to be drawn
;;    (1B  B) height       - Height in *tiles* of the view window to be drawn
;;    (2B DE) tilemapWidth - Width in *tiles* of the complete tilemap
;;    (2B HL) tileset      - Pointer to the start of the tileset definition (list of 32-byte tiles).
;;
;;    *Note*: it also uses current interrupt status (register I) as a value. 
;;            It should be considered as an additional parameter.
;;
;; Assembly call (Input parameters on Registers):
;;    > call cpct_etm_setDrawTilemap4x8_ag_asm
;;
;; Parameter Restrictions:
;;    * *width* (1-256) is the horizontal size in *tiles* of view window. The view window is 
;; the  part of the tilemap that will be drawn when calling <cpct_etm_drawTilemap4x8_ag> (A 
;; region of *width* x *height* tiles of the complete tilemap. This view window must be 
;; part of  the tilemap (or the whole tilemap), but never greater than it. Therefore, *width*
;; must be lower or equal to *tilemapWidth*.
;;    * *height* (1-256) is the vertical size in *tiles* of the view window. This value 
;; determines how much rows will be displayed when calling <cpct_etm_drawTilemap4x8_ag>. 
;; As it shows rows from a tilemap, it must be lower or equal to the height of the 
;; complete tilemap.
;;    * *tilemapWidth* (0-65535) is the horizontal size in *tiles* of the complete tilemap.
;; The tilemap could be wider than the view window, so that <cpct_etm_drawTilemap4x8_ag>
;; shows only part of it. This can be used to produce scrolling effects.
;;    * *tileset* is a pointer to an array of consecutive tile definitions. Each tile 
;; must be 4x8-bytes in size (32-bytes total). Tiles are small sprites to be drawn to
;; form the image of the tilemap. Each tile must be codified in zig-zag screen pixel
;; data (odd rows left-to-right, even rows right-to-left) and in Gray-code row order
;; (rows ordered 0,1,3,2,6,7,5,4). A maximum of 256-tiles are supported in the tileset
;; as the tilemap will use 1-byte as index to refer to each individual tile.
;;
;; Known issues:
;;    * This function *MUST be called at least once* before using <cpct_etm_drawTilemap4x8_ag>.
;; Otherwise, unexpected results will happen.
;;    * Parameters are not checked to be correct. Any mistake in sizes or *tileset* pointer
;; will lead to unexpected results like rubbish on the screen, data overwriting or crashes.
;;    * These functions do not require the height of the complete tilemap. It is up to the
;; programmer to know this value and ensure the view window remains inside the tilemap.
;;
;; Details:
;;    This function sets the internal values that <cpct_etm_drawTilemap4x8_ag> uses to
;; draw a view window of a tilemap in video memory or a hardware backbuffer. These 
;; internal values can be set once and used many times by <cpct_etm_drawTilemap4x8_ag>,
;; as they remain set until a new call to <cpct_etm_setDrawTilemap4x8_ag>. Values 
;; can be set and reset as many times as required. It is only important to remember 
;; that <cpct_etm_setDrawTilemap4x8_ag> has to be called at least once before using
;; <cpct_etm_drawTilemap4x8_ag>, as default values for sizes and pointers are set to 0, 
;; and that will produce unexpected results.
;;
;;    Internal values set include: *width* and *height* of the view window, width of the
;; complete tilemap (*tilemapWidth*), pointer to the *tileset* and interrupt status. 
;; <cpct_etm_drawTilemap4x8_ag> disables interrupts when it starts drawing every row,
;; and returns interrupts to their original status on finishing row drawing. If interrupts
;; are enabled when calling this setting function, interrupts will be set to be reenabled 
;; after every row drawn by <cpct_etm_drawTilemap4x8_ag>. Otherwise, interrupts will be 
;; set as disabled for all the processing of <cpct_etm_drawTilemap4x8_ag>.
;;
;;    Values to be set are shown in the next figure about memory,
;; (start code)
;; ******************************** FIGURE 1 ***************************************************
;;
;; /-----------------------------------------\                   ____________________
;; |                MEMORY                   |                  / COMPLETE |6x5 VIEW \
;; |-----------------------------------------|                  | TILEMAP  | WINDOW  |
;; |    addresses & contents in hexadecimal  |                  |----------|---------|
;; |-----------------------------------------|                  | ######## |         |
;; |ADDRESS|         CONTENTS                |                  | #    **# |         |
;; |-----------------------------------------| 8x8 tilemap      | # |  **# |  |  **# |
;; |        /// 8x8 Tilemap definition ///   | starts at        | #/// 8 # |  // 8 # |
;; |  4000 |[01] 01  01  01  01  01  01  01  |<-/  memory       | #===888# |  ==888# |
;; |  4008 | 01  00  00  00  00  09  09  01  |                  | #=o= | # |  o= | # |
;; |       |       /-----------------------\ | << Width = 6 \   | #=^=_|_# |  ^=_|_# |
;; |  4010 | 01  00|(06) 00  00  09  09  01| | ^            |   | ######## |         |
;; |  4018 | 01  05| 05  05  00  08  00  01| | |            |   \--------------------/
;; |  4020 | 01  02| 02  02  08  08  08  01| | | Height = 5 |   
;; |  4028 | 01  02| 03  02  00  06  00  01| | |            | 6x5 View Window
;; |  4030 | 01  02| 04  02  07  06  07  01| | v            |starts at (0x4012)
;; |       |       \-----------------------/ |              /
;; |  4038 | 01  01  01  01  01  01  01  01  |
;; |         <------------------------------> Complete tilemapWidth = 8 tiles = 8 bytes
;; |-----------------------------------------|
;; |        /// Tileset Definition ///       |    tileset starts at 0x6200 in memory
;; |  6200 |[00  00  00  00  00  00  00  00  |^ <-/      
;; |  6208 | 00  00  00  00  00  00  00  00  || Tile 0     _____________________________________
;; |  6210 | 00  00  00  00  00  00  00  00  || Definition | TILE 0 | TILE 1 | TILE 2 | TILE 3 |
;; |  6218 | 00  00  00  00  00  00  00  00] |V            | [6200] | (6220) | <6240> | {6260} |  
;; |  6220 |(00  AA  55  00  00  55  AA  00  |^            |________|________|________|________|  
;; |  6228 | 00  AA  55  00  FF  FF  FF  FF  || Tile 1     |        |  X  X  |        |        |  
;; |  6230 | 00  AA  55  00  00  55  AA  00  || Definition |        |  X  X  |        |        |  
;; |  6238 | FF  FF  FF  FF  00  55  AA  00) |v            |        |XXXXXXXX|XXXXXXXX|  XXXX  |  
;; |  6240 |<00  00  00  00  00  00  00  00  |^            |        |  X  X  |        | X    X |  
;; |  6248 | 00  00  00  00  FF  FF  FF  FF  || Tile 2     |        |  X  X  |        |X      X|  
;; |  6250 | 00  00  00  00  00  00  00  00  || Definition |        |XXXXXXXX|XXXXXXXX|X      X|  
;; |  6258 | FF  FF  FF  FF  00  00  00  00> |v            |        |  X  X  |        | X    X |  
;; |  6260 |{00  00  00  00  00  00  00  00  |^            |        |  X  X  |        |  XXXX  |  
;; |  6268 | 55  00  00  AA  00  FF  FF  00  || Tile 3     |------------------------------------  
;; |  6270 | 55  00  00  AA  00  FF  FF  00  || Definition   32-bytes per tile. 8 rows of 4 bytes
;; |  6278 | AA  00  00  55  55  00  00  AA} |v                  (8 mode 0 pixels per row)
;; | ..... | ............................... |
;; |-------|---------------------------------|
;; (end code)
;;
;;    Figure 1 clarifies the definitions for *width* x *height* (the view window), 
;; the complete *tilemapWidth*, and the *tileset*. In order to draw the view window of
;; the figure 1, the next two calls would be needed in the code,
;; 
;; (start code)
;;    // We assume tileset and tilemap are already defined. 
;;    // As tileset is a pointer to the start of the set of tiles, tileset=0x6200
;;    // Similarly, tilemap points to the start of the map, so tilemap=0x4000
;; 
;;    // Set up drawTilemap for drawing a view of 6x5 tiles
;;    cpct_etm_setDrawTilemap4x8_ag (6, 5, 8, tileset);
;;
;;    // We draw the view of the tilemap at the start of video memory. As the first
;;    // tile we want to draw is 0x12 (18) bytes beyond the start of the tilemap
;;    // array, we only need to add this offset to the address where tilemap starts
;;    // to pass the address of that tile to the function, as the first tile that will be drawn
;;    cpct_etm_drawTilemap4x8_ag (CPCT_VMEM_START, tilemap + 0x12);
;; (end code)
;;
;;    With these two calls we set up a view window of 6x5 tiles inside a *tilemap* 
;; with a *tilemapWidth* of 8 tiles, and a *tileset* that starts at 0x6200 in memory.
;; After setting it up, we draw the view at the start of video memory, passing a 
;; pointer to the first tile to be drawn to <cpct_etm_drawTilemap4x8_ag> (*tilemap* + 0x12).
;;
;;    With respect to the *tileset*, it is important to notice that tiles must be
;; codified in zig-zag screen pixel data and in Gray-code row order (0,1,3,2,6,7,5,4).
;; Next figure shows this configuration in detail,
;; (start code)
;; ************************ FIGURE 2 ******************************
;;
;; /------------------------\
;; |         MEMORY         |                           MODE 0
;; |------------------------|                         SCREEN VIEW
;; | values in hexadecimal  |--------------------   /-------------\
;; |------------------------|TILE|DRAW | ROW    |   |TILE| SCREEN |
;; |ADDRESS|   CONTENTS     |ROW |SENSE| CONTENT|   |ROW | RESULT |
;; |------------------------|----|---------------   |----|--------|
;; |  6260 |{00  00  00  00 | 0  | >>  |        |   | 0  |        |  
;; |  6264 | 00  00  00  00 | 1  | <<  |        |   | 1  |        |  
;; |  6268 | 55  00  00  AA | 3  | >>  | X    X |   | 2  |  XXXX  |  
;; |  626C | 00  FF  FF  00 | 2  | <<  |  XXXX  |   | 3  | X    X |  
;; |  6270 | 55  00  00  AA | 6  | >>  | X    X |   | 4  |X      X|  
;; |  6274 | 00  FF  FF  00 | 7  | <<  |  XXXX  |   | 5  |X      X|  
;; |  6278 | AA  00  00  55 | 5  | >>  |X      X|   | 6  | X    X |  
;; |  627C | 55  00  00  AA}| 4  | <<  |X      X|   | 7  |  XXXX  |  
;; \------------------------|-------------------/   \----|--------/
;; (end code)
;;
;;    This way of codifying tiles is optimal with respect to drawing them in the 
;; video memory. The drawing starts at 0x6260 and copies first 4 bytes in order to
;; screen row 0. After that, next 4 bytes at 0x6264 are read in the same order, but
;; they are drawn right-to-left at row 1 in the screen. Therefore, screen pointer
;; does a zig-zag drawing (left-to-right, down, right-to-left, down, repeat).
;; Memory codification of the tile is designed to always have the next byte to be
;; drawn at the next memory location. Hence, tile data is read consecutively from
;; 0x6260 to 0x627F. 
;;
;;    CPCtelera automatic conversion utilities produce sprites and tiles in this
;; configuration automatically. Use <cpct_img2tileset> -z option for this purpose.
;; You may also configure automatic conversions you require in your image_conversion.mk
;; file of your projects.
;;
;; Destroyed Register values: 
;;      AF, DE
;;
;; Required memory:
;;      C-bindings - 48 bytes (+165 bytes from <cpct_etm_drawTilemap4x8_ag>-cbindings which is included)
;;    ASM-bindings - 44 bytes (+153 bytes from <cpct_etm_drawTilemap4x8_ag>-asmbindings which is included)
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles  
;; ------------------------------------------
;;    Any      |      71        |    284
;; ------------------------------------------
;; ASM Saving  |     -15        |    -60
;; ------------------------------------------
;; (end code)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; LOCAL MACRO: setDrawTileMap4x8_ag_gen
;;    All code function is defined as a macro to prevent code duplication on reuse
;; between ASM/C bindings. This macro generates the code for setDrawTileMap4x8_ag 
;; and couples it to the appropriate labels that it has to modify. A label prefix 
;; is passed to generate different kinds of labels for different bindings.
;;    Therefore, ASM/C bindings include this file and use this macro to generate
;; specific source code that will be compiled, with the appropriate labels.
;;
.macro setDrawTileMap4x8_ag_gen lblPrf

;; Declare global symbols used here
.globl lblPrf'tilesetPtr
.globl lblPrf'widthHeightSet
.globl lblPrf'restoreWidth
.globl lblPrf'updateWidth
.globl lblPrf'incrementHL
.globl lblPrf'restoreI

   ;; Set (tilesetPtr) placeholder
   ld (lblPrf'tilesetPtr), hl     ;; [5] Save HL into tilesetPtr placeholder

   ;; Set all Width values required by drawTileMap4x8_ag. First two values
   ;; (heightSet, widthSet) are values used at the start of the function for
   ;; initialization. The other one (restoreWidth) restores the value of the
   ;; width after each loop, as it is used as counter and decremented to 0.
   ld (lblPrf'widthHeightSet), bc ;; [6]
   ld     a, c                    ;; [1]
   ld (lblPrf'restoreWidth), a    ;; [4] Set restore width after each loop placeholder
   
   ;; In order to properly show a view of (Width x Height) tiles from within the
   ;; tilemap, every time a row has been drawn, we need to move tilemap pointer
   ;; to the start of the next row. As the complete tilemap is (tilemapWidth) bytes
   ;; wide and we are showing a view only (Width) tiles wide, to complete (tilemapWidth)
   ;; bytes at each loop, we need to add (tilemapWidth - Width) bytes.
   sub_de_a                      ;; [7] tilemapWidth - Width
   ld (lblPrf'updateWidth), de   ;; [6] set the difference in updateWidth placeholder

   ;; Calculate HL update that has to be performed for each new row loop.
   ;; HL advances through video memory as tiles are being drawn. When a row
   ;; is completely drawn, HL is at the right-most place of the screen.
   ;; As each screen row has a width of 0x50 bytes (in standard modes), 
   ;; if the Row that has been drawn has less than 0x50 bytes, this difference
   ;; has to be added to HL to make it point to the start of next screen row.
   ;; As each tile is 4-bytes wide, this amount is (0x50 - 4*Width). Also,
   ;; taking into account that 4*Width cannot exceed 255 (1-byte), a maximum
   ;; of 63 tiles can be considered as Width.
   ld     a, c                ;; [1] A = Width
   add    a                   ;; [1] A = 2*Width
   add    a                   ;; [1] A = 4*Width
   cpl                        ;; [1] A = -4*Width - 1
   add #0x50 + 1              ;; [2] A = -4*Width-1 + 0x50+1 = 0x50 - 4*Width
   ld (lblPrf'incrementHL), a ;; [4] Set HL increment in its placeholder

   ;; Set the restoring of Interrupt Status. drawTileMap4x8_ag disables interrupts before
   ;; drawing each tile row, and then it restores previous interrupt status after the row
   ;; has been drawn. To do this, present interrupt status is considered. This code detects
   ;; present interrupt status and sets a EI/DI instruction at the end of tile row drawing
   ;; to either reactivate interrupts or preserve interrupts disabled.
   ld     a, i             ;; [3] P/V flag set to current interrupt status (IFF2 flip-flop)
   ld     a, #opc_EI       ;; [2] A = Opcode for Enable Interrupts instruction (EI = 0xFB)
   jp    pe, int_enabled   ;; [3] If interrupts are enabled, EI is the appropriate instruction
     ld   a, #opc_DI       ;; [2] Otherwise, it is DI, so A = Opcode for Disable Interrupts instruction (DI = 0xF3)
int_enabled:
   ld (lblPrf'restoreI), a ;; [4] Set the Restore Interrupt status at the end with corresponding DI or EI

   ret                     ;; [3] Return to caller

.endm
