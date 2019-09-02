/*
 * Simulator of microcontrollers (uc390cl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 * uc390cl.h - implemented by Karl Bongers, karl@turbobit.com
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

#ifndef UC390CL_HEADER
#define UC390CL_HEADER

#include "ddconfig.h"

#include "uc52cl.h"

class cl_uc390: public cl_uc52
{
public:
  // memories and cells for faster access
  class cl_address_space *ixram;
  class cl_memory_chip *ixram_chip;
  
  cl_uc390(struct cpu_entry *Itype, class cl_sim *asim);
  virtual void mk_hw_elements (void);
  virtual void make_memories(void);
  virtual void make_address_spaces();
  virtual void make_chips(void);
  virtual void decode_rom(void);
  virtual void decode_xram(void);
  virtual void decode_dptr(void);
  
  virtual void clear_sfr (void);

  // making objects
  //virtual t_addr get_mem_size (enum mem_class type);

  // manipulating memories
  virtual t_mem read_mem (char *id/*enum mem_class type*/, t_addr addr);
  virtual t_mem get_mem (char *id/*enum mem_class type*/, t_addr addr);
  virtual void  write_mem (char *id/*enum mem_class type*/, t_addr addr, t_mem val);
  virtual void  set_mem (char *id/*enum mem_class type*/, t_addr addr, t_mem val);

  /* mods for dual-dptr */
  virtual int instruction_a3/*inst_inc_dptr*/(t_mem/*uchar*/ code);		// a3
  virtual int instruction_73/*inst_jmp_Sa_dptr*/(t_mem/*uchar*/ code);		// 73
  virtual int instruction_90/*inst_mov_dptr_Sdata*/(t_mem/*uchar*/ code);	// 90
  virtual int instruction_93/*inst_movc_a_Sa_dptr*/(t_mem/*uchar*/ code);	// 93
  virtual int instruction_e0/*inst_movx_a_Sdptr*/(t_mem/*uchar*/ code);		// e0
  virtual int instruction_f0/*inst_movx_Sdptr_a*/(t_mem/*uchar*/ code);		// f0

  /* mods for flat24 */
  virtual int instruction_01/*inst_ajmp_addr*/(t_mem/*uchar*/ code);		// [02468abce]1
  virtual int instruction_02/*inst_ljmp*/(t_mem/*uchar*/ code);			// 02
  virtual int instruction_11/*inst_acall_addr*/(t_mem/*uchar*/ code);		// [13579bdf]1
  virtual int inst_lcall(t_mem/*uchar*/ code, uint addr, bool intr);		// 12
  virtual int instruction_22/*inst_ret*/(t_mem/*uchar*/ code);			// 22
  virtual int instruction_32/*inst_reti*/(t_mem/*uchar*/ code);

  /* mods for 10 bit stack */
  virtual int instruction_c0/*inst_push*/ (t_mem/*uchar*/ code);		// c0
  virtual int instruction_d0/*inst_pop*/ (t_mem/*uchar*/ code);			// d0

  /* mods for disassembly of flat24 */
  virtual struct dis_entry *dis_tbl(void);
  virtual char * disass(t_addr addr, const char *sep);
  virtual void   print_regs(class cl_console_base *con);

protected:
  //int flat24_flag; /* true if processor == ds390f */
  virtual void push_byte (t_mem uc);
  virtual t_mem pop_byte (void);
};

/* End of s51.src/uc390cl.h */

#endif
