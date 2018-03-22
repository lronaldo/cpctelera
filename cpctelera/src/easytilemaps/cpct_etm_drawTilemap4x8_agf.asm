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
;; Function: cpct_etm_drawTilemap4x8_agf
;;
;;    Draws an aligned view of a tilemap made of 4x8-bytes tiles. Tiles must be
;; Gray-coded, with scanline order 0,1,3,2,6,7,5,4. It works identical to 
;; <cpct_etm_drawTilemap4x8_ag> but faster depending on horizontal repetitions of tiles.
;;
;; C Definition:
;;    void <cpct_etm_drawTilemap4x8_agf> (void* *memory*, const void* *tilemap*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;    (2B HL) memory  - Video memory location where to draw the tilemap (character & 4-byte aligned)
;;    (2B DE) tilemap - Pointer to the upper-left tile of the view to be drawn of the tilemap
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawTilemap4x8_agf_asm
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
;; <cpct_etm_setDrawTilemap4x8_agf>. This call is required to configure view 
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
;;    Please, refer to documentation about <cpct_etm_drawTilemap4x8_ag> for usage details. 
;; Both this function and <cpct_etm_drawTilemap4x8_ag> work identical. Just remember to 
;; use <cpct_etm_setDrawTilemap4x8_agf> to configure *size* values and *tileset* for this 
;; f-fast version of the function.
;;
;;    This fast version takes into account horizontal repetitions of tiles in the same
;; row of the *tilemap* being drawn. Instead of drawing tiles one by one, it draws 
;; each tile as many times as consecutive repetitions are found. Let us analyze figure 1
;; for illustration,
;;
;; (start code)
;; ***************** FIGURE 1 *******************
;;
;; |---------------------------------|----------|
;; |            MEMORY               | COMPLETE |
;; | address&contents in hexadecimal | TILEMAP  |
;; |---------------------------------|  [4000]  |
;; |ADDRESS|       CONTENTS          |  8 x 8   |
;; |-------|-------------------------|----------|         
;; |  4000 |[01]01 01 01 01 01 01 01 | ######## |
;; |  4008 | 01 00 00 00 00 09 09 01 | #    **# |
;; |  4010 | 01 00 06 00 00 09 09 01 | # |  **# |
;; |  4018 | 01 05 05 05 00 08 00 01 | #/// 8 # |
;; |  4020 | 01 02 02 02 08 08 08 01 | #===888# |
;; |  4028 | 01 02 03 02 00 06 00 01 | #=o= | # |
;; |  4030 | 01 02 04 02 07 06 07 01 | #=^=_|_# | 
;; |  4038 | 01 01 01 01 01 01 01 01 | ######## |
;; |-------|-------------------------|----------|
;; (end code)
;;
;;    Figure 1 contains an 8x8-bytes *tilemap* starting at 0x4000. Calling 
;; <cpct_etm_drawTilemap4x8_agf> to draw this complete *tilemap* will produce next
;; logical sequence,
;;
;;    1 - First tile to draw is 01 (at address 0x4000)
;;    2 - After counting, 8 consecutive 01 tiles are found on first row.
;;    3 - 8 consecutive 01 tiles are drawn.
;;    4 - Next tile is 01 (first from second row)
;;    5 - After counting, only 1 consecutive 01 tile is found.
;;    6 - A 01 tile is drawn.
;;    7 - Next tile is 00 (second tile from second row)
;;    8 - After counting, 4 consecutive 00 tiles are found.
;;    9 - 4 consecutive 00 tiles are drawn.
;;   10 - Next tile is 09 (sixth from second row)
;;   11 - ......
;;
;;    This sequence reduce calculations for repeated tiles, and draws them consecutively. 
;; When applied to tilemaps with a great number of horizontal repetitions, it results on
;; faster drawing. As this is quite common, you may usually prefer this function for 
;; *complete tilemap* drawing instead of <cpct_etm_drawTilemap4x8_ag>. However, it is advisable
;; to test them first and measure gains, specially if you are unsure of the frequency of
;; horizontal regularities in your tilemaps. If your tilemaps have no tile regularities in
;; their horizontal rows, using this function may even be slower (Please, check time measures
;; section for reference).
;;
;;    Last words of advice: it is recommended not to use both versions <cpct_etm_drawTilemap4x8_ag>
;; and <cpct_etm_drawTilemap4x8_agf> simultaneously. They will take a lot of space and potential
;; performance gains will usually be not worth it. Also, using <cpct_etm_drawTilemap4x8_agf> for
;; small tilemap windows or to erase sprites by redrawing tiles over them is not advisable.
;; For such small windows it will probably be much slower than <cpct_etm_drawTilemap4x8_ag>.
;;
;; Destroyed Register values: 
;;      C-bindings - AF, BC, DE, HL
;;    ASM-bindings - AF, BC, DE, HL, IX, IY
;;
;; Required memory:
;;      C-bindings - 187 bytes (+49 bytes from <cpct_etm_setDrawTilemap4x8_agf>-cbindings which is required)
;;    ASM-bindings - 176 bytes (+45 bytes from <cpct_etm_setDrawTilemap4x8_agf>-asmbindings which is required)
;;
;; Time Measures: 
;;    Performance depends on frequency of repetitions of tiles inside tilemap rows. Because 
;; of this, no useful formula for calculating performance can be given. Instead, some measures
;; for general understanding are given for drawing complete tilemaps of 20x10 and 16x16 sizes.
;; For each tilemap, Best Case means all tiles are repeated, Worst Case means no single tile
;; is repeated, and Non-fast is the performance for <cpct_etm_drawTilemap4x8_ag> (Non-fast version)
;;
;; (start code)
;;    Case     |  microSecs (us)   |    CPU Cycles      |
;; ------------------------------------------------------
;; ////    Measures for Tilemap of size (20 x 10)    ////
;; ------------------------------------------------------
;;  Best Case  |   35.475 ( -7,1%) |     141.900        | (1,78 VSync)   
;;             ------------------------------------------
;;  Non-fast   |   38.169          |     152.676        | (1,91 VSync) 
;;             ------------------------------------------           
;;  Worst Case |   42.505 (+11,3%) |     170.020        | (2,19 VSync)      
;; ------------------------------------------------------
;; ////    Measures for Tilemap of size (16 x 16)    ////
;; ------------------------------------------------------
;;  Best Case  |   45.609 ( -7,0%) |     182.436        | (2,28 VSync)   
;;             ------------------------------------------
;;  Non-fast   |   48.963          |     195.852        | (2,45 VSync) 
;;             ------------------------------------------           
;;  Worst Case |   54.489 (+11,3%) |     217.956        | (2,73 VSync)      
;; ------------------------------------------------------  
;;  ASM saving |       -33         |      -132          | 
;; ------------------------------------------------------
;; (end code)
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

;; LOCAL MACRO: drawTilemap4x8_agf_gen
;;    All code function is defined as a macro to prevent code duplication on reuse
;; between ASM/C bindings. As setDrawtilemap4x8_agf ASM/C bindings have to couple
;; each one with its correct version, different global labels can be generated
;; from the same source using different lblPrf parameters. 
;;    Therefore, ASM/C bindings include this file and use this macro to generate
;; specific source code that will be compiled, with the appropriate labels.
;;
.macro drawTilemap4x8_agf_gen lblPrf
   ;; Set Height and Width of the View Window of the current 
   ;; tilemap to be drawn (This is set by setDrawTilemap4x8_agf)
widthHeightSet = .+2
   ld iy, #0000      ;; [4] IYL=View Window Width, IYH=View Window Height
   jp saveSPfirst    ;; [3] For first setup, we need to save SP and start at nextRow point

   ;; Start of the code that draws the next tile of the present row being drawn
   ;;
nexttile:
   ex    de, hl      ;; [1] Make HL Point to Tilemap (use DE to save current Video Memory Pointer)
nextRow:
   ;; Get next tile to be drawn from the tilemap, which is pointed by HL
   ld     a, (hl)    ;; [2] A = present tile-ID of the tile to be drawn
   ld     b, a       ;; [1] B = A (copy of present tile-ID to draw)

   ;; Optimization: Count how many consecutive tiles have same ID. Knowing this
   ;; we can repeat the drawing part many times for same tile, without recalculating
   ;; At the end of this loop, A=Number of times to repeat drawing of present tile-ID
   ld__c_iyl            ;; [2] C = remaining tiles to be drawn in this row (width - drawn tiles)
rep:
   dec    c             ;; [1]   --Width (1 less tile to be drawn)
   jr     z, endwidth   ;; [2/3] if (Width==0) jump to endwidth (as no more tiles to be drawn in this row)
   inc   hl             ;; [2]   ++HL (Point to next tile in this tilemap)
   cp   (hl)            ;; [2]   Is A==(HL)? (A:present tile-ID, (HL): next tile-ID in the tilemap)
   jr     z, rep        ;; [2/3] if (A==(HL)), next tile is equal to present to be drawn, so
                        ;; ..... add 1 to the counter of times to draw this tile and check next one
endwidth:
   ;; Update the Width counter at IYL with its new value (held at C). We need to know
   ;; how many times we have decremented this counter (with DEC C) as this is then number of
   ;; copies of the present tile that we have to draw now. This can be calculated as IYL - C
   ;; (IYL:previous value of Width counter, C: Updated value of Width counter, after decrementing)
   ld__a_iyl      ;; [2]   A = total tiles pending to be drawn in this row
   ld__iyl_c      ;; [2] IYL = tiles that will be pending to be drawn in this row after drawing present tile
   sub    c       ;; [1] A = IYL - C (Number of copies of present tile to be drawn now)

   ;; From the tile-ID we hold in B, we need to calculate the Offset of the 
   ;; tile definition (its 32-bytes of screen pixel data). As each tile takes 32-bytes,
   ;; offsets are tile_0: 0-bytes, tile_1: 32-bytes, tile_2: 64-bytes... tile_N: N*32-bytes.
   ;; Therefore, we need to multiply 32*B (32*tile-ID). As this multiplication may result 
   ;; in a 16-bits value, we will perform BC = 32*B. Considering B=[abcdefgh], result 
   ;; has to be BC = [000abcde][fgh00000] (B shifted 5 times left = BC = 32*tile-ID).
   ld     c, #0   ;; [2] C = [00000000]
   srl    b       ;; [2] B = [0abcdefg] (Right shift, Carry = h)
   rr     c       ;; [2] C = [h0000000] (Rotate Right + Bit7 = h (Carry insertion))
   srl    b       ;; [2] B = [00abcdef] (Right shift, Carry = g)
   rr     c       ;; [2] C = [gh000000] (Rotate Right + Bit7 = g (Carry insertion))
   srl    b       ;; [2] B = [000abcde] (Right shift, Carry = f). 
   rr     c       ;; [2] C = [fgh00000] (Rotate Right + Bit7 = f (Carry insertion)). BC = 32*tile-ID complete.

   ;; Make IX point to the 32-byte screen pixel definition of the selected tile.
   ;; For that, we need to add previous calculated tile Offset (BC) and the start location
   ;; of the tileset (IX). So operation is IX = tilesetPtr + Offset = IX + BC. 
tilesetPtr = .+2
   ld    ix, #0000   ;; [4] IX = Pointer to start of tileset (0000 is placeholder set with tileset address)
   add   ix, bc      ;; [4] IX += Tile Offset
   
   ;; After starting to draw, we need HL pointing to video memory. As that pointer
   ;; is saved in DE, we only need to switch DE and HL.
   ex    de, hl      ;; [1] Make HL= Video memory Pointer and 
                     ;; ... DE= Pointer to next tile in the tilemap (Saved for later use)
   ;;
   ;; This section of the code draws 1 8x8 pixels (4x8 bytes) tile
   ;; Uses:
   ;;    SP = Pointer to the start of the 32-bytes screen pixel definition of the tile
   ;;    HL = Video Memory Pointer (top-left-corner)
   ;; Modifies BC 
   ;;
drawTile:
   ld    sp, ix      ;; [3] Make SP Point to the start of the 32-byte screen pixel data
                     ;; ... definition of the current tile (IX is used to save and restore this pointer)

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

   ;; A holds the number of times the present tile has to be drawn. We have drawn 
   ;; the tile once, so we decrement A and, if it is not 0, we draw the tile again.
   dec    a             ;; [1]   A-- (1 less time we have to draw present tile)
   jr    nz, drawTile   ;; [2/3] if (A!=0) we have to draw the tile more times

   ;; At this point, A=0 as no more tiles have to be drawn.
   ;; We now test if we have finished drawing present row of tiles. If that is
   ;; the case, the Width counter will be 0 (IYL=0). 
   cp__iyl              ;; [2] is IYL==A? (IYL==0? as A is 0)
   jp    nz, nexttile   ;; [3] if (IYL!=0) there are still tiles to draw in this row, 
                        ;; ... so continue with next tile

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
   ;; As Height counter is not 0 (IYH > 0), set up pointers for drawing next row
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
   ;; the tilemap pointer by (width + 1), the operation will be 
   ;; TilemapPtr += tilemapWidth - (width + 1) = tilemapWidth - width - 1.
   ex    de, hl         ;; [1] HL Points to next tile in the tilemap, DE now saves Video Memory Pointer
updateWidth = .+1
   ld    bc, #0000      ;; [3] BC = tilemapWidth - width - 1
   add   hl, bc         ;; [3] HL += tilemapWidth - width - 1 (TilemapPtr points to first tile of next row)

saveSPfirst:
   ;; Disable interrupts and save SP before starting
   di                   ;; [1] Disable interrupts before starting (we are using SP to read values)
   ld (restoreSP), sp   ;; [6] Save actual SP to restore it in the end

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
