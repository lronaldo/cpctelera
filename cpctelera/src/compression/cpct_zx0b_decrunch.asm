;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2021 Einar Saukas (https://github.com/einar-saukas/ZX0)
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
.module cpct_compression

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_zx0b_decrunch
;;
;;    Decompresses an array using ZX0B (ZX0 backwards variant) compression algorithm. This is the
;; standard version of this decompression routine.
;;
;; C Definition:
;;    void <cpct_zx0b_decrunch> (void* *dest_end*, const void* *source_end*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;    (2B  DE) dest_end   - Ending (latest) byte of the destination (decompressed) array
;;    (2B  HL) source_end - Ending (latest) byte of the source (compressed) array
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_zx0b_decrunch_asm
;;
;; Parameter Restrictions:
;;    * *dest_end* should be a 16-bit pointer to the latest byte of the array where decompressed
;; data will be written. It could point anywhere in memory. Data will be written from that
;; byte backwards until all compressed data has been decompressed. No runtime checks
;; are performed. By specially careful to ensure that this pointer is correct; otherwise
;; undesired parts of memory could be overwritten, causing undefined behaviour. 
;;    * *source_end* should be a 16-bit pointer to the latest byte of the array where compressed
;; data is held. ZX0B algorithm will read the array from this byte backwards till its start.
;; No runtime checks are performed: if this value is incorrect, undefined behaviour will follow.
;; Typically, garbage data of undefined size will be produced, potentially overwriting undesired
;; memory parts.
;;
;; Known limitations:
;;     * This function does not do any kind of checking over pointers or data passed. If any of the
;; pointers is badly calculated or incorrect, or compressed data is corrupted, undefined behaviour 
;; will follow.
;;
;; Details:
;;      This function decompresses an array of data previously compressed using ZX0B algorithm
;; by Einar Saukas. In order to perform this decompression, two in-memory arrays are required:
;; a *source* array containing compressed data and a *destination* array where decompressed
;; data will be written. *source* array is only read and never changed, so it might be placed
;; either on RAM or on ROM memory. *destination* array is required to be in RAM, as
;; decompressed data will be written there.
;;
;;      Decompression is performed starting from the end of both arrays, and progressing 
;; backwards until their beginning, as figure 1 shows,
;; (start code)
;;    ---------------------------------------------------------------------------
;;                                  M E M O R Y 
;;    ---------------------------------------------------------------------------
;;     Address |                     Contents                                  |
;;    ---------------------------------------------------------------------------
;;     0x0100  |   |   [_9_|_8_|_7_|_6_|_5_|_4_|_3_|_2_|_1_|_0_]   |   |   |   | << [Destination Array]
;;     0x0110  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
;;     0x0120  |   |   |   {·E·|·D·|·C·|·B·|·A·}   |   |   |   |   |   |   |   | << {Source Array}
;;    ---------------------------------------------------------------------------
;;                  0x0123 {^··Source Array···^} 0x0127
;;              0x0102 [^__________Destination Array__________^] 0x010B
;;
;;    Figure 1. Example memory disposition of a source and a destination array
;; (end code)
;;    The example in figure 1 shows a *source* array that takes 5 bytes from 0x0123 to 0x0127 in 
;; memory, and a 10-bytes destination array from 0x0102 to 0x010B in memory. In this example, 
;; *source_end* equals 0x0127 whereas *dest_end* equals 0x010B. Decompression starts reading byte
;; at *source_end* (0x0127 = A) and writing decompressed data from *dest_end* (0x010B = 0) backwards.
;; Once first byte is decompressed, previous one at *source_end - 1* (0x0126) follows. Decompression
;; continues reading *source* array backwards and writing at *destination* array backwards too, 
;; until the end of compressed data (0x0123 = E) is reached.
;;
;;    Following you will found an use example,
;; (start code)
;;    // Amstrad CPC Video Memory starts at 0xC000 and ends at 0xFFFF
;;    // Decrunching function requires a pointer to the end of this "array"
;;    #define VIDEO_MEMORY_END   (void*)(0xFFFF)
;;
;;    //////////////////////////////////////////////////////////////////////////
;;    // This function draws the background of the next screen of this game.
;;    // It takes the compressed image of the background and directly 
;;    // decompresses it to video memory
;;    //
;;    void decruch_background_to_screen(u8 screen_num) {
;;       // Get a pointer to the end of the array that contains 
;;       // crunched screen data. That is: beginning + size - 1
;;       void *scr_crunched_data_end = game_screens[screen_num].crunched_data + game_screens[screen_num].size - 1;
;;
;;       // Directly decompress crunched screen data to video memory to 
;;       // show the background there
;;       cpct_zx0b_decrunch(VIDEO_MEMORY_END, scr_crunched_data_end);
;;    }      
;;  
;; (end code)
;;
;;    There is no need to specify array sizes. Decompression routine ends when the end marker of 
;; compressed data is found. That end marker is placed at the beginning of the *source* array 
;; (0x0123 in the example). This means that corrupted compressed data or an error in the *source_end*
;; pointer will potentially lead this routine to continue decompressing in an undefined-end loop, 
;; probably overwriting other places in memory, previous to the beginning of *destination* array.
;;
;; Destroyed Register values: 
;;      AF,  BC,  DE,  HL
;;
;; Required memory:
;;      C-bindings - 72 bytes 
;;    ASM-bindings - 69 bytes 
;;
;; Time Measures:
;;      Time taken by this routine is highly dependent on data passed, and not only on its size.
;; If you need to know the exact time it will take to decompress a given array of data, the best
;; way to do it is using an emulator with timing measurements, like WinAPE. 
;;
;;
;; Credits:
;;    * <Original code at https://github.com/einar-saukas/ZX0> ZX0 decoder ("Standard" version - Backwards variant) by Einar Saukas. 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


dzx0_standard_back:
        ld      bc, #1              ;; [3] preserve default offset 1
        push    bc                  ;; [4]
        dec     c                   ;; [1]
        ld      a, #0x80            ;; [2]
dzx0sb_literals:
        call    dzx0sb_elias        ;; [5] obtain length
        lddr                        ;; [5/6] copy literals
        add     a, a                ;; [1] copy from last offset or new offset?
        jr      c, dzx0sb_new_offset;; [2/3]
        call    dzx0sb_elias        ;; [5] obtain length
dzx0sb_copy:
        ex      (sp), hl            ;; [6] preserve source, restore offset
        push    hl                  ;; [4] preserve offset
        add     hl, de              ;; [3] calculate destination - offset
        lddr                        ;; [5/6] copy from offset
        pop     hl                  ;; [3] restore offset
        ex      (sp), hl            ;; [6] preserve offset, restore source
        add     a, a                ;; [1] copy from literals or new offset?
        jr      nc, dzx0sb_literals ;; [2/3]
dzx0sb_new_offset:
        inc     sp                  ;; [2] discard last offset
        inc     sp                  ;; [2]
        call    dzx0sb_elias        ;; [5] obtain offset MSB
        dec     b                   ;; [1]
        ret     z                   ;; [2/4] check end marker
        dec     c                   ;; [1] adjust for positive offset
        ld      b, c                ;; [1]
        ld      c, (hl)             ;; [2] obtain offset LSB
        dec     hl                  ;; [2]
        srl     b                   ;; [2] last offset bit becomes first length bit
        rr      c                   ;; [2]
        inc     bc                  ;; [2]
        push    bc                  ;; [4] preserve new offset
        ld      bc, #1              ;; [3] obtain length
        call    c, dzx0sb_elias_backtrack ;; [4/5]
        inc     bc                  ;; [2]
        jr      dzx0sb_copy         ;; [3]
dzx0sb_elias:
        inc     c                   ;; [1] inverted interlaced Elias gamma coding
dzx0sb_elias_loop:
        add     a, a                ;; [1]
        jr      nz, dzx0sb_elias_skip ;; [2/3]
        ld      a, (hl)             ;; [2] load another group of 8 bits
        dec     hl                  ;; [2]
        rla                         ;; [1]
dzx0sb_elias_skip:
        ret     nc                  ;; [2/3]
dzx0sb_elias_backtrack:
        add     a, a                ;; [1]
        rl      c                   ;; [2]
        rl      b                   ;; [2]
        jr      dzx0sb_elias_loop   ;; [3]
