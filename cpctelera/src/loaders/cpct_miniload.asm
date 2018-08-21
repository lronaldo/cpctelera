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
;; to distinguish a 1 from a 0. As our period is 740T, the standard duration of
;; of a 0 would be 2*740T = 1480T. A 1 would be 2 times the duration of a 0, taking
;; 4*740T=2960T. To make it fault tolerant, the middle of both values is considered 
;; as discriminant. Therefore, 2 pulses of less than 3*740T = 2220T would be 
;; considered a 0, and if they are above 2220T, that would be considered a 1. 
;; As an overflow condition, a value over 400% the length of a 0 (8*740T = 5920T) 
;; would be considered as overflow, giving an error condition. All this makes 
;; the function perform at approximately 1576,58 bits per second. (197,07 bytes per second).
;; 
;;    If there are failures during cassette loading, cassette motors are stopped
;; and the routine returns with an error code (1). Then you may ask user to 
;; rewind and load the block of data again, make a cold reboot or any other 
;; thing you consider.
;;
;; Use example:
;;    This example shows how to create a simple cassette loader using miniload.
;; Cassette tape will need to have the binary of the game encoded in miniload
;; format, 2 pulses per bit and 740T of pulse duration.
;; (start code)
;;     #include <cpctelera.h>
;;     
;;     // Memory address where the binary of the game has to be loaded
;;     #define LOAD_ADDRESS  (void*)0x100
;;     // Memory address where main function of the game starts
;;     #define RUN_ADDRESS   (void*)0x3D08
;;     // Exact size in bytes of the binary of the game, that will be loaded from cassette
;;     #define BIN_SIZE      15396
;;     
;;     // Loader code starts here
;;     void main(void) {
;;        // Define a function pointer to main function of the game 
;;        // And directly initialize it to the memory address where 
;;        // the function starts. This function is void main(void)
;;        void (*gameMain)() = RUN_ADDRESS;
;;     
;;        // Load the binary of the game at the memory address where
;;        // it expects to be. It is important to give the exact byte
;;        // size of the block to be loaded, or errors will happen.
;;        // If loading is successful (return value = 0), 
;;        //  call gameMain to start the game.
;;        if ( !cpct_miniload(LOAD_ADDRESS, BIN_SIZE) )
;;           gameMain();
;;     
;;        // If loading is not successful, execution will reach end
;;        // of main, returning control to firmware and normally 
;;        // producing a machine reset
;;     }
;; (end code)
;;
;;    This simple example would generate a binary (that we may call LOADER.BIN) 
;; that expects to be placed first in the cassette. This first binary will be
;; encoded in a standard firmware loader format. Immediately afterwards, the
;; cassette will contain a second binary (we will call it GAME.BIN) taking up 
;; 15396 bytes of space (BIN_SIZE) codified in 2 pulses per bit and 740T of 
;; pulse duration (MINILOAD basic format). This second binary will be loaded 
;; at 0x100 in memory, taking from there to 0x100 + 15396 = 0x3D24. On successful
;; loading, gameMain() is called jumping to address 0x3D08 and starting 
;; the game if all went well.
;;
;;    Considering this, the layout of the cassette should be as follows
;; (start code)
;;               ---------------------------------------------------------
;;    Contents > | P | LOADER.BIN | P | GAME.BIN                         |
;;    Format   > | - |  Firmware  | - | Miniload basic format (raw1full) |
;;               ---------------------------------------------------------
;; P = Pause (Silence)
;; (end code)
;;
;; Destroyed Register values: 
;;      C-bindings - AF, BC, DE, HL, 
;;    ASM-bindings - AF, BC, DE, HL, IX
;;
;; Required memory:
;;      C-bindings - 131 bytes 
;;    ASM-bindings - 118 bytes
;;
;; Time Measures:
;;    Using miniload basic format, there are 2 pulses per bit, each pulse taking either
;; 740T for a 0, or 1480T for a 1. Therefore, a 0 takes 2*740T = 1480T and a 1 takes
;; 4*740T = 2960T. We can consider its mean value, 3*740T = 2220T per bit.
;; As 1 T = 1/3500000s, Each bit takes 1b = 2220 / 3500000 s = 0,000634285714286 secs.
;; Therefore, 1 byte 1B = 8 x 1b = 0,00507428571429 s. 
;; Dividing, 1 sec can load 197,072072072 bytes, or equivalently, 1576,57657658 bits.
;; 
;;    That makes this loader run at a estimated average speed of:
;;    - 1576,58 bits per second
;;    -  197,07 bytes per second
;;    -    0,19 Kb per second
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
   push  bc          ;; Save BC' value before changing it (it will be restored at the end)
   ld    bc, #0x7F10 ;; B = Gate array port (0x7F), C=Border Register (0x10)
   out  (c), c       ;; Select border register for later color changes
   exx               ;;

