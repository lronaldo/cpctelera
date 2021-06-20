//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

// Offset from the start of a character row to the next in video memory
#define ROW_OFFSET 0x50

//
// Interrupt Handler 
//    This code will be called each time an interrupt occurs.
//    To let us see when do this happen visually, this code changes the color
// of the border each time it is called.
//
void myInterruptHandler() {
   static u8 i;   // Static variable to be preserved from call to call

   // Set the color of the border differently for each interrupt
   cpct_setBorder(i+1);
   
   // Count one more interrupt. There are 6 interrupts in total (0-5)
   if (++i > 5) i=0;
}

//
// Print some messages on the screen about this example
//
void printMessages() {
   u8* pvm = CPCT_VMEM_START;
   cpct_setDrawCharM1(0, 3);
   cpct_drawStringM1("Interrupt Handler Example", pvm);

   pvm += 3 * ROW_OFFSET;
   cpct_setDrawCharM1(1, 0);
   cpct_drawStringM1("This example is running a void loop, but", pvm);
   pvm += ROW_OFFSET;
   cpct_drawStringM1("border color is changed 6 times each", pvm);
   pvm += ROW_OFFSET;
   cpct_drawStringM1("frame. This change  is done by the inte-", pvm);
   pvm += ROW_OFFSET;
   cpct_drawStringM1("rrupt handler. As z80 produces 300 inte-", pvm);
   pvm += ROW_OFFSET;
   cpct_drawStringM1("rrupts per second, you have 6 per frame.", pvm);
}

//
// MAIN FUNCTION
//    Code starts here
//
void main(void) {
   // We disable firmware and then set the function myInterruptHandler to be 
   // called on each interrupt. Disabling firmware is not actually necessary if it 
   // is not going to be reenabled. 
   cpct_disableFirmware();
   cpct_setInterruptHandler( myInterruptHandler );

   // Print some messages about this example
   printMessages();

   // Loop forever. This code does nothing. However, interrupts happen and they
   // automatically call myInterruptHandler, which changes border color.
   while (1);
}
