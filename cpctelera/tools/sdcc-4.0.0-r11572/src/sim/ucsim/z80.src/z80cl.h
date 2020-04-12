/*
 * Simulator of microcontrollers (z80cl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#ifndef Z80CL_HEADER
#define Z80CL_HEADER

#include "uccl.h"

#include "regsz80.h"

class cl_z80;
class cl_sp_limit_opt;

/*
 * Base type of Z80 microcontrollers
 */

class cl_z80: public cl_uc
{
public:
  class cl_memory *ram;
  class cl_memory *rom;
  struct t_regs regs;
  class cl_address_space *regs8;
  class cl_address_space *regs16;
  class cl_address_space *inputs;
  class cl_address_space *outputs;
  class cl_sp_limit_opt *sp_limit_opt;
  t_addr sp_limit;
public:
  cl_z80(struct cpu_entry *Itype, class cl_sim *asim);
  virtual int init(void);
  virtual char *id_string(void);

  //virtual t_addr get_mem_size(enum mem_class type);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);

  virtual struct dis_entry *dis_tbl(void);
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);
  virtual char *disass(t_addr addr, const char *sep);
  virtual void print_regs(class cl_console_base *con);

  virtual int exec_inst(void);

  virtual const char *get_disasm_info(t_addr addr,
                                      int *ret_len,
                                      int *ret_branch,
                                      int *immed_offset,
                                      struct dis_entry **dentry);
  virtual bool is_call(t_addr addr);
  
  virtual void store1( u16_t addr, t_mem val );
  virtual void store2( u16_t addr, u16_t val );

  virtual u8_t  get1( u16_t addr );
  virtual u16_t get2( u16_t addr );

  virtual t_mem fetch1( void );
  virtual u16_t fetch2( void );
  virtual t_mem peek1 ( void );

  virtual u8_t in_byte( u16_t ioaddr );
  virtual void out_byte( u16_t ioaddr, u8_t io_val );

  //virtual t_mem fetch(void);
  virtual u8_t reg_g_read ( t_mem g );
  virtual void reg_g_store( t_mem g, u8_t new_val );

  virtual void stack_check_overflow(class cl_stack_op *op);

  virtual int inst_nop(t_mem code);
  virtual int inst_ld(t_mem code);
  virtual int inst_inc(t_mem code);
  virtual int inst_dec(t_mem code);
  virtual int inst_rlca(t_mem code);
  virtual int inst_rrca(t_mem code);
  virtual int inst_ex(t_mem code);
  virtual int inst_add(t_mem code);
  virtual int inst_djnz(t_mem code);
  virtual int inst_jr(t_mem code);
  virtual int inst_rla(t_mem code);
  virtual int inst_rra(t_mem code);
  virtual int inst_daa(t_mem code);
  virtual int inst_cpl(t_mem code);
  virtual int inst_scf(t_mem code);
  virtual int inst_ccf(t_mem code);
  virtual int inst_halt(t_mem code);
  virtual int inst_adc(t_mem code);
  virtual int inst_sbc(t_mem code);
  virtual int inst_and(t_mem code);
  virtual int inst_xor(t_mem code);
  virtual int inst_or(t_mem code);
  virtual int inst_cp(t_mem code);
  virtual int inst_rst(t_mem code);
  virtual int inst_ret(t_mem code);
  virtual int inst_call(t_mem code);
  virtual int inst_out(t_mem code);
  virtual int inst_push(t_mem code);
  virtual int inst_exx(t_mem code);
  virtual int inst_in(t_mem code);
  virtual int inst_sub(t_mem code);
  virtual int inst_pop(t_mem code);
  virtual int inst_jp(t_mem code);
  virtual int inst_di(t_mem code);
  virtual int inst_ei(t_mem code);

  virtual int inst_fd(t_mem prefix);
  virtual int inst_fd_ld(t_mem code);
  virtual int inst_fd_add(t_mem code);
  virtual int inst_fd_push(t_mem code);
  virtual int inst_fd_inc(t_mem code);
  virtual int inst_fd_dec(t_mem code);
  virtual int inst_fd_misc(t_mem code);

  virtual int inst_dd(t_mem prefix);
  virtual int inst_dd_ld(t_mem code);
  virtual int inst_dd_add(t_mem code);
  virtual int inst_dd_push(t_mem code);
  virtual int inst_dd_inc(t_mem code);
  virtual int inst_dd_dec(t_mem code);
  virtual int inst_dd_misc(t_mem code);

  virtual int inst_ed(t_mem prefix);
  virtual int inst_ed_(t_mem code);

  virtual int inst_cb(void);
  virtual int inst_cb_rlc(t_mem code);
  virtual int inst_cb_rrc(t_mem code);
  virtual int inst_cb_rl(t_mem code);
  virtual int inst_cb_rr(t_mem code);
  virtual int inst_cb_sla(t_mem code);
  virtual int inst_cb_sra(t_mem code);
  virtual int inst_cb_slia(t_mem code);
  virtual int inst_cb_srl(t_mem code);
  virtual int inst_cb_bit(t_mem code);
  virtual int inst_cb_res(t_mem code);
  virtual int inst_cb_set(t_mem code);

  virtual int inst_ddcb(void);
  virtual int inst_ddcb_rlc(t_mem code);
  virtual int inst_ddcb_rrc(t_mem code);
  virtual int inst_ddcb_rl(t_mem code);
  virtual int inst_ddcb_rr(t_mem code);
  virtual int inst_ddcb_sla(t_mem code);
  virtual int inst_ddcb_sra(t_mem code);
  virtual int inst_ddcb_slia(t_mem code);
  virtual int inst_ddcb_srl(t_mem code);
  virtual int inst_ddcb_bit(t_mem code);
  virtual int inst_ddcb_res(t_mem code);
  virtual int inst_ddcb_set(t_mem code);

  virtual int inst_fdcb(void);
  virtual int inst_fdcb_rlc(t_mem code);
  virtual int inst_fdcb_rrc(t_mem code);
  virtual int inst_fdcb_rl(t_mem code);
  virtual int inst_fdcb_rr(t_mem code);
  virtual int inst_fdcb_sla(t_mem code);
  virtual int inst_fdcb_sra(t_mem code);
  virtual int inst_fdcb_slia(t_mem code);
  virtual int inst_fdcb_srl(t_mem code);
  virtual int inst_fdcb_bit(t_mem code);
  virtual int inst_fdcb_res(t_mem code);
  virtual int inst_fdcb_set(t_mem code);

  virtual int inst_dd_spec(t_mem code) { return -1; }
  virtual int inst_fd_spec(t_mem code) { return -1; }
};


unsigned   word_parity( u16_t  x );
/* returns parity for a 16-bit value */

enum z80cpu_confs
  {
   z80cpu_sp_limit	= 0,
   z80cpu_nuof		= 1
  };

class cl_z80_cpu: public cl_hw
{
public:
  cl_z80_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual int cfg_size(void) { return z80cpu_nuof; }
  virtual char *cfg_help(t_addr addr);

  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};

#endif

/* End of z80.src/z80cl.h */
