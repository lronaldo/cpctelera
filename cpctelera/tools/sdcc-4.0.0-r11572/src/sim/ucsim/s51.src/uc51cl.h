/*
 * Simulator of microcontrollers (uc51cl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/*
  This file is part of microcontroller simulator: ucsim.

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
  02111-1307, USA.
*/
/*@1@*/

#ifndef UC51CL_HEADER
#define UC51CL_HEADER

#include <stdio.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif

#include "pobjcl.h"

#include "simcl.h"
#include "memcl.h"
#include "uccl.h"
#include "itsrccl.h"
#include "brkcl.h"
#include "stypes.h"

#include "interruptcl.h"


class t_uc51;

class cl_irq_stop_option: public cl_optref
{
protected:
  class cl_51core *uc51;
public:
  cl_irq_stop_option(class cl_51core *the_uc51);
  virtual int init(void);
  virtual void option_changed(void);
};

class cl_51core: public cl_uc
{
public:
  // Options
  //bool debug;
  class cl_irq_stop_option *irq_stop_option;
  bool stop_at_it;

  // memories and cells for faster access
  class cl_address_space *sfr, *iram, *xram, *regs, *bits;
  class cl_address_space *dptr;
  class cl_memory_cell *acc, *psw, *R[8];
  class cl_memory_chip *rom_chip, *sfr_chip, *iram_chip, *xram_chip;
  
public:
  // Help to detect external it requests (falling edge)
  uchar prev_p1;	// Prev state of P1
  uchar prev_p3;	// Prev state of P3
  int p3_int0_edge, p3_int1_edge;

public:
  // Simulation of interrupt system
  class cl_interrupt *interrupt;
  //bool  was_reti;	// Instruction had an effect on IE

public:
  int result;		// result of instruction execution

  cl_51core(struct cpu_entry *Itype, class cl_sim *asim);
  virtual ~cl_51core(void);
  virtual int    init(void);
  virtual char  *id_string(void);
  virtual void make_cpu_hw(void);
  virtual void mk_hw_elements(void);
  virtual void build_cmdset(class cl_cmdset *cmdset);
  //virtual class cl_m *mk_mem(enum mem_class type, char *class_name);
  virtual void make_memories(void);
  virtual void make_address_spaces(void);
  virtual void make_chips(void);
  virtual void decode_regs(void);
  virtual void decode_bits(void);
  virtual void decode_rom(void);
  virtual void decode_iram(void);
  virtual void decode_sfr(void);
  virtual void decode_xram(void);
  virtual void decode_dptr(void);
  virtual void make_vars(void);
  
  virtual int clock_per_cycle(void) { return(12); }
  virtual struct dis_entry *dis_tbl(void);
  virtual struct name_entry *bit_tbl(void);
  virtual char *disass(t_addr addr, const char *sep);
  virtual void   print_regs(class cl_console_base *con);
  virtual class cl_address_space *bit2mem(t_addr bitaddr,
					  t_addr *memaddr, t_mem *bitmask);
  virtual t_addr bit_address(class cl_memory *mem,
			     t_addr mem_address,
			     int bit_number);
  virtual void   daddr_name(t_addr addr, char *buf);
  virtual void   baddr_name(t_addr addr, char *buf);
  
  virtual void   reset(void);
  virtual void   clear_sfr(void);
  virtual void   analyze(t_addr addr);

  virtual int    do_inst(int step);

  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);

  virtual int  priority_of(uchar nuof_it);
  virtual int  do_interrupt(void);
  virtual int  accept_it(class it_level *il);
  virtual bool it_enabled(void);

  virtual void stack_check_overflow(class cl_stack_op *op);
  
