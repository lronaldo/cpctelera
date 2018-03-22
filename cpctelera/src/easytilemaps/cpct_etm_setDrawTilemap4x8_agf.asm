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
;; Function: cpct_etm_setDrawTilemap4x8_agf
;;
;;    Sets internal configuration values for <cpct_etm_drawTilemap4x8_agf>.
;;
;; C Definition:
;;    void <cpct_etm_setDrawTilemap4x8_agf> (<u8> *width*, <u8> *height*, 
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
;;    > call cpct_etm_setDrawTilemap4x8_agf_asm
;;
;; Details:
;;    Please refer to <cpct_etm_setDrawTilemap4x8_ag> documentation for details. This function
;; works identical to <cpct_etm_setDrawTilemap4x8_ag> except for the fact that it configures
;; values for <cpct_etm_drawTilemap4x8_agf> (f - fast-version).
;;
;; Destroyed Register values: 
;;      AF, DE
;;
;; Required memory:
;;      C-bindings - 49 bytes (+187 bytes from <cpct_etm_drawTilemap4x8_agf>-cbindings which is included)
;;    ASM-bindings - 45 bytes (+176 bytes from <cpct_etm_drawTilemap4x8_agf>-asmbindings which is included)
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; LOCAL MACRO: setDrawTileMap4x8_agf_gen
;;    All code function is defined as a macro to prevent code duplication on reuse
;; between ASM/C bindings. This macro generates the code for setDrawTileMap4x8_agf 
;; and couples it to the appropriate labels that it has to modify. A label prefix 
;; is passed to generate different kinds of labels for different bindings.
;;    Therefore, ASM/C bindings include this file and use this macro to generate
;; specific source code that will be compiled, with the appropriate labels.
;;
.macro setDrawTilemap4x8_agf_gen lblPrf

;; Declare global symbols used here
.globl lblPrf'tilesetPtr
.globl lblPrf'widthHeightSet
.globl lblPrf'restoreWidth
.globl lblPrf'updateWidth
.globl lblPrf'incrementHL
.globl lblPrf'restoreI

   ;; Set (tilesetPtr) placeholder
   ld (lblPrf'tilesetPtr), hl     ;; [5] Save HL into tilesetPtr placeholder

   ;; Set all Width values required by drawTilemap4x8_agf. First two values
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
   ;; bytes at each loop, we need to add (tilemapWidth - Width + 1) bytes.
   dec    a                      ;; [1] A = Width - 1
   sub_de_a                      ;; [7] tilemapWidth - (Width - 1)
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

   ;; Set the restoring of Interrupt Status. drawTilemap4x8_agf disables interrupts before
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
