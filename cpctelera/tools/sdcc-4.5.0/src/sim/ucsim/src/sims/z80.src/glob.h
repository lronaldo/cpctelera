/*
 * Simulator of microcontrollers (glob.h)
 *
 * Copyright (C) 1999 Drotos Daniel
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

extern struct dis_entry disass_z80[];

extern struct dis_entry disass_z80n_ed[];
extern struct dis_entry disass_z80_ed[];
extern struct dis_entry disass_z80_cb[];
extern struct dis_entry disass_z80_dd[];
extern struct dis_entry disass_z80_fd[];
extern struct dis_entry disass_z80_ddcb[];
extern struct dis_entry disass_z80_fdcb[];

extern struct dis_entry disass_gb80[];
extern struct dis_entry disass_gb80_cb[];

extern u16_t z80_ttab_00[256];
extern u16_t z80_ttab_dd[256];
extern u16_t z80_ttab_cb[256];
extern u16_t z80_ttab_ed[256];
extern u16_t z80_ttab_fd[256];
extern u16_t z80_ttab_ddcb[256];
extern u16_t z80_ttab_fdcb[256];

extern u16_t gb_ttab_00[256];
extern u16_t gb_ttab_cb[256];

extern u16_t r800_ttab_00[256];
extern u16_t r800_ttab_dd[256];
extern u16_t r800_ttab_cb[256];
extern u16_t r800_ttab_ed[256];
extern u16_t r800_ttab_fd[256];
extern u16_t r800_ttab_ddcb[256];
extern u16_t r800_ttab_fdcb[256];

extern struct cpu_entry cpus_z80[];

#endif

/* End of z80.src/glob.h */