protected:
  virtual int  idle_pd(void);

  virtual class cl_memory_cell *get_direct(t_mem addr);

  virtual int   exec_inst(void);
  //virtual void  post_inst(void);

  virtual int inst_unknown(void);
  virtual int instruction_00/*inst_nop*/(t_mem/*uchar*/ code);		/* 00 */
  virtual int instruction_01/*inst_ajmp_addr*/(t_mem/*uchar*/ code);	/* [02468ace]1 */
  virtual int instruction_02/*inst_ljmp*/(t_mem/*uchar*/ code);		/* 02 */
  virtual int instruction_03/*inst_rr*/(t_mem/*uchar*/ code);		/* 03 */
  virtual int instruction_04/*inst_inc_a*/(t_mem/*uchar*/ code);	/* 04 */
  virtual int instruction_05/*inst_inc_addr*/(t_mem/*uchar*/ code);	/* 05 */
  virtual int instruction_06/*inst_inc_Sri*/(t_mem/*uchar*/ code);	/* 06,07 */
  virtual int instruction_08/*inst_inc_rn*/(t_mem/*uchar*/ code);	/* 08-0f */
  virtual int instruction_10/*inst_jbc_bit_addr*/(t_mem/*uchar*/ code);	/* 10 */
  virtual int instruction_11/*inst_acall_addr*/(t_mem/*uchar*/ code);	/* [13579bdf]1 */
  virtual int inst_lcall(t_mem code, uint addr, bool intr);		/* 12 */
  virtual int instruction_12(t_mem code) { return inst_lcall(code, 0, false); }
  virtual int instruction_13/*inst_rrc*/(t_mem/*uchar*/ code);		/* 13 */
  virtual int instruction_14/*inst_dec_a*/(t_mem/*uchar*/ code);	/* 14 */
  virtual int instruction_15/*inst_dec_addr*/(t_mem/*uchar*/ code);	/* 15 */
  virtual int instruction_16/*inst_dec_Sri*/(t_mem/*uchar*/ code);	/* 16,17 */
  virtual int instruction_18/*inst_dec_rn*/(t_mem/*uchar*/ code);	/* 18-1f */
  virtual int instruction_20/*inst_jb_bit_addr*/(t_mem/*uchar*/ code);	/* 20 */
  virtual int instruction_22/*inst_ret*/(t_mem/*uchar*/ code);		/* 22 */
  virtual int instruction_23/*inst_rl*/(t_mem/*uchar*/ code);		/* 23 */
  virtual int instruction_24/*inst_add_a_Sdata*/(t_mem/*uchar*/ code);	/* 24 */
  virtual int instruction_25/*inst_add_a_addr*/(t_mem/*uchar*/ code);	/* 25 */
  virtual int instruction_26/*inst_add_a_Sri*/(t_mem/*uchar*/ code);	/* 26,27 */
  virtual int instruction_28/*inst_add_a_rn*/(t_mem/*uchar*/ code);	/* 28-2f */
  virtual int instruction_30/*inst_jnb_bit_addr*/(t_mem/*uchar*/ code);	/* 30 */
  virtual int instruction_32/*inst_reti*/(t_mem/*uchar*/ code);		/* 32 */
  virtual int instruction_33/*inst_rlc*/(t_mem/*uchar*/ code);		/* 33 */
  virtual int instruction_34/*inst_addc_a_Sdata*/(t_mem/*uchar*/ code);	/* 34 */
  virtual int instruction_35/*inst_addc_a_addr*/(t_mem/*uchar*/ code);	/* 35 */
  virtual int instruction_36/*inst_addc_a_Sri*/(t_mem/*uchar*/ code);	/* 36,37 */
  virtual int instruction_38/*inst_addc_a_rn*/(t_mem/*uchar*/ code);	/* 38-3f */
  virtual int instruction_40/*inst_jc_addr*/(t_mem/*uchar*/ code);	/* 40 */
  virtual int instruction_42/*inst_orl_addr_a*/(t_mem/*uchar*/ code);	/* 42 */
  virtual int instruction_43/*inst_orl_addr_Sdata*/(t_mem/*uchar*/ code);/* 43 */
  virtual int instruction_44/*inst_orl_a_Sdata*/(t_mem/*uchar*/ code);	/* 44 */
  virtual int instruction_45/*inst_orl_a_addr*/(t_mem/*uchar*/ code);	/* 45 */
  virtual int instruction_46/*inst_orl_a_Sri*/(t_mem/*uchar*/ code);	/* 46,47 */
  virtual int instruction_48/*inst_orl_a_rn*/(t_mem/*uchar*/ code);	/* 48-4f */
  virtual int instruction_50/*inst_jnc_addr*/(t_mem/*uchar*/ code);	/* 50 */
  virtual int instruction_52/*inst_anl_addr_a*/(t_mem/*uchar*/ code);	/* 52 */
  virtual int instruction_53/*inst_anl_addr_Sdata*/(t_mem/*uchar*/ code);/* 53 */
  virtual int instruction_54/*inst_anl_a_Sdata*/(t_mem/*uchar*/ code);	/* 54 */
  virtual int instruction_55/*inst_anl_a_addr*/(t_mem/*uchar*/ code);	/* 55 */
  virtual int instruction_56/*inst_anl_a_Sri*/(t_mem/*uchar*/ code);	/* 56,57 */
  virtual int instruction_58/*inst_anl_a_rn*/(t_mem/*uchar*/ code);	/* 58-5f */
  virtual int instruction_60/*inst_jz_addr*/(t_mem/*uchar*/ code);	/* 60 */
  virtual int instruction_62/*inst_xrl_addr_a*/(t_mem/*uchar*/ code);	/* 62 */
  virtual int instruction_63/*inst_xrl_addr_Sdata*/(t_mem/*uchar*/ code);/* 63 */
  virtual int instruction_64/*inst_xrl_a_Sdata*/(t_mem/*uchar*/ code);	/* 64 */
  virtual int instruction_65/*inst_xrl_a_addr*/(t_mem/*uchar*/ code);	/* 65 */
  virtual int instruction_66/*inst_xrl_a_Sri*/(t_mem/*uchar*/ code);	/* 66,67 */
  virtual int instruction_68/*inst_xrl_a_rn*/(t_mem/*uchar*/ code);	/* 68-6f */
  virtual int instruction_70/*inst_jnz_addr*/(t_mem/*uchar*/ code);	/* 70 */
  virtual int instruction_72/*inst_orl_c_bit*/(t_mem/*uchar*/ code);	/* 72 */
  virtual int instruction_73/*inst_jmp_Sa_dptr*/(t_mem/*uchar*/ code);	/* 73 */
  virtual int instruction_74/*inst_mov_a_Sdata*/(t_mem/*uchar*/ code);	/* 74 */
  virtual int instruction_75/*inst_mov_addr_Sdata*/(t_mem/*uchar*/ code);/* 75 */
  virtual int instruction_76/*inst_mov_Sri_Sdata*/(t_mem/*uchar*/ code);/* 76,77 */
  virtual int instruction_78/*inst_mov_rn_Sdata*/(t_mem/*uchar*/ code);	/* 78-7f */
  virtual int instruction_80/*inst_sjmp*/(t_mem/*uchar*/ code);		/* 80 */
  virtual int instruction_82/*inst_anl_c_bit*/(t_mem/*uchar*/ code);	/* 82 */
  virtual int instruction_83/*inst_movc_a_Sa_pc*/(t_mem/*uchar*/ code);	/* 83 */
  virtual int instruction_84/*inst_div_ab*/(t_mem/*uchar*/ code);	/* 84 */
  virtual int instruction_85/*inst_mov_addr_addr*/(t_mem/*uchar*/ code);/* 85 */
  virtual int instruction_86/*inst_mov_addr_Sri*/(t_mem/*uchar*/ code);	/* 86,87 */
  virtual int instruction_88/*inst_mov_addr_rn*/(t_mem/*uchar*/ code);	/* 88-8f */
  virtual int instruction_90/*inst_mov_dptr_Sdata*/(t_mem/*uchar*/ code);/* 90 */
  virtual int instruction_92/*inst_mov_bit_c*/(t_mem/*uchar*/ code);	/* 92 */
  virtual int instruction_93/*inst_movc_a_Sa_dptr*/(t_mem/*uchar*/ code);/* 93 */
  virtual int instruction_94/*inst_subb_a_Sdata*/(t_mem/*uchar*/ code);	/* 94 */
  virtual int instruction_95/*inst_subb_a_addr*/(t_mem/*uchar*/ code);	/* 95 */
  virtual int instruction_96/*inst_subb_a_Sri*/(t_mem/*uchar*/ code);	/* 96,97 */
  virtual int instruction_98/*inst_subb_a_rn*/(t_mem/*uchar*/ code);	/* 98-9f */
  virtual int instruction_a0/*inst_orl_c_Sbit*/(t_mem/*uchar*/ code);	/* a0 */
  virtual int instruction_a2/*inst_mov_c_bit*/(t_mem/*uchar*/ code);	/* a2 */
  virtual int instruction_a3/*inst_inc_dptr*/(t_mem/*uchar*/ code);	/* a3 */
  virtual int instruction_a4/*inst_mul_ab*/(t_mem/*uchar*/ code);	/* a4 */
  virtual int instruction_a6/*inst_mov_Sri_addr*/(t_mem/*uchar*/ code);	/* a6,a7 */
  virtual int instruction_a8/*inst_mov_rn_addr*/(t_mem/*uchar*/ code);	/* a8-af */
  virtual int instruction_b0/*inst_anl_c_Sbit*/(t_mem/*uchar*/ code);	/* b0 */
  virtual int instruction_b2/*inst_cpl_bit*/(t_mem/*uchar*/ code);	/* b2 */
  virtual int instruction_b3/*inst_cpl_c*/(t_mem/*uchar*/ code);	/* b3 */
  virtual int instruction_b4/*inst_cjne_a_Sdata_addr*/(t_mem/*uchar*/ code);/* b4 */
  virtual int instruction_b5/*inst_cjne_a_addr_addr*/(t_mem/*uchar*/ code);/* b5 */
  virtual int instruction_b6/*inst_cjne_Sri_Sdata_addr*/(t_mem/*uchar*/ code);/* b6,b7 */
  virtual int instruction_b8/*inst_cjne_rn_Sdata_addr*/(t_mem/*uchar*/ code);/* b8-bf */
  virtual int instruction_c0/*inst_push*/(t_mem/*uchar*/ code);		/* c0 */
  virtual int instruction_c2/*inst_clr_bit*/(t_mem/*uchar*/ code);	/* c2 */
  virtual int instruction_c3/*inst_clr_c*/(t_mem/*uchar*/ code);	/* c3*/
  virtual int instruction_c4/*inst_swap*/(t_mem/*uchar*/ code);		/* c4 */
  virtual int instruction_c5/*inst_xch_a_addr*/(t_mem/*uchar*/ code);	/* c5 */
  virtual int instruction_c6/*inst_xch_a_Sri*/(t_mem/*uchar*/ code);	/* c6,c7 */
  virtual int instruction_c8/*inst_xch_a_rn*/(t_mem/*uchar*/ code);	/* c8-cf */
  virtual int instruction_d0/*inst_pop*/(t_mem/*uchar*/ code);		/* d0 */
  virtual int instruction_d2/*inst_setb_bit*/(t_mem/*uchar*/ code);	/* d2 */
  virtual int instruction_d3/*inst_setb_c*/(t_mem/*uchar*/ code);	/* d3 */
  virtual int instruction_d4/*inst_da_a*/(t_mem/*uchar*/ code);		/* d4 */
  virtual int instruction_d5/*inst_djnz_addr_addr*/(t_mem/*uchar*/ code);/* d5 */
  virtual int instruction_d6/*inst_xchd_a_Sri*/(t_mem/*uchar*/ code);	/* d6,d7 */
  virtual int instruction_d8/*inst_djnz_rn_addr*/(t_mem/*uchar*/ code);	/* d8-df */
  virtual int instruction_e0/*inst_movx_a_Sdptr*/(t_mem/*uchar*/ code);	/* e0 */
  virtual int instruction_e2/*inst_movx_a_Sri*/(t_mem/*uchar*/ code);	/* e2,e3 */
  virtual int instruction_e4/*inst_clr_a*/(t_mem/*uchar*/ code);	/* e4 */
  virtual int instruction_e5/*inst_mov_a_addr*/(t_mem/*uchar*/ code);	/* e5 */
  virtual int instruction_e6/*inst_mov_a_Sri*/(t_mem/*uchar*/ code);	/* e6,e7 */
  virtual int instruction_e8/*inst_mov_a_rn*/(t_mem/*uchar*/ code);	/* e8-ef */
  virtual int instruction_f0/*inst_movx_Sdptr_a*/(t_mem/*uchar*/ code);	/* f0 */
  virtual int instruction_f2/*inst_movx_Sri_a*/(t_mem/*uchar*/ code);	/* f2,f3 */
  virtual int instruction_f4/*inst_cpl_a*/(t_mem/*uchar*/ code);	/* f4 */
  virtual int instruction_f5/*inst_mov_addr_a*/(t_mem/*uchar*/ code);	/* f5 */
  virtual int instruction_f6/*inst_mov_Sri_a*/(t_mem/*uchar*/ code);	/* f6,f7 */
  virtual int instruction_f8/*inst_mov_rn_a*/(t_mem/*uchar*/ code);	/* f8-ff */
};


enum uc51cpu_cfg {
  uc51cpu_aof_mdps	= 0, // addr of multi_DPTR_sfr selector
  uc51cpu_mask_mdps	= 1, // mask in mutli_DPTR_sfr selector
  uc51cpu_aof_mdps1l	= 2, // addr of multi_DPTR_sfr DPL1
  uc51cpu_aof_mdps1h	= 3, // addr of multi_DPTR_sfr DPH1

  uc51cpu_aof_mdpc	= 4, // addr of multi_DPTR_chip selector
  uc51cpu_mask_mdpc	= 5, // mask in multi_DPTR_chip selector
  
  uc51cpu_nuof		= 16
};

class cl_uc51_cpu: public cl_hw
{
 protected:
  class cl_memory_cell *cell_acc, *cell_sp, *cell_psw;
  class cl_memory_cell *acc_bits[8];
 public:
  cl_uc51_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual int cfg_size(void) { return uc51cpu_nuof; }
  virtual char *cfg_help(t_addr addr);
  
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};


#endif

/* End of s51.src/uc51cl.h */
