/*
 * Simulator of microcontrollers (dpedm3.h)
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

#ifndef DPEDM3_HEADER
#define DPEDM3_HEADER

#define LD_EIR_A	instruction_ed_47
#define LD_IIR_A	instruction_ed_4f
#define LD_A_EIR	instruction_ed_57
#define LD_A_IIR	instruction_ed_5f
#define LDI		instruction_ed_a0
#define LDD		instruction_ed_a8
#define LDDR		instruction_ed_b8
#define LDIR		instruction_ed_b0
#define EXX_iSP_HL	instruction_ed_54
#define LD_BC_imn	instruction_ed_4b
#define LD_DE_imn	instruction_ed_5b
#define LD_HL_imn_ped	instruction_ed_6b
#define LD_SP_imn	instruction_ed_7b
#define LD_aBC_BC	instruction_ed_49
#define LD_aDE_BC	instruction_ed_59
#define LD_aHL_BC	instruction_ed_69
#define LD_aBC_DE	instruction_ed_41
#define LD_aDE_DE	instruction_ed_51
#define LD_aHL_DE	instruction_ed_61
#define LD_imn_BC	instruction_ed_43
#define LD_imn_DE	instruction_ed_53
#define LD_imn_HL_ed	instruction_ed_63
#define LD_imn_SP	instruction_ed_73
#define NEG		instruction_ed_44
#define LRET		instruction_ed_45
#define IPSET_0		instruction_ed_46
#define IPSET_1		instruction_ed_56
#define IPSET_2		instruction_ed_4e
#define IPSET_3		instruction_ed_5e
#define RETI		instruction_ed_4d
#define IPRES		instruction_ed_5d
#define LDP_iHL_HL	instruction_ed_64
#define LDP_imn_HL	instruction_ed_65
#define LDP_HL_iHL	instruction_ed_6c
#define LDP_HL_imn	instruction_ed_6d
#define LD_XPC_A	instruction_ed_67
#define LD_A_XPC	instruction_ed_77
#define PUSH_IP		instruction_ed_76
#define POP_IP		instruction_ed_7e

#define SBC_HL_BC	instruction_ed_42
#define SBC_HL_DE	instruction_ed_52
#define SBC_HL_HL	instruction_ed_62
#define SBC_HL_SP	instruction_ed_72
#define ADC_HL_BC	instruction_ed_4a
#define ADC_HL_DE	instruction_ed_5a
#define ADC_HL_HL	instruction_ed_6a
#define ADC_HL_SP	instruction_ed_7a

#endif

/* End of rxk.src/dpedm3.h */
