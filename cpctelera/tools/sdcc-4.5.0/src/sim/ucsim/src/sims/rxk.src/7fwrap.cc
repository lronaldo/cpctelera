/*
 * Simulator of microcontrollers (7fwrap.cc)
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

#include "7fwrap.h"

int instruction_wrapper_7f_none(class cl_uc *uc, t_mem code) { return resINV_INST; }

int instruction_wrapper_7f_40(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_40(code); }
int instruction_wrapper_7f_41(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_41(code); }
int instruction_wrapper_7f_42(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_42(code); }
int instruction_wrapper_7f_43(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_43(code); }
int instruction_wrapper_7f_44(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_44(code); }
int instruction_wrapper_7f_45(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_45(code); }
int instruction_wrapper_7f_47(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_47(code); }
int instruction_wrapper_7f_48(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_48(code); }
int instruction_wrapper_7f_49(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_49(code); }
int instruction_wrapper_7f_4a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_4a(code); }
int instruction_wrapper_7f_4b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_4b(code); }
int instruction_wrapper_7f_4c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_4c(code); }
int instruction_wrapper_7f_4d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_4d(code); }
int instruction_wrapper_7f_4f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_4f(code); }

int instruction_wrapper_7f_50(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_50(code); }
int instruction_wrapper_7f_51(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_51(code); }
int instruction_wrapper_7f_52(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_52(code); }
int instruction_wrapper_7f_53(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_53(code); }
int instruction_wrapper_7f_54(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_54(code); }
int instruction_wrapper_7f_55(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_55(code); }
int instruction_wrapper_7f_57(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_57(code); }
int instruction_wrapper_7f_58(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_58(code); }
int instruction_wrapper_7f_59(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_59(code); }
int instruction_wrapper_7f_5a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_5a(code); }
int instruction_wrapper_7f_5b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_5b(code); }
int instruction_wrapper_7f_5c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_5c(code); }
int instruction_wrapper_7f_5d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_5d(code); }
int instruction_wrapper_7f_5f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_5f(code); }

int instruction_wrapper_7f_60(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_60(code); }
int instruction_wrapper_7f_61(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_61(code); }
int instruction_wrapper_7f_62(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_62(code); }
int instruction_wrapper_7f_63(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_63(code); }
int instruction_wrapper_7f_64(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_64(code); }
int instruction_wrapper_7f_65(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_65(code); }
int instruction_wrapper_7f_67(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_67(code); }
int instruction_wrapper_7f_68(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_68(code); }
int instruction_wrapper_7f_69(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_69(code); }
int instruction_wrapper_7f_6a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_6a(code); }
int instruction_wrapper_7f_6b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_6b(code); }
int instruction_wrapper_7f_6c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_6c(code); }
int instruction_wrapper_7f_6d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_6d(code); }
int instruction_wrapper_7f_6f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_6f(code); }

int instruction_wrapper_7f_78(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_78(code); }
int instruction_wrapper_7f_79(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_79(code); }
int instruction_wrapper_7f_7a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_7a(code); }
int instruction_wrapper_7f_7b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_7b(code); }
int instruction_wrapper_7f_7c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_7c(code); }
int instruction_wrapper_7f_7d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_7d(code); }
int instruction_wrapper_7f_7f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_7f(code); }

int instruction_wrapper_7f_80(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_80(code); }
int instruction_wrapper_7f_81(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_81(code); }
int instruction_wrapper_7f_82(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_82(code); }
int instruction_wrapper_7f_83(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_83(code); }
int instruction_wrapper_7f_84(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_84(code); }
int instruction_wrapper_7f_85(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_85(code); }
int instruction_wrapper_7f_86(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_86(code); }
int instruction_wrapper_7f_87(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_87(code); }
int instruction_wrapper_7f_88(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_88(code); }
int instruction_wrapper_7f_89(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_89(code); }
int instruction_wrapper_7f_8a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_8a(code); }
int instruction_wrapper_7f_8b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_8b(code); }
int instruction_wrapper_7f_8c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_8c(code); }
int instruction_wrapper_7f_8d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_8d(code); }
int instruction_wrapper_7f_8e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_8e(code); }
int instruction_wrapper_7f_8f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_8f(code); }

int instruction_wrapper_7f_90(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_90(code); }
int instruction_wrapper_7f_91(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_91(code); }
int instruction_wrapper_7f_92(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_92(code); }
int instruction_wrapper_7f_93(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_93(code); }
int instruction_wrapper_7f_94(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_94(code); }
int instruction_wrapper_7f_95(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_95(code); }
int instruction_wrapper_7f_96(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_96(code); }
int instruction_wrapper_7f_97(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_97(code); }
int instruction_wrapper_7f_98(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_98(code); }
int instruction_wrapper_7f_99(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_99(code); }
int instruction_wrapper_7f_9a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_9a(code); }
int instruction_wrapper_7f_9b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_9b(code); }
int instruction_wrapper_7f_9c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_9c(code); }
int instruction_wrapper_7f_9d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_9d(code); }
int instruction_wrapper_7f_9e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_9e(code); }
int instruction_wrapper_7f_9f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_9f(code); }

int instruction_wrapper_7f_a0(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a0(code); }
int instruction_wrapper_7f_a1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a1(code); }
int instruction_wrapper_7f_a2(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a2(code); }
int instruction_wrapper_7f_a3(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a3(code); }
int instruction_wrapper_7f_a4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a4(code); }
int instruction_wrapper_7f_a5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a5(code); }
int instruction_wrapper_7f_a6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a6(code); }
int instruction_wrapper_7f_a7(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a7(code); }
int instruction_wrapper_7f_a8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a8(code); }
int instruction_wrapper_7f_a9(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_a9(code); }
int instruction_wrapper_7f_aa(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_aa(code); }
int instruction_wrapper_7f_ab(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ab(code); }
int instruction_wrapper_7f_ac(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ac(code); }
int instruction_wrapper_7f_ad(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ad(code); }
int instruction_wrapper_7f_ae(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ae(code); }
int instruction_wrapper_7f_af(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_af(code); }

int instruction_wrapper_7f_b0(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b0(code); }
int instruction_wrapper_7f_b1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b1(code); }
int instruction_wrapper_7f_b2(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b2(code); }
int instruction_wrapper_7f_b3(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b3(code); }
int instruction_wrapper_7f_b4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b4(code); }
int instruction_wrapper_7f_b5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b5(code); }
int instruction_wrapper_7f_b6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b6(code); }
int instruction_wrapper_7f_b7(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b7(code); }
int instruction_wrapper_7f_b8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b8(code); }
int instruction_wrapper_7f_b9(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_b9(code); }
int instruction_wrapper_7f_ba(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ba(code); }
int instruction_wrapper_7f_bb(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_bb(code); }
int instruction_wrapper_7f_bc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_bc(code); }
int instruction_wrapper_7f_bd(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_bd(code); }
int instruction_wrapper_7f_be(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_be(code); }
int instruction_wrapper_7f_bf(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_bf(code); }


void fill_7f_wrappers(instruction_wrapper_fn itab[])
{
  int i;
  for (i=0; i<256; i++)
    {
      itab[i]= instruction_wrapper_7f_none;
    }
  itab[0x40]= instruction_wrapper_7f_40;
  itab[0x41]= instruction_wrapper_7f_41;
  itab[0x42]= instruction_wrapper_7f_42;
  itab[0x43]= instruction_wrapper_7f_43;
  itab[0x44]= instruction_wrapper_7f_44;
  itab[0x45]= instruction_wrapper_7f_45;
  itab[0x47]= instruction_wrapper_7f_47;
  itab[0x48]= instruction_wrapper_7f_48;
  itab[0x49]= instruction_wrapper_7f_49;
  itab[0x4a]= instruction_wrapper_7f_4a;
  itab[0x4b]= instruction_wrapper_7f_4b;
  itab[0x4c]= instruction_wrapper_7f_4c;
  itab[0x4d]= instruction_wrapper_7f_4d;
  itab[0x4f]= instruction_wrapper_7f_4f;

  itab[0x50]= instruction_wrapper_7f_50;
  itab[0x51]= instruction_wrapper_7f_51;
  itab[0x52]= instruction_wrapper_7f_52;
  itab[0x53]= instruction_wrapper_7f_53;
  itab[0x54]= instruction_wrapper_7f_54;
  itab[0x55]= instruction_wrapper_7f_55;
  itab[0x57]= instruction_wrapper_7f_57;
  itab[0x58]= instruction_wrapper_7f_58;
  itab[0x59]= instruction_wrapper_7f_59;
  itab[0x5a]= instruction_wrapper_7f_5a;
  itab[0x5b]= instruction_wrapper_7f_5b;
  itab[0x5c]= instruction_wrapper_7f_5c;
  itab[0x5d]= instruction_wrapper_7f_5d;
  itab[0x5f]= instruction_wrapper_7f_5f;

  itab[0x60]= instruction_wrapper_7f_60;
  itab[0x61]= instruction_wrapper_7f_61;
  itab[0x62]= instruction_wrapper_7f_62;
  itab[0x63]= instruction_wrapper_7f_63;
  itab[0x64]= instruction_wrapper_7f_64;
  itab[0x65]= instruction_wrapper_7f_65;
  itab[0x67]= instruction_wrapper_7f_67;
  itab[0x68]= instruction_wrapper_7f_68;
  itab[0x69]= instruction_wrapper_7f_69;
  itab[0x6a]= instruction_wrapper_7f_6a;
  itab[0x6b]= instruction_wrapper_7f_6b;
  itab[0x6c]= instruction_wrapper_7f_6c;
  itab[0x6d]= instruction_wrapper_7f_6d;
  itab[0x6f]= instruction_wrapper_7f_6f;

  itab[0x78]= instruction_wrapper_7f_78;
  itab[0x79]= instruction_wrapper_7f_79;
  itab[0x7a]= instruction_wrapper_7f_7a;
  itab[0x7b]= instruction_wrapper_7f_7b;
  itab[0x7c]= instruction_wrapper_7f_7c;
  itab[0x7d]= instruction_wrapper_7f_7d;
  itab[0x7f]= instruction_wrapper_7f_7f;

  itab[0x80]= instruction_wrapper_7f_80;
  itab[0x81]= instruction_wrapper_7f_81;
  itab[0x82]= instruction_wrapper_7f_82;
  itab[0x83]= instruction_wrapper_7f_83;
  itab[0x84]= instruction_wrapper_7f_84;
  itab[0x85]= instruction_wrapper_7f_85;
  itab[0x86]= instruction_wrapper_7f_86;
  itab[0x87]= instruction_wrapper_7f_87;
  itab[0x88]= instruction_wrapper_7f_88;
  itab[0x89]= instruction_wrapper_7f_89;
  itab[0x8a]= instruction_wrapper_7f_8a;
  itab[0x8b]= instruction_wrapper_7f_8b;
  itab[0x8c]= instruction_wrapper_7f_8c;
  itab[0x8d]= instruction_wrapper_7f_8d;
  itab[0x8e]= instruction_wrapper_7f_8e;
  itab[0x8f]= instruction_wrapper_7f_8f;

  itab[0x90]= instruction_wrapper_7f_90;
  itab[0x91]= instruction_wrapper_7f_91;
  itab[0x92]= instruction_wrapper_7f_92;
  itab[0x93]= instruction_wrapper_7f_93;
  itab[0x94]= instruction_wrapper_7f_94;
  itab[0x95]= instruction_wrapper_7f_95;
  itab[0x96]= instruction_wrapper_7f_96;
  itab[0x97]= instruction_wrapper_7f_97;
  itab[0x98]= instruction_wrapper_7f_98;
  itab[0x99]= instruction_wrapper_7f_99;
  itab[0x9a]= instruction_wrapper_7f_9a;
  itab[0x9b]= instruction_wrapper_7f_9b;
  itab[0x9c]= instruction_wrapper_7f_9c;
  itab[0x9d]= instruction_wrapper_7f_9d;
  itab[0x9e]= instruction_wrapper_7f_9e;
  itab[0x9f]= instruction_wrapper_7f_9f;

  itab[0xa0]= instruction_wrapper_7f_a0;
  itab[0xa1]= instruction_wrapper_7f_a1;
  itab[0xa2]= instruction_wrapper_7f_a2;
  itab[0xa3]= instruction_wrapper_7f_a3;
  itab[0xa4]= instruction_wrapper_7f_a4;
  itab[0xa5]= instruction_wrapper_7f_a5;
  itab[0xa6]= instruction_wrapper_7f_a6;
  itab[0xa7]= instruction_wrapper_7f_a7;
  itab[0xa8]= instruction_wrapper_7f_a8;
  itab[0xa9]= instruction_wrapper_7f_a9;
  itab[0xaa]= instruction_wrapper_7f_aa;
  itab[0xab]= instruction_wrapper_7f_ab;
  itab[0xac]= instruction_wrapper_7f_ac;
  itab[0xad]= instruction_wrapper_7f_ad;
  itab[0xae]= instruction_wrapper_7f_ae;
  itab[0xaf]= instruction_wrapper_7f_af;

  itab[0xb0]= instruction_wrapper_7f_b0;
  itab[0xb1]= instruction_wrapper_7f_b1;
  itab[0xb2]= instruction_wrapper_7f_b2;
  itab[0xb3]= instruction_wrapper_7f_b3;
  itab[0xb4]= instruction_wrapper_7f_b4;
  itab[0xb5]= instruction_wrapper_7f_b5;
  itab[0xb6]= instruction_wrapper_7f_b6;
  itab[0xb7]= instruction_wrapper_7f_b7;
  itab[0xb8]= instruction_wrapper_7f_b8;
  itab[0xb9]= instruction_wrapper_7f_b9;
  itab[0xba]= instruction_wrapper_7f_ba;
  itab[0xbb]= instruction_wrapper_7f_bb;
  itab[0xbc]= instruction_wrapper_7f_bc;
  itab[0xbd]= instruction_wrapper_7f_bd;
  itab[0xbe]= instruction_wrapper_7f_be;
  itab[0xbf]= instruction_wrapper_7f_bf;
}

/* End of rxk.src/7fwrap.cc */
