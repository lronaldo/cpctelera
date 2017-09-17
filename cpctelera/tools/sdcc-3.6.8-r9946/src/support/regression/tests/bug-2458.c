/*
   bug-2458.c

   Test suite for bug 2458.
   Copyright 2016 Peter Dons Tychsen (pdt at dontech dot dk)

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
*/

#include <testfwk.h>

static unsigned long testLong = 0x40000;
static unsigned char testResult;
static unsigned char testShift = 0x12;

void testBug (void)
{
  testResult = ((((unsigned long)1) << testShift) & testLong) && 1; 
  ASSERT(testResult); 
  testResult = (0x1000 + testLong) && 1;
  ASSERT(testResult);
}

