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

;; MOV = ( inc | dec )
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
;; C bindings for <cpct_etm_drawTileRow2x8_agf>
;;
;;  xx microSecs, xx bytes
;; cpct_etm_drawTileRow2x8_agf(u16 width, void* tilemap, void* memory, void* tileset)
;; BC  = Tilemap
;; DE  = videomem
;; HL  = Tileset
;; IYL = Width
;; 
_cpct_etm_drawTileRow2x8_agf::
   ;; Parameters
   ld (restoreIY), iy ;; [6] Save IY
   pop   hl       ;; [3] HL = Return address
   pop   iy       ;; [4] IY = Width
   pop   bc       ;; [3] BC = Tilemap Ptr
   pop   de       ;; [3] DE = Videomem Ptr
   ex  (sp), hl   ;; [6] HL = Tileset Ptr
   push  ix       ;; [5] Save IX

   ;; Setup (Should be performed outside)
   ld (videoMemPtr), de    ;; [6]
   ld (tilesetPtr), hl     ;; [5]

   ;; Disable interrupts and save SP
   di                      ;; [1] Disable interrupts before starting (we are using SP to read values)
   ld (restoreSP), sp      ;; [6] Save actual SP to restore it in the end

   ;; Get tile ID
nexttile:
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
;;;   jr     z, return     ;; [2/3]

   ;; Advance HL to next tile location in video memory
   inc    l  ;; [1]
   inc    l  ;; [1]
   inc    l  ;; [1]
   inc   hl  ;; [2] HL += 4

   ;; Repeat tile drawing if there are more identical
   dec    a             ;; [1]
   jr    nz, drawTile   ;; [2/3]

;;;  inc   bc             ;; [2]
   ld (videoMemPtr), hl ;; [5]
   cp__iyl              ;; [2] IYL == 0? (A is 0)
   jp    nz, nexttile   ;; [3]

;;;  jp    nexttile       ;; [3]

return:
restoreSP = .+1
   ld    sp, #0000      ;; [3] Restore SP (#0000 is a placeholder)
   ei                   ;; [1] Reenable interrupts before returning
restoreIY = .+2
   ld    iy, #0000      ;; [4] Restore IX, IY
   pop   ix             ;; [4]
   ret                  ;; [3] Return
