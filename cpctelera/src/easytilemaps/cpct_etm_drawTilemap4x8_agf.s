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
.include "macros/cpct_undocumentedOpcodes.h.s"
.include "macros/cpct_opcodeConstants.h.s"
.include "macros/cpct_maths.h.s"

;; LOCAL MACRO: drawSpriteRow
;;    Copies 4 bytes from the Stack to (HL) using pop DE.
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

;;
;; C bindings for <cpct_etm_setDrawTileMap4x8_agf>
;;
;; cpct_etm_setDrawTileMap4x8_agf(u8 width, u8 height, u16 tilemapWidth, void* tileset)
;; BC  = B:Height, C:Width
;; DE  = TileMapWidth
;; HL  = ptileset
;;
;; Destroyed Register values: 
;;      AF, DE
;;
;; Required memory:
;;      xx bytes (+ xx bytes from <cpct_etm_drawTileMap4x8_agf> which is included)
;;
;; Time Measures:
;; (start code)
;;    Case     | microSecs (us) | CPU Cycles  
;; ------------------------------------------
;;    Any      |      72        |    288
;; ------------------------------------------
;; ASM Saving  |     -15        |    -60
;; ------------------------------------------
;; (end code)
;;    W - Map width (number of horizontal tiles)
;; 
_cpct_etm_setDrawTileMap4x8_agf::
   pop   hl                ;; [3] HL = Return Address
   pop   bc                ;; [3] BC = B:Height, C:Width
   pop   de                ;; [3] DE = Tileset Pointer
   ex  (sp), hl            ;; [6] HL = TilemapWidth, leaving previous HL value (return address)
                           ;; ... at the top of the stack (following __z88dk_callee convention)

   ;; Set (tilesetPtr) placeholder
   ld (tilesetPtr), hl     ;; [5] Save HL into tilesetPtr placeholder

   ;; Set all Width values required by drawTileMap4x8_agf. First two values
   ;; (heightSet, widthSet) are values used at the start of the function for
   ;; initialization. The other one (restoreWidth) restores the value of the
   ;; width after each loop, as it is used as counter and decremented to 0.
   ld (widthSet), bc       ;; [6]
   ld     a, c             ;; [1]
   ld (restoreWidth), a    ;; [4] Set restore width after each loop placeholder
   
   ;; In order to properly show a view of (Width x Height) tiles from within the
   ;; tilemap, every time a row has been drawn, we need to move tilemap pointer
   ;; to the start of the next row. As the complete tilemap is (tilemapWidth) bytes
   ;; wide and we are showing a view only (Width) tiles wide, to complete (tilemapWidth)
   ;; bytes at each loop, we need to add (tilemapWidth - Width + 1) bytes.
   dec    a                ;; [1] A = Width - 1
   sub_de_a                ;; [7] tilemapWidth - (Width - 1)
   ld (updateWidth), de    ;; [6] set the difference in updateWidth placeholder

   ;; Calculate HL update that has to be performed for each new row loop.
   ;; HL advances through video memory as tiles are being drawn. When a row
   ;; is completely drawn, HL is at the right-most place of the screen.
   ;; As each screen row has a width of 0x50 bytes (in standard modes), 
   ;; if the Row that has been drawn has less than 0x50 bytes, this difference
   ;; has to be added to HL to make it point to the start of next screen row.
   ;; As each tile is 4-bytes wide, this amount is (0x50 - 4*Width). Also,
   ;; taking into account that 4*Width cannot exceed 255 (1-byte), a maximum
   ;; of 63 tiles can be considered as Width.
   ld     a, c             ;; [1] A = Width
   add    a                ;; [1] A = 2*Width
   add    a                ;; [1] A = 4*Width
   cpl                     ;; [1] A = -4*Width - 1
   add #0x50 + 1           ;; [2] A = -4*Width-1 + 0x50+1 = 0x50 - 4*Width
   ld (incrementHL), a     ;; [4] Set HL increment in its placeholder

   ;; Set the restoring of Interrupt Status. drawTileMap4x8_agf disables interrupts before
   ;; drawing each tile row, and then it restores previous interrupt status after the row
   ;; has been drawn. To do this, present interrupt status is considered. This code detects
   ;; present interrupt status and sets a EI/DI instruction at the end of tile row drawing
   ;; to either reactivate interrupts or preserve interrupts disabled.
   ld     a, i             ;; [3] P/V flag set to current interrupt status (IFF2 flip-flop)
   ld     a, #opc_EI       ;; [2] A = Opcode for Enable Interrupts instruction (EI = 0xFB)
   jp    pe, int_enabled   ;; [3] If interrupts are enabled, EI is the appropriate instruction
     ld   a, #opc_DI       ;; [2] Otherwise, it is DI, so A = Opcode for Disable Interrupts instruction (DI = 0xF3)
