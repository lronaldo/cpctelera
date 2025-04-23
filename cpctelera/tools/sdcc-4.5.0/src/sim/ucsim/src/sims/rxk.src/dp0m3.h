/*
 * Simulator of microcontrollers (dp0m3.h)
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

#ifndef DP0M3_HEADER
#define DP0M3_HEADER

// opcodes valid in 3k mode only

#define LD_B_B		instruction_40
#define LD_B_C		instruction_41
#define LD_B_E		instruction_43
#define LD_B_H		instruction_44
#define LD_C_C		instruction_49
#define LD_C_D		instruction_4a
#define LD_C_E		instruction_4b

#define LD_D_D		instruction_52
#define LD_D_E		instruction_53
#define LD_E_B		instruction_58
#define LD_E_C		instruction_59
#define LD_E_D		instruction_5a
#define LD_E_H		instruction_5c
#define LD_E_L		instruction_5d

#define LD_H_H		instruction_64
#define LD_L_B		instruction_68
#define LD_L_C		instruction_69
#define LD_L_D		instruction_6a
#define LD_L_E		instruction_6b
#define LD_L_H		instruction_6c

#define ADD_A_B		instruction_80
#define ADC_A_B		instruction_88

#define SUB_A_B		instruction_90


// opcodes mean different insts in 3k/4k modes
// meaning of 3k mode follows

#define LD_B_D		instruction_42
#define LD_B_L		instruction_45
#define LD_C_B		instruction_48
#define LD_C_H		instruction_4c
#define LD_C_L		instruction_4d

#define LD_D_B		instruction_50
#define LD_D_C		instruction_51
#define LD_D_H		instruction_54
#define LD_D_L		instruction_55

#define LD_H_B		instruction_60
#define LD_H_C		instruction_61
#define LD_H_D		instruction_62
#define LD_H_E		instruction_63
#define LD_H_L		instruction_65
#define LD_L_L		instruction_6d

#define LD_A_A		instruction_7f

#define ADD_A_C		instruction_81
#define ADD_A_D		instruction_82
#define ADD_A_E		instruction_83
#define ADD_A_H		instruction_84
#define ADD_A_L		instruction_85
#define ADD_A_iHL	instruction_86
#define ADD_A_A		instruction_87
#define ADC_A_C		instruction_89
#define ADC_A_D		instruction_8a
#define ADC_A_E		instruction_8b
#define ADC_A_H		instruction_8c
#define ADC_A_L		instruction_8d
#define ADC_A_iHL	instruction_8e
#define ADC_A_A		instruction_8f

#define SUB_A_C		instruction_91
#define SUB_A_D		instruction_92
#define SUB_A_E		instruction_93
#define SUB_A_H		instruction_94
#define SUB_A_L		instruction_95
#define SUB_A_iHL	instruction_96
#define SUB_A_A		instruction_97
#define SBC_A_B		instruction_98
#define SBC_A_C		instruction_99
#define SBC_A_D		instruction_9a
#define SBC_A_E		instruction_9b
#define SBC_A_H		instruction_9c
#define SBC_A_L		instruction_9d
#define SBC_A_iHL	instruction_9e
#define SBC_A_A		instruction_9f

#define AND_A_B		instruction_a0
#define AND_A_C		instruction_a1
#define AND_A_D		instruction_a2
#define AND_A_E		instruction_a3
#define AND_A_H		instruction_a4
#define AND_A_L		instruction_a5
#define AND_A_iHL	instruction_a6
#define AND_A_A		instruction_a7
#define XOR_A_B		instruction_a8
#define XOR_A_C		instruction_a9
#define XOR_A_D		instruction_aa
#define XOR_A_E		instruction_ab
#define XOR_A_H		instruction_ac
#define XOR_A_L		instruction_ad
#define XOR_A_iHL	instruction_ae

#define OR_A_B		instruction_b0
#define OR_A_C		instruction_b1
#define OR_A_D		instruction_b2
#define OR_A_E		instruction_b3
#define OR_A_H		instruction_b4
#define OR_A_L		instruction_b5
#define OR_A_iHL	instruction_b6
#define CP_A_B		instruction_b8
#define CP_A_C		instruction_b9
#define CP_A_D		instruction_ba
#define CP_A_E		instruction_bb
#define CP_A_H		instruction_bc
#define CP_A_L		instruction_bd
#define CP_A_iHL	instruction_be
#define CP_A_A		instruction_bf

#endif

/* End of rxk.src/dp0m3.h */
