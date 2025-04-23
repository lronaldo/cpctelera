/*
 * Simulator of microcontrollers (decc02s.h)
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

#ifndef DECC02S_HEADER
#define DECC02S_HEADER


#define WAI	instruction_cb
#define STP	instruction_db

#define RMB0	instruction_07
#define RMB1	instruction_17
#define RMB2	instruction_27
#define RMB3	instruction_37
#define RMB4	instruction_47
#define RMB5	instruction_57
#define RMB6	instruction_67
#define RMB7	instruction_77

#define SMB0	instruction_87
#define SMB1	instruction_97
#define SMB2	instruction_a7
#define SMB3	instruction_b7
#define SMB4	instruction_c7
#define SMB5	instruction_d7
#define SMB6	instruction_e7
#define SMB7	instruction_f7

#define BBR0	instruction_0f
#define BBR1	instruction_1f
#define BBR2	instruction_2f
#define BBR3	instruction_3f
#define BBR4	instruction_4f
#define BBR5	instruction_5f
#define BBR6	instruction_6f
#define BBR7	instruction_7f

#define BBS0	instruction_8f
#define BBS1	instruction_9f
#define BBS2	instruction_af
#define BBS3	instruction_bf
#define BBS4	instruction_cf
#define BBS5	instruction_df
#define BBS6	instruction_ef
#define BBS7	instruction_ff


#endif

/* End of mos6502.src/decc02s.h */
