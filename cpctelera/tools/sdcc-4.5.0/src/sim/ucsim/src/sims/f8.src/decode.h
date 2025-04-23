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

#ifndef F8_DECODE_HEADER
#define F8_DECODE_HEADER

#define PREF		0x9c
#define PREF_MASK	0xfc
#define PREF_SHIFT	0

// Opcodes of prefixes
#define PREF_SWAPOP	0x9c
#define PREF_ALT1	0x9d
#define PREF_ALT2	0x9e
#define PREF_ALT3	0x9f
#define PREF_ALT4	0x94
#define PREF_ALT5	0xd8

// data moves
#define LD8_A_I		instruction_80
#define LD8_A_M		instruction_81
#define LD8_A_NSP	instruction_82
#define LD8_A_NNZ	instruction_83
#define LD8_A_Y		instruction_84
#define LD8_A_NY	instruction_85
#define LD8_A_XH	instruction_86
#define LD8_A_YL	instruction_87
#define LD8_A_YH	instruction_88
#define LD8_A_ZL	instruction_89
#define LD8_A_ZH	instruction_8a
#define LD8_M_A		instruction_8b
#define LD8_NSP_A	instruction_8c
#define LD8_NNZ_A	instruction_8d
#define LD8_Y_A		instruction_8e
#define LD8_NY_A	instruction_8f

#define LDW_A_I		instruction_c0
#define LDW_A_M		instruction_c1
#define LDW_A_NSP	instruction_c2
#define LDW_A_NNZ	instruction_c3
#define LDW_A_NY	instruction_c4
#define LDW_A_AM	instruction_c5
#define LDW_A_X		instruction_c6
#define LDW_A_D		instruction_c7
#define LDW_M_A		instruction_c8
#define LDW_NSP_A	instruction_c9
#define LDW_NNZ_A	instruction_ca
#define LDW_X_A		instruction_cb
#define LDW_Z_A		instruction_cc
#define LDW_AM_X	instruction_cd
#define LDW_NAM_X	instruction_ce
#define LDW_DSP_A	instruction_74
#define LDW_A_Z         instruction_dc
#define LDW_X_AM        instruction_de

#define LDI_Y_Z         instruction_ed
#define LDWI_Y_Z        instruction_cf

#define PUSH_M		instruction_60
#define PUSH_NSP	instruction_61
#define PUSH_A		instruction_62
#define PUSH_NY		instruction_63
#define PUSH_I		instruction_90

#define PUSHW_M		instruction_b0
#define PUSHW_NSP	instruction_b1
#define PUSHW_NNZ	instruction_b2
#define PUSHW_A		instruction_b3
#define PUSHW_I		instruction_e8

#define POP_A		instruction_99
#define POPW_A		instruction_e9

#define XCH_A_NSP	instruction_91
#define XCH_A_Y		instruction_92
#define XCH_A_A		instruction_93
#define XCHW_X_Y	instruction_f4
#define XCHW_Y_NSP	instruction_f5

#define CAX		instruction_9b
#define CAXW		instruction_f9

#define CLR_M		instruction_58
#define CLR_NSP		instruction_59
#define CLR_A		instruction_5a
#define CLR_NY		instruction_5b
#define CLRW_M		instruction_a0
#define CLRW_NSP	instruction_a1
#define CLRW_NNZ	instruction_a2
#define CLRW_A		instruction_a3

#define XCHB_0		instruction_68
#define XCHB_1		instruction_69
#define XCHB_2		instruction_6a
#define XCHB_3		instruction_6b
#define XCHB_4		instruction_6c
#define XCHB_5		instruction_6d
#define XCHB_6		instruction_6e
#define XCHB_7		instruction_6f

#define INC_M		instruction_50
#define INC_NSP		instruction_51
#define INC_A		instruction_52
#define INC_NY		instruction_53
#define DEC_M		instruction_54
#define DEC_NSP		instruction_55
#define DEC_A		instruction_56
#define DEC_NY		instruction_57
#define TST_M		instruction_5c
#define TST_NSP		instruction_5d
#define TST_A		instruction_5e
#define TST_NY		instruction_5f

// arithmetic instructions
#define ADD_I		instruction_10
#define ADD_M		instruction_11
#define ADD_NSP		instruction_12
#define ADD_NNZ		instruction_13
#define ADD_ZL		instruction_14
#define ADD_XH		instruction_15
#define ADD_YL		instruction_16
#define ADD_YH		instruction_17

#define ADC_I		instruction_18
#define ADC_M		instruction_19
#define ADC_NSP		instruction_1a
#define ADC_NNZ		instruction_1b
#define ADC_ZL		instruction_1c
#define ADC_XH		instruction_1d
#define ADC_YL		instruction_1e
#define ADC_YH		instruction_1f

#define SUB_M		instruction_01
#define SUB_NSP		instruction_02
#define SUB_NNZ		instruction_03
#define SUB_ZL		instruction_04
#define SUB_XH		instruction_05
#define SUB_YL		instruction_06
#define SUB_YH		instruction_07

#define SBC_M		instruction_09
#define SBC_NSP		instruction_0a
#define SBC_NNZ		instruction_0b
#define SBC_ZL		instruction_0c
#define SBC_XH		instruction_0d
#define SBC_YL		instruction_0e
#define SBC_YH		instruction_0f

