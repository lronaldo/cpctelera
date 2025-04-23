/*
 * Simulator of microcontrollers (decode.h)
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

#ifndef DECODE_HEADER
#define DECODE_HEADER

#define ALTD		instruction_76
#define IOI		instruction_d3
#define IOE		instruction_db

#define NOP		instruction_00
#define LD_BC_mn	instruction_01
#define LD_DE_mn	instruction_11
#define LD_HL_mn	instruction_21
#define LD_SP_mn	instruction_31
#define LD_imn_HL	instruction_22
#define LD_HL_imn	instruction_2a
#define INC_BC		instruction_03
#define INC_DE		instruction_13
#define INC_HL		instruction_23
#define INC_SP		instruction_33
#define INC_A		instruction_3c
#define INC_B		instruction_04
#define INC_C		instruction_0c
#define INC_D		instruction_14
#define INC_E		instruction_1c
#define INC_H		instruction_24
#define INC_L		instruction_2c
#define DEC_BC		instruction_0b
#define DEC_DE		instruction_1b
#define DEC_HL		instruction_2b
#define DEC_SP		instruction_3b
#define DEC_A		instruction_3d
#define DEC_B		instruction_05
#define DEC_C		instruction_0d
#define DEC_D		instruction_15
#define DEC_E		instruction_1d
#define DEC_H		instruction_25
#define DEC_L		instruction_2d
#define LD_A_n		instruction_3e
#define LD_B_n		instruction_06
#define LD_C_n		instruction_0e
#define LD_D_n		instruction_16
#define LD_E_n		instruction_1e
#define LD_H_n		instruction_26
#define LD_L_n		instruction_2e
#define RLCA		instruction_07
#define RLA		instruction_17
#define RRCA		instruction_0f
#define RRA		instruction_1f
#define LD_iBC_A	instruction_02
#define LD_iDE_A	instruction_12
#define LD_iHL_A	instruction_77
#define LD_iHL_B	instruction_70
#define LD_iHL_C	instruction_71
#define LD_iHL_D	instruction_72
#define LD_iHL_E	instruction_73
#define LD_iHL_H	instruction_74
#define LD_iHL_L	instruction_75
#define LD_iMN_A	instruction_32
#define LD_A_iBC	instruction_0a
#define LD_A_iDE	instruction_1a
#define LD_A_iMN	instruction_3a
#define LD_A_iHL	instruction_7e
#define LD_B_iHL	instruction_46
#define LD_C_iHL	instruction_4e
#define LD_D_iHL	instruction_56
#define LD_E_iHL	instruction_5e
#define LD_H_iHL	instruction_66
#define LD_L_iHL	instruction_6e
#define SCF		instruction_37
#define CPL		instruction_2f
#define CCF		instruction_3f
#define EX_AF_aAF	instruction_08
#define ADD_HL_BC	instruction_09
#define ADD_HL_DE	instruction_19
#define ADD_HL_HL	instruction_29
#define ADD_HL_SP	instruction_39
#define DJNZ		instruction_10
#define JR		instruction_18
#define JR_NZ		instruction_20
#define JR_Z		instruction_28
#define JR_NC		instruction_30
#define JR_C		instruction_38
#define ADD_SP_d	instruction_27
#define INC_iHL		instruction_34
#define DEC_iHL		instruction_35
#define LD_iHL_n	instruction_36
#define LD_B_A		instruction_47
#define LD_C_A		instruction_4f
#define LD_D_A		instruction_57
#define LD_E_E		instruction_5b
#define LD_E_A		instruction_5f
#define LD_L_A		instruction_6f
#define LD_L_L		instruction_6d
#define LD_H_A		instruction_67
#define LD_A_B		instruction_78
#define LD_A_C		instruction_79
#define LD_A_D		instruction_7a
#define LD_A_E		instruction_7b
#define LD_A_H		instruction_7c
#define LD_A_L		instruction_7d
#define LD_A_A		instruction_7f
#define XOR_A		instruction_af
#define OR_A		instruction_b7
#define RET_NZ		instruction_c0
#define RET_Z		instruction_c8
#define RET		instruction_c9
#define RET_NC		instruction_d0
#define RET_C		instruction_d8
#define RET_LZ		instruction_e0
#define RET_LO		instruction_e8
#define RET_P		instruction_f0
#define RET_M		instruction_f8
#define POP_AF		instruction_f1
#define POP_BC		instruction_c1
#define POP_DE		instruction_d1
#define POP_HL		instruction_e1
#define JP_NZ_mn	instruction_c2
#define JP_Z_mn		instruction_ca
#define JP_NC_mn	instruction_d2
#define JP_C_mn		instruction_da
#define JP_LZ_mn	instruction_e2
#define JP_LO_mn	instruction_ea
#define JP_P_mn		instruction_f2
#define JP_M_mn		instruction_fa
#define JP_mn		instruction_c3
#define LD_HL_iSPn	instruction_c4
#define PUSH_AF		instruction_f5
#define PUSH_BC		instruction_c5
#define PUSH_DE		instruction_d5
#define PUSH_HL		instruction_e5
#define ADD_A_n		instruction_c6
#define LJP		instruction_c7
#define BOOL_HL		instruction_cc
#define CALL_mn		instruction_cd
#define ADC_A_n		instruction_ce
#define LCALL_lmn	instruction_cf
#define LD_iSPn_HL	instruction_d4
#define SUB_A_n		instruction_d6
#define RST_10		instruction_d7
#define RST_18		instruction_df
#define RST_20		instruction_e7
#define RST_28		instruction_ef
#define RST_38		instruction_ff
#define EXX		instruction_d9
#define AND_HL_DE	instruction_dc
#define OR_HL_DE	instruction_ec
#define SBC_A_n		instruction_de
#define EX_aDE_HL	instruction_e3
#define EX_DE_HL	instruction_eb
#define LD_HL_iIXd	instruction_e4
#define LD_iIXd_HL	instruction_f4
#define AND_n		instruction_e6
#define JP_HL		instruction_e9
#define XOR_n		instruction_ee
#define RL_DE		instruction_f3
#define OR_n		instruction_f6
#define MUL		instruction_f7
#define LD_SP_HL	instruction_f9
#define RR_DE		instruction_fb
#define RR_HL		instruction_fc
#define CP_n		instruction_fe

#define PAGE_CB		instruction_cb

#endif

/* End of rxk.src/decode.h */
