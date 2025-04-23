/*
 * Simulator of microcontrollers (dpddm3.h)
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

#ifndef DPDDM3_HEADER
#define DPDDM3_HEADER

#define LD_IR_mn	instruction_dd_21

#define ADD_IR_BC	instruction_dd_09
#define ADD_IR_DE	instruction_dd_19
#define ADD_IR_IR	instruction_dd_29
#define ADD_IR_SP	instruction_dd_39
#define INC_iIRd	instruction_dd_34
#define DEC_iIRd	instruction_dd_35
#define CP_A_iIRd	instruction_dd_be
#define SBC_A_iIRd	instruction_dd_9e
#define SUB_A_iIRd	instruction_dd_96
#define ADD_A_iIRd	instruction_dd_86
#define ADC_A_iIRd	instruction_dd_8e
#define INC_IR		instruction_dd_23
#define DEC_IR		instruction_dd_2b
#define RR_IR		instruction_dd_fc

#define XOR_A_iIRd	instruction_dd_ae
#define AND_A_iIRd	instruction_dd_a6
#define OR_A_iIRd	instruction_dd_b6
#define BOOL_IR		instruction_dd_cc
#define AND_IR_DE	instruction_dd_dc
#define OR_IR_DE	instruction_dd_ec

#define POP_IR		instruction_dd_e1
#define PUSH_IR		instruction_dd_e5
#define EX_iSP_IR	instruction_dd_e3

#define LD_iIRd_A	instruction_dd_77
#define LD_iIRd_B	instruction_dd_70
#define LD_iIRd_C	instruction_dd_71
#define LD_iIRd_D	instruction_dd_72
#define LD_iIRd_E	instruction_dd_73
#define LD_iIRd_H	instruction_dd_74
#define LD_iIRd_L	instruction_dd_75
#define LD_A_iIRd	instruction_dd_7e
#define LD_B_iIRd	instruction_dd_46
#define LD_C_iIRd	instruction_dd_4e
#define LD_D_iIRd	instruction_dd_56
#define LD_E_iIRd	instruction_dd_5e
#define LD_H_iIRd	instruction_dd_66
#define LD_L_iIRd	instruction_dd_6e

#define LD_SP_IR	instruction_dd_f9
#define LD_IR_iSPn 	instruction_dd_c4
#define LD_iSPn_IR 	instruction_dd_d4
#define LD_HL_iIRd	instruction_dd_e4
#define LD_iIRd_n	instruction_dd_36
#define LD_imn_IR	instruction_dd_22
#define LD_IR_imn	instruction_dd_2a
#define LD_HL_IR	instruction_dd_7c
#define LD_IR_HL	instruction_dd_7d
#define LD_iHLd_HL	instruction_dd_f4

#define LDP_iIR_HL	instruction_dd_64
#define LDP_imn_IR	instruction_dd_65
#define LDP_HL_iIR	instruction_dd_6c
#define LDP_IR_imn	instruction_dd_6d

#define JP_IR		instruction_dd_e9

#define PAGE_DD_CB	instruction_dd_cb

#endif

/* End of rxk.src/dpddm3.h */
