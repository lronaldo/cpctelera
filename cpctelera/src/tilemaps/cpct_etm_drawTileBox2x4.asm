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
;; Function: cpct_etm_redrawTileBox
;;
;;    Redraws a determined box of tiles inside the tilemap.
;;
;; C Definition:
;;    void <cpct_etm_redrawTileBox> (void* *ptilemap*, u8 *x*, u8 *y*, u8 *w*, u8 *h*) __z88dk_callee;
;;
;; Input Parameters (9 bytes):
;;    (1B  A) map_width - Width in tiles of a complete row of the tilemap
;;    (1B  C) x         - x tile-coordinate of the starting tile inside the tilemap
;;    (1B  B) y         - y tile-coordinate of the starting tile inside the tilemap
;;    (1B  L) w         - width in tiles of the tile-box to be redrawn
;;    (1B  H) h         - height in tiles of the tile-box to be redrawn
;;    Stack:
;;    (2B) pvideomem    - Pointer to the place on video memory (or backbuffer) where to draw the tilebox
;;    (2B) ptilemap     - Pointer to the start of the tilemap
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_redrawTileBox_asm
;;
;; Parameter Restrictions:
;;    * *ptilemap* could be any 16-bits value, representing the memory 
;; address where the <cpct_TEasyTilemap> structure is stored. This function expects
;; the parameter to point to a complete structure like this: if the structure is not
;; well populated or the pointer points to other thing, the result is undefined (most
;; likely a crash or rubbish on the screen).
;;    * (*x*, *y*) represent the coordinates, in tiles, of the upper-left tile of 
;; the tilebox to be redrawn. These coordinates are not checked to be inside the 
;; tilemap, and providing bad coordinates leads to undefined behaviour. On a rectangular
;; screen, *x* would be in the range [0,39] and *y* in the range [0,49].
;;    * (*w*, *h*) represent the width and height, in tiles, of the tilebox being
;; redrawn. Same as (*x*,*y*), these values are not checked and values drawing outside
;; the tilemap could be provided, and would lead to undefined behaviour. 
;;
;; Known limitations:
;;     * This function does not do any kind of checking over the tilemap, its
;; contents or size. If you give a wrong pointer, your tilemap has different
;; dimensions than required or has less / more tiles than will be used later,
;; rubbish can appear on the screen.
;;     * Inside the structure, the pointer to the video memory *must* point to a
;; pixel line 0 from any character line on the screen or back buffer. To know where 
;; pixel lines 0 are located you may have a look at <cpct_drawSprite> documentation. 
;; This function *does not work if pvideomem points to a pixel line 4*.
;;     * This function only draws 8-bytes tiles of size 2x4 (in bytes).
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * Under hardware scroll conditions, tile drawing will fail if asked to draw
;; near 0x?7FF or 0x?FFF addresses (at the end of each one of the 8 pixel lines), as 
;; next screen byte at that locations is -0x7FF and not +1 bytes away.
;;
;; Details:
;;    This function draws tilebox (a group of tiles forming a rectangular box), a portion
;; of the tilemap, to the screen or to a backbuffer. 
;;
;;    Each tile from the tileset must be a 8-bytes, 2x4-sized tile. This function calls
;; <cpct_drawTileAligned2x4_f> for drawing each one of the tiles. 
;;
;; Destroyed Register values: 
;;      AF,  BC,  DE,  HL
;;      AF', BC', DE', HL'
;;
;; Required memory:
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
;; H  = Map height (number of vertical tiles)
;; HO = Number of odd tile rows.
;; HE = Number of even tile rows.
;; For HO and HE, the first row is considered odd if it starts at a pixel line 4,
;; and it is considered even otherwise. The last row of the tilemap is never taken 
;; into account (either for HO or HE).
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Declare tile drawing function we are going to use
.globl cpct_etm_drawTileRow2x4_asm

