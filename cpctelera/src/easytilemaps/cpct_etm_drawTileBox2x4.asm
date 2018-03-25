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
;; Function: cpct_etm_drawTileBox2x4
;;
;;    Draws (or redraws) a determined rectangle of tiles inside a tilemap. It is 
;; mainly used to restore parts of a tilemap.
;;
;; C Definition:
;;    void <cpct_etm_drawTileBox2x4> (<u8> *x*, <u8> *y*, <u8> *w*, <u8> *h*, <u8> *map_width*,
;;                                    void* *pvideomem*, const void* *ptilemap* ) __z88dk_callee;
;;
;; Input Parameters (9 bytes):
;;    (1B  C) x         - x tile-coordinate of the starting tile inside the tilemap
;;    (1B  B) y         - y tile-coordinate of the starting tile inside the tilemap
;;    (1B  E) w         - width in tiles of the tile-box to be redrawn
;;    (1B  D) h         - height in tiles of the tile-box to be redrawn
;;    (1B  A) map_width - Width in tiles of a complete row of the tilemap
;;    * Always received on the stack
;;    (2B) pvideomem    - Pointer to upper left corner of the *tilemap* in video memory.
;;    (2B) ptilemap     - Pointer to the start of the tilemap
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_etm_drawTileBox2x4_asm
;;    It is important to notice that part of the parameters are received on 
;; registers and part of them on the stack. This would be an example code for
;; calling this function,
;; (start code)
;;    ;; Set Parameters on the stack
;;    ld   hl, #ptilemap   ;; HL = pointer to the tilemap
;;    push hl              ;; Push ptilemap to the stack
;;    ld   hl, #pvideomem  ;; HL = Pointer to video memory location where tilemap is drawn
;;    push hl              ;; Push pvideomem to the stack
;;    ;; Set Paramters on registers
;;    ld    a, #map_width  ;; A = map_width
;;    ld    b, #y          ;; B = x tile-coordinate
;;    ld    c, #x          ;; C = y tile-coordinate
;;    ld    d, #h          ;; H = height in tiles of the tile-box
;;    ld    e, #w          ;; L =  width in tiles of the tile-box
;;    call  cpct_etm_drawTileBox2x4_asm ;; Call the function
;; (end code)
;;
;; Parameter Restrictions:
;;    * (*x*, *y*) represent the coordinates, in tiles, of the upper-left corner of 
;; the tilebox to be drawn. These coordinates are relative to the upper-left corner of
;; the tilemap (its first tile). They are not checked to be inside the tilemap, and 
;; providing bad coordinates leads to undefined behaviour. Supported ranges for this
;; coordinates are: *x* in [1,127] and *y* in [1,63] (However, y >= 50 will be outside
;; normal screen, because 50*4 = 200 pixels, which is the height of a normal screen).
;;    * (*w*, *h*) represent the width and height, in tiles, of the tilebox being
;; drawn. Same as (*x*, *y*), these values are not checked and values drawing outside
;; the tilemap or beyond the screen could be provided, and would lead to undefined behaviour. 
;;    * *map_width* is expected to be the width of the tilemap in tiles. Do not confuse
;; this value with *w*, that represents the width of the tilebox (please note the difference).
;; Maximum width of the tilemap is 255 tiles, as it is a <u8> value. However, take into 
;; account that maps wider than 127 tiles can represent a problem, as the initial *x* 
;; parameter of this function cannot be greater than 127. 
;;    * *pvideomem* should be a pointer to the video memory or backbuffer address where
;; the upper-left corner of the tilemap is drawn. Please note that it should point to the
;; start of the tilemap, and *NOT* the start of the tilebox, in video memory or backbuffer.
;;    * *ptilemap* should be a 2-bytes pointer to the start of a tilemap definition in
;; Easytilemaps format: a 2D matrix of 1-byte tile-indexes. If it points to a different
;; kind of data, the result is undefined (most likely a crash or rubbish on the screen).
;;
;; Known limitations:
;;     * This function cannot be called using tail-recursion optimization. It *must*
;; be called using a CALL instruction. If you call it using JP, parameters passed on
;; the stack will be used as return address making your program execute arbitrary code.
;;     * This function does not do any kind of checking over the tilemap, its
;; contents or size. If you give a wrong pointer, your tilemap has different
;; dimensions than required or has less / more tiles than will be used later,
;; rubbish can appear on the screen.
;;     * The *pvideomem* pointer *must* point to a *pixel line 0 or pixel line 4* from 
;; any character line on the screen or back buffer. To know where pixel lines 0/4 are 
;; located you may have a look at <cpct_drawSprite> documentation.
;;     * This function only draws 8-bytes tiles of size 2x4 (in bytes).
;;     * This function *will not work from ROM*, as it uses self-modifying code.
;;     * Under hardware scroll conditions, tile drawing will fail if asked to draw
;; near 0x?7FF or 0x?FFF addresses (at the end of each one of the 8 pixel lines), as 
;; next screen byte at that locations is -0x7FF and not +1 bytes away.
;;
;; Details:
;;    This function draws tile-box, a portion of a concrete tilemap, to the screen 
;; or to a backbuffer. A tile-box is a group of tiles forming a rectangular box, and being
;; part of a tilemap. Data is expected to be in Easytilemaps format, with the tilemap
;; being a 2D matrix of 1-byte tile-indexes, and tileset being an array of pointers
;; (2-bytes each) to tiles. Each tile is expected to be an array of 2x4-bytes in
;; screen pixel format.
;; 
;;    The main intended use of this function is to erase entities that
;; move over a tilemap, redrawing the portion of the tilemap (a tilebox) over the
;; moving entity.
;;
;;    So, first 4 parameters (*x*, *y*, *w*, *h*) of these function define the tilebox as a portion of the
;; given tilemap (last parameter). First 2 parameters locate the upper-left corner of
;; the tilebox (*x*, *y*) and the next two define its width (*w*) and height (*h*) in tiles.
;; *x* and *y* coordinates are considered relative to the upper-left corner of the 
;; tilemap (which is the origin of coordinates, (0,0)), with *x*'s growing right and
;; *y*'s growing down.
;;
;;    This function uses the internal pointer to the tileset (ptileset) for Easytilemaps.
;; This internal pointer *MUST HAVE BEEN SET PREVIOUSLY* to calling this function. 
;; <cpct_etm_setTileset2x4> may be used for setting this internal pointer. Once set
;; it remains unless manually reset.
;;
;;    This function calls <cpct_etm_drawTileRow2x4>, which also calls <cpct_drawTileAligned2x4_f>.
;; So, these two functions will also be included in your code when using <cpct_etm_drawTileBox2x4>
;; (using their assembly bindings).
;;
;; Destroyed Register values: 
;;      AF,  BC,  DE,  HL
;;      AF', BC', DE', HL'
;;
;; Required memory:
;;      C-bindings - 143 bytes (Plus <cpct_etm_drawTileRow2x4> & <cpct_drawTileAligned2x4_f>)
;;    ASM-bindings - 140 bytes (Plus <cpct_etm_drawTileRow2x4> & <cpct_drawTileAligned2x4_f>)
;;
;; Time Measures:
;; (start code)
;;    Case     |      microSecs (us)                |              CPU Cycles                 |
;; --------------------------------------------------------------------------------------------
;;    Any      | 98 + PX + MY + 7PPY + (37 + 103W)H | 392 + 4PX + 4MY + 28PPY + (148 + 412W)H |
;; --------------------------------------------------------------------------------------------
;;  ASM-Saving |            - 10                    |               - 40                      |
;; --------------------------------------------------------------------------------------------
;;  W=2, H=3   |         [ 845 - 937 ]              |           [ 3380 - 3748 ]               |
;; --------------------------------------------------------------------------------------------
;;  W=4, H=6   |        [ 2824 - 2916 ]             |          [ 11296 - 11664 ]               |
;; --------------------------------------------------------------------------------------------
;; (end code)
;;  W   - Tilebox width (number of horizontal tiles)
;;  H   - Tilebox height (number of vertical tiles)
;;  PX  - Extra cycles to add when *y* parameter is odd, for a intra-character lines jump
;;       PX = { *0*, for even values of *y*, 
;;             *11*, for  odd values of *y* and tilemap starting at a Pixel Line 0,
;;             *13*, for  odd values of *y* and tilemap starting at a Pixel Line 4 }
;;  MY  - Cost in cycles of multiplying *map_width* times *y*. Depends on the value of *y*,
;;       and can be calculated with this formula:
;;        (start code)
;;         MY = 12*int(log2(y)) + 2*onbits(y) - 1, 
;;         // onbits(y) = the number of bits that y has set to 1
;;        (end code)
;;       MY is in range [11-83]
;;
;;  PPY - Number of total rows of the tilebox that start at a pixel line 4, not
;;       counting the last row of the tilebox (if it is at a pixel line 4).
;;       It corresponds to one of this two formulas
;;        (start code)
;;         PPY = int(H/2) 
;;         PPY = int(H/2) - 1
;;        (end code)
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

   ;; The location where the upper-left tile from the tilebox is to be drawn
   ;; is calculated this way:
   ;;    TileBoxStart = VideoMemTilemapStart + 0x50 * [y/2] + 0x2000 * (y % 2) + 2*x
   ;;
   ;;    0x50 =   80 bytes = 1 character line = 8 pixel lines = 2 tile lines (hence [y/2])
   ;;  0x2000 = 8192 bytes = 4 pixel lines = 1 tile line (in-between a character line)
   ;;  1 tile =    2 bytes wide, hence adding 2*x to set horizontal offset
   ;;

   ;; First, calculate HL = 0x50 (y/2)
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

   ;; Third, add up 2 * x
   ld    a, c      ;; [1] A = x coordinate
   add   a         ;; [1] A = 2x (Ignore carry, because x will be in the range [1,127])
   add__hl_a       ;; [5] HL += 2x, so finally HL = 0x50 * int(y/2) + 0x2000

   ;; We have to add the start location of the tilemap in video memory
   pop  de         ;; [3] DE=pvideomem (recoveded from the stack)
   add  hl, de     ;; [3] HL += BC, so HL now points to the place where first tile should be drawn

   ;; And do not forget to add displacements for pixel lines 0 and 4
   ;;  - When map starts at pixel line 0, add 0x2000 on odd lines (to jump to pixel line 4)
   ;;  - When map starts at pixel line 4, add 0xE050 on odd lines (to jump to next pixel line 0)
   ;;
   bit   0, b       ;; [2]   Test value of bit 0 from y coordinate (B), to ask "y % 2 = 0?"
   jr    z, dont_add;; [2/3] If y % 2 == 0, do nothing (we are no a even line, no need to jump)
   ld    a, #0x38   ;; [2]   If y % 2 != 0, need to calculate where does map start...
   and   d          ;; [1] ... so check bits 13, 12 and 11 from DE. 
   jr    z, pixline0;; [2/3] ... If the 3 bits are 0, we are a at a pixel line 0, go add 0x2000
   ld   de, #0xE050 ;; [3] ... else, this is pixel line 4, add 0xE050 (jump to next pixel line 0 from pixel line 4)
   jr   doadd       ;; [3] Go to addition line
pixline0:
   ld   de, #0x2000 ;; [3] DE=0x2000 for jumping to pixel line 4 from pixel line 0
doadd:
   add  hl, de      ;; [3] Jump to correct pixel line (0 or 4, depending on previous conditions)
dont_add:

   ;; Finally, save pointer to the place in memory where we should start drawing, and
   ;; Load tilemap pointer to start tile calculations
   ;;
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
   ;; HL ends pointing to the byte the tilemap after the last drawn tile
   ;; Make HL point to the start of the next tilebox row
   ld    a, l      ;; [1] | HL += offset from end of a tilebox row to
next_row_jump:     ;;     |      the start of the next one 
   add   #00       ;; [2] |      (HL += 2 (map_width - tilebox_row_width))
   ld    l, a      ;; [1] |
   adc   h         ;; [1] |
   sub   l         ;; [1] |
   ld    h, a      ;; [1] |

   ;; Calculate next location in video memory to draw 
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
   call  cpct_etm_drawTileRow2x4_asm ;; [7 + 103W]

   dec  c                   ;; [1] 1 less tile row to draw
   jr  nz, drawtiles_height ;; [2/3] If there still are some tile rows to draw (C!=0), continue

   ;; Drawing ends