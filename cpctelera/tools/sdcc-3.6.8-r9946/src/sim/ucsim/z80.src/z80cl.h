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
  virtual u16_t  get2( u16_t addr );

  virtual t_mem       fetch1( void );
  virtual u16_t  fetch2( void );
  virtual t_mem       peek1 ( void );

  virtual u8_t   in_byte( u16_t ioaddr );
  virtual void        out_byte( u16_t ioaddr, u8_t io_val );

  //virtual t_mem fetch(void);
  virtual u8_t  reg_g_read ( t_mem g );
  virtual void        reg_g_store( t_mem g, u8_t new_val );

#include "instcl.h"
};


unsigned   word_parity( u16_t  x );
/* returns parity for a 16-bit value */

#endif

/* End of z80.src/z80cl.h */