;; Macro for multiplying 16-bits by 8 bits, doing
;;   HL += DE * A
;; Important: Carry MUST be 0 at the start
;;   best:   11 us
;;   worst: 111 us
;;
.macro mult_de_a
   ;; Displaced digits multiply
 mul:
   rra               ;; [1]   Move next right bit to the Carry
   jr   nc, skip_add ;; [2/3] If No Carry, this bit does not multiply, so we dont add another DE
   add  hl, de       ;; [3]   If Carry, this bit does multiply, so we add DE to the total 

 skip_add:           ;; For the next iteration, we will add 2*DE if next bit from A is on
   sla   e           ;; [2] | DE = 2*DE 
   rl    d           ;; [2] |
   or    a           ;; [1] Check A while resetting Carry (8-bit digit being displaced for multiplication)
   jr   nz, mul      ;; [2/3] If A != 0, multiplication hasn't finished yet, continue
.endm   

;; Macros for processing HL += A taking carry into account
;;   5 microSecs, 5 bytes
;; 
.macro add__hl_a
   add   l         ;; [1] | L += A (add up Least Significant Bytes)
   ld    l, a      ;; [1] |
   adc   h         ;; [1] A' = H + L' + Carry (L' = L + A)
   sub   l         ;; [1] A' = H + Carry  (As we subtract L')
   ld    h, a      ;; [1] | H = A' = H + Carry, finally making HL = HL + A
.endm

;;
;; ENTRY POINT
;;

   ld (set_map_width + 1), a ;; [4] Save map_width in its placeholder for later use   
   ld    a, l                ;; [1] A = width of the box in tiles
   ld (draw_next_row + 1), a ;; [4] Save the width into its placeholder to restore it every loop into B
   ld (tb_row_width + 1), a  ;; [4] Save tilebox_row_width in its placeholder for later calculations
   
   ld    a, h      ;; [1] | Save tilebox height into A'
   ex   af, af'    ;; [1] |

   ;; The location where the upper-left tile from the tilebox lies is calculated this way:
   ;;    TileBoxStart = 0x50 * [y/2] + 0x2000 * (y % 2) + 2*x

   ;;[20] First, calculate HL = 0x50 (y/2)
   ld    a, b      ;; [1] A = y coordinate
   and  #0xFE      ;; [2] A = 2y' (being y' = int(y / 2))

   ;; Calculate A = 8y', taking into account that y range is [0,49] on a regular screen
   ;; which means that bits 7 and 8 of A are irrelevant, and we can multiply by 4
   ;; without taking carry into account.
   add   a         ;; [1] A = 2y' + 2y' =  4y'
   add   a         ;; [1] A = 4y' + 4y' =  8y'
   add   a         ;; [1] A = 8y' + 8y' = 16y' (It might procude Carry!)


   ;; Move A to HL including carry, to do 16-bits math
   ld    l, a      ;; [1]  L = 16y' (excluding Carry!)
   ld    h, #0     ;; [2] | H = Carry
   rl    h         ;; [2] |

   ;; Now we need 16-bits to do the math
   ld    d, h      ;; [1] | DE = HL = 16y'
   ld    e, l      ;; [1] |

   add  hl, hl     ;; [3] HL = 16y' + 16y' = 32y'
   add  hl, hl     ;; [3] HL = 32y' + 32y' = 64y'
   add  hl, de     ;; [3] HL = 64y' + 16y' = 80y' = 0x50 * y' = 0x50 * [y/2]

   ;; [5/8] Second, add up 0x2000 * (y % 2)
   bit   0, b      ;; [2]   Test value of bit 0 from y coordinate (B), to ask "y % 2 = 0?"
   jr z, dont_add  ;; [2/3] If y % 2 == 0, do nothing (no need to add 0x2000 to HL)
   ld    a, #0x20  ;; [2] y % 2 == 1, so add 0x2000 to HL
   add   h         ;; [1]
   ld    h, a      ;; [1] 

