;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine
;;  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;  Copyright (C) 2021 Nestornillo (https://github.com/nestornillo)
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


;;//////////////////////////////////////////////////////////////////////
;;//////////////////////////////////////////////////////////////////////
;; File: Macros (asm)
;;//////////////////////////////////////////////////////////////////////
;;//////////////////////////////////////////////////////////////////////

;;//////////////////////////////////////////////////////////////////////
;; Group: Firmware Macros
;;//////////////////////////////////////////////////////////////////////

;;
;; Macro: cpctm_createInterruptHandlerWrapper_asm
;;
;;    Macro that creates a custom interrupt handler wrapper function. This created
;; function works in the same way as <cpct_setInterruptHandler>, but saves the
;; registers chosen in the macro calling.
;;
;; ASM Definition:
;; .macro <cpctm_createInterruptHandlerWrapper_asm> *WRAPPERNAME*, *AF*, *BC*,
;; *DE*, *HL*, *IX*, *IY*, *AF*', *BC*', *DE*', *HL*'
;;
;; Parameters:
;; WRAPPERNAME - String (without spaces, not empty) that will be added as a suffix
;; in the name of the resulting function.
;; AF  - *1* = Save AF register. *0* = Don't save AF register.
;; BC  - *1* = Save BC register. *0* = Don't save BC register.
;; DE  - *1* = Save DE register. *0* = Don't save DE register.
;; HL  - *1* = Save HL register. *0* = Don't save HL register.
;; IX  - *1* = Save IX register. *0* = Don't save IX register.
;; IY  - *1* = Save IY register. *0* = Don't save IY register.
;; AF' - *1* = Save AF' register. *0* = Don't save AF' register.
;; BC' - *1* = Save BC' register. *0* = Don't save BC' register.
;; DE' - *1* = Save DE' register. *0* = Don't save DE' register.
;; HL' - *1* = Save HL' register. *0* = Don't save HL' register.
;;
;; Details:
;;   This macro generates a function that modifies the interrupt vector, to establish
;; a new interrupt handler that calls user provided function. The user will have
;; to provide to this generated code a pointer to the function (*intHandler*)
;; that will handle interrupts. This generated function does not call *intHandler*
;; or check it in any way. It just sets it for being called at the next interrupts.
;; Provided *intHandler* function must not return any value nor accept any parameter.
;;
;;   The generated function creates wrapper code to safely call user provided
;; *intHandler*. This code saves on the stack the registers that are set to '1'
;; in the parameters of the macro. The selected registers are restored after
;; user code from *intHandler* finishes. Therefore, user does not have to worry
;; about saving and restoring registers during interrupts. However, when using
;; this macro, user is responsible for choosing which registers are saved. Be
;; aware that in most cases all standard registers (AF BC DE HL IX IY) should be
;; preserved.
;;
;;   This macro needs eleven parameters as input for its calling. First parameter
;; will be added in the name of the generated function. For example, if the string
;; *SomeRegisters* is used as first parameter, the resulting function will be named
;; *cpct_setInterruptHandler_SomeRegisters_asm* (and *cpct_setInterruptHandler_SomeRegisters*
;; for C calling).
;;
;;   Next ten parameters must be either '1' or '0', and will determine which
;; registers are preserved in the generated wrapper code. Those ten parameters
;; refer to registers AF, BC, DE, HL, IX, IY, AF', BC', DE', and HL' (in that
;; order).
;;
;; The resulting code of this macro works in the same way as *cpct_setInterruptHandler_asm*,
;; but the preserved registers are the ones chosen in the macro parameters.
;;
;; Examples of use:
;;  Let's see some examples of this macro.
;; * Saving standard registers
;; (start code)
;; cpctm_createInterruptHandlerWrapper_asm StandardRegs 1 1 1 1 1 1 0 0 0 0
;; (end code)
;; Previous line will create a function named *cpct_setInterruptHandler_StandardRegs_asm*
;; (and *cpct_setInterruptHandler_StandardRegs* for C calling) which will save
;; standard registers (AF, BC, DE, HL, IX, IY) before calling the interrupt handler.
;; In that case the code generated is the same as in *cpct_setInterruptHandler_asm*.
;; Then, in order to use the generated routine, we can use it in the same way as
;; *cpct_setInterruptHandler_asm*.
;; (start code)
;; ld hl,#handlerFunction  ;; where 'handlerFunction' is our interrupt handler function
;; call cpct_setInterruptHandler_StandardRegs_asm
;; (end code)
;; If we want to use our assembler generated function in a C file, first we'll
;; need to declare it using <cpctm_declareInterruptHandlerWrapper> macro, as shown
;; in next example.
;; (start code)
;; cpctm_declareInterruptHandlerWrapper(StandardRegs);
;; // ...
;; void init(void) {
;;  // ...
;;  cpct_setInterruptHandler_StandardRegs(handlerFunction);
;; }
;; (end code)
;; * Saving all registers
;;
;; Next line will create *cpct_setInterruptHandler_AllRegisters* (for C calling) and
;; *cpct_setInterruptHandler_AllRegisters_asm* (for ASM calling). This created function
;; will preserve all standard and alternate registers in the interrupt handler calling.
;; (start code)
;; cpctm_createInterruptHandlerWrapper_asm AllRegisters 1 1 1 1 1 1 1 1 1 1
;; (end code)
;; * Saving no registers
;;
;; If you don't need to preserve any register you can use the next calling. Be
;; aware that in most cases you will need to preserve at least the standard registers.
;; (start code)
;; cpctm_createInterruptHandlerWrapper_asm NoRegisters 0 0 0 0 0 0 0 0 0 0
;; (end code)
;; * Saving standard registers + AF' + HL'
;;
;; As another example, next line creates a custom interrupt handler wrapper that
;; preserves standard registers (AF BC DE HL IX IY) and also preserves AF' and HL'.
;; (start code)
;; cpctm_createInterruptHandlerWrapper_asm MyRegisters 1 1 1 1 1 1 1 0 0 1
;; (end code)
;;
;; Known issues:
;;   * This macro can only be used from assembler code. It is not accessible from
;; C scope. For C programs, please refer to <cpctm_createInterruptHandlerWrapper> C macro.
;; However, the resulting code of this macro can be used from a C scope.
;;   * This macro will work *only* with constant values, as its value needs to
;; be calculated in compilation time. If fed with variable values, it will give
;; an assembler error.
;;
;; Size of generated code:
;;    Depending on which registers you choose to preserve the resulting code will
;; vary its size. Next are some values as a reference.
;; (start code)
;; Case                     | Size of resulting code
;; --------------------------------------------------------------------------------------------------
;; No registers saved       | 24 bytes (17 bytes function + 7 bytes for safe interrupt wrapper code)
;; Standard registers saved | 40 bytes (17 bytes function + 23 bytes for safe interrupt wrapper code)
;; All registers saved      | 52 bytes (17 bytes function + 35 bytes for safe interrupt wrapper code)
;; --------------------------------------------------------------------------------------------------
;; (end code)
;;
;; Time Measures:
;; * This first measure is the time required for the generated code to
;; execute: that is, the time for setting up the hook for the system to call
;; user defined code.
;; (start code)
;; Case | microSecs (us) | CPU Cycles
;; -----------------------------------
;; Any  |      24        |    96
;; -----------------------------------
;; (end code)
;;  * This second measure is the time overhead required for safely calling
;; user defined function. That is, time required by the generated wrapper code
;; to save registers, call user's *intHandler*, restoring the registers and
;; returning. This overhead is to be assumed each time interrupt handler is
;; called, so up to 6 times per frame, 300 times per second. Depending on which
;; registers you choose to be saved, this part of the resulting function will
;; take different time to complete. Next measures show some possible cases.
;; (start code)
;; Case                     | microSecs (us) | CPU Cycles
;; -------------------------------------------------------
;; No registers saved       |      11        |     44
;; Standard registers saved |      57        |    228
;; All registers saved      |      89        |    356
;; -------------------------------------------------------
;; (end code)
;;

.macro cpctm_createInterruptHandlerWrapper_asm WrapperName, ?regAF, ?regBC, ?regDE, ?regHL, ?regIX, ?regIY, ?altRegAF, ?altRegBC, ?altRegDE, ?altRegHL
_cpct_setInterruptHandler_'WrapperName'::
cpct_setInterruptHandler_'WrapperName'_asm::
     ld (cpct_safeInterruptHandlerCall_'WrapperName'+1), hl ;; [5] Place the pointer to the user code just after the call instruction
     ;; Modify interrupt vector to call safe interrupt handler hook (code below)
     ld  a, #0xC3                                           ;; [2] 0xC3 = JP instruction, that may have been removed by other functions
     di                                                     ;; [1] Disable interrupts
     ld (#firmware_RST_jp), a                               ;; [4] Add JP at the start of interrupt vector's code
     ld hl, #cpct_safeInterruptHandlerHook_'WrapperName'    ;; [3] HL = pointer to safe interrupt handler hook
     ld (#firmware_RST_jp+1), hl                            ;; [5] Place interrupt handler hook pointer after JP in the interrupt vector's code
     ei                                                     ;; [1] Reenable interrupts
     ret                                                    ;; [3] Return
     ;; Generate interrupt Handler Safe Wrapper Code.
     cpct_safeInterruptHandlerHook_'WrapperName'::
     di        ;; [1] Disable interrupts
     ;; Save selected standard registers (AF BC DE HL IX IY)
     .if regAF
     push af   ;; [4]
     .endif
     .if regBC
     push bc   ;; [4]
     .endif
     .if regDE
     push de   ;; [4]
     .endif
     .if regHL
     push hl   ;; [4]
     .endif
     .if regIX
     push ix   ;; [5]
     .endif
     .if regIY
     push iy   ;; [5]
     .endif
     ;; If AF' is selected, swap AF/AF' and save AF
     .if altRegAF
     ex af,af' ;; [1]
     push af   ;; [4]
     .endif
      ;; If BC' or DE' or HL' are selected, swap BC_DE_HL/BC'_DE'_HL'
     .if altRegBC+altRegDE+altRegHL
     exx       ;; [1]
     .endif
     ;; Save selected alternate registers (BC' DE' HL')
     .if altRegBC
     push bc   ;; [4]
     .endif
     .if altRegDE
     push de   ;; [4]
     .endif
     .if altRegHL
     push hl   ;; [4]
     .endif
     ;; Safe handler call
     cpct_safeInterruptHandlerCall_'WrapperName':
     call #0x0000 ;; [5] Call Interrupt Handler
     ;; Restore selected alternate registers (BC' DE' HL')
     .if altRegHL
     pop hl    ;; [3]
     .endif
     .if altRegDE
     pop de    ;; [3]
     .endif
     .if altRegBC
     pop bc    ;; [3]
     .endif
     ;; If BC' or DE' or HL' are selected, swap BC_DE_HL/BC'_DE'_HL'
     .if altRegBC+altRegDE+altRegHL
     exx       ;; [1]
     .endif
     ;; If AF' is selected, restore AF and swap AF/AF'
     .if altRegAF
     pop af    ;; [3]
     ex af,af' ;; [1]
     .endif
     ;; Restore selected standard registers (AF BC DE HL IX IY)
     .if regIY
     pop iy    ;; [4]
     .endif
     .if regIX
     pop ix    ;; [4]
     .endif
     .if regHL
     pop hl    ;; [3]
     .endif
     .if regDE
     pop de    ;; [3]
     .endif
     .if regBC
     pop bc    ;; [3]
     .endif
     .if regAF
     pop af    ;; [3]
     .endif
     ei        ;; [1] Reenable interrupts
     reti      ;; [4] Return to main program

.endm
