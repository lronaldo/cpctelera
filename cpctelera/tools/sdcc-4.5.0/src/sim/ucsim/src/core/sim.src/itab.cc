/*
 * Simulator of microcontrollers (itab.cc)
 *
 * Copyright (C) 1999 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
 *
 */

/*
  This file is part of microcontroller simulator: ucsim.

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
  02111-1307, USA.
*/

#include "itabcl.h"

cl_itab::cl_itab(void):
  cl_base()
{
  //uc_itab[0]= &cl_itab::instruction_00;
  uc_itab[0x00]= &cl_itab::instruction_00;
  uc_itab[0x01]= &cl_itab::instruction_01;
  uc_itab[0x02]= &cl_itab::instruction_02;
  uc_itab[0x03]= &cl_itab::instruction_03;
  uc_itab[0x04]= &cl_itab::instruction_04;
  uc_itab[0x05]= &cl_itab::instruction_05;
  uc_itab[0x06]= &cl_itab::instruction_06;
  uc_itab[0x07]= &cl_itab::instruction_07;
  uc_itab[0x08]= &cl_itab::instruction_08;
  uc_itab[0x09]= &cl_itab::instruction_09;
  uc_itab[0x0a]= &cl_itab::instruction_0a;
  uc_itab[0x0b]= &cl_itab::instruction_0b;
  uc_itab[0x0c]= &cl_itab::instruction_0c;
  uc_itab[0x0d]= &cl_itab::instruction_0d;
  uc_itab[0x0e]= &cl_itab::instruction_0e;
  uc_itab[0x0f]= &cl_itab::instruction_0f;
  uc_itab[0x10]= &cl_itab::instruction_10;
  uc_itab[0x11]= &cl_itab::instruction_11;
  uc_itab[0x12]= &cl_itab::instruction_12;
  uc_itab[0x13]= &cl_itab::instruction_13;
  uc_itab[0x14]= &cl_itab::instruction_14;
  uc_itab[0x15]= &cl_itab::instruction_15;
  uc_itab[0x16]= &cl_itab::instruction_16;
  uc_itab[0x17]= &cl_itab::instruction_17;
  uc_itab[0x18]= &cl_itab::instruction_18;
  uc_itab[0x19]= &cl_itab::instruction_19;
  uc_itab[0x1a]= &cl_itab::instruction_1a;
  uc_itab[0x1b]= &cl_itab::instruction_1b;
  uc_itab[0x1c]= &cl_itab::instruction_1c;
  uc_itab[0x1d]= &cl_itab::instruction_1d;
  uc_itab[0x1e]= &cl_itab::instruction_1e;
  uc_itab[0x1f]= &cl_itab::instruction_1f;
  uc_itab[0x20]= &cl_itab::instruction_20;
  uc_itab[0x21]= &cl_itab::instruction_21;
  uc_itab[0x22]= &cl_itab::instruction_22;
  uc_itab[0x23]= &cl_itab::instruction_23;
  uc_itab[0x24]= &cl_itab::instruction_24;
  uc_itab[0x25]= &cl_itab::instruction_25;
  uc_itab[0x26]= &cl_itab::instruction_26;
  uc_itab[0x27]= &cl_itab::instruction_27;
  uc_itab[0x28]= &cl_itab::instruction_28;
  uc_itab[0x29]= &cl_itab::instruction_29;
  uc_itab[0x2a]= &cl_itab::instruction_2a;
  uc_itab[0x2b]= &cl_itab::instruction_2b;
  uc_itab[0x2c]= &cl_itab::instruction_2c;
  uc_itab[0x2d]= &cl_itab::instruction_2d;
  uc_itab[0x2e]= &cl_itab::instruction_2e;
  uc_itab[0x2f]= &cl_itab::instruction_2f;
  uc_itab[0x30]= &cl_itab::instruction_30;
  uc_itab[0x31]= &cl_itab::instruction_31;
  uc_itab[0x32]= &cl_itab::instruction_32;
  uc_itab[0x33]= &cl_itab::instruction_33;
  uc_itab[0x34]= &cl_itab::instruction_34;
  uc_itab[0x35]= &cl_itab::instruction_35;
  uc_itab[0x36]= &cl_itab::instruction_36;
  uc_itab[0x37]= &cl_itab::instruction_37;
  uc_itab[0x38]= &cl_itab::instruction_38;
  uc_itab[0x39]= &cl_itab::instruction_39;
  uc_itab[0x3a]= &cl_itab::instruction_3a;
  uc_itab[0x3b]= &cl_itab::instruction_3b;
  uc_itab[0x3c]= &cl_itab::instruction_3c;
  uc_itab[0x3d]= &cl_itab::instruction_3d;
  uc_itab[0x3e]= &cl_itab::instruction_3e;
  uc_itab[0x3f]= &cl_itab::instruction_3f;
  uc_itab[0x40]= &cl_itab::instruction_40;
  uc_itab[0x41]= &cl_itab::instruction_41;
  uc_itab[0x42]= &cl_itab::instruction_42;
  uc_itab[0x43]= &cl_itab::instruction_43;
  uc_itab[0x44]= &cl_itab::instruction_44;
  uc_itab[0x45]= &cl_itab::instruction_45;
  uc_itab[0x46]= &cl_itab::instruction_46;
  uc_itab[0x47]= &cl_itab::instruction_47;
  uc_itab[0x48]= &cl_itab::instruction_48;
  uc_itab[0x49]= &cl_itab::instruction_49;
  uc_itab[0x4a]= &cl_itab::instruction_4a;
  uc_itab[0x4b]= &cl_itab::instruction_4b;
  uc_itab[0x4c]= &cl_itab::instruction_4c;
  uc_itab[0x4d]= &cl_itab::instruction_4d;
  uc_itab[0x4e]= &cl_itab::instruction_4e;
  uc_itab[0x4f]= &cl_itab::instruction_4f;
  uc_itab[0x50]= &cl_itab::instruction_50;
  uc_itab[0x51]= &cl_itab::instruction_51;
  uc_itab[0x52]= &cl_itab::instruction_52;
  uc_itab[0x53]= &cl_itab::instruction_53;
  uc_itab[0x54]= &cl_itab::instruction_54;
  uc_itab[0x55]= &cl_itab::instruction_55;
  uc_itab[0x56]= &cl_itab::instruction_56;
  uc_itab[0x57]= &cl_itab::instruction_57;
  uc_itab[0x58]= &cl_itab::instruction_58;
  uc_itab[0x59]= &cl_itab::instruction_59;
  uc_itab[0x5a]= &cl_itab::instruction_5a;
  uc_itab[0x5b]= &cl_itab::instruction_5b;
  uc_itab[0x5c]= &cl_itab::instruction_5c;
  uc_itab[0x5d]= &cl_itab::instruction_5d;
  uc_itab[0x5e]= &cl_itab::instruction_5e;
  uc_itab[0x5f]= &cl_itab::instruction_5f;
  uc_itab[0x60]= &cl_itab::instruction_60;
  uc_itab[0x61]= &cl_itab::instruction_61;
  uc_itab[0x62]= &cl_itab::instruction_62;
  uc_itab[0x63]= &cl_itab::instruction_63;
  uc_itab[0x64]= &cl_itab::instruction_64;
  uc_itab[0x65]= &cl_itab::instruction_65;
  uc_itab[0x66]= &cl_itab::instruction_66;
  uc_itab[0x67]= &cl_itab::instruction_67;
  uc_itab[0x68]= &cl_itab::instruction_68;
  uc_itab[0x69]= &cl_itab::instruction_69;
  uc_itab[0x6a]= &cl_itab::instruction_6a;
  uc_itab[0x6b]= &cl_itab::instruction_6b;
  uc_itab[0x6c]= &cl_itab::instruction_6c;
  uc_itab[0x6d]= &cl_itab::instruction_6d;
  uc_itab[0x6e]= &cl_itab::instruction_6e;
  uc_itab[0x6f]= &cl_itab::instruction_6f;
  uc_itab[0x70]= &cl_itab::instruction_70;
  uc_itab[0x71]= &cl_itab::instruction_71;
  uc_itab[0x72]= &cl_itab::instruction_72;
  uc_itab[0x73]= &cl_itab::instruction_73;
  uc_itab[0x74]= &cl_itab::instruction_74;
  uc_itab[0x75]= &cl_itab::instruction_75;
  uc_itab[0x76]= &cl_itab::instruction_76;
  uc_itab[0x77]= &cl_itab::instruction_77;
  uc_itab[0x78]= &cl_itab::instruction_78;
  uc_itab[0x79]= &cl_itab::instruction_79;
  uc_itab[0x7a]= &cl_itab::instruction_7a;
  uc_itab[0x7b]= &cl_itab::instruction_7b;
  uc_itab[0x7c]= &cl_itab::instruction_7c;
  uc_itab[0x7d]= &cl_itab::instruction_7d;
  uc_itab[0x7e]= &cl_itab::instruction_7e;
  uc_itab[0x7f]= &cl_itab::instruction_7f;
  uc_itab[0x80]= &cl_itab::instruction_80;
  uc_itab[0x81]= &cl_itab::instruction_81;
  uc_itab[0x82]= &cl_itab::instruction_82;
  uc_itab[0x83]= &cl_itab::instruction_83;
  uc_itab[0x84]= &cl_itab::instruction_84;
  uc_itab[0x85]= &cl_itab::instruction_85;
  uc_itab[0x86]= &cl_itab::instruction_86;
  uc_itab[0x87]= &cl_itab::instruction_87;
  uc_itab[0x88]= &cl_itab::instruction_88;
  uc_itab[0x89]= &cl_itab::instruction_89;
  uc_itab[0x8a]= &cl_itab::instruction_8a;
  uc_itab[0x8b]= &cl_itab::instruction_8b;
  uc_itab[0x8c]= &cl_itab::instruction_8c;
  uc_itab[0x8d]= &cl_itab::instruction_8d;
  uc_itab[0x8e]= &cl_itab::instruction_8e;
  uc_itab[0x8f]= &cl_itab::instruction_8f;
  uc_itab[0x90]= &cl_itab::instruction_90;
  uc_itab[0x91]= &cl_itab::instruction_91;
  uc_itab[0x92]= &cl_itab::instruction_92;
  uc_itab[0x93]= &cl_itab::instruction_93;
  uc_itab[0x94]= &cl_itab::instruction_94;
  uc_itab[0x95]= &cl_itab::instruction_95;
  uc_itab[0x96]= &cl_itab::instruction_96;
  uc_itab[0x97]= &cl_itab::instruction_97;
  uc_itab[0x98]= &cl_itab::instruction_98;
  uc_itab[0x99]= &cl_itab::instruction_99;
  uc_itab[0x9a]= &cl_itab::instruction_9a;
  uc_itab[0x9b]= &cl_itab::instruction_9b;
  uc_itab[0x9c]= &cl_itab::instruction_9c;
  uc_itab[0x9d]= &cl_itab::instruction_9d;
  uc_itab[0x9e]= &cl_itab::instruction_9e;
  uc_itab[0x9f]= &cl_itab::instruction_9f;
  uc_itab[0xa0]= &cl_itab::instruction_a0;
  uc_itab[0xa1]= &cl_itab::instruction_a1;
  uc_itab[0xa2]= &cl_itab::instruction_a2;
  uc_itab[0xa3]= &cl_itab::instruction_a3;
  uc_itab[0xa4]= &cl_itab::instruction_a4;
  uc_itab[0xa5]= &cl_itab::instruction_a5;
  uc_itab[0xa6]= &cl_itab::instruction_a6;
  uc_itab[0xa7]= &cl_itab::instruction_a7;
  uc_itab[0xa8]= &cl_itab::instruction_a8;
  uc_itab[0xa9]= &cl_itab::instruction_a9;
  uc_itab[0xaa]= &cl_itab::instruction_aa;
  uc_itab[0xab]= &cl_itab::instruction_ab;
  uc_itab[0xac]= &cl_itab::instruction_ac;
  uc_itab[0xad]= &cl_itab::instruction_ad;
  uc_itab[0xae]= &cl_itab::instruction_ae;
  uc_itab[0xaf]= &cl_itab::instruction_af;
  uc_itab[0xb0]= &cl_itab::instruction_b0;
  uc_itab[0xb1]= &cl_itab::instruction_b1;
  uc_itab[0xb2]= &cl_itab::instruction_b2;
  uc_itab[0xb3]= &cl_itab::instruction_b3;
  uc_itab[0xb4]= &cl_itab::instruction_b4;
  uc_itab[0xb5]= &cl_itab::instruction_b5;
  uc_itab[0xb6]= &cl_itab::instruction_b6;
  uc_itab[0xb7]= &cl_itab::instruction_b7;
  uc_itab[0xb8]= &cl_itab::instruction_b8;
  uc_itab[0xb9]= &cl_itab::instruction_b9;
  uc_itab[0xba]= &cl_itab::instruction_ba;
  uc_itab[0xbb]= &cl_itab::instruction_bb;
  uc_itab[0xbc]= &cl_itab::instruction_bc;
  uc_itab[0xbd]= &cl_itab::instruction_bd;
  uc_itab[0xbe]= &cl_itab::instruction_be;
  uc_itab[0xbf]= &cl_itab::instruction_bf;
  uc_itab[0xc0]= &cl_itab::instruction_c0;
  uc_itab[0xc1]= &cl_itab::instruction_c1;
  uc_itab[0xc2]= &cl_itab::instruction_c2;
  uc_itab[0xc3]= &cl_itab::instruction_c3;
  uc_itab[0xc4]= &cl_itab::instruction_c4;
  uc_itab[0xc5]= &cl_itab::instruction_c5;
  uc_itab[0xc6]= &cl_itab::instruction_c6;
  uc_itab[0xc7]= &cl_itab::instruction_c7;
  uc_itab[0xc8]= &cl_itab::instruction_c8;
  uc_itab[0xc9]= &cl_itab::instruction_c9;
  uc_itab[0xca]= &cl_itab::instruction_ca;
  uc_itab[0xcb]= &cl_itab::instruction_cb;
  uc_itab[0xcc]= &cl_itab::instruction_cc;
  uc_itab[0xcd]= &cl_itab::instruction_cd;
  uc_itab[0xce]= &cl_itab::instruction_ce;
  uc_itab[0xcf]= &cl_itab::instruction_cf;
  uc_itab[0xd0]= &cl_itab::instruction_d0;
  uc_itab[0xd1]= &cl_itab::instruction_d1;
  uc_itab[0xd2]= &cl_itab::instruction_d2;
  uc_itab[0xd3]= &cl_itab::instruction_d3;
  uc_itab[0xd4]= &cl_itab::instruction_d4;
  uc_itab[0xd5]= &cl_itab::instruction_d5;
  uc_itab[0xd6]= &cl_itab::instruction_d6;
  uc_itab[0xd7]= &cl_itab::instruction_d7;
  uc_itab[0xd8]= &cl_itab::instruction_d8;
  uc_itab[0xd9]= &cl_itab::instruction_d9;
  uc_itab[0xda]= &cl_itab::instruction_da;
  uc_itab[0xdb]= &cl_itab::instruction_db;
  uc_itab[0xdc]= &cl_itab::instruction_dc;
  uc_itab[0xdd]= &cl_itab::instruction_dd;
  uc_itab[0xde]= &cl_itab::instruction_de;
  uc_itab[0xdf]= &cl_itab::instruction_df;
  uc_itab[0xe0]= &cl_itab::instruction_e0;
  uc_itab[0xe1]= &cl_itab::instruction_e1;
  uc_itab[0xe2]= &cl_itab::instruction_e2;
  uc_itab[0xe3]= &cl_itab::instruction_e3;
  uc_itab[0xe4]= &cl_itab::instruction_e4;
  uc_itab[0xe5]= &cl_itab::instruction_e5;
  uc_itab[0xe6]= &cl_itab::instruction_e6;
  uc_itab[0xe7]= &cl_itab::instruction_e7;
  uc_itab[0xe8]= &cl_itab::instruction_e8;
  uc_itab[0xe9]= &cl_itab::instruction_e9;
  uc_itab[0xea]= &cl_itab::instruction_ea;
  uc_itab[0xeb]= &cl_itab::instruction_eb;
  uc_itab[0xec]= &cl_itab::instruction_ec;
  uc_itab[0xed]= &cl_itab::instruction_ed;
  uc_itab[0xee]= &cl_itab::instruction_ee;
  uc_itab[0xef]= &cl_itab::instruction_ef;
  uc_itab[0xf0]= &cl_itab::instruction_f0;
  uc_itab[0xf1]= &cl_itab::instruction_f1;
  uc_itab[0xf2]= &cl_itab::instruction_f2;
  uc_itab[0xf3]= &cl_itab::instruction_f3;
  uc_itab[0xf4]= &cl_itab::instruction_f4;
  uc_itab[0xf5]= &cl_itab::instruction_f5;
  uc_itab[0xf6]= &cl_itab::instruction_f6;
  uc_itab[0xf7]= &cl_itab::instruction_f7;
  uc_itab[0xf8]= &cl_itab::instruction_f8;
  uc_itab[0xf9]= &cl_itab::instruction_f9;
  uc_itab[0xfa]= &cl_itab::instruction_fa;
  uc_itab[0xfb]= &cl_itab::instruction_fb;
  uc_itab[0xfc]= &cl_itab::instruction_fc;
  uc_itab[0xfd]= &cl_itab::instruction_fd;
  uc_itab[0xfe]= &cl_itab::instruction_fe;
  uc_itab[0xff]= &cl_itab::instruction_ff;

  uc_itab[0x100]= &cl_itab::invalid_instruction;
}

/* End of sim.src/itab.cc */
