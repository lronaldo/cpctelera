/*
 * Simulator of microcontrollers (dp0m4.h)
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

#ifndef DP0M4_HEADER
#define DP0M4_HEADER

// opcodes valid in 4k mode only

// opcodes mean different insts in 3k/4k modes
// meaning of 4k mode follows

#define PAGE_4K6D	instruction_4k6d
#define PAGE_4K7F	instruction_4k7f

#define RL_HL		instruction_4k42
#define RL_BC		instruction_4k62
#define SUB_HL_JK	instruction_4k45
#define SUB_HL_DE	instruction_4k55
#define TEST_HL		instruction_4k4c
#define CP_HL_D		instruction_4k48
#define RLC_BC		instruction_4k60
#define RLC_DE		instruction_4k50
#define RRC_BC		instruction_4k61
#define RRC_DE		instruction_4k51
#define XOR_HL_DE	instruction_4k54
#define RR_BC		instruction_4k63
#define ADD_HL_JK	instruction_4k65

#define LD_HL_BC	instruction_4k81
#define LD_BC_HL	instruction_4k91
#define LD_HL_DE	instruction_4ka1
#define LD_iLMN_HL	instruction_4k82
#define LD_HL_iLMN	instruction_4k92
#define LD_iMN_BCDE	instruction_4k83
#define LD_iMN_JKHL	instruction_4k84
#define LD_BCDE_iMN	instruction_4k93
#define LD_JKHL_iMN	instruction_4k94
#define LD_HL_iPWd	instruction_4k85
#define LD_HL_iPXd	instruction_4k95
#define LD_HL_iPYd	instruction_4ka5
#define LD_HL_iPZd	instruction_4kb5
#define LD_iPWd_HL	instruction_4k86
#define LD_iPXd_HL	instruction_4k96
#define LD_iPYd_HL	instruction_4ka6
#define LD_iPZd_HL	instruction_4kb6
#define LLJP_lxpcmn	instruction_4k87
#define LD_imn_JK	instruction_4k89
#define LD_JK_imn	instruction_4k99
#define LDF_ilmn_A	instruction_4k8a
#define LDF_A_ilmn	instruction_4k9a
#define LD_A_iPWHL	instruction_4k8b
#define LD_A_iPXHL	instruction_4k9b
#define LD_A_iPYHL	instruction_4kab
#define LD_A_iPZHL	instruction_4kbb
#define LD_iPWHL_A	instruction_4k8c
#define LD_iPXHL_A	instruction_4k9c
#define LD_iPYHL_A	instruction_4kac
#define LD_iPZHL_A	instruction_4kbc
#define LD_A_iPWd	instruction_4k8d
#define LD_A_iPXd	instruction_4k9d
#define LD_A_iPYd	instruction_4kad
#define LD_A_iPZd	instruction_4kbd
#define LD_iPWd_A	instruction_4k8e
#define LD_iPXd_A	instruction_4k9e
#define LD_iPYd_A	instruction_4kae
#define LD_iPZd_A	instruction_4kbe
#define LLCALL_lxpcmn	instruction_4k8f
#define LD_LXPC_HL	instruction_4k97
#define LD_HL_LXPC	instruction_4k9f
#define JRE_ee		instruction_4k98
#define JR_GT_e		instruction_4ka0
#define JR_LT_e		instruction_4kb0
#define JR_GTU_e	instruction_4ka8
#define JR_V_e		instruction_4kb8
#define JP_GT_mn	instruction_4ka2
#define JP_LT_mn	instruction_4kb2
#define JP_GTU_mn	instruction_4kaa
#define JP_V_mn		instruction_4kba
#define LD_BCDE_d	instruction_4ka3
#define LD_JKHL_d	instruction_4ka4
#define MULU		instruction_4ka7
#define LD_JK_mn	instruction_4ka9
#define LD_DE_HL	instruction_4kb1
#define EX_BC_HL	instruction_4kb3
#define EX_JKHL_BCDE	instruction_4kb4
#define EX_JK_HL	instruction_4kb9
#define CLR_HL		instruction_4kbf

#endif

/* End of rxk.src/dp0m4.h */
