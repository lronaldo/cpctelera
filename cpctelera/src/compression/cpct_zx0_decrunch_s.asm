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
;; Function: cpct_zx0_decrunch_s
;;
;;    Decompresses an array using ZX0 compression algorithm. This is the smallest version
;;  of this decompression routine.
;;
;; C Definition:
;;    void <cpct_zx0_decrunch_s> (void* *dest_start*, const void* *source_start*) __z88dk_callee;
;;
;; Input Parameters (4 bytes):
;;    (2B  DE) dest_start   - Starting (first) byte of the destination (decompressed) array
;;    (2B  HL) source_start - Starting (first) byte of the source (compressed) array
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_zx0_decrunch_s_asm
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
        ld      bc, #0xffff               ; preserve default offset 1
        push    bc
        inc     bc
        ld      a, #0x80
dzx0s_literals:
        call    dzx0s_elias             ; obtain length
        ldir                            ; copy literals
        add     a, a                    ; copy from last offset or new offset?
        jr      c, dzx0s_new_offset
        call    dzx0s_elias             ; obtain length
dzx0s_copy:
        ex      (sp), hl                ; preserve source, restore offset
        push    hl                      ; preserve offset
        add     hl, de                  ; calculate destination - offset
        ldir                            ; copy from offset
        pop     hl                      ; restore offset
        ex      (sp), hl                ; preserve offset, restore source
        add     a, a                    ; copy from literals or new offset?
        jr      nc, dzx0s_literals
dzx0s_new_offset:
        call    dzx0s_elias             ; obtain offset MSB
        ex      af, af'
        pop     af                      ; discard last offset
        xor     a                       ; adjust for negative offset
        sub     c
        ret     z                       ; check end marker
        ld      b, a
        ex      af, af'
        ld      c, (hl)                 ; obtain offset LSB
        inc     hl
        rr      b                       ; last offset bit becomes first length bit
        rr      c
        push    bc                      ; preserve new offset
        ld      bc, #1                   ; obtain length
        call    nc, dzx0s_elias_backtrack
        inc     bc
        jr      dzx0s_copy
dzx0s_elias:
        inc     c                       ; interlaced Elias gamma coding
dzx0s_elias_loop:
        add     a, a
        jr      nz, dzx0s_elias_skip
        ld      a, (hl)                 ; load another group of 8 bits
        inc     hl
        rla
dzx0s_elias_skip:
        ret     c
dzx0s_elias_backtrack:
        add     a, a
        rl      c
        rl      b
        jr      dzx0s_elias_loop
