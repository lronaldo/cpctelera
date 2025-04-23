/*
 * Simulator of microcontrollers (dpedm3a.h)
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

#ifndef DPEDM3A_HEADER
#define DPEDM3A_HEADER

#define PUSH_SU		instruction_ed_66
#define POP_SU		instruction_ed_6e
#define SETUSR		instruction_ed_6f
#define SURES		instruction_ed_7d
#define RDMODE		instruction_ed_7f
#define SYSCALL		instruction_ed_75
#define LDDSR		instruction_ed_98
#define LDISR		instruction_ed_90
#define LSDDR		instruction_ed_d8
#define LSIDR		instruction_ed_d0
#define LSDR		instruction_ed_f8
#define LSIR		instruction_ed_f0
#define UMA		instruction_ed_c0
#define UMS		instruction_ed_c8

#endif

/* End of rxk.src/dpedm3a.h */
