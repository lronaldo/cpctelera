/*
 * Simulator of microcontrollers (lr35902cl.h)
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

#ifndef LR35902_CL
#define LR35902_CL

#include "z80cl.h"

class cl_lr35902;

const t_addr  lr35902_rom_start = 0x0000;
const t_addr  lr35902_rom_size  = 0x6000;

const t_addr  lr35902_ram_start = 0xA000;
const t_addr  lr35902_ram_size  = 0x5F80;

class lr35902_memory
{
 protected:
  cl_uc  &uc_r;
  
 public:
  cl_memory *rom;
  cl_memory *ram;
  
  lr35902_memory( cl_uc &uc_p );
  
  virtual void init( void );
  
  
  virtual void store1( u16_t addr, t_mem val );
  virtual void store2( u16_t addr, u16_t val );
  
  virtual u8_t  get1( u16_t addr );
  virtual u16_t  get2( u16_t addr );
  
  // fetch not included b/c it only uses the rom
};


class cl_lr35902: public cl_z80
{
public:
  lr35902_memory    mem;
  
public:
  cl_lr35902(struct cpu_entry *Itype, class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  
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
                        int *immed_offset);
  

  // memory access altered to use the 'mem' object
  virtual void store1( u16_t addr, t_mem val );
  virtual void store2( u16_t addr, u16_t val );
  
  virtual u8_t  get1( u16_t addr );
  virtual u16_t  get2( u16_t addr );
  
  
  // see #include "instcl.h" for Z80 versions
  /* instruction function that are add / modified from the Z80 versions */
  virtual int inst_cb(void);
  
  virtual int inst_st_sp_abs(t_mem code);
  virtual int inst_stop0    (t_mem code);
  
  virtual int inst_ldi   (t_mem code);
  virtual int inst_ldd   (t_mem code);
  virtual int inst_ldh   (t_mem code);
  
  virtual int inst_reti    (t_mem code);
  virtual int inst_add_sp_d(t_mem code);
  virtual int inst_ld16    (t_mem code);
  virtual int inst_ldhl_sp (t_mem code);
  
};

#endif /* LR35902_CL */
