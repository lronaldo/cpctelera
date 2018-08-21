;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2018 César Nicolás González (CNGSoft)
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_miniload
;;
;;    Load a binary block of data from cassette and copy it to a given
;; place in memory. Pulse length of data bytes must be 740T (1T = 1/3500000s)
;;
;; C Definition:
;;    <u8>  <cpct_miniload>  (void* *mem_load*, u16 *size*) __z88dk_callee;
;;
;; Input Parameters (4 Bytes):
;;  (2B IX) mem_load - Pointer to the place in memory where loader will start to copy loaded bytes
;;  (2B DE) size     - Size in bytes of the binary block that will be read from cassette
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_miniload_asm
;;
;; Return value (Assembly calls, return L):
;;    <u8>  - (0) = successful loading (no error), (>0) = error happened during loading
;;
;; Other return values (Only for assembly calls)
;;    Carry - Set(1) = successful loading, NotSet(0) = error happened
;;     H    - Final checksum (255 if everything went ok)
;;     DE   - Remaining bytes to be read (0 if everything went ok)
;;     IX   - Final memory address (IX+DE if everything went ok)
;;
;; Parameter Restrictions:
;;    * *mem_load* must be a pointer to the byte where loaded bytes will start
;; to be copied to. Bytes will be copied from *mem_load* onwards into memory.
;; There is no restriction on were this pointer can point to, but it is 
;; programmer's responsibility to ensure that loaded bytes will not overwrite
;; any important data in memory. This function does not do any check in this sense.
;;    * *size*  must be the exact size of the block of bytes that will be loaded
;; from cassette. Erroneous sizes will make loading fail. Moreover, erroneous 
;; sizes could also make loaded data to overwrite valid data from memory. It is
;; programmers responsibility to give correct size values. 
;;
;; Known limitations:
;;    * This function *will not work from a ROM*, as it uses self-modifying code.
;;    * This function does not do any kind of boundary or size checking. If you
;; give an incorrect value for *size* or your *mem_load* pointer is placed wrong,
;; undefined behaviour will happen. This will probably make your programs hang
;; or crash.
;;    * It disables interrupts on call, and re-enables them on return. If you
;; wanted interrupts to be disabled, disable them after calling this function
;; or comment the line with instruction 'ei' for your own version of this routine.
;;
;; Details:
;;    This function loads a binary block of data from cassette tape directly
;; to a given place in memory (*mem_load*). Raw blocks of data must be codified
;; in cassette tape as pulses of length 740T (being T = 1/3500000s). Expected
;; codification is 2 pulses (full) = 1 bit. The duration of both pulses is used
;; to distinguish a 1 from a 0. As our period is 740T, that would be standard
;; duration of a 0. Double duration, 2*740T=1480T would then be considered a 1.
;; To make it fault tolerant, the middle of both values is considered as discriminant.
;; Therefore, 2 pulses of less than 1100T would be considered a 0, and if they are
;; above 1100T, that would be considered a 1. As an overflow condition, a value
;; over 400% the length of a 0 (4*740T = 2960T) would be considered as overflow,
;; giving an error condition. All this makes the function perform at approximately 
;; 1576,58 bits per second. (197,07 bytes per second).
;; 
;;    If there are failures during cassette loading, cassette motors are stopped
;; and the routine returns with an error code (1). Then you may ask user to 
;; rewind and load the block of data again, make a cold reboot or any other 
;; thing you consider.
;;
;; Use example:
;;    <TODO>
;;
;; Destroyed Register values: 
;;      C-bindings - AF, BC, DE, HL, BC'
;;    ASM-bindings - AF, BC, DE, HL, BC', IX
;;
;; Required memory:
;;      C-bindings - 127 bytes 
;;    ASM-bindings - 114 bytes
;;
;; Time Measures:
;; (start code)
;;   Case | microSecs(us) | CPU Cycles
;; -------------------------------------
;;  Best  |      xx       |     xx
;; -------------------------------------
;;  Worst |      xx       |    xxx
;; -------------------------------------
;; (end code)
;;
;; Credits:
;;    This function is a subset of TinyTape by César Nicolás González (CNGSoft)
;; created on 2018/08/20 by <CNGSoft at http://cngsoft.no-ip.org/index.htm>.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; IX=^OFFSET,DE=LENGTH; IX+++,DE---,H=$FF?,ABCLF!,CF=OK?

   di                ;; Disable interrupts before starting
   ld    bc, #0xF610 ;; F6 = PIO Port C (0x10 for cassete start)
   out  (c), c       ;; Cassette start

   ;; Function requires 7F (Gate Array port) at B'
   ;; This is guaranteed if this is called from BASIC, but not otherwise
   exx               ;; 
   ld    bc, #0x7F10 ;; B = Gate array port (0x7F), C=Border Register (0x10)
   out  (c), c       ;; Select border register for later color changes
   exx               ;;

init: 
   ld     h, #0
tone: 
   call  full        ;; lee un pulso
   jr    nc, init    ;; demasiado largo
   sub    b
   jr    nc, init    ;; demasiado corto
   inc    h
   jr    nz, tone    ;; TONE valido
wait: 
   ld     h, #-2
sync: 
   call  full        ;; lee un pulso
   jr    nc, init    ;; demasiado largo
   sub    b
   jr     c, wait    ;; pertenece a TONE, no a SYNC
   inc    h
   jr    nz, sync    ;; SYNC valido
   jr    byte

;;------------------------------------------------------------------------------------
;; Helper routines for reading pulses
;; 

full: 
   ld     b, #2-64   ;; *!* para medir un pulso
half: 
   ld     a, #16-2   ;; retraso de seguridad
secdelay:
   dec    a
   jr    nz, secdelay
edge: 
   inc    b
   ret    z
   ld     a, #0xF5   ;; Read from tape 
   in     a, (0)     ;;
   xor    c
   and   #0x80
   jr     z, edge
   xor    c
   ld     c, a
   exx
   ld     a, r
   or    #0x40
   and    b
   out  (c), a       ;; color del borde
   exx
   ld     a, #2-48   ;; *!* valor intermedio
   scf
   ret
;;------------------------------------------------------------------------------------
;;------------------------------------------------------------------------------------


next: 
   ld  (ix), l       ;; guarda el byte
   inc   ix
   dec   de
byte: 
   ld     l, #1
bits: 
   ld     b, #1-80   ;; *!* para medir dos pulsos
   call  half
   call   c, half
   jr    nc, exit    ;; On error, exit (L != 0 in this case, that will be the error code)
   sub    b
   rl     l          ;; inserta un bit s
   jr    nc, bits
   ld     a, h
   xor    l
   ld     h, a
   ld     a, d
   or     e
   jr    nz, next
   inc    a
   add    h

   ld     l, h       ;; Success loading. Return L=0 (No error). H=0 when no error happened
exit:
   ld    bc, #0xF600 ;; F6 = PIO Port C (0x00 for cassete stop)
   out  (c), c       ;; Cassette stop
   ei                ;; Enable interrupts again
   
;; Return instruction provided by bindings
