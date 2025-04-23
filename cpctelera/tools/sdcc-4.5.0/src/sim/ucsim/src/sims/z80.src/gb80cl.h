/*
 * Simulator of microcontrollers (gb80cl.h)
 *
 * Copyright (C) 2021 Drotos Daniel
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

#ifndef GB80CL_HEADER
#define GB80CL_HEADER


#include "z80cl.h"

class cl_gb80;

const t_addr  lr35902_rom_start = 0x0000;
const t_addr  lr35902_rom_size  = 0x6000;

const t_addr  lr35902_ram_start = 0xA000;
const t_addr  lr35902_ram_size  = 0x5F80;

class cl_gb80: public cl_z80
{
public:
  cl_gb80(struct cpu_entry *Itype, class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);
  
  virtual struct dis_entry *dis_tbl(void);
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);
  virtual char *disass(t_addr addr);
  virtual void print_regs(class cl_console_base *con);

  virtual int exec_inst(void);
  virtual int tickt(t_mem code);
  virtual void xy(u8_t v) {}

  virtual const char *get_disasm_info(t_addr addr,
                        int *ret_len,
                        int *ret_branch,
                        int *immed_offset);
  
  virtual void store1( u16_t addr, t_mem val );
  virtual void store2( u16_t addr, u16_t val );
  
  virtual u8_t  get1( u16_t addr );
  virtual u16_t  get2( u16_t addr );
  
  
  // see #include "instcl.h" for Z80 versions
  /* instruction function that are add / modified from the Z80 versions */
  virtual int inst_cb(void);

  virtual int inst_jr   (t_mem code);
  virtual int inst_call (t_mem code);
  virtual int inst_ret  (t_mem code);
  virtual int inst_jp   (t_mem code);
  virtual int inst_add  (t_mem code);

  virtual int inst_st_sp_abs(t_mem code);
  virtual int inst_stop0    (t_mem code);
  
  virtual int inst_ldi  (t_mem code);
  virtual int inst_ldd  (t_mem code);
  virtual int inst_ldh  (t_mem code);
  
  virtual int inst_reti     (t_mem code);
  virtual int inst_add_sp_d (t_mem code);
  virtual int inst_ld16     (t_mem code);
  virtual int inst_ldhl_sp  (t_mem code);
  
};

#endif /* GB80_CL */

/* End of z80.src/gb80cl.h */
