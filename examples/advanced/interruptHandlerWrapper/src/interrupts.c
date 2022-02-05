//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2022 Nestornillo (https://github.com/nestornillo)
//  Copyright (C) 2022 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <cpctelera.h>

// INTERRUPT HANDLER
//    This code will be called each time an interrupt occurs.
//    To let us see when do this happen visually, this code changes the color
// of the border each time it is called.
//
void myInterruptHandler() {
   static u8 i;            // Static variable to be preserved from call to call
   cpct_setBorder(i+1);    // Set the color of the border differently for each interrupt  
   if (++i > 5) i=0;       // Count one more interrupt. There are 6 interrupts in total (0-5)
}

// INTERRUPT HANDLER WRAPPER
//    Next line will create a safety wrapper function named 'MyInterruptWrapper'
// that will call to the interrupt handler function 'myInterruptHandler',
// preserving the contents of all standard registers (AF, BC, DE, HL, IX, IY)
// and also preserving alternate registers AF' and HL'. Notice that for this
// specific example it is not needed to preserve any alternate register,
// only shown here for demonstration purposes.
//
cpctm_createInterruptHandlerWrapper(MyInterruptWrapper, myInterruptHandler, af, bc, de, hl, ix, iy, alt, af, hl);

//
//    First parameter of the macro is the name that will be given to the created
// wrapper function. The second parameter corresponds to the interrupt handler
// function which will be called inside the created function. And next parameters
// (from 0 to 11 parameters) determine which registers will be saved before
// calling the interrupt handler function (and restored after that). For exaple:
//
// cpctm_createInterruptHandlerWrapper(miniWrapper, myInterruptHandler, af, alt, hl);
//
//    Previous line will create an interrupt handler wrapper function named
// 'miniWrapper' that saves in the stack AF and HL', then calls to
// 'myInterruptHandler', and after that restores AF and HL', reenables interrupts,
// and exits safely from the interrupt.
//    Take into account that in most cases you will need to preserve at least
// the standard registers when calling to interrupt handlers.
//    Interrupt handler wrapper functions created with this macro can be set as
// the system interrupt handler using cpct_setInterruptHandler_naked.
//
