/*
 * Simulator of microcontrollers (uc320.cc)
 *
 * Copyright (C) 2018,18 whitequark
 *
 * To contact author send email to whitequark@whitequark.org
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

//#include <stdio.h>
//#include <stdlib.h>
//#include <ctype.h>
//#include "i_string.h"

//#include "glob.h"
#include "uc320cl.h"
#include "regs51.h"

struct timing_desc
{
  u8_t  opc_a;
  u8_t  opc_b;
  bool  mask;
  int   cycles;
};

static struct timing_desc uc320_timing_desc[] = {
  { 0x00, 0x00, false, 1 }, /*inst_nop*/
  { 0x1f, 0x01,  true, 3 }, /*inst_ajmp_addr*/
  { 0x02, 0x02, false, 4 }, /*inst_ljmp*/
  { 0x03, 0x03, false, 1 }, /*inst_rr*/
  { 0x04, 0x04, false, 1 }, /*inst_inc_a*/
  { 0x05, 0x05, false, 2 }, /*inst_inc_addr*/
  { 0x06, 0x07, false, 1 }, /*inst_inc_Sri*/
  { 0x08, 0x0f, false, 1 }, /*inst_inc_rn*/
  { 0x10, 0x10, false, 4 }, /*inst_jbc_bit_addr*/
  { 0x1f, 0x11,  true, 3 }, /*inst_acall_addr*/
  { 0x12, 0x12, false, 4 }, /*inst_lcall*/
  { 0x13, 0x13, false, 1 }, /*inst_rrc*/
  { 0x14, 0x14, false, 1 }, /*inst_dec_a*/
  { 0x15, 0x15, false, 2 }, /*inst_dec_addr*/
  { 0x16, 0x17, false, 1 }, /*inst_dec_Sri*/
  { 0x18, 0x1f, false, 1 }, /*inst_dec_rn*/
  { 0x20, 0x20, false, 4 }, /*inst_jb_bit_addr*/
  { 0x22, 0x22, false, 4 }, /*inst_ret*/
  { 0x23, 0x23, false, 1 }, /*inst_rl*/
  { 0x24, 0x24, false, 2 }, /*inst_add_a_Sdata*/
  { 0x25, 0x25, false, 2 }, /*inst_add_a_addr*/
  { 0x26, 0x27, false, 1 }, /*inst_add_a_Sri*/
  { 0x28, 0x2f, false, 1 }, /*inst_add_a_rn*/
  { 0x30, 0x30, false, 4 }, /*inst_jnb_bit_addr*/
  { 0x32, 0x32, false, 4 }, /*inst_reti*/
  { 0x33, 0x33, false, 1 }, /*inst_rlc*/
  { 0x34, 0x34, false, 2 }, /*inst_addc_a_Sdata*/
  { 0x35, 0x35, false, 2 }, /*inst_addc_a_addr*/
  { 0x36, 0x37, false, 1 }, /*inst_addc_a_Sri*/
  { 0x38, 0x3f, false, 1 }, /*inst_addc_a_rn*/
  { 0x40, 0x40, false, 3 }, /*inst_jc_addr*/
  { 0x42, 0x42, false, 2 }, /*inst_orl_addr_a*/
  { 0x43, 0x43, false, 3 }, /*inst_orl_addr_Sdata*/
  { 0x44, 0x44, false, 2 }, /*inst_orl_a_Sdata*/
  { 0x45, 0x45, false, 2 }, /*inst_orl_a_addr*/
  { 0x46, 0x47, false, 1 }, /*inst_orl_a_Sri*/
  { 0x48, 0x4f, false, 1 }, /*inst_orl_a_rn*/
  { 0x50, 0x50, false, 3 }, /*inst_jnc_addr*/
  { 0x52, 0x52, false, 2 }, /*inst_anl_addr_a*/
  { 0x53, 0x53, false, 3 }, /*inst_anl_addr_Sdata*/
  { 0x54, 0x54, false, 2 }, /*inst_anl_a_Sdata*/
  { 0x55, 0x55, false, 2 }, /*inst_anl_a_addr*/
  { 0x56, 0x57, false, 1 }, /*inst_anl_a_Sri*/
  { 0x58, 0x5f, false, 1 }, /*inst_anl_a_rn*/
  { 0x60, 0x60, false, 3 }, /*inst_jz_addr*/
  { 0x62, 0x62, false, 2 }, /*inst_xrl_addr_a*/
  { 0x63, 0x63, false, 3 }, /*inst_xrl_addr_Sdata*/
  { 0x64, 0x64, false, 2 }, /*inst_xrl_a_Sdata*/
  { 0x65, 0x65, false, 2 }, /*inst_xrl_a_addr*/
  { 0x66, 0x67, false, 1 }, /*inst_xrl_a_Sri*/
  { 0x68, 0x6f, false, 1 }, /*inst_xrl_a_rn*/
  { 0x70, 0x70, false, 3 }, /*inst_jnz_addr*/
  { 0x72, 0x72, false, 2 }, /*inst_orl_c_bit*/
  { 0x73, 0x73, false, 3 }, /*inst_jmp_Sa_dptr*/
  { 0x74, 0x74, false, 2 }, /*inst_mov_a_Sdata*/
  { 0x75, 0x75, false, 3 }, /*inst_mov_addr_Sdata*/
  { 0x76, 0x77, false, 2 }, /*inst_mov_Sri_Sdata*/
  { 0x78, 0x7f, false, 2 }, /*inst_mov_rn_Sdata*/
  { 0x80, 0x80, false, 3 }, /*inst_sjmp*/
  { 0x82, 0x82, false, 2 }, /*inst_anl_c_bit*/
  { 0x83, 0x83, false, 3 }, /*inst_movc_a_Sa_pc*/
  { 0x84, 0x84, false, 5 }, /*inst_div_ab*/
  { 0x85, 0x85, false, 3 }, /*inst_mov_addr_addr*/
  { 0x86, 0x87, false, 2 }, /*inst_mov_addr_Sri*/
  { 0x88, 0x8f, false, 2 }, /*inst_mov_addr_rn*/
  { 0x90, 0x90, false, 3 }, /*inst_mov_dptr_Sdata*/
  { 0x92, 0x92, false, 2 }, /*inst_mov_bit_c*/
  { 0x93, 0x93, false, 3 }, /*inst_movc_a_Sa_dptr*/
  { 0x94, 0x94, false, 2 }, /*inst_subb_a_Sdata*/
  { 0x95, 0x95, false, 2 }, /*inst_subb_a_addr*/
  { 0x96, 0x97, false, 1 }, /*inst_subb_a_Sri*/
  { 0x98, 0x9f, false, 1 }, /*inst_subb_a_rn*/
  { 0xa0, 0xa0, false, 2 }, /*inst_orl_c_Sbit*/
  { 0xa2, 0xa2, false, 2 }, /*inst_mov_c_bit*/
  { 0xa3, 0xa3, false, 3 }, /*inst_inc_dptr*/
  { 0xa4, 0xa4, false, 5 }, /*inst_mul_ab*/
  { 0xa6, 0xa7, false, 2 }, /*inst_mov_Sri_addr*/
  { 0xa8, 0xaf, false, 2 }, /*inst_mov_rn_addr*/
  { 0xb0, 0xb0, false, 2 }, /*inst_anl_c_Sbit*/
  { 0xb2, 0xb2, false, 2 }, /*inst_cpl_bit*/
  { 0xb3, 0xb3, false, 1 }, /*inst_cpl_c*/
  { 0xb4, 0xb4, false, 4 }, /*inst_cjne_a_Sdata_addr*/
  { 0xb5, 0xb5, false, 4 }, /*inst_cjne_a_addr_addr*/
  { 0xb6, 0xb7, false, 4 }, /*inst_cjne_Sri_Sdata_addr*/
  { 0xb8, 0xbf, false, 4 }, /*inst_cjne_rn_Sdata_addr*/
  { 0xc0, 0xc0, false, 2 }, /*inst_push*/
  { 0xc2, 0xc2, false, 2 }, /*inst_clr_bit*/
  { 0xc3, 0xc3, false, 1 }, /*inst_clr_c*/
  { 0xc4, 0xc4, false, 1 }, /*inst_swap*/
  { 0xc5, 0xc5, false, 2 }, /*inst_xch_a_addr*/
  { 0xc6, 0xc7, false, 1 }, /*inst_xch_a_Sri*/
  { 0xc8, 0xcf, false, 1 }, /*inst_xch_a_rn*/
  { 0xd0, 0xd0, false, 2 }, /*inst_pop*/
  { 0xd2, 0xd2, false, 2 }, /*inst_setb_bit*/
  { 0xd3, 0xd3, false, 1 }, /*inst_setb_c*/
  { 0xd4, 0xd4, false, 1 }, /*inst_da_a*/
  { 0xd5, 0xd5, false, 4 }, /*inst_djnz_addr_addr*/
  { 0xd6, 0xd7, false, 1 }, /*inst_xchd_a_Sri*/
  { 0xd8, 0xdf, false, 3 }, /*inst_djnz_rn_addr*/
  { 0xe0, 0xe0, false, 2 }, /*inst_movx_a_Sdptr*/
  { 0xe2, 0xe3, false, 2 }, /*inst_movx_a_Sri*/
  { 0xe4, 0xe4, false, 1 }, /*inst_clr_a*/
  { 0xe5, 0xe5, false, 2 }, /*inst_mov_a_addr*/
  { 0xe6, 0xe7, false, 1 }, /*inst_mov_a_Sri*/
  { 0xe8, 0xef, false, 1 }, /*inst_mov_a_rn*/
  { 0xf0, 0xf0, false, 2 }, /*inst_movx_Sdptr_a*/
  { 0xf2, 0xf3, false, 2 }, /*inst_movx_Sri_a*/
  { 0xf4, 0xf4, false, 1 }, /*inst_cpl_a*/
  { 0xf5, 0xf5, false, 2 }, /*inst_mov_addr_a*/
  { 0xf6, 0xf7, false, 1 }, /*inst_mov_Sri_a*/
  { 0xf8, 0xff, false, 1 }, /*inst_mov_rn_a*/
};

