;--------------------------------------------------------------------------
;  crt0_rle.s
;
;  Copyright (C) 2001, Michael Hope
;
;  This library is free software; you can redistribute it and/or modify it
;  under the terms of the GNU General Public License as published by the
;  Free Software Foundation; either version 2, or (at your option) any
;  later version.
;
;  This library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License 
;  along with this library; see the file COPYING. If not, write to the
;  Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;   MA 02110-1301, USA.
;
;  As a special exception, if you link this library with other files,
;  some of which are compiled with SDCC, to produce an executable,
;  this library does not by itself cause the resulting executable to
;  be covered by the GNU General Public License. This exception does
;  not however invalidate any other reasons why the executable file
;   might be covered by the GNU General Public License.
;--------------------------------------------------------------------------

        .area   _CODE

        ;; Special RLE decoder used for initing global data
__initrleblock::
        ;; Pull the destination address out
        ld      c,l
        ld      b,h

        ;; Pop the return address
        pop     hl
1$:
        ;; Fetch the run
        ld      e,(hl)
        inc     hl
        ;; Negative means a run
        bit     7,e
        jp      Z,2$
        ;; Code for expanding a run
        ld      a,(hl)
        inc     hl
3$:
        ld      (bc),a
        inc     bc
        inc     e
        jp      NZ,3$
        jp      1$
2$:
        ;; Zero means end of a block
        xor     a
        or      e
        jp      Z,4$
        ;; Code for expanding a block
5$:
        ld      a,(hl)
        inc     hl
        ld      (bc),a
        inc     bc
        dec     e
        jp      NZ,5$
        jp      1$
4$:
        ;; Push the return address back onto the stack
        push    hl
        ret
