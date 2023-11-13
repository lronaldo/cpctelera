/*
 * Simulator of microcontrollers (inst_fd.cc)
 *  FD escaped multi-byte opcodes.
 *
 *
 * Copyright (C) 1999,2001 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
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

//#include "ddconfig.h"

// local
#include "z80cl.h"
//#include "regsz80.h"
#include "z80mac.h"

#define regs_iX_h regs.iy.h
#define regs_iX_l regs.iy.l
#define regs_IX_OR_IY regs.IY
#define inst_Xd_ld inst_fd_ld
#define inst_Xd_add inst_fd_add
#define inst_Xd_push inst_fd_push
#define inst_Xd_inc inst_fd_inc
#define inst_Xd_dec inst_fd_dec
#define inst_Xd_misc inst_fd_misc
#define inst_Xd inst_fd
#define inst_Xdcb inst_fdcb

#define inst_Xfix  0xFD

#include "inst_xd.cc"

/* End of z80.src/inst_fd.cc */
