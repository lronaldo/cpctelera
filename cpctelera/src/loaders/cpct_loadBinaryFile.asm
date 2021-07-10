;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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

.include "firmware/firmware.s"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Function: cpct_loadBinaryFile
;;
;;    Loads a binary file to a given place in memory.
;;
;; C Definition:
;;    <u8>  <cpct_loadBinaryFile>  (void* *mem_load*, void* *filename*) __z88dk_callee;
;;
;; Input Parameters (4 Bytes):
;;  (2B DE) mem_load  - Pointer to the place in memory where loader will start to copy
;;                      the loaded file
;;  (2B HL) filename  - Pointer to the null terminated string containing the filename
;;
;; Assembly call (Input parameters on registers):
;;    > call cpct_loadBinaryFile
;;
;; Return value (Assembly calls, return L):
;;    <u8>  - (0) = successful loading (no error), (1) = error happened during loading
;;
;; Parameter Restrictions:
;;    * *filename*  must be a pointer to a string containing the name of the file 
;; that will be loaded. This string must be zero terminated and must contain a valid
;; filename. It is programmers responsibility to give correct value. 
;;    * *mem_load* must be a pointer to the byte where loaded bytes will start
;; to be copied to. Bytes will be copied from *mem_load* onwards into memory.
;; There is no restriction on were this pointer can point to, but it is 
;; programmer's responsibility to ensure that loaded bytes will not overwrite
;; any important data in memory. This function does not do any check in this sense.
;;
;; Requirements:
;;    This function requires the CPC *firmware* to be *ENABLED*, as it uses
;; firmware routines to load data. Also, for loading a file from disc, it
;; requires *background ROMs* to be *ENABLED*.
;;
;; Known limitations:
;;    * This function does not do any kind of boundary or size checking. If you
;; give an incorrect value for *filename* or your *mem_load* pointer is placed wrong,
;; undefined behaviour will happen. This will probably make your programs hang
;; or crash.
;;
;; Details:
;;    This function loads a binary file to a given place in memory (*mem_load*),
;; using firmare routines. In order to read a file from disc, it is necessary that
;; background ROMs are activated. However, when a program is loaded using Basic's
;; command *run* "*program*", background ROMs are disabled. Therefore, for loading
;; files from disc, background ROMs must be enabled again before using this function.
;; That can be achieved using *cpct_enableBackgroundROMs*.
;;
;;    If background ROMs are not enabled (or when using a system without disc drive),
;; cpct_loadBinaryFile will load files from tape. In this case a 2K buffer (placed at
;; 0xC000) will be used. Also, loading messages will be displayed on screen. It is 
;; recommended to use *cpct_miniload* for tape loading.
;;
;;    If there are failures during loading, the routine returns with an error code (1).
;; When loading has no errors, it returns (0).
;;
;; Use example:
;; (start code)
;; // This example will load a sprite from disc using firmware functions.
;; // For this example an image has been previously converted to sprite, in bin format,
;; // and placed the resulting file "cpct.bin" into the DSK folder of the project.
;; #include <cpctelera.h>
;; void programcontrol(void) {
;;    // Declare a pointer to video memory
;;    u8* pvmem;
;;    // Clear the screen, as some ROMs print messages on screen when activated
;;    cpct_clearScreen(0);
;;    // Make pvmem point to the byte where we want to print a sprite
;;    pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 25, 50);
;;    // Load a file named "cpct.bin" into memory address 0x5000
;;    cpct_loadBinaryFile((void *)0x5000,"cpct.bin");
;;    // Draw the actual sprite (where pvmem is pointing)
;;    cpct_drawSprite((void *)0x5000,pvmem, 30, 21);
;;    // Loop forever
;;    while (1);
;; }
;; // main function doesn't use local variables
;; void main(void) {
;;    // cpct_enableBackgroundROMs is the first call in main, enables disc ROM
;;    cpct_enableBackgroundROMs();
;;    // Jump to central part of the program
;;    programcontrol();
;; }
;; (end code)
;;
;; Destroyed Register values: 
;;       AF, BC, DE, HL, IX 
;;
;; Required memory:
;;      C-bindings - 41 bytes 
;;    ASM-bindings - 38 bytes
;;
;; Time Measures:
;; Next table shows only time used by CPCtelera's code in cpct_loadBinaryFile. It does not
;; take into account time used by firmware functions to actually read the file. The total
;; time to read a file will be much bigger.
;; (start code)
;;  Case      | microSecs (us) |    CPU Cycles
;; ----------------------------------------------
;;  Best      |       74       |      296
;; ----------------------------------------------
;; Asm saving |      -12       |      -48
;; ----------------------------------------------
;; (end code)   
;;
;; Credits:
;;    This function was created based on code from Kevin Tacker, and some tutorials
;; from Mochilote.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


   ;; cpct_loadBinaryFile starts here
   push de                     ;; [4] Save loading address
   push hl                     ;; [4] Save string location

   ;; Get filename string length
   ld b,#0                     ;; [2] Reset counter
getStrLen_loop:
   ld a,(hl)                   ;; [2] Load a char from filename string
   or a                        ;; [1] Test if it's 0
   jr z,getStrLen_exit         ;; [2/3] If so, exit loop
   inc b                       ;; [1] Increment counter
   inc hl                      ;; [2] HL points to next char
   jr getStrLen_loop           ;; [3] Next char
getStrLen_exit:                ;; B = Filename length

   ;; Read file using firmware routines
   pop hl                      ;; [3] Restore string location
   ld de,#0xC000               ;; [3] two K buffer (not used with cas_in_open when reading disc)
   call firmware_cas_in_open   ;; [5] Open file
   pop hl                      ;; [3] Restore loading address
   jr nc,lb_error              ;; [2/3] Exit if an error was detected
   call firmware_cas_in_direct ;; [5] Read file
   jr nc,lb_error              ;; [2/3] Exit if an error was detected
   call firmware_cas_in_close  ;; [5] Close file
   jr nc,lb_error              ;; [2/3] Exit if an error was detected

   ;; Exit from function
   ld l,#0                     ;; [2] L = 0  ==>  No errors detected
   ret                         ;; [3] Return to caller
lb_error:
   ld l,#1                     ;; [2] L = 1  ==>  Error detected
   ret                         ;; [3] Return to caller
