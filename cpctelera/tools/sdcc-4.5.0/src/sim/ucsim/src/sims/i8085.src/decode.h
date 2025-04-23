/*
 * Simulator of microcontrollers (decode.h)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include "dcmovrr.h"

#define NOP	instruction_00
#define HLT	instruction_76
#define EI	instruction_fb
#define DI	instruction_f3

#define MVI_Ai8		instruction_3e
#define MVI_Bi8		instruction_06
#define MVI_Ci8		instruction_0e
#define MVI_Di8		instruction_16
#define MVI_Ei8		instruction_1e
#define MVI_Hi8		instruction_26
#define MVI_Li8		instruction_2e
#define MVI_Mi8		instruction_36

#define LXI_Bi16	instruction_01
#define LXI_Di16	instruction_11
#define LXI_Hi16	instruction_21
#define LXI_Si16	instruction_31

#define LDA_a16		instruction_3a
#define STA_a16		instruction_32
#define LHLD_a16	instruction_2a
#define SHLD_a16	instruction_22
#define LDAX_B		instruction_0a
#define LDAX_D		instruction_1a
#define STAX_B		instruction_02
#define STAX_D		instruction_12
#define XCHG		instruction_eb

#define IN_INST		instruction_db
#define OUT_INST	instruction_d3

#define PUSH_B		instruction_c5
#define PUSH_D		instruction_d5
#define PUSH_H		instruction_e5
#define PUSH_PSW	instruction_f5
#define POP_B		instruction_c1
#define POP_D		instruction_d1
#define POP_H		instruction_e1
#define POP_PSW		instruction_f1
#define XTHL		instruction_e3
#define SPHL		instruction_f9

#define ADD_A		instruction_87
#define ADD_B		instruction_80
#define ADD_C		instruction_81
#define ADD_D		instruction_82
#define ADD_E		instruction_83
#define ADD_H		instruction_84
#define ADD_L		instruction_85
#define ADD_M		instruction_86
#define ADI		instruction_c6

#define ADC_A		instruction_8f
#define ADC_B		instruction_88
#define ADC_C		instruction_89
#define ADC_D		instruction_8a
#define ADC_E		instruction_8b
#define ADC_H		instruction_8c
#define ADC_L		instruction_8d
#define ADC_M		instruction_8e
#define ACI		instruction_ce

#define SUB_A		instruction_97
#define SUB_B		instruction_90
#define SUB_C		instruction_91
#define SUB_D		instruction_92
#define SUB_E		instruction_93
#define SUB_H		instruction_94
#define SUB_L		instruction_95
#define SUB_M		instruction_96
#define SUI		instruction_d6

#define SBB_A		instruction_9f
#define SBB_B		instruction_98
#define SBB_C		instruction_99
#define SBB_D		instruction_9a
#define SBB_E		instruction_9b
#define SBB_H		instruction_9c
#define SBB_L		instruction_9d
#define SBB_M		instruction_9e
#define SBI		instruction_de

#define CMP_A		instruction_bf
#define CMP_B		instruction_b8
#define CMP_C		instruction_b9
#define CMP_D		instruction_ba
#define CMP_E		instruction_bb
#define CMP_H		instruction_bc
#define CMP_L		instruction_bd
#define CMP_M		instruction_be
#define CPI		instruction_fe

#define DAD_B		instruction_09
#define DAD_D		instruction_19
#define DAD_H		instruction_29
#define DAD_S		instruction_39

#define INR_A		instruction_3c
#define INR_B		instruction_04
#define INR_C		instruction_0c
#define INR_D		instruction_14
#define INR_E		instruction_1c
#define INR_H		instruction_24
#define INR_L		instruction_2c
#define INR_M		instruction_34

#define DCR_A		instruction_3d
#define DCR_B		instruction_05
#define DCR_C		instruction_0d
#define DCR_D		instruction_15
#define DCR_E		instruction_1d
#define DCR_H		instruction_25
#define DCR_L		instruction_2d
#define DCR_M		instruction_35

#define INX_B		instruction_03
#define INX_D		instruction_13
#define INX_H		instruction_23
#define INX_S		instruction_33
#define DCX_B		instruction_0b
#define DCX_D		instruction_1b
#define DCX_H		instruction_2b
#define DCX_S		instruction_3b

#define ANA_A		instruction_a7
#define ANA_B		instruction_a0
#define ANA_C		instruction_a1
#define ANA_D		instruction_a2
#define ANA_E		instruction_a3
#define ANA_H		instruction_a4
#define ANA_L		instruction_a5
#define ANA_M		instruction_a6
#define ANI		instruction_e6

#define ORA_A		instruction_b7
#define ORA_B		instruction_b0
#define ORA_C		instruction_b1
#define ORA_D		instruction_b2
#define ORA_E		instruction_b3
#define ORA_H		instruction_b4
#define ORA_L		instruction_b5
#define ORA_M		instruction_b6
#define ORI		instruction_f6

#define XRA_A		instruction_af
#define XRA_B		instruction_a8
#define XRA_C		instruction_a9
#define XRA_D		instruction_aa
#define XRA_E		instruction_ab
#define XRA_H		instruction_ac
#define XRA_L		instruction_ad
#define XRA_M		instruction_ae
#define XRI		instruction_ee

#define RLC		instruction_07
#define RRC		instruction_0f
#define RAL		instruction_17
#define RAR		instruction_1f

#define DAA		instruction_27
#define CMA		instruction_2f
#define CMC		instruction_3f
#define STC		instruction_37

#define JMP		instruction_c3
#define JNZ		instruction_c2
#define JZ		instruction_ca
#define JNC		instruction_d2
#define JC		instruction_da
#define JPO		instruction_e2
#define JPE		instruction_ea
#define JP		instruction_f2
#define JM		instruction_fa

#define CALL		instruction_cd
#define CNZ		instruction_c4
#define CZ		instruction_cc
#define CNC		instruction_d4
#define CC		instruction_dc
#define CPO		instruction_e4
#define CPE		instruction_ec
#define CP		instruction_f4
#define CM		instruction_fc

#define RET		instruction_c9
#define RNZ		instruction_c0
#define RZ		instruction_c8
#define RNC		instruction_d0
#define RC		instruction_d8
#define RPO		instruction_e0
#define RPE		instruction_e8
#define RP		instruction_f0
#define RM		instruction_f8

#define RST_0		instruction_c7
#define RST_1		instruction_cf
#define RST_2		instruction_d7
#define RST_3		instruction_df
#define RST_4		instruction_e7
#define RST_5		instruction_ef
#define RST_6		instruction_f7
#define RST_7		instruction_ff

#define PCHL		instruction_e9

#endif

/* End of i8085.src/decode.h */
