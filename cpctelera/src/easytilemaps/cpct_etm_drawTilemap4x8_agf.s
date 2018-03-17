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
   pop   de             ;; [3] Get next 2 sprite bytes
   ld  (hl), e          ;; [2] Copy byte 1
   MOV    l             ;; [1] HL++ / HL-- (4-byte aligned) -> next video mem location
   ld  (hl), d          ;; [2] Copy byte 2
   MOV    l             ;; [1] HL++ / HL-- (4-byte aligned) -> next video mem location
   pop   de             ;; [3] Get next 2 sprite bytes
   ld  (hl), e          ;; [2] Copy byte 3
   MOV    l             ;; [1] HL++ / HL-- (4-byte aligned) -> next video mem location
   ld  (hl), d          ;; [2] Copy byte 4
.endm

;;
;; C bindings for <cpct_etm_setDrawTileMap4x8_agf>
;;
;;  xx microSecs, xx bytes
;; cpct_etm_setDrawTileMap4x8_agf(u8 width, u8 height, u16 tilemapWidth, void* tileset)
;; BC  = B:Height, C:Width
;; DE  = TileMapWidth
;; HL  = ptileset
;; 
_cpct_etm_setDrawTileMap4x8_agf::
   pop   hl                ;; [3] HL = Return Address
   pop   bc                ;; [3] BC = B:Height, C:Width
   pop   de                ;; [3] DE = TilemapWidth
   ex  (sp), hl            ;; [6] HL = ptileset

   ld (tilesetPtr), hl     ;; [5]
   ;; Setup Width Update for every row
   ld     a, b             ;; [1] A = height
   ld (heightSet), a       ;; [4] Set Height
   ld     a, c             ;; [1] A = Width
   ld (widthSet), a        ;; [4]
   ld (restoreWidth), a    ;; [4]
   dec    a                ;; [1] A = Width - 1
   sub_de_a                ;; [7] tilemapWidth - (Width - 1)
   ld (updateWidthLow), a  ;; [4] (as A == E right now)
   ld     a, d             ;; [1]
   ld (updateWidthHigh), a ;; [4]

   ;; Setup Increment HL for each row
   ld     a, c             ;; [1] A = Width
   add    a                ;; [1] A = 2*Width
   add    a                ;; [1] A = 4*Width
   cpl                     ;; [1] A = - 4*Width - 1
   add #0x50 + 1           ;; [2] 0x50 - 4*Width (( ==> Maximum showable width = 64))
   ld (incrementHL), a     ;; [4] 

   ;; Setup restoring of previous interrupt status
   ld     a, i             ;; [3] P/V flag set to current interrupt status (IFF2 flip-flop)
   ld     a, #opc_EI       ;; [2] A = Opcode for Enable Interrupts instruction (EI = 0xFB)
   jp    pe, int_enabled   ;; [3] If interrupts are enabled, EI is the appropriate instruction
     ld   a, #opc_DI       ;; [2] Otherwise, it is DI, so A = Opcode for Disable Interrupts instruction (DI = 0xF3)
int_enabled:
   ld (restoreI), a        ;; [4] Restore Interrupt status at the end with corresponding DI or EI

   ret

;;
;; C bindings for <cpct_etm_drawTileMap4x8_agf>
;;
;;  xx microSecs, xx bytes
;; cpct_etm_drawTileMap4x8_agf(void* tilemap, void* memory)
;; BC  = Tilemap
;; HL  = VideoMem
;; 
_cpct_etm_drawTileMap4x8_agf::
   ;; Parameters
   pop   hl       ;; [3] HL = Return address
   pop   bc       ;; [3] BC = Tilemap Ptr
   ex  (sp), hl   ;; [6] HL = Videomem Ptr
   push  ix       ;; [5] Save IX
   push  iy       ;; [5] Save IY

widthSet  = .+2
heightSet = .+3
   ld iy, #0000            ;; [4] IYL=Width, IYH=Height

nextRow:
   ;; Disable interrupts and save SP
   di                      ;; [1] Disable interrupts before starting (we are using SP to read values)
   ld (restoreSP), sp      ;; [6] Save actual SP to restore it in the end

   ;; Get tile ID
nexttile:
   ;; Setup VideoMemPtr Restore
   ld (videoMemPtr), hl    ;; [5]
   ld     a, (bc)          ;; [2] Next tile

   ;; HL = 32*A (Only calculate L component)
   ld     h, a             ;; [1] 
   and   #0x07             ;; [2]
   rrca                    ;; [1]
   rrca                    ;; [1]
   rrca                    ;; [1] 
   ld     l, a             ;; [1] 

   ;; A = Count how many consecutive tiles there are
   ld__e_iyl               ;; [2] IYL = row width
rep:
   dec    e                ;; [1]
   jr     z, endwidth      ;; [2/3]
   inc   bc                ;; [2]
   ld     a, (bc)          ;; [2]
   cp     h                ;; [1]
   jr     z, rep           ;; [2/3]
endwidth:
   ld__a_iyl               ;; [2]
   ld__iyl_e               ;; [2]         
   sub    e                ;; [1] A = IYL - E

   ;; HL = 32*A (H Component. Offset of the tile in the tileset)
   srl    h                ;; [2] 
   srl    h                ;; [2]
   srl    h                ;; [2]

   ;; IX Points to the tile
tilesetPtr = .+1
   ld    de, #0000         ;; [3] <= Placeholder for #tileset
   add   hl, de            ;; [3] HL = tileset + 32*A
   ex    de, hl            ;; [1]
   ld__ixh_d               ;; [2]
   ld__ixl_e               ;; [2] IX = HL

   ;;
   ;; Draw Tile
   ;; SP = End of the tile - 1
   ;; HL = Video Memory (top-left-corner)
   ;; Uses DE
   ;;
videoMemPtr = .+1
   ld    hl, #0000       ;; [3] <= Placeholder for Video Mem
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
   ld    de, #0000      ;; [3] 0x50 - 4*width (#0000 placeholder)
   add   hl, de         ;; [3] HL + 0x50 - 4*width

restoreWidth = .+2
   ld__iyl  #00         ;; [2] IYL = Width (#00 placeholder)

;; Update Width adding difference FullWidth - DrawnWidth
   ld     a, c          ;; [1]
updateWidthLow = .+1
   add   #00            ;; [2] << LowWidthAddition (#00 Placeholder)
   ld     c, a          ;; [1]
   ld     a, b          ;; [1]
updateWidthHigh = .+1
   adc   #00            ;; [2] << HighWidthAddition (#00 Placeholder)
   ld     b, a          ;; [1]

   jp    nextRow        ;; [3] Next Row

return:
restoreIY = .+2
   pop   iy             ;; [4] Restore IX, IY
   pop   ix             ;; [4]
   ret                  ;; [3] Return