/*
 * Unpacking the timing table for fast indexing
 */

static int uc320_timing[0x100];

static void unpack_timing()
{
  static bool unpacked;
  size_t i;

  if (unpacked)
    return;

  for (i = 0; i < sizeof(uc320_timing_desc) / sizeof(uc320_timing_desc[0]); i++)
    {
      struct timing_desc *td = &uc320_timing_desc[i];
      int opc;

      for (opc = 0; opc < 0x100; opc++)
        {
          if ((td->mask && ((opc & td->opc_a) == td->opc_b)) ||
              (!td->mask && opc >= td->opc_a && opc <= td->opc_b))
            uc320_timing[opc] = td->cycles;
        }
    }

  unpacked = true;
}

/*
 * Making an 320 CPU object
 */

cl_uc320::cl_uc320 (struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc521 (Itype, asim)
{
  unpack_timing();
}

/*
 * Setting up SFR area to reset value
 */

void
cl_uc320::clear_sfr(void)
{
  cl_uc521::clear_sfr();
  sfr->write(CKCON, 0x01);
}

/*
 * Execution
 */

int
cl_uc320::exec_inst(void)
{
  int res;
  t_mem code;

  instPC= PC;
  pending_ticks= 0;
  code= rom->read(PC);
  res= cl_uc521::exec_inst();

  if (res != resNOT_DONE)
    tick(uc320_timing[code] - pending_ticks);

  return(res);
}

int
cl_uc320::tick(int cycles)
{
  pending_ticks += 1;
  return(cl_uc521::tick(cycles));
}

int
cl_uc320::tick_hw(int cycles)
{
  return(cl_uc521::tick_hw(cycles*3));
}

/*
 * 0xe0 1 24 MOVX A,@DPTR
 *____________________________________________________________________________
 *
 */

int
cl_uc320::instruction_e0/*inst_movx_a_Sdptr*/(t_mem/*uchar*/ code)
{
  int res= cl_uc521::instruction_e0(code);

  u8_t stretch= sfr->read(CKCON) & 0x7;
  cl_uc521::tick(stretch);

  return(res);
}


/*
 * 0xe2-0xe3 1 24 MOVX A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_uc320::instruction_e2/*inst_movx_a_Sri*/(t_mem/*uchar*/ code)
{
  int res= cl_uc521::instruction_e2(code);

  u8_t stretch= sfr->read(CKCON) & 0x7;
  cl_uc521::tick(stretch);

  return(res);
}


/*
 * 0xf0 1 24 MOVX @DPTR,A
 *____________________________________________________________________________
 *
 */

int
cl_uc320::instruction_f0/*inst_movx_Sdptr_a*/(t_mem/*uchar*/ code)
{
  int res= cl_uc521::instruction_f0(code);

  u8_t stretch= sfr->read(CKCON) & 0x7;
  cl_uc521::tick(stretch);

  return(res);
}


/*
 * 0xf2-0xf3 1 24 MOVX @Ri,A
 *____________________________________________________________________________
 *
 */

int
cl_uc320::instruction_f2/*inst_movx_Sri_a*/(t_mem/*uchar*/ code)
{
  int res= cl_uc521::instruction_f2(code);

  u8_t stretch= sfr->read(CKCON) & 0x7;
  cl_uc521::tick(stretch);

  return(res);
}
