/*
 * Simulator of microcontrollers (dpddm4.h)
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

#ifndef DPDDM4_HEADER
#define DPDDM4_HEADER

#define LD_A_iIRA	instruction_dd_06
#define TEST_IR		instruction_dd_4c
#define LD_IRR_iHL	instruction_dd_1a
#define LD_IRR_iIXd	instruction_dd_ce
#define LD_IRR_iIYd	instruction_dd_de
#define LD_IRR_iSPn	instruction_dd_ee
#define LD_iHL_IRR	instruction_dd_1b
#define LD_iIXd_IRR	instruction_dd_cf
#define LD_iIYd_IRR	instruction_dd_df
#define LD_iSPn_IRR	instruction_dd_ef
#define NEG_IRR		instruction_dd_4d
#define POP_IRR		instruction_dd_f1
#define PUSH_IRR	instruction_dd_f5
#define RL_1_IRR	instruction_dd_68
#define RL_2_IRR	instruction_dd_69
#define RL_4_IRR	instruction_dd_6b
#define RLC_1_IRR	instruction_dd_48
#define RLC_2_IRR	instruction_dd_49
#define RLC_4_IRR	instruction_dd_4b
#define RLC_8_IRR	instruction_dd_4f
#define RLB_A_IRR	instruction_dd_6f
#define SLA_1_IRR	instruction_dd_88
#define SLA_2_IRR	instruction_dd_89
#define SLA_4_IRR	instruction_dd_8b
#define SLL_1_IRR	instruction_dd_a8
#define SLL_2_IRR	instruction_dd_a9
#define SLL_4_IRR	instruction_dd_ab
#define TEST_IRR	instruction_dd_5c
#define RR_1_IRR	instruction_dd_78
#define RR_2_IRR	instruction_dd_79
#define RR_4_IRR	instruction_dd_7b
#define RRC_1_IRR	instruction_dd_58
#define RRC_2_IRR	instruction_dd_59
#define RRC_4_IRR	instruction_dd_5b
#define RRC_8_IRR	instruction_dd_5f
#define RRB_A_IRR	instruction_dd_7f
#define SRA_1_IRR	instruction_dd_98
#define SRA_2_IRR	instruction_dd_99
#define SRA_4_IRR	instruction_dd_9b
#define SRL_1_IRR	instruction_dd_b8
#define SRL_2_IRR	instruction_dd_b9
#define SRL_4_IRR	instruction_dd_bb

#define LDF_IRR_iLMN	instruction_dd_0a
#define LDF_iLMN_IRR	instruction_dd_0b
#define LD_IRR_iPW_HL	instruction_dd_0c
#define LD_IRR_iPX_HL	instruction_dd_1c
#define LD_IRR_iPY_HL	instruction_dd_2c
#define LD_IRR_iPZ_HL	instruction_dd_3c
#define LD_iPW_HL_IRR	instruction_dd_0d
#define LD_iPX_HL_IRR	instruction_dd_1d
#define LD_iPY_HL_IRR	instruction_dd_2d
#define LD_iPZ_HL_IRR	instruction_dd_3d
#define LD_IRR_iPW_D	instruction_dd_0e
#define LD_IRR_iPX_D	instruction_dd_1e
#define LD_IRR_iPY_D	instruction_dd_2e
#define LD_IRR_iPZ_D	instruction_dd_3e
#define LD_iPW_D_IRR	instruction_dd_0f
#define LD_iPX_D_IRR	instruction_dd_1f
#define LD_iPY_D_IRR	instruction_dd_2f
#define LD_iPZ_D_IRR	instruction_dd_3f
#define LDL_PW_IR	instruction_dd_8c
#define LDL_PX_IR	instruction_dd_9c
#define LDL_PY_IR	instruction_dd_ac
#define LDL_PZ_IR	instruction_dd_bc
#define LD_PW_IRR	instruction_dd_8d
#define LD_PX_IRR	instruction_dd_9d
#define LD_PY_IRR	instruction_dd_ad
#define LD_PZ_IRR	instruction_dd_bd
#define LDL_PW_IRRL	instruction_dd_8f
#define LDL_PX_IRRL	instruction_dd_9f
#define LDL_PY_IRRL	instruction_dd_af
#define LDL_PZ_IRRL	instruction_dd_bf
#define LD_IRR_PW	instruction_dd_cd
#define LD_IRR_PX	instruction_dd_dd
#define LD_IRR_PY	instruction_dd_ed
#define LD_IRR_PZ	instruction_dd_fd
#define LD_iSP_HL_IRR	instruction_dd_ff
#define CALL_iIR	instruction_dd_ea
#define LD_IRR_iSP_HL	instruction_dd_fe

#endif

/* End of rxk.src/dpddm4.h */