int_enabled:
   ld (restoreI), a        ;; [4] Set the Restore Interrupt status at the end with corresponding DI or EI

   ret                     ;; [3] Return to caller

;;
;; C bindings for <cpct_etm_drawTileMap4x8_agf>
;;
;;  xx microSecs, xx bytes
;; cpct_etm_drawTileMap4x8_agf(void* memory, void* tilemap)
;; DE  = VideoMem
;; HL  = Tilemap
;; 
_cpct_etm_drawTileMap4x8_agf::
   ;; Parameters
   pop   hl          ;; [3] HL = Return address
   pop   de          ;; [3] DE = Tilemap Pointer
   ex  (sp), hl      ;; [6] HL = Video Memory Pointer, leaving previous HL value (return address)
                     ;; ... at the top of the stack (following __z88dk_callee convention)
   push  ix          ;; [5] Save IX and IY to let this function...
   push  iy          ;; [5] ...use and restore them before returning

   ;; Set Height and Width of the View Window of 
   ;; the current tilemap to be drawn (This is set by setDrawTilemap4x8_agf)
widthSet  = .+2
heightSet = .+3
   ld iy, #0000      ;; [4] IYL=View Window Width, IYH=View Window Height
   jp saveSPfirst    ;; [3]

   ;; Start of the code that draws the next tile of the present row being drawn
   ;;
nexttile:
   ex    de, hl      ;; [1]
nextRow:
   ;; Get next tile to be drawn from the tilemap, which is pointed by BC
   ld     a, (hl)    ;; [2] A = present tile-ID of the tile to be drawn
   ld     b, a       ;; [1] B = A (copy of present tile-ID to draw)

   ;; Optimization: Count how many consecutive tiles have same ID. Knowing this
   ;; we can repeat the drawing part many times for same tile, without recalculating
   ;; At the end of this loop, A=Number of times to repeat drawing of present tile-ID
;;;;; -1*rep
   ld__c_iyl            ;; [2] E = remaining tiles to be drawn in this row (width - drawn tiles)
rep:
   dec    c             ;; [1]   --Width (1 less tile to be drawn)
   jr     z, endwidth   ;; [2/3] If (Width==0) jump to endwidth (as no more tiles to be drawn in this row)
   inc   hl             ;; [2]   ++HL (Point to next tile in this tilemap)
   cp   (hl)            ;; [2]   Check A==(HL) A:present tile-ID, (HL): next tile-ID in the tilemap
   jr     z, rep        ;; [2/3] if (A==(HL)), next tile is equal to present to be drawn, so
                        ;; ..... add 1 to the counter of times to draw this tile and check next one
endwidth:
   ;; Update the Width counter at IYL with its new value (hold at E). We need to know
   ;; how many times we have decremented this counter (with DEC E) as this is then number of
   ;; copies of the present tile that we have to draw now. This can be calculated as IYL - E
   ;; (IYL:previous value of Width counter, E: Updated value of Width counter, after decrementing)
   ld__a_iyl      ;; [2]   A = total tiles pending to be drawn in this row
   ld__iyl_c      ;; [2] IYL = tiles that will be pending to be drawn in this row after drawing present tile
   sub    c       ;; [1] A = IYL - E (Number of copies of present tile to be drawn now)

   ;; From the tile-ID we hold in A, we need to calculate the Offset of the 
   ;; tile definition (its 32-bytes of screen pixel data). As each tile takes 32-bytes,
   ;; offsets are tile_0: 0-bytes, tile_1: 32-bytes, tile_2: 64-bytes... tile_N: N*32-bytes.
   ;; Therefore, to calculate offset of tile-ID in A, we need to multiply 32*A. As this
   ;; multiplication may result in a 16-bits value, we will perform DE = 32*A. 
   ;; If A=[abcdefgh], result has to be DE = [000abcde][fgh00000] (5 shifts left = 32*tile-ID).
   ;; This multiplication will be performed in 2 parts for performance reasons. Next
   ;; code performs the computation for the final value of E=[fgh00000], and leaves D=[abcdefgh] 
   ;; to be calculated afterwards with 3 shifts right.
   ;; Calculate second part of the Offset for present tile-ID (HL = 32*tile-ID).
   ;; Starting from tile-ID=[abcdefgh], previous code left calculus as 
   ;; HL = [abcdefgh][fgh00000]. To achieve final value of 32*tile-ID=[000abcde][fgh00000]
   ;; we need to shift H right 3 times
   ld     c, #0   ;; [2] C = [00000000]
   srl    b       ;; [2] B = [0abcdefg] (Right shift)
   rr     c       ;; [2] C = [h0000000] (Rotate Right + Carry Insertion in bit 7)
   srl    b       ;; [2] B = [00abcdef] (Right shift)
   rr     c       ;; [2] C = [gh000000] (Rotate Right + Carry Insertion in bit 7)
   srl    b       ;; [2] B = [000abcde] (Right shift). 
   rr     c       ;; [2] C = [fgh00000] (Rotate Right + Carry Insertion in bit 7). BC = 32*tile-ID complete.

   ;; Make IX point to the 32-byte screen pixel definition of the selecte tile.
   ;; For that, we need to add previous calculated tile Offset and the start location
   ;; of the tileset. So operation is IX = tilesetPtr + Offset. 
