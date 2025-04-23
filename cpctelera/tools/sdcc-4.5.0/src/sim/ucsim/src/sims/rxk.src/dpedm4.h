/*
 * Simulator of microcontrollers (dpedm4.h)
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

#ifndef DPEDM4_HEADER
#define DPEDM4_HEADER

#define CBM_N		instruction_ed_00
#define LD_PW_iHTR_HL	instruction_ed_01
#define LD_PX_iHTR_HL	instruction_ed_11
#define LD_PY_iHTR_HL	instruction_ed_21
#define LD_PZ_iHTR_HL	instruction_ed_31
#define SBOX_A		instruction_ed_02
#define IBOX_A		instruction_ed_12
#define DWJNZ		instruction_ed_10
#define CP_HL_DE	instruction_ed_48
#define TEST_BC		instruction_ed_4c
#define LLJP_GT_LXPC_MN	instruction_ed_a2
#define LLJP_GTU_LXPC_MN instruction_ed_aa
#define LLJP_LT_LXPC_MN	instruction_ed_b2
#define LLJP_V_LXPC_MN	instruction_ed_ba
#define LLJP_NZ_LXPC_MN	instruction_ed_c2
#define LLJP_Z_LXPC_MN	instruction_ed_ca
#define LLJP_NC_LXPC_MN	instruction_ed_d2
#define LLJP_C_LXPC_MN	instruction_ed_da
#define PUSH_MN		instruction_ed_a5
#define JRE_GT_EE	instruction_ed_a3
#define JRE_LT_EE	instruction_ed_b3
#define JRE_GTU_EE	instruction_ed_ab
#define JRE_V_EE	instruction_ed_bb
#define JRE_NZ_EE	instruction_ed_c3
#define JRE_Z_EE	instruction_ed_cb
#define JRE_NC_EE	instruction_ed_d3
#define JRE_C_EE	instruction_ed_db
#define FLAG_NZ_HL	instruction_ed_c4
#define FLAG_Z_HL	instruction_ed_cc
#define FLAG_NC_HL	instruction_ed_d4
#define FLAG_C_HL	instruction_ed_dc
#define FLAG_GT_HL	instruction_ed_a4
#define FLAG_LT_HL	instruction_ed_b4
#define FLAG_GTU_HL	instruction_ed_ac
#define FLAG_V_HL	instruction_ed_bc
#define CALL_iHL	instruction_ed_ea
#define LLRET		instruction_ed_8b
#define EXP		instruction_ed_d9
#define LD_HTR_A	instruction_ed_40
#define LD_A_HTR	instruction_ed_50
#define FSYSCALL	instruction_ed_55
#define SYSRET		instruction_ed_83
#define SETUSRP		instruction_ed_b5
#define SETSYSP		instruction_ed_b1
#define LLCALL_iJKHL	instruction_ed_fa
#define LDL_PW_iSPn	instruction_ed_03
#define LDL_PX_iSPn	instruction_ed_13
#define LDL_PY_iSPn	instruction_ed_23
#define LDL_PZ_iSPn	instruction_ed_33
#define LD_PW_iSPn	instruction_ed_04
#define LD_PX_iSPn	instruction_ed_14
#define LD_PY_iSPn	instruction_ed_24
#define LD_PZ_iSPn	instruction_ed_34
#define LD_iSPn_PW	instruction_ed_05
#define LD_iSPn_PX	instruction_ed_15
#define LD_iSPn_PY	instruction_ed_25
#define LD_iSPn_PZ	instruction_ed_35
#define LD_HL_iPWBC	instruction_ed_06
#define LD_HL_iPXBC	instruction_ed_16
#define LD_HL_iPYBC	instruction_ed_26
#define LD_HL_iPZBC	instruction_ed_36
#define LD_iPWBC_HL	instruction_ed_07
#define LD_iPXBC_HL	instruction_ed_17
#define LD_iPYBC_HL	instruction_ed_27
#define LD_iPZBC_HL	instruction_ed_37
#define LDF_PW_ilmn	instruction_ed_08
#define LDF_PX_ilmn	instruction_ed_18
#define LDF_PY_ilmn	instruction_ed_28
#define LDF_PZ_ilmn	instruction_ed_38
#define LDF_ilmn_PW	instruction_ed_09
#define LDF_ilmn_PX	instruction_ed_19
#define LDF_ilmn_PY	instruction_ed_29
#define LDF_ilmn_PZ	instruction_ed_39
#define LDF_BC_ilmn	instruction_ed_0a
#define LDF_DE_ilmn	instruction_ed_1a
#define LDF_IX_ilmn	instruction_ed_2a
#define LDF_IY_ilmn	instruction_ed_3a
#define LDF_ilmn_BC	instruction_ed_0b
#define LDF_ilmn_DE	instruction_ed_1b
#define LDF_ilmn_IX	instruction_ed_2b
#define LDF_ilmn_IY	instruction_ed_3b
#define LD_PW_klmn	instruction_ed_0c
#define LD_PX_klmn	instruction_ed_1c
#define LD_PY_klmn	instruction_ed_2c
#define LD_PZ_klmn	instruction_ed_3c
#define LDL_PW_mn	instruction_ed_0d
#define LDL_PX_mn	instruction_ed_1d
#define LDL_PY_mn	instruction_ed_2d
#define LDL_PZ_mn	instruction_ed_3d
#define CONVC_PW	instruction_ed_0e
#define CONVC_PX	instruction_ed_1e
#define CONVC_PY	instruction_ed_2e
#define CONVC_PZ	instruction_ed_3e
#define CONVD_PW	instruction_ed_0f
#define CONVD_PX	instruction_ed_1f
#define CONVD_PY	instruction_ed_2f
#define CONVD_PZ	instruction_ed_3f
#define CP_JKHL_BCDE	instruction_ed_58
#define EX_aBC_HL	instruction_ed_74
#define EX_aJK_HL	instruction_ed_7c
#define COPY		instruction_ed_80
#define COPYR		instruction_ed_88
#define POP_PW		instruction_ed_c1
#define POP_PX		instruction_ed_d1
#define POP_PY		instruction_ed_e1
#define POP_PZ		instruction_ed_f1
#define PUSH_PW		instruction_ed_c5
#define PUSH_PX		instruction_ed_d5
#define PUSH_PY		instruction_ed_e5
#define PUSH_PZ		instruction_ed_f5
#define ADD_JKHL_BCDE	instruction_ed_c6
#define SUB_JKHL_BCDE	instruction_ed_d6
#define AND_JKHL_BCDE	instruction_ed_e6
#define OR_JKHL_BCDE	instruction_ed_f6
#define XOR_JKHL_BCDE	instruction_ed_ee
#define LD_HL_iSPHL	instruction_ed_fe


#endif

/* End of rxk.src/dpedm4.h */
