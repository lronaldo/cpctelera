/*
 * Simulator of microcontrollers (gpedm3a.cc)
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

#include "gpedm3a.h"

struct dis_entry disass_pedm3a[]=
  {
    { 0x66, 0xff, ' ', 2, "PUSH SU" },
    { 0x6e, 0xff, ' ', 2, "POP SU" },
    { 0x6f, 0xff, ' ', 2, "SETUSR" },
    { 0x7d, 0xff, ' ', 2, "SURES" },
    { 0x7f, 0xff, ' ', 2, "RDMODE" },
    { 0x75, 0xff, ' ', 2, "SYSCALL" },
    { 0x98, 0xff, ' ', 2, "LDDSR" },
    { 0x90, 0xff, ' ', 2, "LDISR" },
    { 0xd8, 0xff, ' ', 2, "LSDDR" },
    { 0xd0, 0xff, ' ', 2, "LSIDR" },
    { 0xf8, 0xff, ' ', 2, "LSDR" },
    { 0xf0, 0xff, ' ', 2, "LSIR" },
    { 0xc0, 0xff, ' ', 2, "UMA" },
    { 0xc8, 0xff, ' ', 2, "UMS" },

    { 0, 0, 0, 0, 0, 0, 0 }
  };
  
/* End of rxk.src/gpedm3a.cc */
