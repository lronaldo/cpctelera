;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2012 Einar Saukas (https://www.ime.usp.br/~einar/)
;;  Copyright (C) 2017 Antonio Villena (https://github.com/antoniovillena)
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
.module cpct_compression

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_zx7b_decrunch_s
;;
;;    Decompresses an array using ZX7B compression algorithm. This is the smallest
;; version of this decompression routine.
;;
;; C Definition:
;;    void <cpct_zx7b_decrunch_s> (void* *dest_end*, const void* *source_end*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;    (2B  DE) dest_end   - Ending (latest) byte of the destination (decompressed) array
;;    (2B  HL) source_end - Ending (latest) byte of the source (compressed) array
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_zx7b_decrunch_s_asm
;;
;; Parameter Restrictions:
;;    * *dest_end* should be a 16-bit pointer to the latest byte of the array where decompressed
;; data will be written. It could point anywhere in memory. Data will be written from that
;; byte backwards until all compressed data has been decompressed. No runtime checks
;; are performed. By specially careful to ensure that this pointer is correct; otherwise
;; undesired parts of memory could be overwritten, causing undefined behaviour. 
;;    * *source_end* should be a 16-bit pointer to the latest byte of the array where compressed
;; data is held. ZX7B algorithm will read the array from this byte backwards till its start.
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
;;      This function decompresses an array of data previously compressed using ZX7B algorithm
;; by Einar Saukas and Antonio Villena. In order to perform this decompression, two in-memory
;; arrays are required: a *source* array containing compressed data and a *destination* array
;; where decompressed data will be written. *source* array is only read and never changed, so
;; it might be placed either on RAM or on ROM memory. *destination* array is required to be 
;; in RAM, as decompressed data will be written there.
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
;;       cpct_zx7b_decrunch_s(VIDEO_MEMORY_END, scr_crunched_data_end);
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
;;      C-bindings - 67 bytes 
;;    ASM-bindings - 64 bytes 
;;
;; Time Measures:
;;      Time taken by this routine is highly dependent on data passed, and not only on its size.
;; If you need to know the exact time it will take to decompress a given array of data, the best
;; way to do it is using an emulator with timing measurements, like WinAPE. Following measures
;; were taking as benchmark to give a comparison idea between different versions of this 
;; same routine.
;;
;; (start code)
;;  ---------------------------------------------------------------
;;      CPU Clock Cycles taken by different routine versions
;;  ---------------------------------------------------------------
;;    file used    |  this   | general  (f)ast0  (f)ast1  (f)ast2
;;  ---------------------------------------------------------------
;;    lena1k       |  136724 |  112287    83707    81511    81040
;;    lena16k      | 2578233 | 2091259  1518892  1468437  1462568
;;    lena32k      | 4974712 | 4023756  2913537  2818843  2803116
;;    alice1k      |  115167 |   95877    75410    73616    73459
;;    alice16k     | 2158335 | 1769433  1366443  1326895  1328886
;;    alice32k     | 4312995 | 3534254  2730160  2649630  2654236
;;    128rom1k     |  103020 |   83629    64069    62825    62000
;;    128rom16k    | 1970827 | 1605062  1219271  1188707  1180569
;;    128rom32k    | 4003487 | 3257233  2463147  2399125  2381847
;;  ---------------------------------------------------------------
;;    routine size |    64   |    67      156      177      191
;;  ---------------------------------------------------------------
;; (end code)
;;      You may divide CPU Clock Cycles by 4 to get an *estimation* of Microseconds
;; or NOPS that this routine will take.
;;
;;      Thanks to Antonio Villena for these benchmarks.
;;
;; Credits:
;;    * <Original code at https://github.com/antoniovillena/zx7b/blob/master/dzx7b_slow.asm> 
;;      from Antonio Villena, based on ZX7 algorithm by Einar Saukas.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ld      bc, #0x8000   ;; [3] marker bit at MSB of B
        ld      a, b          ;; [1] move marker bit to A
copyby: inc     c             ;; [1] to keep BC value after LDD
        ldd                   ;; [5] copy literal byte
mainlo: call    getbit        ;; [5+(5/11)] read next bit
        jr      nc, copyby    ;; [2/3] next bit indicates either literal or sequence

; determine number of bits used for length (Elias gamma coding)
        push    de            ;; [4] store destination on stack
        ld      d, c          ;; [1] D= 0, assume BC=$0000 here (or $8000)
lenval: call    nc, getbit    ;; [5+(5/11)] don't call first time (to save bytes)
        rl      c             ;; [2] 
        rl      b             ;; [2] insert bit on BC
        call    getbit        ;; [5+(5/11)] more bits?
        jr      nc, lenval    ;; [2/3] repeat, final length on BC

; check escape sequence
        inc     c             ;; [1] detect escape sequence
        jr      z, exitdz     ;; [2/3] end of algorithm

; determine offset
        ld      e, (hl)       ;; [2] load offset flag (1 bit) + offset value (7 bits)
        dec     hl            ;; [2]
        sll__e                ;; [2]
        jr      nc, offend    ;; [2/3] if offset flag is set, load 4 extra bits
        ld      d, #0x10      ;; [2] bit marker to load 4 bits
nexbit: call    getbit        ;; [5+(5/11)]
        rl      d             ;; [2] insert next bit into D
        jr      nc, nexbit    ;; [2/3] repeat 4 times, until bit marker is out
        inc     d             ;; [1] add 128 to DE
        srl     d             ;; [2] retrieve fourth bit from D
offend: rr      e             ;; [2] insert fourth bit into E
        ex      (sp), hl      ;; [6] store source, restore destination
        ex      de, hl        ;; [1] destination from HL to DE
        adc     hl, de        ;; [3] HL = destination + offset + 1
        lddr                  ;; [5xsize (BC)]
exitdz: pop     hl            ;; [3] restore source address (compressed data)
        jr      nc, mainlo    ;; [2/3] end of loop. exit with getbit instead ret (-1 byte)
getbit: add     a, a          ;; [1] check next bit
        ret     nz            ;; [2/4] no more bits left?
        ld      a, (hl)       ;; [2] load another group of 8 bits
        dec     hl            ;; [2]
        adc     a, a          ;; [1]
        ret                   ;; [3]
