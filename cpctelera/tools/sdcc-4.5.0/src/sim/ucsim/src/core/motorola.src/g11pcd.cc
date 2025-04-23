/*
 * Simulator of microcontrollers (g11pcd.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include <stdio.h>

#include "g11pcd.h"

// code, mask, branch, len, mnem, iscall, ticks
struct dis_entry disass11pcd[]= {

  { 0xa3, 0xff, ' ', 3, "CPD %X" },  
  { 0xad, 0xff, ' ', 3, "CPD %X" },
  { 0xee, 0xff, ' ', 2, "LDX %x" },
  { 0xef, 0xff, ' ', 2, "STX %x" },

  { 0, 0, 0, 0, 0, 0 }
};

/* End of motorola.src/g11pcd.cc */
