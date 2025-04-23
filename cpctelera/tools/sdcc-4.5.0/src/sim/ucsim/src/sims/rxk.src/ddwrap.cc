/*
 * Simulator of microcontrollers (ddwrap.cc)
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

#include "rxkcl.h"

#include "ddwrap.h"

int instruction_wrapper_dd_none(class cl_uc *uc, t_mem code) { return resINV_INST; }

int instruction_wrapper_dd_06(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_06(code); }
int instruction_wrapper_dd_09(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_09(code); }
int instruction_wrapper_dd_0a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_0a(code); }
int instruction_wrapper_dd_0b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_0b(code); }
int instruction_wrapper_dd_0c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_0c(code); }
int instruction_wrapper_dd_0d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_0d(code); }
int instruction_wrapper_dd_0e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_0e(code); }
int instruction_wrapper_dd_0f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_0f(code); }

int instruction_wrapper_dd_19(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_19(code); }
int instruction_wrapper_dd_1a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_1a(code); }
int instruction_wrapper_dd_1b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_1b(code); }
int instruction_wrapper_dd_1c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_1c(code); }
int instruction_wrapper_dd_1d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_1d(code); }
int instruction_wrapper_dd_1e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_1e(code); }
int instruction_wrapper_dd_1f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_1f(code); }

int instruction_wrapper_dd_21(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_21(code); }
int instruction_wrapper_dd_22(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_22(code); }
int instruction_wrapper_dd_23(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_23(code); }
int instruction_wrapper_dd_29(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_29(code); }
int instruction_wrapper_dd_2a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_2a(code); }
int instruction_wrapper_dd_2b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_2b(code); }
int instruction_wrapper_dd_2c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_2c(code); }
int instruction_wrapper_dd_2d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_2d(code); }
int instruction_wrapper_dd_2e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_2e(code); }
int instruction_wrapper_dd_2f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_2f(code); }

int instruction_wrapper_dd_34(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_34(code); }
int instruction_wrapper_dd_35(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_35(code); }
int instruction_wrapper_dd_36(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_36(code); }
int instruction_wrapper_dd_39(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_39(code); }
int instruction_wrapper_dd_3c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_3c(code); }
int instruction_wrapper_dd_3d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_3d(code); }
int instruction_wrapper_dd_3e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_3e(code); }
int instruction_wrapper_dd_3f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_3f(code); }

int instruction_wrapper_dd_46(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_46(code); }
int instruction_wrapper_dd_48(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_48(code); }
int instruction_wrapper_dd_49(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_49(code); }
int instruction_wrapper_dd_4b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_4b(code); }
int instruction_wrapper_dd_4c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_4c(code); }
int instruction_wrapper_dd_4d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_4d(code); }
int instruction_wrapper_dd_4e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_4e(code); }
int instruction_wrapper_dd_4f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_4f(code); }

int instruction_wrapper_dd_56(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_56(code); }
int instruction_wrapper_dd_58(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_58(code); }
int instruction_wrapper_dd_59(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_59(code); }
int instruction_wrapper_dd_5b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_5b(code); }
int instruction_wrapper_dd_5c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_5c(code); }
int instruction_wrapper_dd_5e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_5e(code); }
int instruction_wrapper_dd_5f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_5f(code); }

int instruction_wrapper_dd_64(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_64(code); }
int instruction_wrapper_dd_65(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_65(code); }
int instruction_wrapper_dd_66(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_66(code); }
int instruction_wrapper_dd_68(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_68(code); }
int instruction_wrapper_dd_69(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_69(code); }
int instruction_wrapper_dd_6b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_6b(code); }
int instruction_wrapper_dd_6c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_6c(code); }
int instruction_wrapper_dd_6d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_6d(code); }
int instruction_wrapper_dd_6e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_6e(code); }
int instruction_wrapper_dd_6f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_6f(code); }

int instruction_wrapper_dd_70(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_70(code); }
int instruction_wrapper_dd_71(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_71(code); }
int instruction_wrapper_dd_72(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_72(code); }
int instruction_wrapper_dd_73(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_73(code); }
int instruction_wrapper_dd_74(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_74(code); }
int instruction_wrapper_dd_75(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_75(code); }
int instruction_wrapper_dd_77(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_77(code); }
int instruction_wrapper_dd_78(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_78(code); }
int instruction_wrapper_dd_79(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_79(code); }
int instruction_wrapper_dd_7b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_7b(code); }
int instruction_wrapper_dd_7c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_7c(code); }
int instruction_wrapper_dd_7d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_7d(code); }
int instruction_wrapper_dd_7e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_7e(code); }
int instruction_wrapper_dd_7f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_7f(code); }

int instruction_wrapper_dd_86(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_86(code); }
int instruction_wrapper_dd_88(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_88(code); }
int instruction_wrapper_dd_89(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_89(code); }
int instruction_wrapper_dd_8b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_8b(code); }
int instruction_wrapper_dd_8c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_8c(code); }
int instruction_wrapper_dd_8d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_8d(code); }
int instruction_wrapper_dd_8e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_8e(code); }
int instruction_wrapper_dd_8f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_8f(code); }

int instruction_wrapper_dd_96(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_96(code); }
int instruction_wrapper_dd_98(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_98(code); }
int instruction_wrapper_dd_99(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_99(code); }
int instruction_wrapper_dd_9b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_9b(code); }
int instruction_wrapper_dd_9c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_9c(code); }
int instruction_wrapper_dd_9d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_9d(code); }
int instruction_wrapper_dd_9e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_9e(code); }
int instruction_wrapper_dd_9f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_9f(code); }

int instruction_wrapper_dd_a6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_a6(code); }
int instruction_wrapper_dd_a8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_a8(code); }
int instruction_wrapper_dd_a9(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_a9(code); }
int instruction_wrapper_dd_ab(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ab(code); }
int instruction_wrapper_dd_ac(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ac(code); }
int instruction_wrapper_dd_ad(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ad(code); }
int instruction_wrapper_dd_ae(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ae(code); }
int instruction_wrapper_dd_af(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_af(code); }

int instruction_wrapper_dd_b6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_b6(code); }
int instruction_wrapper_dd_b8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_b8(code); }
int instruction_wrapper_dd_b9(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_b9(code); }
int instruction_wrapper_dd_bb(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_bb(code); }
int instruction_wrapper_dd_bc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_bc(code); }
int instruction_wrapper_dd_bd(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_bd(code); }
int instruction_wrapper_dd_be(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_be(code); }
int instruction_wrapper_dd_bf(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_bf(code); }

int instruction_wrapper_dd_c4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_c4(code); }
int instruction_wrapper_dd_cb(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_cb(code); }
int instruction_wrapper_dd_cc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_cc(code); }
int instruction_wrapper_dd_cd(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_cd(code); }
int instruction_wrapper_dd_ce(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ce(code); }
int instruction_wrapper_dd_cf(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_cf(code); }

int instruction_wrapper_dd_d4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_d4(code); }
int instruction_wrapper_dd_dc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_dc(code); }
int instruction_wrapper_dd_dd(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_dd(code); }
int instruction_wrapper_dd_de(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_de(code); }
int instruction_wrapper_dd_df(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_df(code); }

int instruction_wrapper_dd_e1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_e1(code); }
int instruction_wrapper_dd_e3(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_e3(code); }
int instruction_wrapper_dd_e4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_e4(code); }
int instruction_wrapper_dd_e5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_e5(code); }
int instruction_wrapper_dd_e9(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_e9(code); }
int instruction_wrapper_dd_ea(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ea(code); }
int instruction_wrapper_dd_ec(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ec(code); }
int instruction_wrapper_dd_ed(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ed(code); }
int instruction_wrapper_dd_ee(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ee(code); }
int instruction_wrapper_dd_ef(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ef(code); }

int instruction_wrapper_dd_f1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_f1(code); }
int instruction_wrapper_dd_f4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_f4(code); }
int instruction_wrapper_dd_f5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_f5(code); }
int instruction_wrapper_dd_f9(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_f9(code); }
int instruction_wrapper_dd_fc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_fc(code); }
int instruction_wrapper_dd_fd(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_fd(code); }
int instruction_wrapper_dd_fe(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_fe(code); }
int instruction_wrapper_dd_ff(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_dd_ff(code); }


void fill_dd_wrappers(instruction_wrapper_fn itab[])
{
  int i;
  for (i=0; i<256; i++)
    {
      itab[i]= instruction_wrapper_dd_none;
    }
  itab[0x06]= instruction_wrapper_dd_06;
  itab[0x09]= instruction_wrapper_dd_09;
  itab[0x0a]= instruction_wrapper_dd_0a;
  itab[0x0b]= instruction_wrapper_dd_0b;
  itab[0x0c]= instruction_wrapper_dd_0c;
  itab[0x0d]= instruction_wrapper_dd_0d;
  itab[0x0e]= instruction_wrapper_dd_0e;
  itab[0x0f]= instruction_wrapper_dd_0f;

  itab[0x19]= instruction_wrapper_dd_19;
  itab[0x1a]= instruction_wrapper_dd_1a;
  itab[0x1b]= instruction_wrapper_dd_1b;
  itab[0x1c]= instruction_wrapper_dd_1c;
  itab[0x1d]= instruction_wrapper_dd_1d;
  itab[0x1e]= instruction_wrapper_dd_1e;
  itab[0x1f]= instruction_wrapper_dd_1f;

  itab[0x21]= instruction_wrapper_dd_21;
  itab[0x22]= instruction_wrapper_dd_22;
  itab[0x23]= instruction_wrapper_dd_23;
  itab[0x29]= instruction_wrapper_dd_29;
  itab[0x2a]= instruction_wrapper_dd_2a;
  itab[0x2b]= instruction_wrapper_dd_2b;
  itab[0x2c]= instruction_wrapper_dd_2c;
  itab[0x2d]= instruction_wrapper_dd_2d;
  itab[0x2e]= instruction_wrapper_dd_2e;
  itab[0x2f]= instruction_wrapper_dd_2f;

  itab[0x34]= instruction_wrapper_dd_34;
  itab[0x35]= instruction_wrapper_dd_35;
  itab[0x36]= instruction_wrapper_dd_36;
  itab[0x39]= instruction_wrapper_dd_39;
  itab[0x3c]= instruction_wrapper_dd_3c;
  itab[0x3d]= instruction_wrapper_dd_3d;
  itab[0x3e]= instruction_wrapper_dd_3e;
  itab[0x3f]= instruction_wrapper_dd_3f;

  itab[0x46]= instruction_wrapper_dd_46;
  itab[0x48]= instruction_wrapper_dd_48;
  itab[0x49]= instruction_wrapper_dd_49;
  itab[0x4b]= instruction_wrapper_dd_4b;
  itab[0x4c]= instruction_wrapper_dd_4c;
  itab[0x4d]= instruction_wrapper_dd_4d;
  itab[0x4e]= instruction_wrapper_dd_4e;
  itab[0x4f]= instruction_wrapper_dd_4f;

  itab[0x56]= instruction_wrapper_dd_56;
  itab[0x58]= instruction_wrapper_dd_58;
  itab[0x59]= instruction_wrapper_dd_59;
  itab[0x5b]= instruction_wrapper_dd_5b;
  itab[0x5c]= instruction_wrapper_dd_5c;
  itab[0x5e]= instruction_wrapper_dd_5e;
  itab[0x5f]= instruction_wrapper_dd_5f;

  itab[0x64]= instruction_wrapper_dd_64;
  itab[0x65]= instruction_wrapper_dd_65;
  itab[0x66]= instruction_wrapper_dd_66;
  itab[0x68]= instruction_wrapper_dd_68;
  itab[0x69]= instruction_wrapper_dd_69;
  itab[0x6b]= instruction_wrapper_dd_6b;
  itab[0x6c]= instruction_wrapper_dd_6c;
  itab[0x6d]= instruction_wrapper_dd_6d;
  itab[0x6e]= instruction_wrapper_dd_6e;
  itab[0x6f]= instruction_wrapper_dd_6f;

  itab[0x70]= instruction_wrapper_dd_70;
  itab[0x71]= instruction_wrapper_dd_71;
  itab[0x72]= instruction_wrapper_dd_72;
  itab[0x73]= instruction_wrapper_dd_73;
  itab[0x74]= instruction_wrapper_dd_74;
  itab[0x75]= instruction_wrapper_dd_75;
  itab[0x77]= instruction_wrapper_dd_77;
  itab[0x78]= instruction_wrapper_dd_78;
  itab[0x79]= instruction_wrapper_dd_79;
  itab[0x7b]= instruction_wrapper_dd_7b;
  itab[0x7c]= instruction_wrapper_dd_7c;
  itab[0x7d]= instruction_wrapper_dd_7d;
  itab[0x7e]= instruction_wrapper_dd_7e;
  itab[0x7f]= instruction_wrapper_dd_7f;

  itab[0x86]= instruction_wrapper_dd_86;
  itab[0x88]= instruction_wrapper_dd_88;
  itab[0x89]= instruction_wrapper_dd_89;
  itab[0x8b]= instruction_wrapper_dd_8b;
  itab[0x8c]= instruction_wrapper_dd_8c;
  itab[0x8d]= instruction_wrapper_dd_8d;
  itab[0x8e]= instruction_wrapper_dd_8e;
  itab[0x8f]= instruction_wrapper_dd_8f;

  itab[0x96]= instruction_wrapper_dd_96;
  itab[0x98]= instruction_wrapper_dd_98;
  itab[0x99]= instruction_wrapper_dd_99;
  itab[0x9b]= instruction_wrapper_dd_9b;
  itab[0x9c]= instruction_wrapper_dd_9c;
  itab[0x9d]= instruction_wrapper_dd_9d;
  itab[0x9e]= instruction_wrapper_dd_9e;
  itab[0x9f]= instruction_wrapper_dd_9f;

  itab[0xa6]= instruction_wrapper_dd_a6;
  itab[0xa8]= instruction_wrapper_dd_a8;
  itab[0xa9]= instruction_wrapper_dd_a9;
  itab[0xab]= instruction_wrapper_dd_ab;
  itab[0xac]= instruction_wrapper_dd_ac;
  itab[0xad]= instruction_wrapper_dd_ad;
  itab[0xae]= instruction_wrapper_dd_ae;
  itab[0xaf]= instruction_wrapper_dd_af;

  itab[0xb6]= instruction_wrapper_dd_b6;
  itab[0xb8]= instruction_wrapper_dd_b8;
  itab[0xb9]= instruction_wrapper_dd_b9;
  itab[0xbb]= instruction_wrapper_dd_bb;
  itab[0xbc]= instruction_wrapper_dd_bc;
  itab[0xbd]= instruction_wrapper_dd_bd;
  itab[0xbe]= instruction_wrapper_dd_be;
  itab[0xbf]= instruction_wrapper_dd_bf;

  itab[0xc4]= instruction_wrapper_dd_c4;
  itab[0xcb]= instruction_wrapper_dd_cb;
  itab[0xcc]= instruction_wrapper_dd_cc;
  itab[0xcd]= instruction_wrapper_dd_cd;
  itab[0xce]= instruction_wrapper_dd_ce;
  itab[0xcf]= instruction_wrapper_dd_cf;

  itab[0xd4]= instruction_wrapper_dd_d4;
  itab[0xdc]= instruction_wrapper_dd_dc;
  itab[0xdd]= instruction_wrapper_dd_dd;
  itab[0xde]= instruction_wrapper_dd_de;
  itab[0xdf]= instruction_wrapper_dd_df;

  itab[0xe1]= instruction_wrapper_dd_e1;
  itab[0xe3]= instruction_wrapper_dd_e3;
  itab[0xe4]= instruction_wrapper_dd_e4;
  itab[0xe5]= instruction_wrapper_dd_e5;
  itab[0xe9]= instruction_wrapper_dd_e9;
  itab[0xea]= instruction_wrapper_dd_ea;
  itab[0xec]= instruction_wrapper_dd_ec;
  itab[0xed]= instruction_wrapper_dd_ed;
  itab[0xee]= instruction_wrapper_dd_ee;
  itab[0xef]= instruction_wrapper_dd_ef;

  itab[0xf1]= instruction_wrapper_dd_f1;
  itab[0xf4]= instruction_wrapper_dd_f4;
  itab[0xf5]= instruction_wrapper_dd_f5;
  itab[0xf9]= instruction_wrapper_dd_f9;
  itab[0xfc]= instruction_wrapper_dd_fc;
  itab[0xfd]= instruction_wrapper_dd_fd;
  itab[0xfe]= instruction_wrapper_dd_fe;
  itab[0xff]= instruction_wrapper_dd_ff;
}

/* End of rxk.src/ddwrap.cc */