#define CP_I		instruction_20
#define CP_M		instruction_21
#define CP_NSP		instruction_22
#define CP_NNZ		instruction_23
#define CP_ZL		instruction_24
#define CP_XH		instruction_25
#define CP_YL		instruction_26
#define CP_YH		instruction_27

#define OR_I		instruction_28
#define OR_M		instruction_29
#define OR_NSP		instruction_2a
#define OR_NNZ		instruction_2b
#define OR_ZL		instruction_2c
#define OR_XH		instruction_2d
#define OR_YL		instruction_2e
#define OR_YH		instruction_2f

#define AND_I		instruction_30
#define AND_M		instruction_31
#define AND_NSP		instruction_32
#define AND_NNZ		instruction_33
#define AND_ZL		instruction_34
#define AND_XH		instruction_35
#define AND_YL		instruction_36
#define AND_YH		instruction_37

#define XOR_I		instruction_38
#define XOR_M		instruction_39
#define XOR_NSP		instruction_3a
#define XOR_NNZ		instruction_3b
#define XOR_ZL		instruction_3c
#define XOR_XH		instruction_3d
#define XOR_YL		instruction_3e
#define XOR_YH		instruction_3f

#define SUBW_M		instruction_71
#define SUBW_NSP	instruction_72
#define SUBW_X		instruction_73
#define SBCW_M		instruction_75
#define SBCW_NSP	instruction_76
#define SBCW_X		instruction_77

#define ADDW_I		instruction_78
#define ADDW_M		instruction_79
#define ADDW_NSP	instruction_7a
#define ADDW_X		instruction_7b
#define ADCW_I		instruction_7c
#define ADCW_M		instruction_7d
#define ADCW_NSP	instruction_7e
#define ADCW_X		instruction_7f

#define ORW_I		instruction_f0
#define ORW_M		instruction_f1
#define ORW_NSP		instruction_f2
#define ORW_X		instruction_f3

#define XORW_I		instruction_fc
#define XORW_M		instruction_fd
#define XORW_NSP	instruction_fe
#define XORW_X		instruction_ff

#define SRL_M		instruction_40
#define SRL_NSP		instruction_41
#define SRL_A		instruction_42
#define SRL_NY		instruction_43
#define SLL_M		instruction_44
#define SLL_NSP		instruction_45
#define SLL_A		instruction_46
#define SLL_NY		instruction_47
#define RRC_M		instruction_48
#define RRC_NSP		instruction_49
#define RRC_A		instruction_4a
#define RRC_NY		instruction_4b
#define RLC_M		instruction_4c
#define RLC_NSP		instruction_4d
#define RLC_A		instruction_4e
#define RLC_NY		instruction_4f

#define INCW_M		instruction_a4
#define INCW_NSP	instruction_a5
#define INCW_NNZ	instruction_a6
#define INCW_A		instruction_a7
#define ADCW1_M		instruction_a8
#define ADCW1_NSP	instruction_a9
#define ADCW1_NNZ	instruction_aa
#define ADCW1_A		instruction_ab
#define SBCW1_M		instruction_ac
#define SBCW1_NSP	instruction_ad
#define SBCW1_NNZ	instruction_ae
#define SBCW1_A		instruction_af
#define TSTW1_M		instruction_b4
#define TSTW1_NSP	instruction_b5
#define TSTW1_NNZ	instruction_b6
#define TSTW1_A		instruction_b7

#define ROT		instruction_95
#define SRA		instruction_96
#define DAA		instruction_97
#define BOOL_A		instruction_98
#define MSK		instruction_b8
#define MAD_M		instruction_bc
#define MAD_NSP		instruction_bd
#define MAD_NNZ		instruction_be
#define MAD_Z		instruction_bf
#define XCH_F_NSP	instruction_ec

#define MUL		instruction_b9
#define NEGW		instruction_fa
#define BOOLW		instruction_fb
#define SRLW		instruction_e0
#define SLLW		instruction_e1
#define RRCW		instruction_e2
#define RLCW_A		instruction_e3
#define RRCW_NSP	instruction_e6
#define RLCW_NSP	instruction_e7
#define SRAW		instruction_e4
#define ADDW_SP_D	instruction_ea
#define ADDW_A_D	instruction_eb
#define LDW_A_SP	instruction_70
#define CPW		instruction_f8
#define INCNW		instruction_f6
#define DECW_NSP	instruction_f7
#define SLLW_A_XL	instruction_e5
#define SEX		instruction_ee
#define ZEX		instruction_ef

// branches
#define JP_I		instruction_64
#define JP_A		instruction_65
#define CALL_I		instruction_66
#define CALL_A		instruction_67
#define RET		instruction_ba
#define RETI		instruction_bb

#define JR		instruction_d0
#define DNJNZ		instruction_d1
#define JRZ		instruction_d2
#define JRNZ		instruction_d3
#define JRC		instruction_d4
#define JRNC		instruction_d5
#define JRN		instruction_d6
#define JRNN		instruction_d7
#define JRNO		instruction_d9
#define JRSGE		instruction_da
#define JRSLT		instruction_db
#define JRSLE		instruction_dd
#define JRLE		instruction_df

// other instructions
#define NOP		instruction_08
#define TRAP		instruction_00
#define THRD		instruction_9a

#endif

/* End of f8.src/decode.h */
