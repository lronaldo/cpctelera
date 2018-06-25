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

// This example is built at 0x8000, so we can page at 0x4000.
// See the build configuration.
void main(void) {
   u8* pvmem;  // Pointer to video memory
   u8* firstByteInPage = (u8*) 0x4000;
   u8 i;

   cpct_pageMemory(RAMCFG_0 | BANK_0);				// Not needed, sets the memory with the first 64kb accesible, in consecutive banks.
   // firstByteInPage point to address 0x4000. With this memory 
   // configuration, that is physical address 0x4000. 
   *firstByteInPage = cpct_px2byteM1(1, 1, 1, 1);	// Set the first byte in page to all pixels with colour 1 (yellow by default).

   cpct_pageMemory(RAMCFG_4 | BANK_0);				// Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   // RAMCFG_4: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_4, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
   // firstByteInPage point to address 0x4000. With this memory 
   // configuration, 0x4000 is the first byte in BANK_0, RAM_4, 
   // which is physical address 0x10000. 
   *firstByteInPage = cpct_px2byteM1(2, 2, 2, 2);	// Set the first byte in page to all pixels with colour 2 (cyan by default ).

   cpct_pageMemory(RAMCFG_5 | BANK_0);				// Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   // RAMCFG_5: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_5, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
   // firstByteInPage point to address 0x4000. With this memory 
   // configuration, 0x4000 is the first byte in BANK_0, RAM_5, 
   // which is physical address 0x14000. 
   *firstByteInPage = cpct_px2byteM1(3, 3, 3, 3);	// Set the first byte in page to all pixels with colour 3 (red by default ).

   cpct_pageMemory(RAMCFG_6 | BANK_0);				// Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   // RAMCFG_6: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_6, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
   // firstByteInPage point to address 0x4000. With this memory 
   // configuration, 0x4000 is the first byte in BANK_0, RAM_6, 
   // which is physical address 0x18000. 
   *firstByteInPage = cpct_px2byteM1(1, 1, 2, 2);	// Set the first byte in page to all pixels with colours 1, 2 (yellow, cyan by default ).

   cpct_pageMemory(RAMCFG_7 | BANK_0);				// Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   // RAMCFG_7: 0000-3FFF -> RAM_0, 4000-7FFF -> RAM_7, 8000-BFFF -> RAM_2, C000-FFFF -> RAM_3
   // firstByteInPage point to address 0x4000. With this memory 
   // configuration, 0x4000 is the first byte in BANK_0, RAM_7, 
   // which is physical address 0x1C000. 
   *firstByteInPage = cpct_px2byteM1(1, 1, 3, 3);	// Set the first byte in page to all pixels with colours 1, 3 (yellow, cyan by default ).

   cpct_pageMemory(RAMCFG_0 | BANK_0); 				// Set the memory again to default state

   // Clear Screen
   cpct_memset(CPCT_VMEM_START, 0, 0x4000);

   // Let's make visible the values we stored.
   cpct_pageMemory(RAMCFG_0 | BANK_0); // Not needed, sets the memory with the first 64kb accesible, in consecutive banks.
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 0, 0);
   cpct_drawSolidBox(pvmem, *firstByteInPage, 2, 8);
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 4, 0);
   cpct_setDrawCharM1(1, 0);
   cpct_drawStringM1("RAMCFG_0", pvmem);

   cpct_pageMemory(RAMCFG_4 | BANK_0); // Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 0, 16);
   cpct_drawSolidBox(pvmem, *firstByteInPage, 2, 8);
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 4, 16);
   cpct_drawStringM1("RAMCFG_4", pvmem);

   cpct_pageMemory(RAMCFG_5 | BANK_0); // Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 0, 32);
   cpct_drawSolidBox(pvmem, *firstByteInPage, 2, 8);
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 4, 32);
   cpct_drawStringM1("RAMCFG_5", pvmem);

   cpct_pageMemory(RAMCFG_6 | BANK_0); // Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 0, 48);
   cpct_drawSolidBox(pvmem, *firstByteInPage, 2, 8);
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 4, 48);
   cpct_drawStringM1("RAMCFG_6", pvmem);

   cpct_pageMemory(RAMCFG_7 | BANK_0); // Set the 4th page (64kb to 80kb) in 0x4000-0x7FFF
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 0, 64);
   cpct_drawSolidBox(pvmem, *firstByteInPage, 2, 8);
   pvmem = cpct_getScreenPtr(CPCT_VMEM_START, 4, 64);
   cpct_drawStringM1("RAMCFG_7", pvmem);

   cpct_pageMemory(DEFAULT_MEM_CFG); // Equivalent to RAMCFG_0 | BANK_0 

   // Loop forever
   while (1);
}
