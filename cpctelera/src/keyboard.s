;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU General Public License for more details.
;;
;;  You should have received a copy of the GNU General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
;#####################################################################
;### MODULE: Keyboard                                              ###
;#####################################################################
;### Routines control and check keyboard status, keys and joystick ###
;#####################################################################
;### TECHNICAL INFORMATION:                                        ###
;### Keyboard and joystick are connected to AY-3-8912 Programable  ###
;### Sound Generator (PSG) which recieves, processes and stores    ###
;### pressed/not pressed information. The PSG is connected to the  ###
;### 8255 Programable Peripheral Interface (PPI), which the CPU    ###
;### can directly address. Therefore, to read the keyboard and     ###
;### joystick status, the Z80 has to communicate with the PPI and  ###
;### ask it to read the PSG status. This is done using OUT         ###
;### instruction to the 8255 PPI ports:                            ###
;###  > PPI Port A           = #F4xx                               ###
;###  > PPI Port B           = #F5xx                               ###
;###  > PPI Port C           = #F6xx                               ###
;###  > PPI Control-Register = #F7xx                               ###
;###                                                               ###
;### Keyboard and joystick switches and buttons are arranged in a  ###
;### 10x8 matrix. Each element of the matrix represents the state  ###
;### of one button/switch/key (pressed/not pressed). That means    ###
;### the CPC can control up to 80 keys/switches/buttons in total.  ###
;###                                                               ###
;### We're able to read a complete column of the matrix each time. ###
;### That means we get the state of 8 switches at a time, in the   ###
;### form of a byte (each bit represents the state of an switch).  ###
;### Each bit will hold a "0" if the switch is "pressed" or a "1"  ###
;### when the switch is "not pressed".                             ###
;###                                                               ###
;### It is relevant to notice something about joysticks. Although  ###
;### joystick 0 has its own column in the matrix (9th) to control  ###
;### its 6 switches, joystick 1 shares its switches with other     ###
;### keys in the 6th column (namely: F, G, T, R, 5, 6). Therefore, ###
;### it is possible to emulate a 2nd joystick using keyboard. The  ###
;### exact mapping between matrix values and switches pressed is   ###
;### included below as Table1.                                     ###
;###                                                               ###
;### To query for the values, we should select PSG's Register 14,  ###
;### which is done writting 0Eh (14) to 8255 PPI port A (which is  ###
;### directly connected to the PSG). Then, Bits 3..0 of PPI port C ###
;### are connected to a decoder that sends this to the keyboard,   ###
;### selecting Matrix Line to be read. Bits 7-6 are conected to    ###
;### PSG's operation mode selector, and lets us select between     ###
;### (00)inactive / (01)read / (10)write / (11)register_select     ###
;### operation modes. So, writting C0h (11 000000) to Port C we    ###
;### tell the PSG to select a register (the 0Eh that previouly was ###
;### send to PSG through port A). Then, it is possible to start    ###
;### asking for Matrix Lines and reading the Reg14 through Port A  ###
;### to get the pressed/not pressed values from the Matrix.        ###
;### Just one detail left: it is necessary to put PSG into inactive###
;### mode between different opperations.                           ###
;### Summing up:                                                   ###
;###     > 1: Configure PPI Operation Mode for:                    ###
;###          >> Port A: Ouput, Port C: Output (10000010b = 82h)   ###
;###     > 2: Write 14 (0Eh) to Port A (the index of the register) ###
;###     > 3: Write C0h to Port C (11 000000) to tell PSG that we  ###
;###          want to select a register (indexed at Port A).       ###
;###     > 4: Write 0 (00 000000) to Port C to finish operation    ###
;###         (put PSG inactive between different operations)       ###
;###     > 5: Configure PPI Operation Mode for:                    ###
;###          >> Port A: Input, Port C: Output (10010010b = 92h)   ###
;###     > 6: Write Matrix Line ID to Port C                       ###
;###     > 7: Read Matrix Line Status from Port A                  ###
;###     > 8: Repeat 6 until all Matrix Lines are read             ###
;###     > 9: Configure Again PPI as in (1) (82h Output/Output)    ###
;###          to leave it in this state.                           ###
;#####################################################################
;

