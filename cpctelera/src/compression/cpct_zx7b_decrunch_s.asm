;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2012 Einar Saukas
;;  Copyright (C) 2017 Antonio Villena
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
;;     * This function does not do any kind of checking over pointers passed. If any of the
;; pointers is badly calculated or incorrect, undefined behaviour will follow.
;;
;; Details:
;;    <TODO>
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
;;    file used    |  this   |(m)edium  (f)ast0  (f)ast1  (f)ast2
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Original code from Antonio Villena, based on ZX7 algorithm by Einar Saukas.
;; Got from https://github.com/antoniovillena/zx7b/blob/master/dzx7b_slow.asm

        ld      bc, #0x8000
        ld      a, b
copyby: inc     c
        ldd
mainlo: call    getbit
        jr      nc, copyby
        push    de
        ld      d, c
lenval: call    nc, getbit
        rl      c
        rl      b
        call    getbit
        jr      nc, lenval
        inc     c
        jr      z, exitdz
        ld      e, (hl)
        dec     hl
        sll__e
        jr      nc, offend
        ld      d, #0x10
nexbit: call    getbit
        rl      d
        jr      nc, nexbit
        inc     d
        srl     d
offend: rr      e
        ex      (sp), hl
        ex      de, hl
        adc     hl, de
        lddr
exitdz: pop     hl
        jr      nc, mainlo
getbit: add     a, a
        ret     nz
        ld      a, (hl)
        dec     hl
        adc     a, a
        ret
