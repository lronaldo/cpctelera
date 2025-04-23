/*
 * Simulator of microcontrollers (decode68.h)
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

#ifndef DECODE68_HEADER
#define DECODE68_HEADER

#define NOP   instruction_01
#define TAP   instruction_06
#define TPA   instruction_07
#define INX   instruction_08
#define DEX   instruction_09
#define CLV   instruction_0a
#define SEV   instruction_0b
#define CLC   instruction_0c
#define SEc   instruction_0d
#define CLI   instruction_0e
#define SEI   instruction_0f

#define SBA   instruction_10
#define CBA   instruction_11
#define TAB   instruction_16
#define TBA   instruction_17
#define DAA   instruction_19
#define ABA   instruction_1b

#define BRA   instruction_20
#define BHI   instruction_22
#define BLS   instruction_23
#define BCC   instruction_24
#define BCS   instruction_25
#define BNE   instruction_26
#define BEQ   instruction_27
#define BVC   instruction_28
#define BVS   instruction_29
#define BPL   instruction_2a
#define BMI   instruction_2b
#define BGE   instruction_2c
#define BLT   instruction_2d
#define BGT   instruction_2e
#define BLE   instruction_2f

#define TSX   instruction_30
#define INS   instruction_31
#define PULA  instruction_32
#define PULB  instruction_33
#define DES   instruction_34
#define TXS   instruction_35
#define PSHA  instruction_36
#define PSHB  instruction_37
#define RTS   instruction_39

#define RTI   instruction_3b
#define WAI   instruction_3e
#define SWI   instruction_3f

#define NEGA  instruction_40
#define COMA  instruction_43
#define LSRA  instruction_44
#define RORA  instruction_46
#define ASRA  instruction_47
#define ASLA  instruction_48
#define ROLA  instruction_49
#define DECA  instruction_4a
#define INCA  instruction_4c
#define TSTA  instruction_4d
#define CLRA  instruction_4f

#define NEGB  instruction_50
#define COMB  instruction_53
#define LSRB  instruction_54
#define RORB  instruction_56
#define ASRB  instruction_57
#define ASLB  instruction_58
#define ROLB  instruction_59
#define DECB  instruction_5a
#define INCB  instruction_5c
#define TSTB  instruction_5d
#define CLRB  instruction_5f

#define NEGi  instruction_60
#define COMi  instruction_63
#define LSRi  instruction_64
#define RORi  instruction_66
#define ASRi  instruction_67
#define ASLi  instruction_68
#define ROLi  instruction_69
#define DECi  instruction_6a
#define INCi  instruction_6c
#define TSTi  instruction_6d
#define JMPi  instruction_6e
#define CLRi  instruction_6f

#define NEGe  instruction_70
#define COMe  instruction_73
#define LSRe  instruction_74
#define RORe  instruction_76
#define ASRe  instruction_77
#define ASLe  instruction_78
#define ROLe  instruction_79
#define DECe  instruction_7a
#define INCe  instruction_7c
#define TSTe  instruction_7d
#define JMPe  instruction_7e
#define CLRe  instruction_7f

#define SUBA8 instruction_80
#define CMPA8 instruction_81
#define SBCA8 instruction_82
#define ANDA8 instruction_84
#define BITA8 instruction_85
#define LDAA8 instruction_86
#define EORA8 instruction_88
#define ADCA8 instruction_89
#define ORAA8 instruction_8a
#define ADDA8 instruction_8b
#define CPX16 instruction_8c
#define BSR   instruction_8d
#define LDS16 instruction_8e

#define SUBAd instruction_90
#define CMPAd instruction_91
#define SBCAd instruction_92
#define ANDAd instruction_94
#define BITAd instruction_95
#define LDAAd instruction_96
#define STAAd instruction_97
#define EORAd instruction_98
#define ADCAd instruction_99
#define ORAAd instruction_9a
#define ADDAd instruction_9b
#define CPXd  instruction_9c
#define LDSd  instruction_9e
#define STSd  instruction_9f

#define SUBAi instruction_a0
#define CMPAi instruction_a1
#define SBCAi instruction_a2
#define ANDAi instruction_a4
#define BITAi instruction_a5
#define LDAAi instruction_a6
#define STAAi instruction_a7
#define EORAi instruction_a8
#define ADCAi instruction_a9
#define ORAAi instruction_aa
#define ADDAi instruction_ab
#define CPXi  instruction_ac
#define JSRi  instruction_ad
#define LDSi  instruction_ae
#define STSi  instruction_af

#define SUBAe instruction_b0
#define CMPAe instruction_b1
#define SBCAe instruction_b2
#define ANDAe instruction_b4
#define BITAe instruction_b5
#define LDAAe instruction_b6
#define STAAe instruction_b7
#define EORAe instruction_b8
#define ADCAe instruction_b9
#define ORAAe instruction_ba
#define ADDAe instruction_bb
#define CPXe  instruction_bc
#define JSRe  instruction_bd
#define LDSe  instruction_be
#define STSe  instruction_bf

#define SUBB8 instruction_c0
#define CMPB8 instruction_c1
#define SBCB8 instruction_c2
#define ANDB8 instruction_c4
#define BITB8 instruction_c5
#define LDAB8 instruction_c6
#define EORB8 instruction_c8
#define ADCB8 instruction_c9
#define ORAB8 instruction_ca
#define ADDB8 instruction_cb
#define LDX16 instruction_ce

#define SUBBd instruction_d0
#define CMPBd instruction_d1
#define SBCBd instruction_d2
#define ANDBd instruction_d4
#define BITBd instruction_d5
#define LDABd instruction_d6
#define STABd instruction_d7
#define EORBd instruction_d8
#define ADCBd instruction_d9
#define ORABd instruction_da
#define ADDBd instruction_db
#define LDXd  instruction_de
#define STXd  instruction_df

#define SUBBi instruction_e0
#define CMPBi instruction_e1
#define SBCBi instruction_e2
#define ANDBi instruction_e4
#define BITBi instruction_e5
#define LDABi instruction_e6
#define STABi instruction_e7
#define EORBi instruction_e8
#define ADCBi instruction_e9
#define ORABi instruction_ea
#define ADDBi instruction_eb
#define LDXi  instruction_ee
#define STXi  instruction_ef

#define SUBBe instruction_f0
#define CMPBe instruction_f1
#define SBCBe instruction_f2
#define ANDBe instruction_f4
#define BITBe instruction_f5
#define LDABe instruction_f6
#define STABe instruction_f7
#define EORBe instruction_f8
#define ADCBe instruction_f9
#define ORABe instruction_fa
#define ADDBe instruction_fb
#define LDXe  instruction_fe
#define STXe  instruction_ff

#endif

/* End of m6800.src/decode68.h */
