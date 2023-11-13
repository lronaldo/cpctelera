/*
 * Simulator of microcontrollers (stm8.src/flashcl.h)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
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

#ifndef FLASHCL_HEADER
#define FLASHCL_HEADER

#include "hwcl.h"
#include "memcl.h"


/* Special memory/address space to handle flash ops */

class cl_flash_cell: public cl_cell8
{
 public:
 cl_flash_cell(uchar awidth): cl_cell8(awidth) {}
  virtual t_mem write(t_mem val);
};

class cl_flash_as: public cl_address_space
{
 public:
  cl_flash_as(const char *id, t_addr astart, t_addr asize);
  virtual int init(void);
};

enum stm8_flash_cfg {
  stm8_flash_on= 0,
  stm8_flash_nuof_cfg= 1
};

enum stm8_mass {
  PMASS1= 0x56,
  PMASS2= 0xae,
  DMASS1= 0xae,
  DMASS2= 0x56
};

enum stm8_flash_state {
  fs_wait_mode= 0x00,
  fs_wait_data= 0x01,
  fs_pre_erase= 0x02,
  fs_program= 0x04,
  fs_busy= fs_pre_erase|fs_program
};

enum stm8_flash_mode {
  fm_unknown= 0,
  fm_byte= 1,
  fm_word= 2,
  fm_fast_word= 3,
  fm_block= 4,
  fm_fast_block= 5,
  fm_erase= 6
};

class cl_flash: public cl_hw
{
 protected:
  t_addr base;
  cl_memory_cell *cr1r, *cr2r, *ncr2r, *pukr, *dukr, *iapsr;
  bool puk1st, duk1st;
  bool p_unlocked, d_unlocked;
  bool p_failed, d_failed;
  u8_t wbuf[256]; /* buffer of block */
  bool wbuf_started; /* any writes happened to block */
  int wbuf_size; /* block size */
  int wbuf_writes; /* nr of writes to block */
  t_addr wbuf_start; /* start address of block */
  bool rww;
  enum stm8_flash_state state;
  enum stm8_flash_mode mode;
  double tprog; /* programing time in usec */
  double start_time;
 public:
  cl_flash(class cl_uc *auc, t_addr abase, const char *aname);
  virtual int init(void);
  virtual void registration(void) {}
  virtual int tick(int cycles);
  virtual void reset(void);

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual const char *cfg_help(t_addr addr);

  virtual void flash_write(t_addr a, t_mem val);
  virtual void set_flash_mode(t_mem cr2val);
  virtual void start_wbuf(t_addr addr);
  virtual void start_program(enum stm8_flash_state start_state);
  virtual void finish_program(bool ok);

  virtual const char *state_name(enum stm8_flash_state s);
  virtual void print_info(class cl_console_base *con);
};

class cl_saf_flash: public cl_flash
{
 public:
  cl_saf_flash(class cl_uc *auc, t_addr abase);
  virtual void registration(void);
};

class cl_l_flash: public cl_flash
{
 public:
  cl_l_flash(class cl_uc *auc, t_addr abase);
  virtual void registration(void);
};

#endif

/* End of stm8.src/flashcl.h */
