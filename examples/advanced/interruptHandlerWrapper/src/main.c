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
#include "interrupts.h" // include MyInterruptWrapper declaration

// MAIN FUNCTION
//    Code starts here
//
void main(void) {
   //    Set 'MyInterruptWrapper' as the function that will handle interrupts
   // from now on. As opposed to cpct_setInterruptHandler_allRegisters or
   // cpct_setInterruptHandler, cpct_setInterruptHandler_naked assumes that
   // the function passed as parameter preserves needed registers and reenables
   // interrupts on exit. A function with this behaviour can be easily created
   // using CPCtelera's cpctm_createInterruptHandlerWrapper.
   cpct_setInterruptHandler_naked(MyInterruptWrapper);

   // a. Set Mode 1 Character&String drawing functions to use colours
   //     1 (yellow) for foreground and 0 (blue) for background
   // b. Draw a string at coordinates (6, 96) in bytes, (24, 96) in pixels,
   //     as each byte is 4 pixels wide in mode 1
   cpct_setDrawCharM1(1, 0);
   { 
      u8 * const location = cpctm_screenPtr(CPCT_VMEM_START, 6, 96);
      cpct_drawStringM1("Interrupt Handler Wrapper Example", location);
   }

   // Loop forever. This code does nothing. However, interrupts happen and they
   // automatically call MyInterruptWrapper, which changes border color.
   while (1);
}