tilesetPtr = .+2
   ld    ix, #0000   ;; [4] < tileset
   add   ix, bc      ;; [4] < IX = tileset + 32*a
   ex    de, hl      ;; [1] HL < video mem, DE < tilemap

   ;;
   ;; Draw Tile
   ;; SP = End of the tile - 1
   ;; HL = Video Memory (top-left-corner)
   ;; Uses DE
   ;;
drawTile:
   ld    sp, ix          ;; [3]

   ;; Sprite Line 0. Drawing order [>>]
   drawSpriteRow inc    ;; [17] Copy tile line Left-to-Right
   set    3, h          ;; [ 2] --000---=>--001--- (Next sprite line: 1)

   ;; Sprite Line 1. Drawing order [<<]
   drawSpriteRow dec    ;; [17] Copy tile line Right-to-Left 
   set    4, h          ;; [ 2] --001---=>--011--- (Next sprite line: 3)

   ;; Sprite Line 3. Drawing order [>>]
   drawSpriteRow inc    ;; [17] Copy tile line Left-to-Right
   res    3, h          ;; [ 2] --011---=>--010--- (Next sprite line: 2)

   ;; Sprite Line 2. Drawing order [<<]
   drawSpriteRow dec     ;; [17] Copy tile line Right-to-Left
   set    5, h          ;; [ 2] --010---=>--110--- (Next sprite line: 6)

   ;; Sprite Line 6. Drawing order [>>]
   drawSpriteRow inc    ;; [17] Copy tile line Left-to-Right
   set    3, h          ;; [ 2] --110---=>--111--- (Next sprite line: 7)

   ;; Sprite Line 7. Drawing order [<<]
   drawSpriteRow dec    ;; [17] Copy tile line Right-to-Left 
   res    4, h          ;; [ 2] --111---=>--101--- (Next sprite line: 5)

   ;; Sprite Line 5. Drawing order [>>]
   drawSpriteRow inc    ;; [17] Copy tile line Left-to-Right
   res    3, h          ;; [ 2] --101---=>--100--- (Next sprite line: 4)

   ;; Sprite Line 4. Drawing order [<<]
   drawSpriteRow dec    ;; [17] Copy tile line Right-to-Left
   res    5, h          ;; [ 2] --100---=>--000--- (Next sprite line: 0)

;;;   dec__iyl             ;; [2]
;;;   jr     z, rowEnd     ;; [2/3]

   ;; Advance HL to next tile location in video memory
   inc    l  ;; [1]
   inc    l  ;; [1]
   inc    l  ;; [1]
   inc   hl  ;; [2] HL += 4

   ;; Repeat tile drawing if there are more identical
   dec    a             ;; [1]
   jr    nz, drawTile   ;; [2/3]

;;;  inc   bc           ;; [2]
   cp__iyl              ;; [2] IYL == 0? (A is 0)
   jp    nz, nexttile   ;; [3]

;;;  jp    nexttile       ;; [3]

rowEnd:
restoreSP = .+1
   ld    sp, #0000      ;; [3] Restore SP (#0000 is a placeholder)
restoreI = .
   ei                   ;; [1] Reenable interrupts before returning

   dec__iyh             ;; [3] --IYH (--Height)
   jr     z, return     ;; [2/3] Height==0? Then return

incrementHL = .+1
   ld    bc, #0000      ;; [3] 0x50 - 4*width (#0000 placeholder)
   add   hl, bc         ;; [3] HL + 0x50 - 4*width

restoreWidth = .+2
   ld__iyl  #00         ;; [2] IYL = Width (#00 placeholder)

;; Update Width adding difference FullWidth - DrawnWidth
   ex    de, hl         ;; [1]
updateWidth = .+1
   ld    bc, #0000      ;; [3] 0x50 - 4*width (#0000 placeholder)
   add   hl, bc         ;; [3] HL + 0x50 - 4*width

saveSPfirst:
   ;; Disable interrupts and save SP before starting
   di                   ;; [1] Disable interrupts before starting (we are using SP to read values)
   ld (restoreSP), sp   ;; [6] Save actual SP to restore it in the end

   jp    nextRow        ;; [3] Next Row

return:
restoreIY = .+2
   pop   iy             ;; [4] Restore IX, IY
   pop   ix             ;; [4]
   ret                  ;; [3] Return
