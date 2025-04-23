/*
 * Simulator of microcontrollers (d11p0.h)
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


#ifndef D11P0_HEADER
#define D11P0_HEADER


#define TEST	instruction_00
#define IDIV	instruction_02
#define FDIV	instruction_03
#define LSRD	instruction_04
#define ASLD	instruction_05
#define BRN	instruction_21
#define PULxy	instruction_38
#define ABxy	instruction_3a
#define PSHxy	instruction_3c
#define MUL	instruction_3d
#define SUBD16	instruction_83
#define SUBDd	instruction_93
#define SUBDxy	instruction_a3
#define SUBDe	instruction_b3
#define XGDxy	instruction_8f
#define JSRd	instruction_9d
#define ADDD16	instruction_c3
#define ADDDd	instruction_d3
#define ADDDi	instruction_e3
#define ADDDe	instruction_f3
#define LDD16	instruction_cc
#define LDDd	instruction_dc
#define LDDi	instruction_ec
#define LDDe	instruction_fc
#define STOP	instruction_cf
#define STDd	instruction_dd
#define STDi	instruction_ed
#define STDe	instruction_fd
#define BRSETd	instruction_12
#define BRSETi	instruction_1e
#define BRCLRd	instruction_13
#define BRCLRi	instruction_1f
#define BSETd	instruction_14
#define BSETi	instruction_1c
#define BCLRd	instruction_15
#define BCLRi	instruction_1d

#define PAGE18	instruction_18
#define PAGE1A	instruction_1a
#define PAGECD	instruction_cd


#endif

/* End of m6800.src/d11p0.h */
