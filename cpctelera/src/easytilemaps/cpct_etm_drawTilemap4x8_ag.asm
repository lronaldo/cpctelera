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
;; Function: cpct_etm_drawTilemap4x8_ag
;;
;;    Draws an aligned view of a tilemap made of 4x8-bytes tiles. Tiles must be 
;; codified as zig-zagged rows (left-to-right, then right-to-left) and with 
;; scanlines in Gray-code order 0,1,3,2,6,7,5,4 (zgtiles format).
;;
;; C Definition:
;;    void <cpct_etm_drawTilemap4x8_ag> (void* *memory*, const void* *tilemap*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;    (2B HL) memory  - Video memory location where to draw the tilemap (character & 4-byte aligned)
;;    (2B DE) tilemap - Pointer to the upper-left tile of the view to be drawn of the tilemap
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawTilemap4x8_ag_asm
;;
;; Parameter Restrictions:
;;    * *memory* must be the location in video memory or backbuffer where to draw
;; the tilemap. This location *must be* a *4-byte-aligned* location at a 
;; *character pixel line 0*. *4-byte-aligned* means this address must be divisible
;; by 4. Details about character pixel line 0 are explained in <cpct_drawSprite> documentation.
;;    * *tilemap* must be the memory address of the first tile to be drawn inside a Tilemap. 
;; A Tilemap is a 2D tile-index matrix at 1-byte-per-tile. You may point to any tile inside
;; a Tilemap and that one will be considered the upper-left corner of the view to be drawn.
;; Please, consult details section for a deeper explanation.
;;
;; Known limitations:
;;     * This function *SHALL NOT* be used without a previous call to 
;; <cpct_etm_setDrawTilemap4x8_ag>. This call is required to configure view 
;; sizes and the tileset to be used.
;;     * This function does not do any kind of checking over the tilemap, its
;; contents or size. If you give a wrong pointer, your tilemap has different
;; dimensions than required or has less / more tiles than will be used later,
;; undefined behaviour may happen.
;;     * This function only draws 32-bytes tiles of size 4x8 (in bytes).
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * Under hardware scroll conditions, tile drawing might fail if asked to draw
;; near 0x?7FF or 0x?FFF addresses (at the end of each one of the 8 pixel lines), as 
;; next screen byte at that locations is -0x7FF and not +1 bytes away.
;;
;; Details:
;;    This function draws a view of a Tilemap on video memory. A Tilemap is a 2D tile-index
;; matrix in which each item (1-byte) represents a tile to be drawn. Each tile is a 32-byte
;; array containing a 2D sprite of 8x8 pixels in video screen format. Tiles must be 
;; codified as zig-zagged rows (left-to-right, then right-to-left) and with scanlines 
;; in Gray-code order 0,1,3,2,6,7,5,4. This format is known as 'zgtiles' format and can 
;; be obtained as output from IMG2SP macro, configuring SET_FORMAT as 'zgtiles'. 
;; A window of size (*width* x *height*) inside the Tilemap is named as a 'view'. Next 
;; figure sums up all involved concepts,
;; (start code)
;; ************************* FIGURE 1 *******************************
;;
;; |---------------------------------|----------|---------|---------|
;; |            MEMORY               | COMPLETE | TILEMAP | TILEMAP |
;; | address&contents in hexadecimal | TILEMAP  | VIEW 1  | VIEW 2  |
;; |---------------------------------|  [4000]  | (4012)  | <4019>  |
;; |ADDRESS|       CONTENTS          |  8 x 8   |  6 x 5  |  5 x 4  |
;; |-------|-------------------------|----------|---------|---------|         
;; |  4000 |[01]01 01 01 01 01 01 01 | ######## | |  **#  | /// 8   |
;; |  4008 | 01 00 00 00 00 09 09 01 | #    **# | // 8 #  | ===88   |
;; |  4010 | 01 00(06)00 00 09 09 01 | # |  **# | ==888#  | =o= |   |
;; |  4018 | 01<05>05 05 00 08 00 01 | #/// 8 # | o= | #  | =^=_|   |
;; |  4020 | 01 02 02 02 08 08 08 01 | #===888# | ^=_|_#  |         |
;; |  4028 | 01 02 03 02 00 06 00 01 | #=o= | # |         |         |
;; |  4030 | 01 02 04 02 07 06 07 01 | #=^=_|_# |         |         | 
;; |  4038 | 01 01 01 01 01 01 01 01 | ######## |         |         |
;; |-------|-------------------------|----------|---------|---------|
;;           ^
;;           \-- 64-bytes that define an 8x8 tilemap starting at [4000]. 
;;               Each byte represents a tile-index. A tileset defining each one of
;;               the tiles as a 32-bytes (4x8-byte) sprite is required for drawing the tiles.
;; (end code)
;;
;;    This function can draw any desired view, as shown in figure 1. The whole *tilemap*
;; takes 64 bytes from 0x4000 to 0x403F in memory, which is 8x8 bytes (width x height). 
;; So, the complete *tilemap* can be drawn as follows,
;; (start code)
;;    // Definition of tilemap and tileset (This is normally generated by tools.
;;    // You might create your tiles with Gimp or Aseprite, and your tilemap with tiled,
;;    // then use automatic conversion to generate these arrays)
;;    u8 tilemap[8*8] = { 1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,6 /*.....*/ ,1 };
;;    u8 tileset[8*8*4*8] = { /*....*/ }
;;
;;    //....
;;    
;;    // At some point in your code, you set up the tilemap drawing function.
;;    // This set up is for drawing the full tilemap (8x8).
;;    cpct_etm_setDrawTilemap4x8_ag (8, 8, 8, tileset);   // 8x8 view, fullwidth of 8, using tileset set of tiles
;;    
;;    //....
;;
;;    // Then, when you wanted to draw the full tilemap, you only have to call this function.
;;    // This will draw the tilemap at the start of video memory
;;    cpct_etm_drawTilemap4x8_ag (CPCT_VMEM_START, tilemap);
;; (end code)
;;
;;    This code configures the function with <cpct_etm_setDrawTilemap4x8_ag>
;; for drawing a view of 8x8 tiles (the full tilemap). You also specify that the tilemap
;; is 8-tiles wide (independent of the view), and that you want to use the TileSet
;; at the address of the tileset array. That one contains the definitions of the tiles
;; to be drawn, 4x8-bytes each.
;;
;;    After configuration, to draw the view a single call to <cpct_etm_drawTilemap4x8_ag>
;; will do the job. This only requires the video memory location where to draw the
;; Tilemap (upper-left corner) and a pointer to the first tile to be drawn which, in 
;; this case is also the first tile of the tilemap, and hence its address is *tilemap*
;; (the address where the array starts).
;; 
;;    *Important!*: Video memory address *must be* 4-byte aligned and at a character 
;; pixel line 0. In standard video modes, character pixel line 0 goes from 0xC000 
;; (CPCT_VMEM_START) to 0xC7FF (0x8000-to-0x8FFF, 0x4000-0x4FFF. 0x0000-0x0FFF for 
;; hardware backbuffers). Inside this range, address used *must also be divisible*
;; *by 4* to be 4-bytes aligned (0xC000, 0xC004, 0xC008...). Be always sure that
;; you meet these requirements when drawing.
;;
;;    Following Figure 1, the Tilemap View 1 of (6x5) can be drawn as follows,
;; (start code)
;;    // We assume tileset and tilemap are defined as in previous example
;; 
;;    // Set up drawTilemap for drawing a view of 6x5 tiles. Full tilemap
;;    // width is always 8, and we will be using same tileset as before
;;    cpct_etm_setDrawTilemap4x8_ag (6, 5, 8, tileset);
;;    
;;    //....
;;
;;    // We draw the view of the tilemap at the start of video memory. As the first
;;    // tile we want to draw is 0x12 (18) bytes beyond the start of the tilemap
;;    // array, we only need to add this offset to the address where tilemap starts
;;    // to pass the address of that tile to the function, as the first tile that will be drawn
;;    cpct_etm_drawTilemap4x8_ag (CPCT_VMEM_START, tilemap + 0x12);
;; (end code)
;;    
;;    This second example sets up a view window of 6x5 tiles from the 8-tiles-wide *tilemap*. 
;; using the same *tileset* as on previous example. Then, starting tile address is changed 
;; on the call to <cpct_etm_drawTilemap4x8_ag>. That makes the function draw a window of 
;; 6x5 tiles that effectively starts at the tile *tilemap + 0x12* which is marked in Figure 1
;; with parentheses *()*. Using tilemap coordinates, that tile is (2,2) (2 rows = 8*2 = 16 (0x10)
;; plus 2 tiles, 0x10 + 2 = 0x12). Therefore, drawing starts at tile (2,2), and spans a view
;; window of 6x5 tiles, which expands view up to tile (8,7). Next figure clarifies this window,
;; (start code)
;; *************** FIGURE 2 ********************
;;
;; |------------------------------------------|---------|
;; |               MEMORY                     | TILEMAP |
;; |    addresses & contents in hexadecimal   | VIEW 1  |
;; |------------------------------------------| (4012)  |
;; |ADDRESS|          CONTENTS                |  6 x 5  |
;; |-------|----------------------------------|---------|         
;; |  4000 |[01] 01   01  01  01  01  01  01  |         |
;; |  4008 | 01  00   00  00  00  09  09  01  |         |
;; |       |        /-----------------------\ |         | << Width = 6 \
;; |  4010 | 01  00 |(06) 00  00  09  09  01| | |  **#  | ^            | View
;; |  4018 | 01  05 | 05  05  00  08  00  01| | // 8 #  | |            | Window
;; |  4020 | 01  02 | 02  02  08  08  08  01| | ==888#  | | Height = 5 |
;; |  4028 | 01  02 | 03  02  00  06  00  01| | o= | #  | |            |
;; |  4030 | 01  02 | 04  02  07  06  07  01| | ^=_|_#  | v            |
;; |       |        \-----------------------/ |         |              /
;; |  4038 | 01  01   01  01  01  01  01  01  |         |
;; |-------|----------------------------------|---------|
;;           <------------------------------> Complete tilemapWidth = 8 tiles = 8 bytes
;;           ^        ^
;;           /        \- First tile to draw at location (4012) in memory: tile (2,2).
;;  tilemap at           Offset = 2 rows of 8 bytes plus 2 bytes = 14 bytes = 0x12 bytes
;;  location [4000]
;; (end code)
;;
;;    Figure 2 shows how view windows are drawn. Once *width*, *height* and *tilemapWidth* have
;; been configured using <cpct_etm_setDrawTilemap4x8_ag>, any view inside the tilemap can be
;; drawn just by selecting the first tile to be drawn. In Figure 2, tile (2,2) is selected and
;; that selects the complete 6x5 view window to be drawn. This could easily be used to 
;; perform software scrolling effects by moving this first tile and redrawing the view.
;; The only thing to take into account is that the tile is selected by its memory location. 
;; Therefore, we start from the address of the *tilemap* array (0x4000 in the example) and 
;; add an offset to select the concrete tile we want. Adding/Subtracting 1 byte is equivalent
;; to moving one tile to the right/left, while adding/subtracting *tilemapWidth* bytes is 
;; similar to moving one tile up/down in the *tilemap* space.
;;
;;    Similarly to previous example, the Tilemap View 2 from Figure 1 can be drawn using the
;; following code,
;; (start code)
;;    // We assume tileset and tilemap are defined as in previous examples
;; 
;;    // Set up drawTilemap for drawing a view of 5x4 tiles. Full tilemap
;;    // width is always 8, and we will be using same tileset as before
;;    cpct_etm_setDrawTilemap4x8_ag (5, 4, 8, tileset);
;;    
;;    //....
;;
;;    // We draw the view of the tilemap at the start of video memory. First tile we want
;;    // to draw is (1,3) (3 rows of 8 tiles + 1 tile = 8*3 + 1 = 24 + 1) = 0x19 bytes away
;;    // from tilemap start.
;;    cpct_etm_drawTilemap4x8_ag (CPCT_VMEM_START, tilemap + 0x19);
;; (end code)
;;
;;    Following explanations, previous code will produce a view of 5x4 tiles from the *tilemap*,
;; as detailed in next figure,
;;
;; (start code)
;; *************** FIGURE 3 ********************
;;
;;
;; |----------------------------------------|---------|
;; |               MEMORY                   | TILEMAP |
;; |    addresses & contents in hexadecimal | VIEW 2  |
;; |----------------------------------------| <4019>  |
;; |ADDRESS|          CONTENTS              |  5 x 4  |
;; |-------|--------------------------------|---------|         
;; |  4000 |[01] 01  01  01  01  01  01  01 |         |
;; |  4008 | 01  00  00  00  00  09  09  01 |         | 
;; |  4010 | 01| 00  06  00  00  09  09  01 |         |
;; |       |   /-------------------\        |         | << Width = 5 \
;; |  4018 | 01|<05> 05  05  00  08| 00  01 | /// 8   | ^            | View
;; |  4020 | 01| 02  02  02  08  08| 08  01 | ===88   | | Height = 4 | Window
;; |  4028 | 01| 02  03  02  00  06| 00  01 | =o= |   | |            |
;; |  4030 | 01| 02  04  02  07  06| 07  01 | =^=_|   | v            |
;; |       |   \-------------------/        |         |              /
;; |  4038 | 01  01  01  01  01  01  01  01 |         |
;; |-------|--------------------------------|---------|
;;           <------------------------------> Complete tilemapWidth = 8 tiles = 8 bytes
;;           ^   ^
;;           /   \---- First tile to draw at location (4019) in memory: tile (1,3).
;;  tilemap at         Offset = 3 rows of 8 bytes plus 1 byte = 25 bytes = 0x19 bytes
;;  location [4000]
;; (end code)
;;
;; Destroyed Register values: 
;;      C-bindings - AF, BC, DE, HL
;;    ASM-bindings - AF, BC, DE, HL, IX, IY
;;
;; Required memory:
;;      C-bindings - 165 bytes (+48 bytes from <cpct_etm_setDrawTilemap4x8_ag>-cbindings which is required)
;;    ASM-bindings - 153 bytes (+44 bytes from <cpct_etm_setDrawTilemap4x8_ag>-asmbindings which is required)
;;
;; Time Measures: 
;; (start code)
;;    Case     |  microSecs (us)   |    CPU Cycles      |
;; ------------------------------------------------------
;;    Any      | 19 + (35 + 189W)H | 76 + (140 + 756W)H |
;; ------------------------------------------------------
;;  ASM saving |       -33         |      -132          |
;; ------------------------------------------------------
;;  W=20, H=10 |      38.169       |     152.676        | (1,91 VSync)
;; ------------------------------------------------------
;;  W=16, H=16 |      48.963       |     195.852        | (2,45 VSync)
;; ------------------------------------------------------
;;    ^
;;    \--- 16x16 Screen is used in most games like AMC or Target Renegade, for instance.
;; (end code)
;;    W - View Width in tiles
;;    H - View Height in tiles
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;; LOCAL MACRO: drawSpriteRow
;;    Copies 4 bytes from the Stack to (HL) using pop BC.
;; It can copy the sprite left-to-right or right-to-left. For left-to-right
;; use 'inc' as parameter (MOV=inc), and for right-to-left use 'dec' (MOV=dec).
;; The copy assumes that destination is 4-byte aligned (L + 2 < 0xFF)
;; Parameters:
;;    MOV = ( inc | dec )
;;
.macro drawSpriteRow MOV
   pop   bc             ;; [3] Get next 2 sprite bytes
   ld  (hl), c          ;; [2] Copy byte 1
   MOV    l             ;; [1] HL++ / HL-- (4-byte aligned) -> next video mem location
   ld  (hl), b          ;; [2] Copy byte 2
   MOV    l             ;; [1] HL++ / HL-- (4-byte aligned) -> next video mem location
   pop   bc             ;; [3] Get next 2 sprite bytes
   ld  (hl), c          ;; [2] Copy byte 3
   MOV    l             ;; [1] HL++ / HL-- (4-byte aligned) -> next video mem location
   ld  (hl), b          ;; [2] Copy byte 4
.endm

;; LOCAL MACRO: drawTilemap4x8_ag_gen
;;    All code function is defined as a macro to prevent code duplication on reuse
;; between ASM/C bindings. As setDrawtilemap4x8_ag ASM/C bindings have to couple
;; each one with its correct version, different global labels can be generated
;; from the same source using different lblPrf parameters. 
;;    Therefore, ASM/C bindings include this file and use this macro to generate
;; specific source code that will be compiled, with the appropriate labels.
;;
.macro drawTilemap4x8_ag_gen lblPrf
   ;; Set Height and Width of the View Window of the current 
   ;; tilemap to be drawn (This is set by setDrawTilemap4x8_agf)
widthHeightSet = .+2
   ld iy, #0000         ;; [4] IYL=View Window Width, IYH=View Window Height

nextRow:
   ;; Disable interrupts and save SP before starting
   di                   ;; [1] Disable interrupts before starting (we are using SP to read values)
   ld (restoreSP), sp   ;; [6] Save actual SP to restore it in the end
   ;; Start of the code that draws the next tile of the present row being drawn
   ;;
nexttile:
   ;; Get next tile to be drawn from the tilemap, which is pointed by DE
   ld     a, (de)    ;; [2] A = present tile-ID of the tile to be drawn
   ld     b, a       ;; [1] B = A

   ;; From the tile-ID we hold in B, we need to calculate the Offset of the 
   ;; tile definition (its 32-bytes of screen pixel data). As each tile takes 32-bytes,
   ;; offsets are tile_0: 0-bytes, tile_1: 32-bytes, tile_2: 64-bytes... tile_N: N*32-bytes.
   ;; Therefore, we need to multiply 32*B (32*tile-ID). As this multiplication may result 
   ;; in a 16-bits value, we will perform BC = 32*B. Considering B=[abcdefgh], result 
   ;; has to be BC = [000abcde][fgh00000] (B shifted 5 times left = BC = 32*tile-ID).  
   xor    a       ;; [1] A = [00000000]
   srl    b       ;; [2] B = [0abcdefg] (Right shift, Carry = h)
   rra            ;; [1] A = [h0000000] (Rotate Right + Bit7 = h (Carry insertion))
   srl    b       ;; [2] B = [00abcdef] (Right shift, Carry = g)
   rra            ;; [1] A = [gh000000] (Rotate Right + Bit7 = g (Carry insertion))
   srl    b       ;; [2] B = [000abcde] (Right shift, Carry = f). 
   rra            ;; [1] A = [fgh00000] (Rotate Right + Bit7 = f (Carry insertion)). 
   ld     c, a    ;; [1] C = A. BC = 32*tile-ID complete.

   ;; Make IX point to the 32-byte screen pixel definition of the selected tile.
   ;; For that, we need to add previous calculated tile Offset (BC) and the start location
   ;; of the tileset (IX). So operation is IX = tilesetPtr + Offset = IX + BC. 
tilesetPtr = .+2
   ld    ix, #0000   ;; [4] IX = Pointer to start of tileset (0000 is placeholder set with tileset address)
   add   ix, bc      ;; [4] IX += Tile Offset  
   ld    sp, ix      ;; [3] Make SP Point to the start of the 32-byte screen pixel data
                     ;; ... definition of the current tile (IX is used to save and restore this pointer)
   ;;
   ;; This section of the code draws 1 8x8 pixels (4x8 bytes) tile
   ;; Uses:
   ;;    SP = Pointer to the start of the 32-bytes screen pixel definition of the tile
   ;;    HL = Video Memory Pointer (top-left-corner)
   ;; Modifies BC 
   ;;

   ;; Draw Sprite Lines using Gray-Code order and Zig-Zag movement
   ;; Gray Code scanline order: 0,1,3,2,6,7,5,4
   drawSpriteRow inc ;; [17] Copy tile line Left-to-Right [>>]
   set    3, h       ;; [ 2] --000---=>--001--- (Next sprite line: 1)
   drawSpriteRow dec ;; [17] Copy tile line Right-to-Left [<<]
   set    4, h       ;; [ 2] --001---=>--011--- (Next sprite line: 3)
   drawSpriteRow inc ;; [17] Copy tile line Left-to-Right [>>]
   res    3, h       ;; [ 2] --011---=>--010--- (Next sprite line: 2)
   drawSpriteRow dec ;; [17] Copy tile line Right-to-Left [<<]
   set    5, h       ;; [ 2] --010---=>--110--- (Next sprite line: 6)
   drawSpriteRow inc ;; [17] Copy tile line Left-to-Right [>>]
   set    3, h       ;; [ 2] --110---=>--111--- (Next sprite line: 7)
   drawSpriteRow dec ;; [17] Copy tile line Right-to-Left [<<] 
   res    4, h       ;; [ 2] --111---=>--101--- (Next sprite line: 5)
   drawSpriteRow inc ;; [17] Copy tile line Left-to-Right [>>]
   res    3, h       ;; [ 2] --101---=>--100--- (Next sprite line: 4)
   drawSpriteRow dec ;; [17] Copy tile line Right-to-Left [<<]
   res    5, h       ;; [ 2] --100---=>--000--- (Next sprite line: 0)

   ;; After drawing the tile, HL points to the same place in video memory
   ;; as it started. We need to move it to the right 4 bytes (the width of 1 tile)
   ;; to point to the place in video memory for the next tile. As this function
   ;; requires tilemap to be 4-bytes aligned in video memory, maximum value 
   ;; for L will be L=0xFC, so we can always safely add 3 to L with INC L, without 
   ;; modifying H. Then for safety reasons, last increment will be INC HL to 
   ;; ensure that H gets incremented when L=0xFF. This saves 1 microsecond
   ;; from LD BC, #4: ADD HL, BC.
   inc    l  ;; [1] /
   inc    l  ;; [1] | HL+=3 (Incrementing L only)  
   inc    l  ;; [1] \ 
   inc   hl  ;; [2] HL++ (HL += 4 in total)

   ;; We now test if we have finished drawing present row of tiles. If that is
   ;; the case, the Width counter will be 0 (IYL=0). 
   inc   de           ;; [2] ++DE (Make tilemapPtr point to next tile to be drawn)
   dec__iyl           ;; [2] --IYL (--Width, One less tile to be drawn in this row)
   jp    nz, nexttile ;; [3] if (IYL!=0), then more tiles are left to be drawn in this row,
                      ;; ... so continue with next tile.
rowEnd:
   ;; We have finished drawing present row of tiles. We restore SP original value
   ;; and previous interrupt status. This will enable interrupts to occur in a
   ;; safe way, permitting the use of this function along with split rasters
   ;; and/or music played on interrupts
restoreSP = .+1
   ld    sp, #0000      ;; [3] Restore SP (#0000 is a placeholder)
restoreI = .
   ei                   ;; [1] Restore previous interrupt status (Enabled or disabled)
                        ;; ... EI gets modified by setDrawTilemap_agf and could by DI instead

   ;; Decrement the Height counter (IYH) as we have finished a complete row.
   ;; If the counter is 0, then we have finished drawing the whole tilemap.
   dec__iyh             ;; [3]   --IYH (--Height)
   jr     z, return     ;; [2/3] if (Height==0) then return

   ;;
   ;; As Height counter is not 0 (IYH > 0), there are more rows to draw.
   ;; Set up pointers before drawing next tile row.
   ;;

   ;; Video Memory Pointer (Currently HL) has to point to next row in the screen.
   ;; As each row takes 0x50 bytes (in standard modes) we need to add to HL
   ;; the difference between the bytes drawn in this row and 0x50 to ensure that
   ;; each loop makes HL increment exactly 0x50 bytes, so that it points to next line.
   ;; Also, as width is measured in tiles, and each tile is 4 bytes-wide, the 
   ;; final calculation will be HL += screenWidth - drawnWidth = 0x50 - 4*width
incrementHL = .+1
   ld    bc, #0000      ;; [3] BC = (0x50 - 4*width) (#0000 is a placeholder that gets the value)
   add   hl, bc         ;; [3] HL += 0x50 - 4*width

   ;; As IYL has been used as width counter, it has been decremented to 0.
   ;; Restore it to the width value before using it again.
restoreWidth = .+2
   ld__iyl  #00         ;; [2] IYL = Width (#00 is a placeholder)

   ;; Tilemap pointer (Currently at DE) has to point to the start of the next row 
   ;; of the tilemap to be drawn. Similarly to the Video Memory Pointer, if our tilemap
   ;; is wider than our view width, we need to increment the pointer with the 
   ;; difference between tilemapWidth and our view width to ensure that the pointer
   ;; gets incremented by exactly tilemapWidth at each loop (ensuring that we always
   ;; end up pointing to the first tile of the next row). As we have incremented
   ;; the tilemap pointer by (width), the operation will be 
   ;; TilemapPtr += tilemapWidth - width
   ex    de, hl         ;; [1] Temporarily exchange HL<=>DE to do 16-bit maths for updating DE
updateWidth = .+1
   ld    bc, #0000      ;; [3] BC = tilemapWidth - width
   add   hl, bc         ;; [3] HL += tilemapWidth - width (TilemapPtr points to first tile of next row)
   ex    de, hl         ;; [1] Restore DE,HL into its proper registers, now with DE incremented

   jp    nextRow        ;; [3] Next Row

   ;; When everything is finished, we safely return
   ;; 
return:

;; Set up association of global symbols with locals
lblPrf'tilesetPtr     == tilesetPtr
lblPrf'widthHeightSet == widthHeightSet
lblPrf'restoreWidth   == restoreWidth
lblPrf'updateWidth    == updateWidth
lblPrf'incrementHL    == incrementHL
lblPrf'restoreI       == restoreI

.endm