init: 
   ld     h, #0      ;; H will hold the CRC

   ;; Identify pilot Tone
tone: 
   call  full        ;; Read 1 pulse
   jr    nc, init    ;; Is it too long?
   sub    b
   jr    nc, init    ;; Is it too long?
   inc    h
   jr    nz, tone    ;; valid TONE

   ;; Wait for sync signal
wait: 
   ld     h, #-2
sync: 
   call  full        ;; Read 1 pulse
   jr    nc, init    ;; Is it too long?
   sub    b
   jr     c, wait    ;; If it belongs to Tone, continue waiting
   inc    h
   jr    nz, sync    ;; valid SYNC signal
   jr    byte

;;------------------------------------------------------------------------------------
;; Helper routines for reading pulses
;; 

full: 
   ld     b, #2-64   ;; *!* to measure 1 pulse
half: 
   ld     a, #16-2   ;; safety delay
secdelay:
   dec    a
   jr    nz, secdelay
edge: 
   inc    b
   ret    z
   ld     a, #0xF5   ;; Read 1 bit from tape
   in     a, (0)     ;;
   xor    c
   and   #0x80
   jr     z, edge
   xor    c
   ld     c, a
   
   ;; Set new random border colour
   exx               ;; use B' = 0x7F to send data to the Gate Array
   ld     a, r       ;; read R to get some randomness
   or    #0x40       ;; Add this bit for colour commands (hardware values)
   and    b          ;; Remove upper bit doing and with 0x7F (unrequired bit)
   out  (c), a       ;; set random border colour
   exx
   
   ld     a, #2-48   ;; *!* intermediate pulse value
   scf
   ret
;;------------------------------------------------------------------------------------
;;------------------------------------------------------------------------------------


next: 
   ld  (ix), l       ;; Store last read byte
   inc   ix          ;; IN++ : Point to next memory location where to store a byte
   dec   de          ;; DE-- : One less byte to be read
byte: 
   ld     l, #1      ;; L will be shifted left with each new bit read. Use this first bit
                     ;; to know when 8 bits have been read (a carry will be produced)
bits: 
   ld     b, #1-80   ;; *!* value to measure 2 pulses
   call  half        ;; Read half pulse
   call   c, half    ;; Read another half pulse if first was right
   jr    nc, exit    ;; On error, exit (L != 0 in this case, that will be the error code)
   sub    b
   rl     l          ;; Insert last bit read into the next byte being read
   jr    nc, bits    ;; Continue reading bits until the byte is full (Carry will appear)
   
   ld     a, h       ;; A = present CRC
   xor    l          ;; XOR CRC with last read byte
   ld     h, a       ;; Store new CRC value
   
   ld     a, d       ;; Check if we have read all the bytes
   or     e          ;; then remaining bytes (DE) will be 0
   jr    nz, next    ;; If DE != 0, continue reading next byte
   ;; Calculate final error status depending on the CRC
   inc    a          ;; | Final error status. If load was successful A=0, H=255. Then, these 
   add    h          ;; | 2 instructions will produce Carry, signaling everything went OK.

   ld     l, a       ;; Success loading. Return L=0 (No error). A=0 when no error happened
exit:
   exx               ;; 
   ld    bc, #0xF600 ;; F6 = PIO Port C (0x00 for cassette stop)
   out  (c), c       ;; Cassette stop
   pop   bc          ;; Restore BC' before ending (Leave alternate register set as it was)
   exx               ;;
   ei                ;; Enable interrupts again
   
;; Return instruction provided by bindings
