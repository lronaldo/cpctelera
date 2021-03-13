/*
   bug-3178.c
   a bug in peephole optimizer helper function that affects empty functions
   with function arguments when compiled with#
   --opt-code-size --reserve-regs-iy and targeting z80 or z180.
 */
 
#include <testfwk.h>

#pragma opt_code_size
#pragma disable_warning 85

static void L (char c) { } // Bug messed up stack frame here.

void
testBug (void)
{
    L('L');
    ASSERT (1);
}

