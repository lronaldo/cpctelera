/*
 * Simulator of microcontrollers (tlcs.src/tlcscl.h)
 *
 * Copyright (C) 2016,16 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

#ifndef TLCSCL_HEADER
#define TLCSCL_HEADER

// sim.src
#include "uccl.h"
#include "memcl.h"


enum tlcs_flags {
  FLAG_S= 0x80,
  FLAG_Z= 0x40,
  FLAG_I= 0x20,
  FLAG_H= 0x10,
  FLAG_X= 0x08,
  FLAG_V= 0x04,
  FLAG_N= 0x02,
  FLAG_C= 0x01
};

/*
 * Base type of TLCS microcontrollers
 */

class cl_tlcs: public cl_uc
{
 protected:
  struct {
    union {
      u16_t af;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t a;
	u8_t f;
#else
	u8_t f;
	u8_t a;
#endif
      } raf;
    };
    union {
      u16_t bc;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t b;
	u8_t c;
#else
	u8_t c;
	u8_t b;
#endif
      } rbc;
    };
    union {
      u16_t de;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t d;
	u8_t e;
#else
	u8_t e;
	u8_t d;
#endif
       } rde;
    };
    union {
      u16_t hl;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t h;
	u8_t l;
#else
	u8_t l;
	u8_t h;
#endif
      } rhl;
    };
    u16_t ix;
    u16_t iy;
    u16_t sp;
    u8_t dummy;
    u16_t dummy16;
    union {
      u16_t alt_af;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t alt_a;
	u8_t alt_f;
#else
	u8_t alt_f;
	u8_t alt_a;
#endif
      } ralt_af;
    };
    union {
      u16_t alt_bc;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t alt_b;
	u8_t alt_c;
#else
	u8_t alt_c;
	u8_t alt_b;
#endif
      } ralt_bc;
    };
    union {
      u16_t alt_de;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t alt_d;
	u8_t alt_e;
#else
	u8_t alt_e;
	u8_t alt_d;
#endif
      } ralt_de;
    };
    union {
      u16_t alt_hl;
      struct {
#ifdef WORDS_BIGENDIAN
	u8_t alt_h;
	u8_t alt_l;
#else
	u8_t alt_l;
	u8_t alt_h;
#endif
      } ralt_hl;
    };
  } reg;

 public:
  class cl_address_space *nas;
  class cl_address_space *xas;
  class cl_address_space *yas;
  class cl_address_space *regs8;
  class cl_address_space *regs16;
  t_addr sp_limit;
 public:
  cl_tlcs(class cl_sim *asim);
  virtual int init(void);
  virtual char *id_string(void);

  //virtual t_addr get_mem_size(enum mem_class type);
  //virtual int get_mem_width(enum mem_class type);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);

  virtual struct dis_entry *dis_tbl(void);
  //virtual struct name_entry *sfr_tbl(void);
  //virtual struct name_entry *bit_tbl(void);
  virtual const char *regname_r(u8_t r);
  virtual const char *regname_Q(u8_t Q);
  virtual const char *regname_R(u8_t R);
  virtual const char *regname_i(u8_t i);
  virtual const char *bitname(u8_t b);
  virtual const char *condname_cc(u8_t cc);
  virtual const char *condname_C(u8_t cc);
  virtual char *disass(t_addr addr, const char *sep);
  virtual void print_regs(class cl_console_base *con);
  virtual int inst_length(t_addr addr);

  virtual void stack_check_overflow(class cl_stack_op *op);
  
  virtual int exec_inst(void);
  virtual int exec_inst2(u8_t c1);
  virtual int exec_inst2_f3(u8_t c1);
  virtual int exec_inst2_fe(u8_t c1);
  virtual int exec_inst2_f7(u8_t c1);
  virtual int exec_inst2_e0gg(u8_t c1, u8_t c2);
  virtual int exec_inst2_e8gg(u8_t c1, u8_t c2);
  virtual int exec_inst2_f8gg(u8_t c1, u8_t c2);
  virtual int exec_inst3_e7(u8_t c1, u8_t c2, u8_t c3);
  virtual int exec_inst3(u8_t c1, u8_t c2);
  virtual int exec_inst3_f0ix(u8_t c1);
  virtual int exec_inst4_f4ix(u8_t c1, u8_t c2, u8_t c3);
  virtual int exec_inst4_e3(u8_t c1, u8_t c2, u8_t c3, u8_t c4);
  virtual int exec_inst4_ef(u8_t c1, u8_t c2, u8_t c3);
  virtual int exec_inst4_eb(u8_t c1, u8_t c2, u8_t c3, u8_t c4);
  
  virtual t_addr do_push(t_mem data);
  virtual t_addr do_pop(t_mem *data);
  virtual int exec_push(t_addr PC_of_inst, t_mem data);
  virtual int exec_ret(t_addr PC_of_inst, t_mem *data);
  virtual int exec_reti(t_addr PC_of_inst, t_mem *data);
  virtual int exec_pop(t_addr PC_of_inst, t_mem *data);
  virtual int exec_intr(t_addr PC_of_inst, t_addr called, t_mem data);
  virtual int exec_call(t_addr PC_of_inst, t_addr called, t_mem data);
  virtual void set_p(u8_t data);
  virtual u8_t *aof_reg8(u8_t data_r);
  virtual u16_t *aof_reg16_rr(u8_t data_rr);
  virtual u16_t *aof_reg16_qq(u8_t data_qq);
  virtual u16_t *aof_reg16_ix(u8_t data_ix);
  virtual u16_t *aof_reg16_gg(u8_t data_gg);
  virtual class cl_memory_cell *cell_hl_a();
  virtual class cl_memory_cell *cell_gg(u8_t gg);
  virtual class cl_memory_cell *cell_n(u8_t n);
  virtual class cl_memory_cell *cell_ixd(u8_t ix, i8_t d);
  
  virtual u16_t mem16(t_addr addr);
  virtual u16_t mem16gg(u8_t gg);
  virtual u16_t mem16ixd(u8_t ix, i8_t d);
  virtual void write16(t_addr addr, u16_t val);
  virtual void write16gg(u8_t gg, u16_t val);
  virtual void write16ixd(u8_t ix, i8_t d, u16_t val);
  virtual bool flag(enum tlcs_flags f);
  virtual bool cc(u8_t cc);
  
  // (1) 8-bit data transfer

  // (2) 16-bit data transfer
  virtual int inst_pop(t_mem c1);	// 58+qq
  
  // (3) exchange, block transfer and search
  virtual int ex_de_hl();		// 08
  virtual int ex_af_alt_af();		// 09
  virtual int exx();			// 0a
  virtual int ldi();			// Fe 58
  virtual int ldir();			// Fe 59
  virtual int ldd();			// Fe 5a
  virtual int lddr();			// Fe 5b
  virtual int cpi();			// Fe 5c
  virtual int cpir();		       	// Fe 5d
  virtual int cpd();			// Fe 5e
  virtual int cpdr();			// Fe 5f
  
  // (4) 8-bit arithmetic and logic operation
  virtual u8_t op_inc(u8_t data);			// INC 8 bit
  virtual void inst_inc(cl_memory_cell *cell);		// INC mem
  virtual void inst_incx(cl_memory_cell *cell);		// INCX mem
  virtual u8_t op_dec(u8_t data);			// DEC 8 bit
  virtual void inst_dec(cl_memory_cell *cell);		// DEC mem
  virtual void inst_decx(cl_memory_cell *cell);		// DECX mem
  virtual u8_t op_add8(u8_t d1, u8_t d2);	// ADD 8-bit
  virtual u8_t op_add_a(u8_t d);			// ADD A,8-bit
  virtual int inst_add_a(class cl_memory_cell *cell);	// ADD A,mem
  virtual u8_t op_adc8(u8_t d1, u8_t d2);	// ADC 8-bit
  virtual int inst_adc_a(u8_t d);			// ADC A,8-bit
  virtual int inst_adc_a(class cl_memory_cell *cell);	// ADC A,mem
  virtual u8_t op_sub8(u8_t d1, u8_t d2);	// SUB 8-bit
  virtual int inst_sub_a(u8_t d);			// SUB A,8-bit
  virtual int inst_sub_a(class cl_memory_cell *cell);	// SUB A,mem
  virtual u8_t op_sbc8(u8_t d1, u8_t d2);	// SBC 8-bit
  virtual int inst_sbc_a(u8_t d);			// SBC A,8-bit
  virtual int inst_sbc_a(class cl_memory_cell *cell);	// SBC A,mem
  virtual u8_t op_and8(u8_t d1, u8_t d2);	// AND 8-bit
  virtual int inst_and_a(u8_t d);			// AND A,8-bit
  virtual int inst_and_a(class cl_memory_cell *cell);	// AND A,mem
  virtual u8_t op_xor8(u8_t d1, u8_t d2);	// XOR 8-bit
  virtual int inst_xor_a(u8_t d);			// XOR A,8-bit
  virtual int inst_xor_a(class cl_memory_cell *cell);	// XOR A,mem
  virtual u8_t op_or8(u8_t d1, u8_t d2);	// OR 8-bit
  virtual int inst_or_a(u8_t d);			// OR A,8-bit
  virtual int inst_or_a(class cl_memory_cell *cell);	// OR A,mem
  virtual u8_t op_cp8(u8_t d1, u8_t d2);	// CP 8-bit
  virtual int op_cp_a(u8_t d);			// CP A,8-bit
  virtual int op_cp_a(class cl_memory_cell *cell);	// CP A,mem

  // (5) 16-bit arithmetic and logic operation
  virtual u16_t op_inc16(u16_t data);		// INC 16 bit
  virtual u16_t inst_inc16(t_addr addr);		// INCW mem
  virtual u16_t inst_inc16gg(u8_t gg,t_addr addr);// INCW mem
  virtual u16_t inst_inc16ix(u8_t ix,t_addr addr);// INCW mem
  virtual u16_t op_dec16(t_mem data);		// DEC 16 bit
  virtual u16_t inst_dec16(t_addr addr);		// DECW mem
  virtual u16_t inst_dec16gg(u8_t gg,t_addr addr);// DECW mem
  virtual u16_t inst_dec16ix(u8_t ix,t_addr addr);// DECW mem
  virtual u16_t op_add_hl(t_mem val);		// ADD HL,16-bit
  virtual u16_t op_add_hl(t_addr addr);		// ADD HL,mem
  virtual u16_t op_adc_hl(t_mem val);		// ADC HL,16-bit
  virtual u16_t op_adc_hl(t_addr addr);		// ADC HL,mem
  virtual u16_t op_sub_hl(t_mem val);		// SUB HL,16-bit
  virtual u16_t op_sub_hl(t_addr addr);		// SUB HL,mem
  virtual u16_t op_sbc_hl(t_mem val);		// SBC HL,16-bit
  virtual u16_t op_sbc_hl(t_addr addr);		// SBC HL,mem
  virtual u16_t op_and_hl(t_mem val);		// AND HL,16-bit
  virtual u16_t op_and_hl(t_addr addr);		// AND HL,mem
  virtual u16_t op_xor_hl(t_mem val);		// XOR HL,16-bit
  virtual u16_t op_xor_hl(t_addr addr);		// XOR HL,mem
  virtual u16_t op_or_hl(t_mem val);			// OR HL,16-bit
  virtual u16_t op_or_hl(t_addr addr);		// OR HL,mem
  virtual u16_t op_add16(t_mem op1, t_mem op2);	// ADD 16-bit
  virtual u16_t op_sub16(t_mem op1, t_mem op2);	// SUB 16-bit

  // (6) cpu control and others
  virtual int inst_daa_a();		// 0b
  virtual int inst_cpl_a();		// 10
  virtual int inst_neg_a();		// 11
  virtual int inst_ccf();		// 0e
  virtual int inst_scf();		// 0d
  virtual int inst_rcf();		// 0c
  //virtual int nop();			// 00
  //virtual int halt();			// 01
  //virtual int di();			// 02
  //virtual int ei();			// 03
  virtual int inst_swi();		// ff
  virtual int inst_mul_hl(class cl_memory_cell *cell);
  virtual int inst_div_hl(class cl_memory_cell *cell);
  virtual int inst_div_hl(u8_t d);
  
  // (7) rotate and shift
  virtual u8_t op_rlc(u8_t data, bool set_sz);	// RLC 8-bit
  virtual u8_t inst_rlc(cl_memory_cell *cell);	// RLC mem
  virtual u8_t op_rrc(u8_t data, bool set_sz);	// RRC 8-bit
  virtual u8_t inst_rrc(cl_memory_cell *cell);	// RRC mem
  virtual u8_t op_rl(u8_t data, bool set_sz);	// RL 8-bit
  virtual u8_t inst_rl(cl_memory_cell *cell);	// RL mem
  virtual u8_t op_rr(u8_t data, bool set_sz);	// RR 8-bit
  virtual u8_t inst_rr(cl_memory_cell *cell);	// RR mem
  virtual u8_t op_sla(u8_t data, bool set_sz);	// SLA 8-bit
  virtual u8_t inst_sla(cl_memory_cell *cell);	// SLA mem
  virtual u8_t op_sra(u8_t data, bool set_sz);	// SRA 8-bit
  virtual u8_t inst_sra(cl_memory_cell *cell);	// SRA mem
  virtual u8_t op_srl(u8_t data, bool set_sz);	// SRL 8-bit
  virtual u8_t inst_srl(cl_memory_cell *cell);	// SRL mem
  virtual int inst_rld(class cl_memory_cell *cell);	// RLD
  virtual int inst_rrd(class cl_memory_cell *cell);	// RLD

  // (8) bit manipulation
  virtual u8_t op_tset(u8_t val, u8_t bitnr);	// TSET 8-bit
  virtual u8_t inst_tset(cl_memory_cell *cell, u8_t bitnr);
  virtual u8_t op_bit(u8_t val, u8_t bitnr);	// BIT 8-bit
  virtual u8_t inst_bit(cl_memory_cell *cell, u8_t bitnr);
  virtual u8_t op_res(u8_t val, u8_t bitnr);	// RES 8-bit
  virtual u8_t inst_res(cl_memory_cell *cell, u8_t bitnr);
  virtual u8_t op_set(u8_t val, u8_t bitnr);	// SET 8-bit
  virtual u8_t inst_set(cl_memory_cell *cell, u8_t bitnr);

  // (9) jump, call and return
  virtual int inst_ret();			// 1e
  virtual int inst_reti();			// 1f
  virtual int inst_call(t_addr PC_of_inst, u16_t addr);	// CALL addr
  virtual int inst_djnz_b(i8_t d);
  virtual int inst_djnz_bc(i8_t d);
  
  // ?
};


enum tlcscpu_confs
  {
   tlcscpu_sp_limit	= 0,
   tlcscpu_nuof		= 1
  };

class cl_tlcs_cpu: public cl_hw
{
public:
  cl_tlcs_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual int cfg_size(void) { return tlcscpu_nuof; }
  virtual char *cfg_help(t_addr addr);

  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};

#endif

/* End of tlcs.src/tlcscl.h */
