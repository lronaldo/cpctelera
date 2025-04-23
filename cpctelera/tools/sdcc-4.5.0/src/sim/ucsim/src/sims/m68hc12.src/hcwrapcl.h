/*
 * Simulator of microcontrollers (hcwrapcl.h)
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

#ifndef HCWRAPCL_HEADER
#define HCWRAPCL_HEADER

#include "glob12.h"
#include "m68hc12cl.h"

#define _NONE
#define _cA		uc->cA
#define _cB		uc->cB
#define _cD		uc->cD
#define _cC		uc->cCC
#define _cX		uc->cIX
#define _cY		uc->cIY
#define _cS		uc->cSP
#define _cSP		uc->cSP

#define _i8		uc->i8()

#define _d		uc->ddst()
#define _d_Aop		uc->ddst(),uc->acc.DAB.a8.Ar
#define _d_Bop		uc->ddst(),uc->acc.DAB.a8.Br
#define _d_i8		uc->ddst(),uc->i8()
#define _d_i8n		uc->ddst(),~(uc->i8())

#define _da		uc->daddr()
#define _da_Dop		uc->daddr(),uc->acc.DAB.Dr
#define _da_Xop		uc->daddr(),uc->IX
#define _da_Yop		uc->daddr(),uc->IY
#define _da_Sop		uc->daddr(),uc->SP

#define _e		uc->edst()
#define _e_Aop		uc->edst(),uc->acc.DAB.a8.Ar
#define _e_Bop		uc->edst(),uc->acc.DAB.a8.Br
#define _e_i8		uc->edst(),uc->i8()
#define _e_i8n		uc->edst(),~(uc->i8())

#define _ea		uc->eaddr()
#define _ea_Dop		uc->eaddr(),uc->acc.DAB.Dr
#define _ea_Xop		uc->eaddr(),uc->IX
#define _ea_Yop		uc->eaddr(),uc->IY
#define _ea_Sop		uc->eaddr(),uc->SP

#define _xb		uc->xbdst()
#define _xb_Aop		uc->xbdst(),uc->acc.DAB.a8.Ar
#define _xb_Bop		uc->xbdst(),uc->acc.DAB.a8.Br
#define _xb_i8		uc->xbdst(),uc->i8()
#define _xb_i8n		uc->xbdst(),~(uc->i8())

#define _xba		uc->xbaddr()
#define _xba_Dop	uc->xbaddr(),uc->acc.DAB.Dr
#define _xba_Xop	uc->xbaddr(),uc->IX
#define _xba_Yop	uc->xbaddr(),uc->IY
#define _xba_Sop	uc->xbaddr(),uc->SP

#define _Aop		uc->acc.DAB.a8.Ar
#define _Bop		uc->acc.DAB.a8.Br
#define _Dop		uc->acc.DAB.Dr
#define _Cop		uc->CC.cc16.cc8.ccl
#define _Xop		uc->IX
#define _Yop		uc->IY
#define _dop		uc->dop()
#define _eop		uc->eop()
#define _xbop8		uc->xbop8()

#define _A_B		uc->cA,uc->acc.DAB.a8.Br

#define _A_i8		uc->cA,uc->i8()
#define _A_dop		uc->cA,uc->dop()
#define _A_eop		uc->cA,uc->eop()
#define _A_xbop8	uc->cA,uc->xbop8()

#define _Aop_i8		uc->acc.DAB.a8.Ar,uc->i8()
#define _Aop_dop	uc->acc.DAB.a8.Ar,uc->dop()
#define _Aop_eop	uc->acc.DAB.a8.Ar,uc->eop()
#define _Aop_xbop8	uc->acc.DAB.a8.Ar,uc->xbop8()
#define _Aop_Bop	uc->acc.DAB.a8.Ar,uc->acc.DAB.a8.Br

#define _B_i8		uc->cB,uc->i8()
#define _B_dop		uc->cB,uc->dop()
#define _B_eop		uc->cB,uc->eop()
#define _B_xbop8	uc->cB,uc->xbop8()

#define _Bop_i8		uc->acc.DAB.a8.Br,uc->i8()
#define _Bop_dop	uc->acc.DAB.a8.Br,uc->dop()
#define _Bop_eop	uc->acc.DAB.a8.Br,uc->eop()
#define _Bop_xbop8	uc->acc.DAB.a8.Br,uc->xbop8()

#define _D_i16		uc->cD,uc->i16()
#define _D_dop16	uc->cD,uc->dop16()
#define _D_eop16	uc->cD,uc->eop16()
#define _D_xbop16	uc->cD,uc->xbop16()

#define _Dop_i16	uc->acc.DAB.Dr,uc->i16()
#define _Dop_dop16	uc->acc.DAB.Dr,uc->dop16()
#define _Dop_eop16	uc->acc.DAB.Dr,uc->eop16()
#define _Dop_xbop16	uc->acc.DAB.Dr,uc->xbop16()

#define _X_i16		uc->cIX,uc->i16()
#define _X_dop16	uc->cIX,uc->dop16()
#define _X_eop16	uc->cIX,uc->eop16()
#define _X_xbop16	uc->cIX,uc->xbop16()

#define _Xop_i16	uc->IX,uc->i16()
#define _Xop_dop16	uc->IX,uc->dop16()
#define _Xop_eop16	uc->IX,uc->eop16()
#define _Xop_xbop16	uc->IX,uc->xbop16()

#define _Y_i16		uc->cIY,uc->i16()
#define _Y_dop16	uc->cIY,uc->dop16()
#define _Y_eop16	uc->cIY,uc->eop16()
#define _Y_xbop16	uc->cIY,uc->xbop16()

#define _Yop_i16	uc->IY,uc->i16()
#define _Yop_dop16	uc->IY,uc->dop16()
#define _Yop_eop16	uc->IY,uc->eop16()
#define _Yop_xbop16	uc->IY,uc->xbop16()

#define _S_i16		uc->cSP,uc->i16()
#define _S_dop16	uc->cSP,uc->dop16()
#define _S_eop16	uc->cSP,uc->eop16()
#define _S_xbop16	uc->cSP,uc->xbop16()

#define _Sop_i16	uc->SP,uc->i16()
#define _Sop_dop16	uc->SP,uc->dop16()
#define _Sop_eop16	uc->SP,uc->eop16()
#define _Sop_xbop16	uc->SP,uc->xbop16()

extern int wrap_INV(class CL12 *uc, t_mem code);
extern int wrap_TRAP(class CL12 *uc, t_mem code);

class cl_wrap: public cl_base
{
public:
  hcwrapper_fn page0[256], page0x18[256];
public:
  cl_wrap();
  virtual void set_disass(int page, int code, const char *mnemo, char branch, int len);
  virtual void set_ticks(int page, int code, int ticks);
  virtual int init()
  {
    {
      int i;
      chars s;
      for (i= 0; i<256; i++)
	{
	  s.format("TRAP $%02x", i);
	  disass12p18[i].code= i;
	  disass12p18[i].branch= ' ';
	  disass12p18[i].length= 2;
	  disass12p18[i].mnemonic= strdup(s.c_str());
	  ticks12p18[i]= 11;
	}
    }
    fill_0_00();
    fill_0_01();
    fill_0_02();
    fill_0_03();
    fill_0_04();
    fill_0_05();
    fill_0_06();
    fill_0_07();
    fill_0_08();
    fill_0_09();
    fill_0_0a();
    fill_0_0b();
    fill_0_0c();
    fill_0_0d();
    fill_0_0e();
    fill_0_0f();
    fill_0_10();
    fill_0_11();
    fill_0_12();
    fill_0_13();
    fill_0_14();
    fill_0_15();
    fill_0_16();
    fill_0_17();
    fill_0_18();
    fill_0_19();
    fill_0_1a();
    fill_0_1b();
    fill_0_1c();
    fill_0_1d();
    fill_0_1e();
    fill_0_1f();
    fill_0_20();
    fill_0_21();
    fill_0_22();
    fill_0_23();
    fill_0_24();
    fill_0_25();
    fill_0_26();
    fill_0_27();
    fill_0_28();
    fill_0_29();
    fill_0_2a();
    fill_0_2b();
    fill_0_2c();
    fill_0_2d();
    fill_0_2e();
    fill_0_2f();
    fill_0_30();
    fill_0_31();
    fill_0_32();
    fill_0_33();
    fill_0_34();
    fill_0_35();
    fill_0_36();
    fill_0_37();
    fill_0_38();
    fill_0_39();
    fill_0_3a();
    fill_0_3b();
    fill_0_3c();
    fill_0_3d();
    fill_0_3e();
    fill_0_3f();
    fill_0_40();
    fill_0_41();
    fill_0_42();
    fill_0_43();
    fill_0_44();
    fill_0_45();
    fill_0_46();
    fill_0_47();
    fill_0_48();
    fill_0_49();
    fill_0_4a();
    fill_0_4b();
    fill_0_4c();
    fill_0_4d();
    fill_0_4e();
    fill_0_4f();
    fill_0_50();
    fill_0_51();
    fill_0_52();
    fill_0_53();
    fill_0_54();
    fill_0_55();
    fill_0_56();
    fill_0_57();
    fill_0_58();
    fill_0_59();
    fill_0_5a();
    fill_0_5b();
    fill_0_5c();
    fill_0_5d();
    fill_0_5e();
    fill_0_5f();
    fill_0_60();
    fill_0_61();
    fill_0_62();
    fill_0_63();
    fill_0_64();
    fill_0_65();
    fill_0_66();
    fill_0_67();
    fill_0_68();
    fill_0_69();
    fill_0_6a();
    fill_0_6b();
    fill_0_6c();
    fill_0_6d();
    fill_0_6e();
    fill_0_6f();
    fill_0_70();
    fill_0_71();
    fill_0_72();
    fill_0_73();
    fill_0_74();
    fill_0_75();
    fill_0_76();
    fill_0_77();
    fill_0_78();
    fill_0_79();
    fill_0_7a();
    fill_0_7b();
    fill_0_7c();
    fill_0_7d();
    fill_0_7e();
    fill_0_7f();
    fill_0_80();
    fill_0_81();
    fill_0_82();
    fill_0_83();
    fill_0_84();
    fill_0_85();
    fill_0_86();
    fill_0_87();
    fill_0_88();
    fill_0_89();
    fill_0_8a();
    fill_0_8b();
    fill_0_8c();
    fill_0_8d();
    fill_0_8e();
    fill_0_8f();
    fill_0_90();
    fill_0_91();
    fill_0_92();
    fill_0_93();
    fill_0_94();
    fill_0_95();
    fill_0_96();
    fill_0_97();
    fill_0_98();
    fill_0_99();
    fill_0_9a();
    fill_0_9b();
    fill_0_9c();
    fill_0_9d();
    fill_0_9e();
    fill_0_9f();
    fill_0_a0();
    fill_0_a1();
    fill_0_a2();
    fill_0_a3();
    fill_0_a4();
    fill_0_a5();
    fill_0_a6();
    fill_0_a7();
    fill_0_a8();
    fill_0_a9();
    fill_0_aa();
    fill_0_ab();
    fill_0_ac();
    fill_0_ad();
    fill_0_ae();
    fill_0_af();
    fill_0_b0();
    fill_0_b1();
    fill_0_b2();
    fill_0_b3();
    fill_0_b4();
    fill_0_b5();
    fill_0_b6();
    fill_0_b7();
    fill_0_b8();
    fill_0_b9();
    fill_0_ba();
    fill_0_bb();
    fill_0_bc();
    fill_0_bd();
    fill_0_be();
    fill_0_bf();
    fill_0_c0();
    fill_0_c1();
    fill_0_c2();
    fill_0_c3();
    fill_0_c4();
    fill_0_c5();
    fill_0_c6();
    fill_0_c7();
    fill_0_c8();
    fill_0_c9();
    fill_0_ca();
    fill_0_cb();
    fill_0_cc();
    fill_0_cd();
    fill_0_ce();
    fill_0_cf();
    fill_0_d0();
    fill_0_d1();
    fill_0_d2();
    fill_0_d3();
    fill_0_d4();
    fill_0_d5();
    fill_0_d6();
    fill_0_d7();
    fill_0_d8();
    fill_0_d9();
    fill_0_da();
    fill_0_db();
    fill_0_dc();
    fill_0_dd();
    fill_0_de();
    fill_0_df();
    fill_0_e0();
    fill_0_e1();
    fill_0_e2();
    fill_0_e3();
    fill_0_e4();
    fill_0_e5();
    fill_0_e6();
    fill_0_e7();
    fill_0_e8();
    fill_0_e9();
    fill_0_ea();
    fill_0_eb();
    fill_0_ec();
    fill_0_ed();
    fill_0_ee();
    fill_0_ef();
    fill_0_f0();
    fill_0_f1();
    fill_0_f2();
    fill_0_f3();
    fill_0_f4();
    fill_0_f5();
    fill_0_f6();
    fill_0_f7();
    fill_0_f8();
    fill_0_f9();
    fill_0_fa();
    fill_0_fb();
    fill_0_fc();
    fill_0_fd();
    fill_0_fe();
    fill_0_ff();

    fill_0x18_00();
    fill_0x18_01();
    fill_0x18_02();
    fill_0x18_03();
    fill_0x18_04();
    fill_0x18_05();
    fill_0x18_06();
    fill_0x18_07();
    fill_0x18_08();
    fill_0x18_09();
    fill_0x18_0a();
    fill_0x18_0b();
    fill_0x18_0c();
    fill_0x18_0d();
    fill_0x18_0e();
    fill_0x18_0f();
    fill_0x18_10();
    fill_0x18_11();
    fill_0x18_12();
    fill_0x18_13();
    fill_0x18_14();
    fill_0x18_15();
    fill_0x18_16();
    fill_0x18_17();
    fill_0x18_18();
    fill_0x18_19();
    fill_0x18_1a();
    fill_0x18_1b();
    fill_0x18_1c();
    fill_0x18_1d();
    fill_0x18_1e();
    fill_0x18_1f();
    fill_0x18_20();
    fill_0x18_21();
    fill_0x18_22();
    fill_0x18_23();
    fill_0x18_24();
    fill_0x18_25();
    fill_0x18_26();
    fill_0x18_27();
    fill_0x18_28();
    fill_0x18_29();
    fill_0x18_2a();
    fill_0x18_2b();
    fill_0x18_2c();
    fill_0x18_2d();
    fill_0x18_2e();
    fill_0x18_2f();
    fill_0x18_30();
    fill_0x18_31();
    fill_0x18_32();
    fill_0x18_33();
    fill_0x18_34();
    fill_0x18_35();
    fill_0x18_36();
    fill_0x18_37();
    fill_0x18_38();
    fill_0x18_39();
    fill_0x18_3a();
    fill_0x18_3b();
    fill_0x18_3c();
    fill_0x18_3d();
    fill_0x18_3e();
    fill_0x18_3f();
    fill_0x18_40();
    fill_0x18_41();
    fill_0x18_42();
    fill_0x18_43();
    fill_0x18_44();
    fill_0x18_45();
    fill_0x18_46();
    fill_0x18_47();
    fill_0x18_48();
    fill_0x18_49();
    fill_0x18_4a();
    fill_0x18_4b();
    fill_0x18_4c();
    fill_0x18_4d();
    fill_0x18_4e();
    fill_0x18_4f();
    fill_0x18_50();
    fill_0x18_51();
    fill_0x18_52();
    fill_0x18_53();
    fill_0x18_54();
    fill_0x18_55();
    fill_0x18_56();
    fill_0x18_57();
    fill_0x18_58();
    fill_0x18_59();
    fill_0x18_5a();
    fill_0x18_5b();
    fill_0x18_5c();
    fill_0x18_5d();
    fill_0x18_5e();
    fill_0x18_5f();
    fill_0x18_60();
    fill_0x18_61();
    fill_0x18_62();
    fill_0x18_63();
    fill_0x18_64();
    fill_0x18_65();
    fill_0x18_66();
    fill_0x18_67();
    fill_0x18_68();
    fill_0x18_69();
    fill_0x18_6a();
    fill_0x18_6b();
    fill_0x18_6c();
    fill_0x18_6d();
    fill_0x18_6e();
    fill_0x18_6f();
    fill_0x18_70();
    fill_0x18_71();
    fill_0x18_72();
    fill_0x18_73();
    fill_0x18_74();
    fill_0x18_75();
    fill_0x18_76();
    fill_0x18_77();
    fill_0x18_78();
    fill_0x18_79();
    fill_0x18_7a();
    fill_0x18_7b();
    fill_0x18_7c();
    fill_0x18_7d();
    fill_0x18_7e();
    fill_0x18_7f();
    fill_0x18_80();
    fill_0x18_81();
    fill_0x18_82();
    fill_0x18_83();
    fill_0x18_84();
    fill_0x18_85();
    fill_0x18_86();
    fill_0x18_87();
    fill_0x18_88();
    fill_0x18_89();
    fill_0x18_8a();
    fill_0x18_8b();
    fill_0x18_8c();
    fill_0x18_8d();
    fill_0x18_8e();
    fill_0x18_8f();
    fill_0x18_90();
    fill_0x18_91();
    fill_0x18_92();
    fill_0x18_93();
    fill_0x18_94();
    fill_0x18_95();
    fill_0x18_96();
    fill_0x18_97();
    fill_0x18_98();
    fill_0x18_99();
    fill_0x18_9a();
    fill_0x18_9b();
    fill_0x18_9c();
    fill_0x18_9d();
    fill_0x18_9e();
    fill_0x18_9f();
    fill_0x18_a0();
    fill_0x18_a1();
    fill_0x18_a2();
    fill_0x18_a3();
    fill_0x18_a4();
    fill_0x18_a5();
    fill_0x18_a6();
    fill_0x18_a7();
    fill_0x18_a8();
    fill_0x18_a9();
    fill_0x18_aa();
    fill_0x18_ab();
    fill_0x18_ac();
    fill_0x18_ad();
    fill_0x18_ae();
    fill_0x18_af();
    fill_0x18_b0();
    fill_0x18_b1();
    fill_0x18_b2();
    fill_0x18_b3();
    fill_0x18_b4();
    fill_0x18_b5();
    fill_0x18_b6();
    fill_0x18_b7();
    fill_0x18_b8();
    fill_0x18_b9();
    fill_0x18_ba();
    fill_0x18_bb();
    fill_0x18_bc();
    fill_0x18_bd();
    fill_0x18_be();
    fill_0x18_bf();
    fill_0x18_c0();
    fill_0x18_c1();
    fill_0x18_c2();
    fill_0x18_c3();
    fill_0x18_c4();
    fill_0x18_c5();
    fill_0x18_c6();
    fill_0x18_c7();
    fill_0x18_c8();
    fill_0x18_c9();
    fill_0x18_ca();
    fill_0x18_cb();
    fill_0x18_cc();
    fill_0x18_cd();
    fill_0x18_ce();
    fill_0x18_cf();
    fill_0x18_d0();
    fill_0x18_d1();
    fill_0x18_d2();
    fill_0x18_d3();
    fill_0x18_d4();
    fill_0x18_d5();
    fill_0x18_d6();
    fill_0x18_d7();
    fill_0x18_d8();
    fill_0x18_d9();
    fill_0x18_da();
    fill_0x18_db();
    fill_0x18_dc();
    fill_0x18_dd();
    fill_0x18_de();
    fill_0x18_df();
    fill_0x18_e0();
    fill_0x18_e1();
    fill_0x18_e2();
    fill_0x18_e3();
    fill_0x18_e4();
    fill_0x18_e5();
    fill_0x18_e6();
    fill_0x18_e7();
    fill_0x18_e8();
    fill_0x18_e9();
    fill_0x18_ea();
    fill_0x18_eb();
    fill_0x18_ec();
    fill_0x18_ed();
    fill_0x18_ee();
    fill_0x18_ef();
    fill_0x18_f0();
    fill_0x18_f1();
    fill_0x18_f2();
    fill_0x18_f3();
    fill_0x18_f4();
    fill_0x18_f5();
    fill_0x18_f6();
    fill_0x18_f7();
    fill_0x18_f8();
    fill_0x18_f9();
    fill_0x18_fa();
    fill_0x18_fb();
    fill_0x18_fc();
    fill_0x18_fd();
    fill_0x18_fe();
    fill_0x18_ff();
    return 0;
  }
  virtual void fill_0_00() { page0[0x00]= wrap_INV; }
  virtual void fill_0_01() { page0[0x01]= wrap_INV; }
  virtual void fill_0_02() { page0[0x02]= wrap_INV; }
  virtual void fill_0_03() { page0[0x03]= wrap_INV; }
  virtual void fill_0_04() { page0[0x04]= wrap_INV; }
  virtual void fill_0_05() { page0[0x05]= wrap_INV; }
  virtual void fill_0_06() { page0[0x06]= wrap_INV; }
  virtual void fill_0_07() { page0[0x07]= wrap_INV; }
  virtual void fill_0_08() { page0[0x08]= wrap_INV; }
  virtual void fill_0_09() { page0[0x09]= wrap_INV; }
  virtual void fill_0_0a() { page0[0x0a]= wrap_INV; }
  virtual void fill_0_0b() { page0[0x0b]= wrap_INV; }
  virtual void fill_0_0c() { page0[0x0c]= wrap_INV; }
  virtual void fill_0_0d() { page0[0x0d]= wrap_INV; }
  virtual void fill_0_0e() { page0[0x0e]= wrap_INV; }
  virtual void fill_0_0f() { page0[0x0f]= wrap_INV; }
  virtual void fill_0_10() { page0[0x10]= wrap_INV; }
  virtual void fill_0_11() { page0[0x11]= wrap_INV; }
  virtual void fill_0_12() { page0[0x12]= wrap_INV; }
  virtual void fill_0_13() { page0[0x13]= wrap_INV; }
  virtual void fill_0_14() { page0[0x14]= wrap_INV; }
  virtual void fill_0_15() { page0[0x15]= wrap_INV; }
  virtual void fill_0_16() { page0[0x16]= wrap_INV; }
  virtual void fill_0_17() { page0[0x17]= wrap_INV; }
  virtual void fill_0_18() { page0[0x18]= wrap_INV; }
  virtual void fill_0_19() { page0[0x19]= wrap_INV; }
  virtual void fill_0_1a() { page0[0x1a]= wrap_INV; }
  virtual void fill_0_1b() { page0[0x1b]= wrap_INV; }
  virtual void fill_0_1c() { page0[0x1c]= wrap_INV; }
  virtual void fill_0_1d() { page0[0x1d]= wrap_INV; }
  virtual void fill_0_1e() { page0[0x1e]= wrap_INV; }
  virtual void fill_0_1f() { page0[0x1f]= wrap_INV; }
  virtual void fill_0_20() { page0[0x20]= wrap_INV; }
  virtual void fill_0_21() { page0[0x21]= wrap_INV; }
  virtual void fill_0_22() { page0[0x22]= wrap_INV; }
  virtual void fill_0_23() { page0[0x23]= wrap_INV; }
  virtual void fill_0_24() { page0[0x24]= wrap_INV; }
  virtual void fill_0_25() { page0[0x25]= wrap_INV; }
  virtual void fill_0_26() { page0[0x26]= wrap_INV; }
  virtual void fill_0_27() { page0[0x27]= wrap_INV; }
  virtual void fill_0_28() { page0[0x28]= wrap_INV; }
  virtual void fill_0_29() { page0[0x29]= wrap_INV; }
  virtual void fill_0_2a() { page0[0x2a]= wrap_INV; }
  virtual void fill_0_2b() { page0[0x2b]= wrap_INV; }
  virtual void fill_0_2c() { page0[0x2c]= wrap_INV; }
  virtual void fill_0_2d() { page0[0x2d]= wrap_INV; }
  virtual void fill_0_2e() { page0[0x2e]= wrap_INV; }
  virtual void fill_0_2f() { page0[0x2f]= wrap_INV; }
  virtual void fill_0_30() { page0[0x30]= wrap_INV; }
  virtual void fill_0_31() { page0[0x31]= wrap_INV; }
  virtual void fill_0_32() { page0[0x32]= wrap_INV; }
  virtual void fill_0_33() { page0[0x33]= wrap_INV; }
  virtual void fill_0_34() { page0[0x34]= wrap_INV; }
  virtual void fill_0_35() { page0[0x35]= wrap_INV; }
  virtual void fill_0_36() { page0[0x36]= wrap_INV; }
  virtual void fill_0_37() { page0[0x37]= wrap_INV; }
  virtual void fill_0_38() { page0[0x38]= wrap_INV; }
  virtual void fill_0_39() { page0[0x39]= wrap_INV; }
  virtual void fill_0_3a() { page0[0x3a]= wrap_INV; }
  virtual void fill_0_3b() { page0[0x3b]= wrap_INV; }
  virtual void fill_0_3c() { page0[0x3c]= wrap_INV; }
  virtual void fill_0_3d() { page0[0x3d]= wrap_INV; }
  virtual void fill_0_3e() { page0[0x3e]= wrap_INV; }
  virtual void fill_0_3f() { page0[0x3f]= wrap_INV; }
  virtual void fill_0_40() { page0[0x40]= wrap_INV; }
  virtual void fill_0_41() { page0[0x41]= wrap_INV; }
  virtual void fill_0_42() { page0[0x42]= wrap_INV; }
  virtual void fill_0_43() { page0[0x43]= wrap_INV; }
  virtual void fill_0_44() { page0[0x44]= wrap_INV; }
  virtual void fill_0_45() { page0[0x45]= wrap_INV; }
  virtual void fill_0_46() { page0[0x46]= wrap_INV; }
  virtual void fill_0_47() { page0[0x47]= wrap_INV; }
  virtual void fill_0_48() { page0[0x48]= wrap_INV; }
  virtual void fill_0_49() { page0[0x49]= wrap_INV; }
  virtual void fill_0_4a() { page0[0x4a]= wrap_INV; }
  virtual void fill_0_4b() { page0[0x4b]= wrap_INV; }
  virtual void fill_0_4c() { page0[0x4c]= wrap_INV; }
  virtual void fill_0_4d() { page0[0x4d]= wrap_INV; }
  virtual void fill_0_4e() { page0[0x4e]= wrap_INV; }
  virtual void fill_0_4f() { page0[0x4f]= wrap_INV; }
  virtual void fill_0_50() { page0[0x50]= wrap_INV; }
  virtual void fill_0_51() { page0[0x51]= wrap_INV; }
  virtual void fill_0_52() { page0[0x52]= wrap_INV; }
  virtual void fill_0_53() { page0[0x53]= wrap_INV; }
  virtual void fill_0_54() { page0[0x54]= wrap_INV; }
  virtual void fill_0_55() { page0[0x55]= wrap_INV; }
  virtual void fill_0_56() { page0[0x56]= wrap_INV; }
  virtual void fill_0_57() { page0[0x57]= wrap_INV; }
  virtual void fill_0_58() { page0[0x58]= wrap_INV; }
  virtual void fill_0_59() { page0[0x59]= wrap_INV; }
  virtual void fill_0_5a() { page0[0x5a]= wrap_INV; }
  virtual void fill_0_5b() { page0[0x5b]= wrap_INV; }
  virtual void fill_0_5c() { page0[0x5c]= wrap_INV; }
  virtual void fill_0_5d() { page0[0x5d]= wrap_INV; }
  virtual void fill_0_5e() { page0[0x5e]= wrap_INV; }
  virtual void fill_0_5f() { page0[0x5f]= wrap_INV; }
  virtual void fill_0_60() { page0[0x60]= wrap_INV; }
  virtual void fill_0_61() { page0[0x61]= wrap_INV; }
  virtual void fill_0_62() { page0[0x62]= wrap_INV; }
  virtual void fill_0_63() { page0[0x63]= wrap_INV; }
  virtual void fill_0_64() { page0[0x64]= wrap_INV; }
  virtual void fill_0_65() { page0[0x65]= wrap_INV; }
  virtual void fill_0_66() { page0[0x66]= wrap_INV; }
  virtual void fill_0_67() { page0[0x67]= wrap_INV; }
  virtual void fill_0_68() { page0[0x68]= wrap_INV; }
  virtual void fill_0_69() { page0[0x69]= wrap_INV; }
  virtual void fill_0_6a() { page0[0x6a]= wrap_INV; }
  virtual void fill_0_6b() { page0[0x6b]= wrap_INV; }
  virtual void fill_0_6c() { page0[0x6c]= wrap_INV; }
  virtual void fill_0_6d() { page0[0x6d]= wrap_INV; }
  virtual void fill_0_6e() { page0[0x6e]= wrap_INV; }
  virtual void fill_0_6f() { page0[0x6f]= wrap_INV; }
  virtual void fill_0_70() { page0[0x70]= wrap_INV; }
  virtual void fill_0_71() { page0[0x71]= wrap_INV; }
  virtual void fill_0_72() { page0[0x72]= wrap_INV; }
  virtual void fill_0_73() { page0[0x73]= wrap_INV; }
  virtual void fill_0_74() { page0[0x74]= wrap_INV; }
  virtual void fill_0_75() { page0[0x75]= wrap_INV; }
  virtual void fill_0_76() { page0[0x76]= wrap_INV; }
  virtual void fill_0_77() { page0[0x77]= wrap_INV; }
  virtual void fill_0_78() { page0[0x78]= wrap_INV; }
  virtual void fill_0_79() { page0[0x79]= wrap_INV; }
  virtual void fill_0_7a() { page0[0x7a]= wrap_INV; }
  virtual void fill_0_7b() { page0[0x7b]= wrap_INV; }
  virtual void fill_0_7c() { page0[0x7c]= wrap_INV; }
  virtual void fill_0_7d() { page0[0x7d]= wrap_INV; }
  virtual void fill_0_7e() { page0[0x7e]= wrap_INV; }
  virtual void fill_0_7f() { page0[0x7f]= wrap_INV; }
  virtual void fill_0_80() { page0[0x80]= wrap_INV; }
  virtual void fill_0_81() { page0[0x81]= wrap_INV; }
  virtual void fill_0_82() { page0[0x82]= wrap_INV; }
  virtual void fill_0_83() { page0[0x83]= wrap_INV; }
  virtual void fill_0_84() { page0[0x84]= wrap_INV; }
  virtual void fill_0_85() { page0[0x85]= wrap_INV; }
  virtual void fill_0_86() { page0[0x86]= wrap_INV; }
  virtual void fill_0_87() { page0[0x87]= wrap_INV; }
  virtual void fill_0_88() { page0[0x88]= wrap_INV; }
  virtual void fill_0_89() { page0[0x89]= wrap_INV; }
  virtual void fill_0_8a() { page0[0x8a]= wrap_INV; }
  virtual void fill_0_8b() { page0[0x8b]= wrap_INV; }
  virtual void fill_0_8c() { page0[0x8c]= wrap_INV; }
  virtual void fill_0_8d() { page0[0x8d]= wrap_INV; }
  virtual void fill_0_8e() { page0[0x8e]= wrap_INV; }
  virtual void fill_0_8f() { page0[0x8f]= wrap_INV; }
  virtual void fill_0_90() { page0[0x90]= wrap_INV; }
  virtual void fill_0_91() { page0[0x91]= wrap_INV; }
  virtual void fill_0_92() { page0[0x92]= wrap_INV; }
  virtual void fill_0_93() { page0[0x93]= wrap_INV; }
  virtual void fill_0_94() { page0[0x94]= wrap_INV; }
  virtual void fill_0_95() { page0[0x95]= wrap_INV; }
  virtual void fill_0_96() { page0[0x96]= wrap_INV; }
  virtual void fill_0_97() { page0[0x97]= wrap_INV; }
  virtual void fill_0_98() { page0[0x98]= wrap_INV; }
  virtual void fill_0_99() { page0[0x99]= wrap_INV; }
  virtual void fill_0_9a() { page0[0x9a]= wrap_INV; }
  virtual void fill_0_9b() { page0[0x9b]= wrap_INV; }
  virtual void fill_0_9c() { page0[0x9c]= wrap_INV; }
  virtual void fill_0_9d() { page0[0x9d]= wrap_INV; }
  virtual void fill_0_9e() { page0[0x9e]= wrap_INV; }
  virtual void fill_0_9f() { page0[0x9f]= wrap_INV; }
  virtual void fill_0_a0() { page0[0xa0]= wrap_INV; }
  virtual void fill_0_a1() { page0[0xa1]= wrap_INV; }
  virtual void fill_0_a2() { page0[0xa2]= wrap_INV; }
  virtual void fill_0_a3() { page0[0xa3]= wrap_INV; }
  virtual void fill_0_a4() { page0[0xa4]= wrap_INV; }
  virtual void fill_0_a5() { page0[0xa5]= wrap_INV; }
  virtual void fill_0_a6() { page0[0xa6]= wrap_INV; }
  virtual void fill_0_a7() { page0[0xa7]= wrap_INV; }
  virtual void fill_0_a8() { page0[0xa8]= wrap_INV; }
  virtual void fill_0_a9() { page0[0xa9]= wrap_INV; }
  virtual void fill_0_aa() { page0[0xaa]= wrap_INV; }
  virtual void fill_0_ab() { page0[0xab]= wrap_INV; }
  virtual void fill_0_ac() { page0[0xac]= wrap_INV; }
  virtual void fill_0_ad() { page0[0xad]= wrap_INV; }
  virtual void fill_0_ae() { page0[0xae]= wrap_INV; }
  virtual void fill_0_af() { page0[0xaf]= wrap_INV; }
  virtual void fill_0_b0() { page0[0xb0]= wrap_INV; }
  virtual void fill_0_b1() { page0[0xb1]= wrap_INV; }
  virtual void fill_0_b2() { page0[0xb2]= wrap_INV; }
  virtual void fill_0_b3() { page0[0xb3]= wrap_INV; }
  virtual void fill_0_b4() { page0[0xb4]= wrap_INV; }
  virtual void fill_0_b5() { page0[0xb5]= wrap_INV; }
  virtual void fill_0_b6() { page0[0xb6]= wrap_INV; }
  virtual void fill_0_b7() { page0[0xb7]= wrap_INV; }
  virtual void fill_0_b8() { page0[0xb8]= wrap_INV; }
  virtual void fill_0_b9() { page0[0xb9]= wrap_INV; }
  virtual void fill_0_ba() { page0[0xba]= wrap_INV; }
  virtual void fill_0_bb() { page0[0xbb]= wrap_INV; }
  virtual void fill_0_bc() { page0[0xbc]= wrap_INV; }
  virtual void fill_0_bd() { page0[0xbd]= wrap_INV; }
  virtual void fill_0_be() { page0[0xbe]= wrap_INV; }
  virtual void fill_0_bf() { page0[0xbf]= wrap_INV; }
  virtual void fill_0_c0() { page0[0xc0]= wrap_INV; }
  virtual void fill_0_c1() { page0[0xc1]= wrap_INV; }
  virtual void fill_0_c2() { page0[0xc2]= wrap_INV; }
  virtual void fill_0_c3() { page0[0xc3]= wrap_INV; }
  virtual void fill_0_c4() { page0[0xc4]= wrap_INV; }
  virtual void fill_0_c5() { page0[0xc5]= wrap_INV; }
  virtual void fill_0_c6() { page0[0xc6]= wrap_INV; }
  virtual void fill_0_c7() { page0[0xc7]= wrap_INV; }
  virtual void fill_0_c8() { page0[0xc8]= wrap_INV; }
  virtual void fill_0_c9() { page0[0xc9]= wrap_INV; }
  virtual void fill_0_ca() { page0[0xca]= wrap_INV; }
  virtual void fill_0_cb() { page0[0xcb]= wrap_INV; }
  virtual void fill_0_cc() { page0[0xcc]= wrap_INV; }
  virtual void fill_0_cd() { page0[0xcd]= wrap_INV; }
  virtual void fill_0_ce() { page0[0xce]= wrap_INV; }
  virtual void fill_0_cf() { page0[0xcf]= wrap_INV; }
  virtual void fill_0_d0() { page0[0xd0]= wrap_INV; }
  virtual void fill_0_d1() { page0[0xd1]= wrap_INV; }
  virtual void fill_0_d2() { page0[0xd2]= wrap_INV; }
  virtual void fill_0_d3() { page0[0xd3]= wrap_INV; }
  virtual void fill_0_d4() { page0[0xd4]= wrap_INV; }
  virtual void fill_0_d5() { page0[0xd5]= wrap_INV; }
  virtual void fill_0_d6() { page0[0xd6]= wrap_INV; }
  virtual void fill_0_d7() { page0[0xd7]= wrap_INV; }
  virtual void fill_0_d8() { page0[0xd8]= wrap_INV; }
  virtual void fill_0_d9() { page0[0xd9]= wrap_INV; }
  virtual void fill_0_da() { page0[0xda]= wrap_INV; }
  virtual void fill_0_db() { page0[0xdb]= wrap_INV; }
  virtual void fill_0_dc() { page0[0xdc]= wrap_INV; }
  virtual void fill_0_dd() { page0[0xdd]= wrap_INV; }
  virtual void fill_0_de() { page0[0xde]= wrap_INV; }
  virtual void fill_0_df() { page0[0xdf]= wrap_INV; }
  virtual void fill_0_e0() { page0[0xe0]= wrap_INV; }
  virtual void fill_0_e1() { page0[0xe1]= wrap_INV; }
  virtual void fill_0_e2() { page0[0xe2]= wrap_INV; }
  virtual void fill_0_e3() { page0[0xe3]= wrap_INV; }
  virtual void fill_0_e4() { page0[0xe4]= wrap_INV; }
  virtual void fill_0_e5() { page0[0xe5]= wrap_INV; }
  virtual void fill_0_e6() { page0[0xe6]= wrap_INV; }
  virtual void fill_0_e7() { page0[0xe7]= wrap_INV; }
  virtual void fill_0_e8() { page0[0xe8]= wrap_INV; }
  virtual void fill_0_e9() { page0[0xe9]= wrap_INV; }
  virtual void fill_0_ea() { page0[0xea]= wrap_INV; }
  virtual void fill_0_eb() { page0[0xeb]= wrap_INV; }
  virtual void fill_0_ec() { page0[0xec]= wrap_INV; }
  virtual void fill_0_ed() { page0[0xed]= wrap_INV; }
  virtual void fill_0_ee() { page0[0xee]= wrap_INV; }
  virtual void fill_0_ef() { page0[0xef]= wrap_INV; }
  virtual void fill_0_f0() { page0[0xf0]= wrap_INV; }
  virtual void fill_0_f1() { page0[0xf1]= wrap_INV; }
  virtual void fill_0_f2() { page0[0xf2]= wrap_INV; }
  virtual void fill_0_f3() { page0[0xf3]= wrap_INV; }
  virtual void fill_0_f4() { page0[0xf4]= wrap_INV; }
  virtual void fill_0_f5() { page0[0xf5]= wrap_INV; }
  virtual void fill_0_f6() { page0[0xf6]= wrap_INV; }
  virtual void fill_0_f7() { page0[0xf7]= wrap_INV; }
  virtual void fill_0_f8() { page0[0xf8]= wrap_INV; }
  virtual void fill_0_f9() { page0[0xf9]= wrap_INV; }
  virtual void fill_0_fa() { page0[0xfa]= wrap_INV; }
  virtual void fill_0_fb() { page0[0xfb]= wrap_INV; }
  virtual void fill_0_fc() { page0[0xfc]= wrap_INV; }
  virtual void fill_0_fd() { page0[0xfd]= wrap_INV; }
  virtual void fill_0_fe() { page0[0xfe]= wrap_INV; }
  virtual void fill_0_ff() { page0[0xff]= wrap_INV; }

  virtual void fill_0x18_00() { page0x18[0x00]= wrap_TRAP; }
  virtual void fill_0x18_01() { page0x18[0x01]= wrap_TRAP; }
  virtual void fill_0x18_02() { page0x18[0x02]= wrap_TRAP; }
  virtual void fill_0x18_03() { page0x18[0x03]= wrap_TRAP; }
  virtual void fill_0x18_04() { page0x18[0x04]= wrap_TRAP; }
  virtual void fill_0x18_05() { page0x18[0x05]= wrap_TRAP; }
  virtual void fill_0x18_06() { page0x18[0x06]= wrap_TRAP; }
  virtual void fill_0x18_07() { page0x18[0x07]= wrap_TRAP; }
  virtual void fill_0x18_08() { page0x18[0x08]= wrap_TRAP; }
  virtual void fill_0x18_09() { page0x18[0x09]= wrap_TRAP; }
  virtual void fill_0x18_0a() { page0x18[0x0a]= wrap_TRAP; }
  virtual void fill_0x18_0b() { page0x18[0x0b]= wrap_TRAP; }
  virtual void fill_0x18_0c() { page0x18[0x0c]= wrap_TRAP; }
  virtual void fill_0x18_0d() { page0x18[0x0d]= wrap_TRAP; }
  virtual void fill_0x18_0e() { page0x18[0x0e]= wrap_TRAP; }
  virtual void fill_0x18_0f() { page0x18[0x0f]= wrap_TRAP; }
  virtual void fill_0x18_10() { page0x18[0x10]= wrap_TRAP; }
  virtual void fill_0x18_11() { page0x18[0x11]= wrap_TRAP; }
  virtual void fill_0x18_12() { page0x18[0x12]= wrap_TRAP; }
  virtual void fill_0x18_13() { page0x18[0x13]= wrap_TRAP; }
  virtual void fill_0x18_14() { page0x18[0x14]= wrap_TRAP; }
  virtual void fill_0x18_15() { page0x18[0x15]= wrap_TRAP; }
  virtual void fill_0x18_16() { page0x18[0x16]= wrap_TRAP; }
  virtual void fill_0x18_17() { page0x18[0x17]= wrap_TRAP; }
  virtual void fill_0x18_18() { page0x18[0x18]= wrap_TRAP; }
  virtual void fill_0x18_19() { page0x18[0x19]= wrap_TRAP; }
  virtual void fill_0x18_1a() { page0x18[0x1a]= wrap_TRAP; }
  virtual void fill_0x18_1b() { page0x18[0x1b]= wrap_TRAP; }
  virtual void fill_0x18_1c() { page0x18[0x1c]= wrap_TRAP; }
  virtual void fill_0x18_1d() { page0x18[0x1d]= wrap_TRAP; }
  virtual void fill_0x18_1e() { page0x18[0x1e]= wrap_TRAP; }
  virtual void fill_0x18_1f() { page0x18[0x1f]= wrap_TRAP; }
  virtual void fill_0x18_20() { page0x18[0x20]= wrap_TRAP; }
  virtual void fill_0x18_21() { page0x18[0x21]= wrap_TRAP; }
  virtual void fill_0x18_22() { page0x18[0x22]= wrap_TRAP; }
  virtual void fill_0x18_23() { page0x18[0x23]= wrap_TRAP; }
  virtual void fill_0x18_24() { page0x18[0x24]= wrap_TRAP; }
  virtual void fill_0x18_25() { page0x18[0x25]= wrap_TRAP; }
  virtual void fill_0x18_26() { page0x18[0x26]= wrap_TRAP; }
  virtual void fill_0x18_27() { page0x18[0x27]= wrap_TRAP; }
  virtual void fill_0x18_28() { page0x18[0x28]= wrap_TRAP; }
  virtual void fill_0x18_29() { page0x18[0x29]= wrap_TRAP; }
  virtual void fill_0x18_2a() { page0x18[0x2a]= wrap_TRAP; }
  virtual void fill_0x18_2b() { page0x18[0x2b]= wrap_TRAP; }
  virtual void fill_0x18_2c() { page0x18[0x2c]= wrap_TRAP; }
  virtual void fill_0x18_2d() { page0x18[0x2d]= wrap_TRAP; }
  virtual void fill_0x18_2e() { page0x18[0x2e]= wrap_TRAP; }
  virtual void fill_0x18_2f() { page0x18[0x2f]= wrap_TRAP; }
  virtual void fill_0x18_30() { page0x18[0x30]= wrap_TRAP; }
  virtual void fill_0x18_31() { page0x18[0x31]= wrap_TRAP; }
  virtual void fill_0x18_32() { page0x18[0x32]= wrap_TRAP; }
  virtual void fill_0x18_33() { page0x18[0x33]= wrap_TRAP; }
  virtual void fill_0x18_34() { page0x18[0x34]= wrap_TRAP; }
  virtual void fill_0x18_35() { page0x18[0x35]= wrap_TRAP; }
  virtual void fill_0x18_36() { page0x18[0x36]= wrap_TRAP; }
  virtual void fill_0x18_37() { page0x18[0x37]= wrap_TRAP; }
  virtual void fill_0x18_38() { page0x18[0x38]= wrap_TRAP; }
  virtual void fill_0x18_39() { page0x18[0x39]= wrap_TRAP; }
  virtual void fill_0x18_3a() { page0x18[0x3a]= wrap_TRAP; }
  virtual void fill_0x18_3b() { page0x18[0x3b]= wrap_TRAP; }
  virtual void fill_0x18_3c() { page0x18[0x3c]= wrap_TRAP; }
  virtual void fill_0x18_3d() { page0x18[0x3d]= wrap_TRAP; }
  virtual void fill_0x18_3e() { page0x18[0x3e]= wrap_TRAP; }
  virtual void fill_0x18_3f() { page0x18[0x3f]= wrap_TRAP; }
  virtual void fill_0x18_40() { page0x18[0x40]= wrap_TRAP; }
  virtual void fill_0x18_41() { page0x18[0x41]= wrap_TRAP; }
  virtual void fill_0x18_42() { page0x18[0x42]= wrap_TRAP; }
  virtual void fill_0x18_43() { page0x18[0x43]= wrap_TRAP; }
  virtual void fill_0x18_44() { page0x18[0x44]= wrap_TRAP; }
  virtual void fill_0x18_45() { page0x18[0x45]= wrap_TRAP; }
  virtual void fill_0x18_46() { page0x18[0x46]= wrap_TRAP; }
  virtual void fill_0x18_47() { page0x18[0x47]= wrap_TRAP; }
  virtual void fill_0x18_48() { page0x18[0x48]= wrap_TRAP; }
  virtual void fill_0x18_49() { page0x18[0x49]= wrap_TRAP; }
  virtual void fill_0x18_4a() { page0x18[0x4a]= wrap_TRAP; }
  virtual void fill_0x18_4b() { page0x18[0x4b]= wrap_TRAP; }
  virtual void fill_0x18_4c() { page0x18[0x4c]= wrap_TRAP; }
  virtual void fill_0x18_4d() { page0x18[0x4d]= wrap_TRAP; }
  virtual void fill_0x18_4e() { page0x18[0x4e]= wrap_TRAP; }
  virtual void fill_0x18_4f() { page0x18[0x4f]= wrap_TRAP; }
  virtual void fill_0x18_50() { page0x18[0x50]= wrap_TRAP; }
  virtual void fill_0x18_51() { page0x18[0x51]= wrap_TRAP; }
  virtual void fill_0x18_52() { page0x18[0x52]= wrap_TRAP; }
  virtual void fill_0x18_53() { page0x18[0x53]= wrap_TRAP; }
  virtual void fill_0x18_54() { page0x18[0x54]= wrap_TRAP; }
  virtual void fill_0x18_55() { page0x18[0x55]= wrap_TRAP; }
  virtual void fill_0x18_56() { page0x18[0x56]= wrap_TRAP; }
  virtual void fill_0x18_57() { page0x18[0x57]= wrap_TRAP; }
  virtual void fill_0x18_58() { page0x18[0x58]= wrap_TRAP; }
  virtual void fill_0x18_59() { page0x18[0x59]= wrap_TRAP; }
  virtual void fill_0x18_5a() { page0x18[0x5a]= wrap_TRAP; }
  virtual void fill_0x18_5b() { page0x18[0x5b]= wrap_TRAP; }
  virtual void fill_0x18_5c() { page0x18[0x5c]= wrap_TRAP; }
  virtual void fill_0x18_5d() { page0x18[0x5d]= wrap_TRAP; }
  virtual void fill_0x18_5e() { page0x18[0x5e]= wrap_TRAP; }
  virtual void fill_0x18_5f() { page0x18[0x5f]= wrap_TRAP; }
  virtual void fill_0x18_60() { page0x18[0x60]= wrap_TRAP; }
  virtual void fill_0x18_61() { page0x18[0x61]= wrap_TRAP; }
  virtual void fill_0x18_62() { page0x18[0x62]= wrap_TRAP; }
  virtual void fill_0x18_63() { page0x18[0x63]= wrap_TRAP; }
  virtual void fill_0x18_64() { page0x18[0x64]= wrap_TRAP; }
  virtual void fill_0x18_65() { page0x18[0x65]= wrap_TRAP; }
  virtual void fill_0x18_66() { page0x18[0x66]= wrap_TRAP; }
  virtual void fill_0x18_67() { page0x18[0x67]= wrap_TRAP; }
  virtual void fill_0x18_68() { page0x18[0x68]= wrap_TRAP; }
  virtual void fill_0x18_69() { page0x18[0x69]= wrap_TRAP; }
  virtual void fill_0x18_6a() { page0x18[0x6a]= wrap_TRAP; }
  virtual void fill_0x18_6b() { page0x18[0x6b]= wrap_TRAP; }
  virtual void fill_0x18_6c() { page0x18[0x6c]= wrap_TRAP; }
  virtual void fill_0x18_6d() { page0x18[0x6d]= wrap_TRAP; }
  virtual void fill_0x18_6e() { page0x18[0x6e]= wrap_TRAP; }
  virtual void fill_0x18_6f() { page0x18[0x6f]= wrap_TRAP; }
  virtual void fill_0x18_70() { page0x18[0x70]= wrap_TRAP; }
  virtual void fill_0x18_71() { page0x18[0x71]= wrap_TRAP; }
  virtual void fill_0x18_72() { page0x18[0x72]= wrap_TRAP; }
  virtual void fill_0x18_73() { page0x18[0x73]= wrap_TRAP; }
  virtual void fill_0x18_74() { page0x18[0x74]= wrap_TRAP; }
  virtual void fill_0x18_75() { page0x18[0x75]= wrap_TRAP; }
  virtual void fill_0x18_76() { page0x18[0x76]= wrap_TRAP; }
  virtual void fill_0x18_77() { page0x18[0x77]= wrap_TRAP; }
  virtual void fill_0x18_78() { page0x18[0x78]= wrap_TRAP; }
  virtual void fill_0x18_79() { page0x18[0x79]= wrap_TRAP; }
  virtual void fill_0x18_7a() { page0x18[0x7a]= wrap_TRAP; }
  virtual void fill_0x18_7b() { page0x18[0x7b]= wrap_TRAP; }
  virtual void fill_0x18_7c() { page0x18[0x7c]= wrap_TRAP; }
  virtual void fill_0x18_7d() { page0x18[0x7d]= wrap_TRAP; }
  virtual void fill_0x18_7e() { page0x18[0x7e]= wrap_TRAP; }
  virtual void fill_0x18_7f() { page0x18[0x7f]= wrap_TRAP; }
  virtual void fill_0x18_80() { page0x18[0x80]= wrap_TRAP; }
  virtual void fill_0x18_81() { page0x18[0x81]= wrap_TRAP; }
  virtual void fill_0x18_82() { page0x18[0x82]= wrap_TRAP; }
  virtual void fill_0x18_83() { page0x18[0x83]= wrap_TRAP; }
  virtual void fill_0x18_84() { page0x18[0x84]= wrap_TRAP; }
  virtual void fill_0x18_85() { page0x18[0x85]= wrap_TRAP; }
  virtual void fill_0x18_86() { page0x18[0x86]= wrap_TRAP; }
  virtual void fill_0x18_87() { page0x18[0x87]= wrap_TRAP; }
  virtual void fill_0x18_88() { page0x18[0x88]= wrap_TRAP; }
  virtual void fill_0x18_89() { page0x18[0x89]= wrap_TRAP; }
  virtual void fill_0x18_8a() { page0x18[0x8a]= wrap_TRAP; }
  virtual void fill_0x18_8b() { page0x18[0x8b]= wrap_TRAP; }
  virtual void fill_0x18_8c() { page0x18[0x8c]= wrap_TRAP; }
  virtual void fill_0x18_8d() { page0x18[0x8d]= wrap_TRAP; }
  virtual void fill_0x18_8e() { page0x18[0x8e]= wrap_TRAP; }
  virtual void fill_0x18_8f() { page0x18[0x8f]= wrap_TRAP; }
  virtual void fill_0x18_90() { page0x18[0x90]= wrap_TRAP; }
  virtual void fill_0x18_91() { page0x18[0x91]= wrap_TRAP; }
  virtual void fill_0x18_92() { page0x18[0x92]= wrap_TRAP; }
  virtual void fill_0x18_93() { page0x18[0x93]= wrap_TRAP; }
  virtual void fill_0x18_94() { page0x18[0x94]= wrap_TRAP; }
  virtual void fill_0x18_95() { page0x18[0x95]= wrap_TRAP; }
  virtual void fill_0x18_96() { page0x18[0x96]= wrap_TRAP; }
  virtual void fill_0x18_97() { page0x18[0x97]= wrap_TRAP; }
  virtual void fill_0x18_98() { page0x18[0x98]= wrap_TRAP; }
  virtual void fill_0x18_99() { page0x18[0x99]= wrap_TRAP; }
  virtual void fill_0x18_9a() { page0x18[0x9a]= wrap_TRAP; }
  virtual void fill_0x18_9b() { page0x18[0x9b]= wrap_TRAP; }
  virtual void fill_0x18_9c() { page0x18[0x9c]= wrap_TRAP; }
  virtual void fill_0x18_9d() { page0x18[0x9d]= wrap_TRAP; }
  virtual void fill_0x18_9e() { page0x18[0x9e]= wrap_TRAP; }
  virtual void fill_0x18_9f() { page0x18[0x9f]= wrap_TRAP; }
  virtual void fill_0x18_a0() { page0x18[0xa0]= wrap_TRAP; }
  virtual void fill_0x18_a1() { page0x18[0xa1]= wrap_TRAP; }
  virtual void fill_0x18_a2() { page0x18[0xa2]= wrap_TRAP; }
  virtual void fill_0x18_a3() { page0x18[0xa3]= wrap_TRAP; }
  virtual void fill_0x18_a4() { page0x18[0xa4]= wrap_TRAP; }
  virtual void fill_0x18_a5() { page0x18[0xa5]= wrap_TRAP; }
  virtual void fill_0x18_a6() { page0x18[0xa6]= wrap_TRAP; }
  virtual void fill_0x18_a7() { page0x18[0xa7]= wrap_TRAP; }
  virtual void fill_0x18_a8() { page0x18[0xa8]= wrap_TRAP; }
  virtual void fill_0x18_a9() { page0x18[0xa9]= wrap_TRAP; }
  virtual void fill_0x18_aa() { page0x18[0xaa]= wrap_TRAP; }
  virtual void fill_0x18_ab() { page0x18[0xab]= wrap_TRAP; }
  virtual void fill_0x18_ac() { page0x18[0xac]= wrap_TRAP; }
  virtual void fill_0x18_ad() { page0x18[0xad]= wrap_TRAP; }
  virtual void fill_0x18_ae() { page0x18[0xae]= wrap_TRAP; }
  virtual void fill_0x18_af() { page0x18[0xaf]= wrap_TRAP; }
  virtual void fill_0x18_b0() { page0x18[0xb0]= wrap_TRAP; }
  virtual void fill_0x18_b1() { page0x18[0xb1]= wrap_TRAP; }
  virtual void fill_0x18_b2() { page0x18[0xb2]= wrap_TRAP; }
  virtual void fill_0x18_b3() { page0x18[0xb3]= wrap_TRAP; }
  virtual void fill_0x18_b4() { page0x18[0xb4]= wrap_TRAP; }
  virtual void fill_0x18_b5() { page0x18[0xb5]= wrap_TRAP; }
  virtual void fill_0x18_b6() { page0x18[0xb6]= wrap_TRAP; }
  virtual void fill_0x18_b7() { page0x18[0xb7]= wrap_TRAP; }
  virtual void fill_0x18_b8() { page0x18[0xb8]= wrap_TRAP; }
  virtual void fill_0x18_b9() { page0x18[0xb9]= wrap_TRAP; }
  virtual void fill_0x18_ba() { page0x18[0xba]= wrap_TRAP; }
  virtual void fill_0x18_bb() { page0x18[0xbb]= wrap_TRAP; }
  virtual void fill_0x18_bc() { page0x18[0xbc]= wrap_TRAP; }
  virtual void fill_0x18_bd() { page0x18[0xbd]= wrap_TRAP; }
  virtual void fill_0x18_be() { page0x18[0xbe]= wrap_TRAP; }
  virtual void fill_0x18_bf() { page0x18[0xbf]= wrap_TRAP; }
  virtual void fill_0x18_c0() { page0x18[0xc0]= wrap_TRAP; }
  virtual void fill_0x18_c1() { page0x18[0xc1]= wrap_TRAP; }
  virtual void fill_0x18_c2() { page0x18[0xc2]= wrap_TRAP; }
  virtual void fill_0x18_c3() { page0x18[0xc3]= wrap_TRAP; }
  virtual void fill_0x18_c4() { page0x18[0xc4]= wrap_TRAP; }
  virtual void fill_0x18_c5() { page0x18[0xc5]= wrap_TRAP; }
  virtual void fill_0x18_c6() { page0x18[0xc6]= wrap_TRAP; }
  virtual void fill_0x18_c7() { page0x18[0xc7]= wrap_TRAP; }
  virtual void fill_0x18_c8() { page0x18[0xc8]= wrap_TRAP; }
  virtual void fill_0x18_c9() { page0x18[0xc9]= wrap_TRAP; }
  virtual void fill_0x18_ca() { page0x18[0xca]= wrap_TRAP; }
  virtual void fill_0x18_cb() { page0x18[0xcb]= wrap_TRAP; }
  virtual void fill_0x18_cc() { page0x18[0xcc]= wrap_TRAP; }
  virtual void fill_0x18_cd() { page0x18[0xcd]= wrap_TRAP; }
  virtual void fill_0x18_ce() { page0x18[0xce]= wrap_TRAP; }
  virtual void fill_0x18_cf() { page0x18[0xcf]= wrap_TRAP; }
  virtual void fill_0x18_d0() { page0x18[0xd0]= wrap_TRAP; }
  virtual void fill_0x18_d1() { page0x18[0xd1]= wrap_TRAP; }
  virtual void fill_0x18_d2() { page0x18[0xd2]= wrap_TRAP; }
  virtual void fill_0x18_d3() { page0x18[0xd3]= wrap_TRAP; }
  virtual void fill_0x18_d4() { page0x18[0xd4]= wrap_TRAP; }
  virtual void fill_0x18_d5() { page0x18[0xd5]= wrap_TRAP; }
  virtual void fill_0x18_d6() { page0x18[0xd6]= wrap_TRAP; }
  virtual void fill_0x18_d7() { page0x18[0xd7]= wrap_TRAP; }
  virtual void fill_0x18_d8() { page0x18[0xd8]= wrap_TRAP; }
  virtual void fill_0x18_d9() { page0x18[0xd9]= wrap_TRAP; }
  virtual void fill_0x18_da() { page0x18[0xda]= wrap_TRAP; }
  virtual void fill_0x18_db() { page0x18[0xdb]= wrap_TRAP; }
  virtual void fill_0x18_dc() { page0x18[0xdc]= wrap_TRAP; }
  virtual void fill_0x18_dd() { page0x18[0xdd]= wrap_TRAP; }
  virtual void fill_0x18_de() { page0x18[0xde]= wrap_TRAP; }
  virtual void fill_0x18_df() { page0x18[0xdf]= wrap_TRAP; }
  virtual void fill_0x18_e0() { page0x18[0xe0]= wrap_TRAP; }
  virtual void fill_0x18_e1() { page0x18[0xe1]= wrap_TRAP; }
  virtual void fill_0x18_e2() { page0x18[0xe2]= wrap_TRAP; }
  virtual void fill_0x18_e3() { page0x18[0xe3]= wrap_TRAP; }
  virtual void fill_0x18_e4() { page0x18[0xe4]= wrap_TRAP; }
  virtual void fill_0x18_e5() { page0x18[0xe5]= wrap_TRAP; }
  virtual void fill_0x18_e6() { page0x18[0xe6]= wrap_TRAP; }
  virtual void fill_0x18_e7() { page0x18[0xe7]= wrap_TRAP; }
  virtual void fill_0x18_e8() { page0x18[0xe8]= wrap_TRAP; }
  virtual void fill_0x18_e9() { page0x18[0xe9]= wrap_TRAP; }
  virtual void fill_0x18_ea() { page0x18[0xea]= wrap_TRAP; }
  virtual void fill_0x18_eb() { page0x18[0xeb]= wrap_TRAP; }
  virtual void fill_0x18_ec() { page0x18[0xec]= wrap_TRAP; }
  virtual void fill_0x18_ed() { page0x18[0xed]= wrap_TRAP; }
  virtual void fill_0x18_ee() { page0x18[0xee]= wrap_TRAP; }
  virtual void fill_0x18_ef() { page0x18[0xef]= wrap_TRAP; }
  virtual void fill_0x18_f0() { page0x18[0xf0]= wrap_TRAP; }
  virtual void fill_0x18_f1() { page0x18[0xf1]= wrap_TRAP; }
  virtual void fill_0x18_f2() { page0x18[0xf2]= wrap_TRAP; }
  virtual void fill_0x18_f3() { page0x18[0xf3]= wrap_TRAP; }
  virtual void fill_0x18_f4() { page0x18[0xf4]= wrap_TRAP; }
  virtual void fill_0x18_f5() { page0x18[0xf5]= wrap_TRAP; }
  virtual void fill_0x18_f6() { page0x18[0xf6]= wrap_TRAP; }
  virtual void fill_0x18_f7() { page0x18[0xf7]= wrap_TRAP; }
  virtual void fill_0x18_f8() { page0x18[0xf8]= wrap_TRAP; }
  virtual void fill_0x18_f9() { page0x18[0xf9]= wrap_TRAP; }
  virtual void fill_0x18_fa() { page0x18[0xfa]= wrap_TRAP; }
  virtual void fill_0x18_fb() { page0x18[0xfb]= wrap_TRAP; }
  virtual void fill_0x18_fc() { page0x18[0xfc]= wrap_TRAP; }
  virtual void fill_0x18_fd() { page0x18[0xfd]= wrap_TRAP; }
  virtual void fill_0x18_fe() { page0x18[0xfe]= wrap_TRAP; }
  virtual void fill_0x18_ff() { page0x18[0xff]= wrap_TRAP; }
};

#include "wdecls.h"

class cl_12wrap: public cl_wrap
{
public:
  cl_12wrap(): cl_wrap() {}

#include "wfills.h"
};


#endif

/* End of m68hc12.src/hcwrap.cc */
