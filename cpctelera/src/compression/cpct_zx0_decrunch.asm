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
;; Function: cpct_zx0_decrunch
;;
;;    Decompresses an array using ZX0 compression algorithm. This is the standard version
;;  of this decompression routine.
;;
;; C Definition:
;;    void <cpct_zx0_decrunch> (void* *dest_start*, const void* *source_start*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;    (2B  DE) dest_start   - Starting (first) byte of the destination (decompressed) array
;;    (2B  HL) source_start - Starting (first) byte of the source (compressed) array
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_zx0_decrunch_asm
;;
;; Parameter Restrictions:
;;    * *dest_start* should be a 16-bit pointer to the first byte of the array where decompressed
;; data will be written. It could point anywhere in memory. Data will be written from that
;; byte onwards until all compressed data has been decompressed. No runtime checks
;; are performed. By specially careful to ensure that this pointer is correct; otherwise
;; undesired parts of memory could be overwritten, causing undefined behaviour. 
;;    * *source_start* should be a 16-bit pointer to the first byte of the array where compressed
;; data is held. ZX0 algorithm will read the array from this byte onwards till its end.
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
;;      This function decompresses an array of data previously compressed using ZX0 algorithm
;; by Einar Saukas. In order to perform this decompression, two in-memory arrays are required:
;; a *source* array containing compressed data and a *destination* array where decompressed data
;; will be written. *source* array is only read and never changed, so it might be placed either
;; on RAM or on ROM memory. *destination* array is required to be in RAM, as decompressed data
;; will be written there.
;;
;;      Decompression is performed starting from the start of both arrays, and progressing 
;; onwards until their end, as figure 1 shows,
;; (start code)
;;    ---------------------------------------------------------------------------
;;                                  M E M O R Y 
;;    ---------------------------------------------------------------------------
;;     Address |                     Contents                                  |
;;    ---------------------------------------------------------------------------
;;     0x0100  |   |   [_0_|_1_|_2_|_3_|_4_|_5_|_6_|_7_|_8_|_9_]   |   |   |   | << [Destination Array]
;;     0x0110  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
;;     0x0120  |   |   |   {·A·|·B·|·C·|·D·|·E·}   |   |   |   |   |   |   |   | << {Source Array}
;;    ---------------------------------------------------------------------------
;;                  0x0123 {^··Source Array···^} 0x0127
;;              0x0102 [^__________Destination Array__________^] 0x010B
;;
;;    Figure 1. Example memory disposition of a source and a destination array
;; (end code)
;;    The example in figure 1 shows a *source* array that takes 5 bytes from 0x0123 to 0x0127 in 
;; memory, and a 10-bytes destination array from 0x0102 to 0x010B in memory. In this example, 
;; *source_start* equals 0x0123 whereas *dest_start* equals 0x0102. Decompression starts reading byte
;; at *source_start* (0x0123 = A) and writing decompressed data from *dest_start* (0x0102 = 0) onkwards.
;; Once first byte is decompressed, next one at *source_start + 1* (0x0124) follows. Decompression
;; continues reading *source* array onwards and writing at *destination* array onwards too, 
;; until the end of compressed data (0x0127 = E) is reached.
;;
;;    There is no need to specify array sizes. Decompression routine ends when the end marker of 
;; compressed data is found. That end marker is placed at the end of the *source* array 
;; (0x0127 in the example). This means that corrupted compressed data or an error in the *source_start*
;; pointer will potentially lead this routine to continue decompressing in an undefined-end loop, 
;; probably overwriting other places in memory, previous to the beginning of *destination* array.
;;
;; Destroyed Register values: 
;;      AF,  AF',  BC,  DE,  HL
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
;; Credits:
;;    * <Original code at https://github.com/einar-saukas/ZX0> ZX0 decoder ("Standard" version) by Einar Saukas. 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


dzx0_standard:
        ld      bc, #0xffff         ;; [3] preserve default offset 1
        push    bc                  ;; [4]
        inc     bc                  ;; [2]
        ld      a, #0x80            ;; [2]
dzx0s_literals:
        call    dzx0s_elias         ;; [5] obtain length
        ldir                        ;; [5/6] copy literals
        add     a, a                ;; [1] copy from last offset or new offset?
        jr      c, dzx0s_new_offset ;; [2/3]
        call    dzx0s_elias         ;; [5] obtain length
dzx0s_copy:
        ex      (sp), hl            ;; [6] preserve source, restore offset
        push    hl                  ;; [4] preserve offset
        add     hl, de              ;; [3] calculate destination - offset
        ldir                        ;; [5/6] copy from offset
        pop     hl                  ;; [3] restore offset
        ex      (sp), hl            ;; [6] preserve offset, restore source
        add     a, a                ;; [1] copy from literals or new offset?
        jr      nc, dzx0s_literals  ;; [2/3]
dzx0s_new_offset:
        call    dzx0s_elias         ;; [5] obtain offset MSB
        ex      af, af'             ;; [1]
        pop     af                  ;; [3] discard last offset
        xor     a                   ;; [1] adjust for negative offset
        sub     c                   ;; [1]
        ret     z                   ;; [2/4] check end marker
        ld      b, a                ;; [1]
        ex      af, af'             ;; [1]
        ld      c, (hl)             ;; [2] obtain offset LSB
        inc     hl                  ;; [2]
        rr      b                   ;; [2] last offset bit becomes first length bit
        rr      c                   ;; [2]
        push    bc                  ;; [4] preserve new offset
        ld      bc, #1              ;; [3] obtain length
        call    nc, dzx0s_elias_backtrack ;; [4/5]
        inc     bc                  ;; [2]
        jr      dzx0s_copy          ;; [3]
dzx0s_elias:
        inc     c                   ;; [1] interlaced Elias gamma coding
dzx0s_elias_loop:
        add     a, a                ;; [1]
        jr      nz, dzx0s_elias_skip;; [2/3]
        ld      a, (hl)             ;; [2] load another group of 8 bits
        inc     hl                  ;; [2]
        rla                         ;; [1]
dzx0s_elias_skip:
        ret     c                   ;; [2/3]
dzx0s_elias_backtrack:
        add     a, a                ;; [1]
        rl      c                   ;; [2]
        rl      b                   ;; [2]
        jr      dzx0s_elias_loop    ;; [3]