; (Table1) MAPPING OF KEYBOARD LINES TO CONCRETE KEYS/SWITCHES
;=========================================================================================================
;|     |                                       L I N E                                                   |
;|     |-------------------------------------------------------------------------------------------------|
;| BIT |      0      |     1      |   2   |  3  |  4  |  5   |      6       |  7  |    8     |     9     |
;|=====|=============|============|=======|=====|=====|======|==============|=====|==========|===========|
;|  7  | f.          | f0         | Ctrl  | > , | < . | Space| V            | X   | Z        | Del       |
;|  6  | Enter       | f2         | ` \   | ? / | M   | N    | B            | C   | Caps Lock| Unused    |
;|  5  | f3          | f1         | Shift | * : | K   | J    | F Joy1_Fire1 | D   | A        | Joy0_Fire1|
;|  4  | f6          | f5         | f4    | + ; | L   | H    | G Joy1_Fire2 | S   | Tab      | Joy0_Fire2|
;|  3  | f9          | f8         | } ]   | P   | I   | Y    | T Joy1_Right | W   | Q        | Joy0_Right|
;|  2  | Cursor Down | f7         | Return| | @ | O   | U    | R Joy1_Left  | E   | Esc      | Joy0_Left |
;|  1  | Cursor Right| Copy       | { [   | = - | ) 9 | ' 7  | % 5 Joy1_Down| # 3 | " 2      | Joy0_Down |
;|  0  | Cursor Up   | Cursor Left| Clr   | £ ^ | _ 0 | ( 8  | & 6 Joy1_Up  | $ 4 | ! 1      | Joy0_Up   |
;=========================================================================================================
; Notes:
;   > Bit 6 on lines 9 and 6, may be used to report a third fire button on a joystick. This bit is also used as the middle button on an AMX compatible mouse.  
;   > "f." is the "." key on the numeric keypad. 
;   > Enter is the Small enter key, whereas Return is the large one.
;   > If matrix line 11-14 are selected, the byte is always &ff. After testing on a real CPC, it is found that these never change, they always return &FF. 
;

;
;########################################################################
;### FUNCTION: _cpct_scanKeyboard                                     ###
;########################################################################
;### This function reads the status of keyboard and joysticks and     ###
;### stores it in the 10 bytes reserverd as "keyboardStatusBuffer"    ###
;########################################################################
;### INPUTS (-)                                                       ###
;########################################################################
;### OUTPUTS (10B)                                                    ###
;###   -> KeyboardStatusBuffer full with pressed/not pressed info     ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  136 + 54*10 + 36 = 712 cycles (178 us)                          ###
;########################################################################
;### CREDITS:                                                         ###
;###    This fragment of code is based on a scanKeyboard code issued  ###
;### by CPCWiki.                                                      ###
;### http://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning    ###
;########################################################################
;
; Define a 10-byte buffer to store keyboard data
keyboardStatusBuffer: .ds 10

.globl _cpct_scanKeyboard
_cpct_scanKeyboard:: 
    
    DI                              ;; [ 4c] Disable interrupts

    LD  HL,  #keyboardStatusBuffer  ;; [10c] HL Points to the start of the keyboardBuffer, where scanned data will be stored

    ;; Configure PPI: Select Register 14 (the one connected with keyboard status) and set it for reading
    ;;
    LD  BC,  #0xF782                ;; [10c] Configure PPI 8255: Set Both Port A and Port C as Output. 
    OUT (C), C                      ;; [12c] 82 = 1000 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=0)=> Port A=Output, 
                                    ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output
    
    LD  BC,  #0xF40E                ;; [10c] Write (0Eh = 14) on PPI 8255 Port A (F4h): the register we want to select on AY-3-8912  
    LD  E,   B                      ;; [ 4c] Save F4h into E to use it later in the loop
    OUT (C), C                      ;; [12c] 
    
    LD  BC,  #0xF6C0                ;; [10c] Write (C0h = 11 000000b) on PPI Port C (F6h): operation > select register 
    LD  D,   B                      ;; [ 4c] Save F6h into D to use it later in the loop
    OUT (C), C                      ;; [12c]
    .DW #0x71ED                     ;; [12c] OUT (C), 0 => Write 0 on PPI's Port C to put PSG's in inactive mode (required in between different operations)

    LD  BC,  #0xF792                ;; [10c] Configure PPI 8255: Set Port A = Input, Port C = Output. 
    OUT (C), C                      ;; [12c] 92h= 1001 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=1)=> Port A=Input, 
                                    ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output
    
    ;; Read Loop: We read the 10-bytes that define the pressed/not pressed status
    ;;
    LD  A,   #0x40                  ;; [ 7c] A refers to the next keyboard line to be read (40h to 49h)
    LD  C,   #0x4a                  ;; [ 7c] 4a is used to compare A and know when we have read all the Matrix Lines
    
rfks_nextKeyboardLine:
    LD  B,   D                      ;; [ 4c] B = F6h => Write the value of A to PPI's Port C to select next Matrix Line
    OUT (C), A                      ;; [12c]
   
    LD  B,   E                      ;; [ 4c] B = F4h => Read from PPI's Port A: Pressed/Not Pressed Values from PSG
    INI                             ;; [16c] The read value is written to (HL), then HL<-HL+1 and B<-B-1

    INC A                           ;; [ 4c] Loop: Increment A => Next Matrix Line. 
    CP  C                           ;; [ 4c] Check if we have arrived to line 4a, which is the end 
    JP  C, rfks_nextKeyboardLine    ;; [10c] Repeat loop if we are not done.
    

    ;; Restore PPI status to Port A=Output, Port C=Output
    ;;
    LD  BC,  #0xF782                ;; [10c] Put again PPI in Output/Output mode for Ports A/C.
    OUT (C), C                      ;; [12c]

    EI                              ;; [ 4c] Reenable interrupts
    
    RET                             ;; [10c] Return

;
;########################################################################
;### FUNCTION: _cpct_scanKeyboardFast                                 ###
;########################################################################
;### This function reads the status of keyboard and joysticks and     ###
;### stores it in the 10 bytes reserverd as "keyboardStatusBuffer"    ###
;### It uses an unrolled version of its main loop to gain 147 cycles. ###
;########################################################################
;### INPUTS (-)                                                       ###
;########################################################################
;### OUTPUTS (10B)                                                    ###
;###   -> KeyboardStatusBuffer full with pressed/not pressed info     ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  129 + 40*9 + 36 + 36 = 561 cycles (140,25 us)                   ###
;########################################################################
;### CREDITS:                                                         ###
;###    This fragment of code is based on a scanKeyboard code issued  ###
;### by CPCWiki.                                                      ###
;### http://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning    ###
;########################################################################
;

.globl _cpct_scanKeyboardFast
_cpct_scanKeyboardFast:: 
    
    DI                              ;; [ 4c] Disable interrupts

    LD  HL,  #keyboardStatusBuffer  ;; [10c] HL Points to the start of the keyboardBuffer, where scanned data will be stored

    ;; Configure PPI: Select Register 14 (the one connected with keyboard status) and set it for reading
    ;;
    LD  BC,  #0xF782                ;; [10c] Configure PPI 8255: Set Both Port A and Port C as Output. 
    OUT (C), C                      ;; [12c] 82 = 1000 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=0)=> Port A=Output, 
                                    ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output
    
    LD  BC,  #0xF40E                ;; [10c] Write (0Eh = 14) on PPI 8255 Port A (F4h): the register we want to select on AY-3-8912  
    LD  E,   B                      ;; [ 4c] Save F4h into E to use it later in the loop
    OUT (C), C                      ;; [12c] 
    
    LD  BC,  #0xF6C0                ;; [10c] Write (C0h = 11 000000b) on PPI Port C (F6h): operation > select register 
    LD  D,   B                      ;; [ 4c] Save F6h into D to use it later in the loop
    OUT (C), C                      ;; [12c]
    .DW #0x71ED                     ;; [12c] OUT (C), 0 => Write 0 on PPI's Port C to put PSG's in inactive mode (required in between different operations)

    LD  BC,  #0xF792                ;; [10c] Configure PPI 8255: Set Port A = Input, Port C = Output. 
    OUT (C), C                      ;; [12c] 92h= 1001 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          (B4=1)=> Port A=Input, 
                                    ;;                        (B3=0)=> Port Cu=Output, (B2=0)   => Group B, Mode 0, (B1=1)=> Port B=Input,  (B0=0)=> Port Cl=Output
    
    ;; Read Loop (Unrolled version): We read the 10-bytes that define the pressed/not pressed status
    ;;
    LD  A,   #0x40                  ;; [ 7c] A refers to the next keyboard line to be read (40h to 49h)

    ;; Read line 40h
    LD  B,   D                      ;; [ 4c] B = F6h => Write the value of A to PPI's Port C to select next Matrix Line
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c] B = F4h => Read from PPI's Port A: Pressed/Not Pressed Values from PSG
    INI                             ;; [16c] The read value is written to (HL), then HL<-HL+1 and B<-B-1
    INC A                           ;; [ 4c] Loop: Increment A => Next Matrix Line. 
    
    ;; Read line 41h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]

    ;; Read line 42h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]
    
    ;; Read line 43h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]

    ;; Read line 44h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]
 
    ;; Read line 45h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]

    ;; Read line 46h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]
    
    ;; Read line 47h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]

    ;; Read line 48h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]
    INC A                           ;; [ 4c]
    
    ;; Read line 49h
    LD  B,   D                      ;; [ 4c]
    OUT (C), A                      ;; [12c] 
    LD  B,   E                      ;; [ 4c]
    INI                             ;; [16c]

    ;; Restore PPI status to Port A=Output, Port C=Output
    ;;
    LD  BC,  #0xF782                ;; [10c] Put again PPI in Output/Output mode for Ports A/C.
    OUT (C), C                      ;; [12c]

    EI                              ;; [ 4c] Reenable interrupts
    
    RET                             ;; [10c] Return
;
;########################################################################
;### FUNCTION: _cpct_isKeyPressed                                     ###
;########################################################################
;### Checks if a concrete key is pressed or not. It does it looking   ###
;### at the keyboardStatusBuffer, which is filled up by scan routines.###
;### So, take into account that keyboard has to be scanned before     ###
;### using this routine or it won't work.                             ###
;########################################################################
;### INPUTS (2B)                                                      ###
;###   -> KeyID, which contains Matrix Line(1B) and Bit Mask(1B)      ### 
;########################################################################
;### OUTPUTS (1B)                                                     ###
;###   -> True if the selected key is pressed, False otherwise.       ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURED TIME                                                    ###
;###  109/113 cyles (27,25/28,25 us)                                  ###
;########################################################################
;

.globl _cpct_isKeyPressed
_cpct_isKeyPressed::
   POP  AF                          ;; [10c] Get the return address from stack
   POP  BC                          ;; [10c] Get the KeyID parameter from stack
   PUSH BC                          ;; [11c] Restore stack status
   PUSH AF                          ;; [11c]
  
   LD  A,  B                        ;; [ 4c] Save Key's Bit Mask into A 
   LD  B,  #0                       ;; [ 7c] B = 0 to leave BC with the contents of C (Matrix Line)
   LD  HL, #keyboardStatusBuffer    ;; [10c] Make HL Point to &keyboardStatusBuffer
   ADD HL, BC                       ;; [11c] Make HL Point to &keyboardStatusBuffer + Matrix Line (C)
   AND (HL)                         ;; [ 7c] A = AND operation between Key's Bit Mask (A) and the Matrix Line of the Key (HL)
   LD  H,  B                        ;; [ 4c] H = 0, prepare for returning value in L
   LD  L,  B                        ;; [ 4c] L = 0
   JP NZ,  ikp_returnFalse          ;; [10c] If AND resulted non-zero, Key's bit was 1, what means key was not pressed (return false = 0)
   INC L                            ;; [ 4c] Else, Key's bit was 0, what means key was pressed (return true = 1)
ikp_returnFalse:
   RET                              ;; [10c]
