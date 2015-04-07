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
  cl_uc390(int Itype, int Itech, class cl_sim *asim);
  virtual void mk_hw_elements (void);
  virtual void make_memories(void);

  virtual void clear_sfr (void);

  // making objects
  //virtual t_addr get_mem_size (enum mem_class type);

  // manipulating memories
  virtual t_mem read_mem (const char *id/*enum mem_class type*/, t_addr addr);
  virtual t_mem get_mem (const char *id/*enum mem_class type*/, t_addr addr);
  virtual void  write_mem (const char *id/*enum mem_class type*/, t_addr addr, t_mem val);
  virtual void  set_mem (const char *id/*enum mem_class type*/, t_addr addr, t_mem val);

  /* mods for dual-dptr */
  virtual int inst_inc_dptr(uchar code);
  virtual int inst_jmp_Sa_dptr(uchar code);
  virtual int inst_mov_dptr_Sdata(uchar code);
  virtual int inst_movc_a_Sa_dptr(uchar code);
  virtual int inst_movx_a_Sdptr(uchar code);
  virtual int inst_movx_Sdptr_a(uchar code);

  /* mods for flat24 */
  virtual int inst_ajmp_addr(uchar code);
  virtual int inst_ljmp(uchar code);
  virtual int inst_acall_addr(uchar code);
  virtual int inst_lcall(uchar code, uint addr, bool intr);/* 12 */
  virtual int inst_ret(uchar code);
  virtual int inst_reti(uchar code);

  /* mods for 10 bit stack */
  virtual int inst_push (uchar code);
  virtual int inst_pop (uchar code);

  /* mods for disassembly of flat24 */
  virtual struct dis_entry *dis_tbl(void);
  virtual const char * disass(t_addr addr, const char *sep);
  virtual void   print_regs(class cl_console_base *con);

protected:
  int flat24_flag; /* true if processor == ds390f */
  virtual void push_byte (t_mem uc);
  virtual t_mem pop_byte (void);
};

/* End of s51.src/uc390cl.h */

#endif