dont_add:
   ;; [7] Third, add up 2 * x
   ld    a, c      ;; [1] A = x coordinate
   add   a         ;; [1] A = 2x (Ignore carry, because on a 80-byte wide screen x shouldn't be greater than 40)
   add__hl_a       ;; [5] HL += 2x, so finally HL = 0x50 * int(y/2) + 0x2000

   ;; [13] We have to also add the start location of the tilemap in video memory
   pop  de         ;; [3] DE=pvideomem (recoveded from the stack)
   add  hl, de     ;; [3] HL += BC, so HL now points to the place where first tile should be drawn
   ex   de, hl     ;; [1] DE = HL (DE points to the place to draw in)
   pop  hl         ;; [3] HL=ptilemap (recovered from the stack)
   push de         ;; [4] Save pointer to the place in video memory where to draw in

   ;; Calculate the offset of the first tile of the box inside the tilemap and add it
   ;; to the main tilemap pointer (ptilemap)
   ;;    Offset = y * map_width + x
   ;;    HL = ptilemap + Offset
   ;;
   ld    a, c      ;; [1]   | HL = ptilemap + x
   add__hl_a       ;; [5]   |  
set_map_width:
   ld   de, #0000  ;; [3] DE = map_width
   ld    c, e      ;; [1]  C = map_width (save value to use it later)
   ld    a, b      ;; [1] A = y 
   cp    a         ;; [1] Reset Carry Flag (Required for multiplying)
   mult_de_a       ;; [11-83] HL += DE * A (HL = y * map_width + x)
   ;; HL now points to the next tile to draw from the tilemap!

   ;; Calculate number of tiles to increment to jump from the end of a row
   ;; to the start of the next one
   ld   a, c                   ;; [1] A = map_width
tb_row_width:
   sub  #00                    ;; [2] A = A - tilebox_row_width (A is offset jump to next row)
   ld  (next_row_jump + 1), a  ;; [5] Save offset jump to next row in its placeholder

   ;; Restore tilemap height into C
   ex   af, af'       ;; [1] | C = tilebox height (recovered from AF')
   ld    c, a         ;; [1] |

   ;; Restore pvideomem into DE
   pop  de            ;; [3] DE = pvideomem
   jr   draw_next_row ;; [3] Start by drawing first row

;;
;; Draw the tile-box
;;
drawtiles_height:
   ld    a, d      ;; [1] DE += 0x2000 (0x800 x 4) to jump 4 pixel lines from here
   add  #0x20      ;; [2] 
   ld    d, a      ;; [1]
   and  #0x38      ;; [2] Check bits 11,12,13 of D to know if we have jumped to a 
                   ;;     ... new character line (pixel line 0, all three bits = 0)
   jr nz, draw_next_row ;; [2/3] Not 0 => Pixel Line 4 => No need to adjust

   ;; We are jumping to a new pixel line 0, 
   ;; so we have to jump to the next character line (adding 0xC050)
   ld    a, e      ;; [1] DE += 0xC050
   add  #0x50      ;; [2] ... jump 4 lines and, as we will overflow video memory 
   ld    e, a      ;; [1] ... add 0xC050 to move to next character line
   ld    a, d      ;; [1]  
   adc  #0xC0      ;; [2]
   ld    d, a      ;; [1] 

draw_next_row:
   ld    b, #00    ;; [2] B = Width of the box (it must be restored for each new row to be drawn, #00 is a placeholder)

   ;; HL' = DE (HL' is the pointer to video memory which 
   ;; ... we are changing, so put the result in there
   ld   a, e       ;; [1] A = E
   exx             ;; [1] Change to alternate register set
   ld   e, a       ;; [1] E' = A
   exx             ;; [1] Back to normal register set
   ld   a, d       ;; [1] A = D
   exx             ;; [1] Change to alternate register set
   ld   d, a       ;; [1] D' = A,  DE' = DE (Pointer to video memory passed to DE')
   exx             ;; [1] Back to normal register set

;; (B) numtiles (DE') pvideomem (HL) ptilemap
   call  cpct_etm_drawTileRow2x4_asm

   ;; Make HL point to the start of the next tilebox row
   ld    a, l      ;; [1] | HL += offset from end of a tilebox row to
next_row_jump:     ;;     |      the start of the next one 
   add   #00       ;; [2] |      (HL += 2 (map_width - tilebox_row_width))
   ld    l, a      ;; [1] |
   adc   h         ;; [1] |
   sub   l         ;; [1] |
   ld    h, a      ;; [1] |

   dec  c                   ;; [1] 1 less tile row to draw
   jr  nz, drawtiles_height ;; [2/3] If there still are some tile rows to draw (C!=0), continue

   ;; Drawing ends