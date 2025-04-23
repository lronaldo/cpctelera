/*
 * Simulator of microcontrollers (edwrap.cc)
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

#include "edwrap.h"

int instruction_wrapper_ed_none(class cl_uc *uc, t_mem code) { return resINV_INST; }

int instruction_wrapper_ed_00(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_00(code); }
int instruction_wrapper_ed_01(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_01(code); }
int instruction_wrapper_ed_02(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_02(code); }
int instruction_wrapper_ed_03(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_03(code); }
int instruction_wrapper_ed_04(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_04(code); }
int instruction_wrapper_ed_05(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_05(code); }
int instruction_wrapper_ed_06(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_06(code); }
int instruction_wrapper_ed_07(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_07(code); }
int instruction_wrapper_ed_08(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_08(code); }
int instruction_wrapper_ed_09(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_09(code); }
int instruction_wrapper_ed_0a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_0a(code); }
int instruction_wrapper_ed_0b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_0b(code); }
int instruction_wrapper_ed_0c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_0c(code); }
int instruction_wrapper_ed_0d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_0d(code); }
int instruction_wrapper_ed_0e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_0e(code); }
int instruction_wrapper_ed_0f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_0f(code); }

int instruction_wrapper_ed_10(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_10(code); }
int instruction_wrapper_ed_11(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_11(code); }
int instruction_wrapper_ed_12(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_12(code); }
int instruction_wrapper_ed_13(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_13(code); }
int instruction_wrapper_ed_14(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_14(code); }
int instruction_wrapper_ed_15(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_15(code); }
int instruction_wrapper_ed_16(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_16(code); }
int instruction_wrapper_ed_17(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_17(code); }
int instruction_wrapper_ed_18(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_18(code); }
int instruction_wrapper_ed_19(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_19(code); }
int instruction_wrapper_ed_1a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_1a(code); }
int instruction_wrapper_ed_1b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_1b(code); }
int instruction_wrapper_ed_1c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_1c(code); }
int instruction_wrapper_ed_1d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_1d(code); }
int instruction_wrapper_ed_1e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_1e(code); }
int instruction_wrapper_ed_1f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_1f(code); }

int instruction_wrapper_ed_21(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_21(code); }
int instruction_wrapper_ed_23(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_23(code); }
int instruction_wrapper_ed_24(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_24(code); }
int instruction_wrapper_ed_25(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_25(code); }
int instruction_wrapper_ed_26(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_26(code); }
int instruction_wrapper_ed_27(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_27(code); }
int instruction_wrapper_ed_28(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_28(code); }
int instruction_wrapper_ed_29(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_29(code); }
int instruction_wrapper_ed_2a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_2a(code); }
int instruction_wrapper_ed_2b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_2b(code); }
int instruction_wrapper_ed_2c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_2c(code); }
int instruction_wrapper_ed_2d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_2d(code); }
int instruction_wrapper_ed_2e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_2e(code); }
int instruction_wrapper_ed_2f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_2f(code); }

int instruction_wrapper_ed_31(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_31(code); }
int instruction_wrapper_ed_33(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_33(code); }
int instruction_wrapper_ed_34(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_34(code); }
int instruction_wrapper_ed_35(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_35(code); }
int instruction_wrapper_ed_36(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_36(code); }
int instruction_wrapper_ed_37(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_37(code); }
int instruction_wrapper_ed_38(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_38(code); }
int instruction_wrapper_ed_39(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_39(code); }
int instruction_wrapper_ed_3a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_3a(code); }
int instruction_wrapper_ed_3b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_3b(code); }
int instruction_wrapper_ed_3c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_3c(code); }
int instruction_wrapper_ed_3d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_3d(code); }
int instruction_wrapper_ed_3e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_3e(code); }
int instruction_wrapper_ed_3f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_3f(code); }

int instruction_wrapper_ed_40(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_40(code); }
int instruction_wrapper_ed_41(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_41(code); }
int instruction_wrapper_ed_42(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_42(code); }
int instruction_wrapper_ed_43(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_43(code); }
int instruction_wrapper_ed_44(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_44(code); }
int instruction_wrapper_ed_45(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_45(code); }
int instruction_wrapper_ed_46(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_46(code); }
int instruction_wrapper_ed_47(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_47(code); }
int instruction_wrapper_ed_48(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_48(code); }
int instruction_wrapper_ed_49(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_49(code); }
int instruction_wrapper_ed_4a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_4a(code); }
int instruction_wrapper_ed_4b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_4b(code); }
int instruction_wrapper_ed_4c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_4c(code); }
int instruction_wrapper_ed_4d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_4d(code); }
int instruction_wrapper_ed_4e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_4e(code); }
int instruction_wrapper_ed_4f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_4f(code); }

int instruction_wrapper_ed_50(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_50(code); }
int instruction_wrapper_ed_51(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_51(code); }
int instruction_wrapper_ed_52(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_52(code); }
int instruction_wrapper_ed_53(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_53(code); }
int instruction_wrapper_ed_54(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_54(code); }
int instruction_wrapper_ed_55(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_55(code); }
int instruction_wrapper_ed_56(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_56(code); }
int instruction_wrapper_ed_57(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_57(code); }
int instruction_wrapper_ed_58(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_58(code); }
int instruction_wrapper_ed_59(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_59(code); }
int instruction_wrapper_ed_5a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_5a(code); }
int instruction_wrapper_ed_5b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_5b(code); }
int instruction_wrapper_ed_5d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_5d(code); }
int instruction_wrapper_ed_5e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_5e(code); }
int instruction_wrapper_ed_5f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_5f(code); }

int instruction_wrapper_ed_61(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_61(code); }
int instruction_wrapper_ed_62(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_62(code); }
int instruction_wrapper_ed_63(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_63(code); }
int instruction_wrapper_ed_64(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_64(code); }
int instruction_wrapper_ed_65(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_65(code); }
int instruction_wrapper_ed_66(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_66(code); }
int instruction_wrapper_ed_67(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_67(code); }
int instruction_wrapper_ed_69(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_69(code); }
int instruction_wrapper_ed_6a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_6a(code); }
int instruction_wrapper_ed_6b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_6b(code); }
int instruction_wrapper_ed_6c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_6c(code); }
int instruction_wrapper_ed_6d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_6d(code); }
int instruction_wrapper_ed_6e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_6e(code); }
int instruction_wrapper_ed_6f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_6f(code); }

int instruction_wrapper_ed_72(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_72(code); }
int instruction_wrapper_ed_73(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_73(code); }
int instruction_wrapper_ed_74(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_74(code); }
int instruction_wrapper_ed_75(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_75(code); }
int instruction_wrapper_ed_76(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_76(code); }
int instruction_wrapper_ed_77(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_77(code); }
int instruction_wrapper_ed_7a(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_7a(code); }
int instruction_wrapper_ed_7b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_7b(code); }
int instruction_wrapper_ed_7c(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_7c(code); }
int instruction_wrapper_ed_7d(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_7d(code); }
int instruction_wrapper_ed_7e(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_7e(code); }
int instruction_wrapper_ed_7f(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_7f(code); }

int instruction_wrapper_ed_80(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_80(code); }
int instruction_wrapper_ed_83(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_83(code); }
int instruction_wrapper_ed_88(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_88(code); }
int instruction_wrapper_ed_8b(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_8b(code); }

int instruction_wrapper_ed_90(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_90(code); }
int instruction_wrapper_ed_98(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_98(code); }

int instruction_wrapper_ed_a0(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_a0(code); }
int instruction_wrapper_ed_a2(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_a2(code); }
int instruction_wrapper_ed_a3(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_a3(code); }
int instruction_wrapper_ed_a4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_a4(code); }
int instruction_wrapper_ed_a5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_a5(code); }
int instruction_wrapper_ed_a8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_a8(code); }
int instruction_wrapper_ed_aa(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_aa(code); }
int instruction_wrapper_ed_ab(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_ab(code); }
int instruction_wrapper_ed_ac(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_ac(code); }

int instruction_wrapper_ed_b0(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_b0(code); }
int instruction_wrapper_ed_b1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_b1(code); }
int instruction_wrapper_ed_b2(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_b2(code); }
int instruction_wrapper_ed_b3(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_b3(code); }
int instruction_wrapper_ed_b4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_b4(code); }
int instruction_wrapper_ed_b5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_b5(code); }
int instruction_wrapper_ed_b8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_b8(code); }
int instruction_wrapper_ed_ba(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_ba(code); }
int instruction_wrapper_ed_bb(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_bb(code); }
int instruction_wrapper_ed_bc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_bc(code); }

int instruction_wrapper_ed_c0(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c0(code); }
int instruction_wrapper_ed_c1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c1(code); }
int instruction_wrapper_ed_c2(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c2(code); }
int instruction_wrapper_ed_c3(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c3(code); }
int instruction_wrapper_ed_c4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c4(code); }
int instruction_wrapper_ed_c5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c5(code); }
int instruction_wrapper_ed_c6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c6(code); }
int instruction_wrapper_ed_c8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_c8(code); }
int instruction_wrapper_ed_ca(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_ca(code); }
int instruction_wrapper_ed_cb(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_cb(code); }
int instruction_wrapper_ed_cc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_cc(code); }

int instruction_wrapper_ed_d0(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d0(code); }
int instruction_wrapper_ed_d1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d1(code); }
int instruction_wrapper_ed_d2(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d2(code); }
int instruction_wrapper_ed_d3(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d3(code); }
int instruction_wrapper_ed_d4(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d4(code); }
int instruction_wrapper_ed_d5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d5(code); }
int instruction_wrapper_ed_d6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d6(code); }
int instruction_wrapper_ed_d8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d8(code); }
int instruction_wrapper_ed_d9(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_d9(code); }
int instruction_wrapper_ed_da(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_da(code); }
int instruction_wrapper_ed_db(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_db(code); }
int instruction_wrapper_ed_dc(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_dc(code); }

int instruction_wrapper_ed_e1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_e1(code); }
int instruction_wrapper_ed_e5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_e5(code); }
int instruction_wrapper_ed_e6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_e6(code); }
int instruction_wrapper_ed_ea(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_ea(code); }
int instruction_wrapper_ed_ee(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_ee(code); }

int instruction_wrapper_ed_f0(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_f0(code); }
int instruction_wrapper_ed_f1(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_f1(code); }
int instruction_wrapper_ed_f5(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_f5(code); }
int instruction_wrapper_ed_f6(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_f6(code); }
int instruction_wrapper_ed_f8(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_f8(code); }
int instruction_wrapper_ed_fa(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_fa(code); }
int instruction_wrapper_ed_fe(class cl_uc *uc, t_mem code) { return ((class cl_rxk *)uc)->instruction_ed_fe(code); }


void fill_ed_wrappers(instruction_wrapper_fn itab[])
{
  int i;
  for (i=0; i<256; i++)
    {
      itab[i]= instruction_wrapper_ed_none;
    }

  itab[0x00]= instruction_wrapper_ed_00;
  itab[0x01]= instruction_wrapper_ed_01;
  itab[0x02]= instruction_wrapper_ed_02;
  itab[0x03]= instruction_wrapper_ed_03;
  itab[0x04]= instruction_wrapper_ed_04;
  itab[0x05]= instruction_wrapper_ed_05;
  itab[0x06]= instruction_wrapper_ed_06;
  itab[0x07]= instruction_wrapper_ed_07;
  itab[0x08]= instruction_wrapper_ed_08;
  itab[0x09]= instruction_wrapper_ed_09;
  itab[0x0a]= instruction_wrapper_ed_0a;
  itab[0x0b]= instruction_wrapper_ed_0b;
  itab[0x0c]= instruction_wrapper_ed_0c;
  itab[0x0d]= instruction_wrapper_ed_0d;
  itab[0x0e]= instruction_wrapper_ed_0e;
  itab[0x0f]= instruction_wrapper_ed_0f;

  itab[0x10]= instruction_wrapper_ed_10;
  itab[0x11]= instruction_wrapper_ed_11;
  itab[0x12]= instruction_wrapper_ed_12;
  itab[0x13]= instruction_wrapper_ed_13;
  itab[0x14]= instruction_wrapper_ed_14;
  itab[0x15]= instruction_wrapper_ed_15;
  itab[0x16]= instruction_wrapper_ed_16;
  itab[0x17]= instruction_wrapper_ed_17;
  itab[0x18]= instruction_wrapper_ed_18;
  itab[0x19]= instruction_wrapper_ed_19;
  itab[0x1a]= instruction_wrapper_ed_1a;
  itab[0x1b]= instruction_wrapper_ed_1b;
  itab[0x1c]= instruction_wrapper_ed_1c;
  itab[0x1d]= instruction_wrapper_ed_1d;
  itab[0x1e]= instruction_wrapper_ed_1e;
  itab[0x1f]= instruction_wrapper_ed_1f;

  itab[0x21]= instruction_wrapper_ed_21;
  itab[0x23]= instruction_wrapper_ed_23;
  itab[0x24]= instruction_wrapper_ed_24;
  itab[0x25]= instruction_wrapper_ed_25;
  itab[0x26]= instruction_wrapper_ed_26;
  itab[0x27]= instruction_wrapper_ed_27;
  itab[0x28]= instruction_wrapper_ed_28;
  itab[0x29]= instruction_wrapper_ed_29;
  itab[0x2a]= instruction_wrapper_ed_2a;
  itab[0x2b]= instruction_wrapper_ed_2b;
  itab[0x2c]= instruction_wrapper_ed_2c;
  itab[0x2d]= instruction_wrapper_ed_2d;
  itab[0x2e]= instruction_wrapper_ed_2e;
  itab[0x2f]= instruction_wrapper_ed_2f;

  itab[0x31]= instruction_wrapper_ed_31;
  itab[0x33]= instruction_wrapper_ed_33;
  itab[0x34]= instruction_wrapper_ed_34;
  itab[0x35]= instruction_wrapper_ed_35;
  itab[0x36]= instruction_wrapper_ed_36;
  itab[0x37]= instruction_wrapper_ed_37;
  itab[0x38]= instruction_wrapper_ed_38;
  itab[0x39]= instruction_wrapper_ed_39;
  itab[0x3a]= instruction_wrapper_ed_3a;
  itab[0x3b]= instruction_wrapper_ed_3b;
  itab[0x3c]= instruction_wrapper_ed_3c;
  itab[0x3d]= instruction_wrapper_ed_3d;
  itab[0x3e]= instruction_wrapper_ed_3e;
  itab[0x3f]= instruction_wrapper_ed_3f;

  itab[0x40]= instruction_wrapper_ed_40;
  itab[0x41]= instruction_wrapper_ed_41;
  itab[0x42]= instruction_wrapper_ed_42;
  itab[0x43]= instruction_wrapper_ed_43;
  itab[0x44]= instruction_wrapper_ed_44;
  itab[0x45]= instruction_wrapper_ed_45;
  itab[0x46]= instruction_wrapper_ed_46;
  itab[0x47]= instruction_wrapper_ed_47;
  itab[0x48]= instruction_wrapper_ed_48;
  itab[0x49]= instruction_wrapper_ed_49;
  itab[0x4a]= instruction_wrapper_ed_4a;
  itab[0x4b]= instruction_wrapper_ed_4b;
  itab[0x4c]= instruction_wrapper_ed_4c;
  itab[0x4d]= instruction_wrapper_ed_4d;
  itab[0x4e]= instruction_wrapper_ed_4e;
  itab[0x4f]= instruction_wrapper_ed_4f;

  itab[0x50]= instruction_wrapper_ed_50;
  itab[0x51]= instruction_wrapper_ed_51;
  itab[0x52]= instruction_wrapper_ed_52;
  itab[0x53]= instruction_wrapper_ed_53;
  itab[0x54]= instruction_wrapper_ed_54;
  itab[0x55]= instruction_wrapper_ed_55;
  itab[0x56]= instruction_wrapper_ed_56;
  itab[0x57]= instruction_wrapper_ed_57;
  itab[0x58]= instruction_wrapper_ed_58;
  itab[0x59]= instruction_wrapper_ed_59;
  itab[0x5a]= instruction_wrapper_ed_5a;
  itab[0x5b]= instruction_wrapper_ed_5b;
  itab[0x5d]= instruction_wrapper_ed_5d;
  itab[0x5e]= instruction_wrapper_ed_5e;
  itab[0x5f]= instruction_wrapper_ed_5f;

  itab[0x61]= instruction_wrapper_ed_61;
  itab[0x62]= instruction_wrapper_ed_62;
  itab[0x63]= instruction_wrapper_ed_63;
  itab[0x64]= instruction_wrapper_ed_64;
  itab[0x65]= instruction_wrapper_ed_65;
  itab[0x66]= instruction_wrapper_ed_66;
  itab[0x67]= instruction_wrapper_ed_67;
  itab[0x69]= instruction_wrapper_ed_69;
  itab[0x6a]= instruction_wrapper_ed_6a;
  itab[0x6b]= instruction_wrapper_ed_6b;
  itab[0x6c]= instruction_wrapper_ed_6c;
  itab[0x6d]= instruction_wrapper_ed_6d;
  itab[0x6e]= instruction_wrapper_ed_6e;
  itab[0x6f]= instruction_wrapper_ed_6f;

  itab[0x72]= instruction_wrapper_ed_72;
  itab[0x73]= instruction_wrapper_ed_73;
  itab[0x74]= instruction_wrapper_ed_74;
  itab[0x75]= instruction_wrapper_ed_75;
  itab[0x76]= instruction_wrapper_ed_76;
  itab[0x77]= instruction_wrapper_ed_77;
  itab[0x7a]= instruction_wrapper_ed_7a;
  itab[0x7b]= instruction_wrapper_ed_7b;
  itab[0x7c]= instruction_wrapper_ed_7c;
  itab[0x7d]= instruction_wrapper_ed_7d;
  itab[0x7e]= instruction_wrapper_ed_7e;
  itab[0x7f]= instruction_wrapper_ed_7f;

  itab[0x80]= instruction_wrapper_ed_80;
  itab[0x83]= instruction_wrapper_ed_83;
  itab[0x88]= instruction_wrapper_ed_88;
  itab[0x8b]= instruction_wrapper_ed_8b;

  itab[0x90]= instruction_wrapper_ed_90;
  itab[0x98]= instruction_wrapper_ed_98;

  itab[0xa0]= instruction_wrapper_ed_a0;
  itab[0xa2]= instruction_wrapper_ed_a2;
  itab[0xa3]= instruction_wrapper_ed_a3;
  itab[0xa4]= instruction_wrapper_ed_a4;
  itab[0xa5]= instruction_wrapper_ed_a5;
  itab[0xa8]= instruction_wrapper_ed_a8;
  itab[0xaa]= instruction_wrapper_ed_aa;
  itab[0xab]= instruction_wrapper_ed_ab;
  itab[0xac]= instruction_wrapper_ed_ac;

  itab[0xb0]= instruction_wrapper_ed_b0;
  itab[0xb1]= instruction_wrapper_ed_b1;
  itab[0xb2]= instruction_wrapper_ed_b2;
  itab[0xb3]= instruction_wrapper_ed_b3;
  itab[0xb4]= instruction_wrapper_ed_b4;
  itab[0xb5]= instruction_wrapper_ed_b5;
  itab[0xb8]= instruction_wrapper_ed_b8;
  itab[0xba]= instruction_wrapper_ed_ba;
  itab[0xbb]= instruction_wrapper_ed_bb;
  itab[0xbc]= instruction_wrapper_ed_bc;

  itab[0xc0]= instruction_wrapper_ed_c0;
  itab[0xc1]= instruction_wrapper_ed_c1;
  itab[0xc2]= instruction_wrapper_ed_c2;
  itab[0xc3]= instruction_wrapper_ed_c3;
  itab[0xc4]= instruction_wrapper_ed_c4;
  itab[0xc5]= instruction_wrapper_ed_c5;
  itab[0xc6]= instruction_wrapper_ed_c6;
  itab[0xc8]= instruction_wrapper_ed_c8;
  itab[0xca]= instruction_wrapper_ed_ca;
  itab[0xcb]= instruction_wrapper_ed_cb;
  itab[0xcc]= instruction_wrapper_ed_cc;

  itab[0xd0]= instruction_wrapper_ed_d0;
  itab[0xd1]= instruction_wrapper_ed_d1;
  itab[0xd2]= instruction_wrapper_ed_d2;
  itab[0xd3]= instruction_wrapper_ed_d3;
  itab[0xd4]= instruction_wrapper_ed_d4;
  itab[0xd5]= instruction_wrapper_ed_d5;
  itab[0xd6]= instruction_wrapper_ed_d6;
  itab[0xd8]= instruction_wrapper_ed_d8;
  itab[0xd9]= instruction_wrapper_ed_d9;
  itab[0xda]= instruction_wrapper_ed_da;
  itab[0xdb]= instruction_wrapper_ed_db;
  itab[0xdc]= instruction_wrapper_ed_dc;

  itab[0xe1]= instruction_wrapper_ed_e1;
  itab[0xe5]= instruction_wrapper_ed_e5;
  itab[0xe6]= instruction_wrapper_ed_e6;
  itab[0xea]= instruction_wrapper_ed_ea;
  itab[0xee]= instruction_wrapper_ed_ee;

  itab[0xf0]= instruction_wrapper_ed_f0;
  itab[0xf1]= instruction_wrapper_ed_f1;
  itab[0xf5]= instruction_wrapper_ed_f5;
  itab[0xf6]= instruction_wrapper_ed_f6;
  itab[0xf8]= instruction_wrapper_ed_f8;
  itab[0xfa]= instruction_wrapper_ed_fa;
  itab[0xfe]= instruction_wrapper_ed_fe;
}

/* End of rxk.src/edwrap.cc */
