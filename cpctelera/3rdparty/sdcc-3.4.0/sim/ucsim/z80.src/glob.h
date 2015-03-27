/*
 * Simulator of microcontrollers (glob.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

extern struct dis_entry disass_z80_ed[];
extern struct dis_entry disass_z80_cb[];
extern struct dis_entry disass_z80_dd[];
extern struct dis_entry disass_z80_fd[];
extern struct dis_entry disass_z80_ddcb[];
extern struct dis_entry disass_z80_fdcb[];


extern struct dis_entry disass_r2k[];

extern struct dis_entry disass_r2k_ed[];
extern struct dis_entry disass_r2k_cb[];
extern struct dis_entry disass_r2k_dd[];
extern struct dis_entry disass_r2k_fd[];
extern struct dis_entry disass_r2k_ddcb[];
extern struct dis_entry disass_r2k_fdcb[];

extern struct dis_entry disass_lr35902[];
extern struct dis_entry disass_lr35902_cb[];

#endif

/* End of z80.src/glob.h */
