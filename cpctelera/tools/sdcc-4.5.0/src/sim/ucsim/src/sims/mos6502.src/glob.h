/*
 * Simulator of microcontrollers (glob.h)
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

#ifndef GLOB_HEADER
#define GLOB_HEADER

#include "stypes.h"
#include "iwrap.h"


extern instruction_wrapper_fn itab[256];

extern struct dis_entry disass_mos6502[];
extern struct dis_entry disass_mos65c02[];
extern struct dis_entry disass_mos65c02s[];

extern struct cpu_entry cpus_6502[];

#endif

/* End of mos6502.src/glob.h */
