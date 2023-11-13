/*
 * Simulator of microcontrollers (stm8.src/timercl.h)
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

#ifndef STM8_TIMERCL_HEADER
#define STM8_TIMERCL_HEADER

#include "hwcl.h"


enum stm8_tim_cfg {
  stm8_tim_on= 0,
  stm8_tim_nuof_cfg= 1
};


class cl_tim: public cl_hw
{
 protected:
  struct
  {
    int
    // register indexes
    cr1, // control 1
      cr2, // control 2 (used in Master/Slave timers only, all except SAF 235)
      smcr, // slave mode control
      etr, // external trigger
      der, //
      ier, // interrupt enable
      sr1, // status 1
      sr2, // status 2
      egr, // event generation
      ccmr1, // capture/compare mode 1
      ccmr2, // capture/compare mode 2
      ccmr3, // capture/compare mode 3
      ccmr4, // capture/compare mode 4
      ccer1, // capture/compare enable 1
      ccer2, // capture/compare enable 2
      cntrh, // counter high
      cntrl, // counter low
      pscrh, // prescaler high (used only n TIM1)
      pscrl, // prescaler low
      arrh, // auto-reload high (used in 16 bit counters)
      arrl, // auto-reload low
      rcr, // repetition counter
      ccr1h, // capture/compare 1 high
      ccr1l, // capture/compare 1 low
      ccr2h, // capture/compare 2 high
      ccr2l, // capture/compare 2 low
      ccr3h, // capture/compare 3 high
      ccr3l, // capture/compare 3 low
      ccr4h, // capture/compare 4 high
      ccr4l, // capture/compare 4 low
      bkr, // break
      dtr, // deadtime
      oisr;  // output idle state
  }
  idx;
 protected:
  t_addr base;
  cl_memory_cell *regs[32+6];
  bool clk_enabled;
  
  int cnt; // copy of counter value

  // Features
  int bits; // size of counter: 8 or 16
  int mask; // binary mask according to counter size
  int pbits; // nuof bits used in prescaler value
  bool bidir;
  
  // Internal "regs"
  u16_t prescaler_cnt; // actual downcounter
  u16_t prescaler_preload; // start value of prescaler downcount
  u8_t prescaler_ms_buffer; // written MS buffered until LS write
  u8_t arr_ms_buffer; // written MS buffered until LS write
  u8_t timer_ls_buffer; // LS buffered at MS read
  
 public:
  cl_tim(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual int cfg_size(void) { return stm8_tim_nuof_cfg; }
  virtual const char *cfg_help(t_addr addr);
 
  virtual int tick(int cycles);
  virtual void reset(void);
  virtual void happen(class cl_hw *where, enum hw_event he,
                      void *params);

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void count(void);
  virtual u16_t get_counter();
  virtual u16_t set_counter(u16_t val);
  virtual void update_event(void);
  virtual bool get_dir(); // true: UP, false: down
  virtual u16_t get_arr();
  virtual u16_t calc_prescaler();
  
  virtual void print_info(class cl_console_base *con);
};


// Advanced
class cl_tim1: public cl_tim
{
 public:
  cl_tim1(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};

class cl_tim1_saf: public cl_tim1
{
 public:
  cl_tim1_saf(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};

class cl_tim1_all: public cl_tim1
{
 public:
  cl_tim1_all(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};


// General purpose 2, 3, and 5
class cl_tim235: public cl_tim
{
 public:
  cl_tim235(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};

class cl_tim2_saf_a: public cl_tim235
{
 public:
  cl_tim2_saf_a(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim2_saf_b: public cl_tim235
{
 public:
  cl_tim2_saf_b(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim2_all: public cl_tim235
{
 public:
  cl_tim2_all(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};

class cl_tim2_l101: public cl_tim235
{
 public:
  cl_tim2_l101(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};


class cl_tim3_saf: public cl_tim235
{
 public:
  cl_tim3_saf(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim3_all: public cl_tim235
{
 public:
  cl_tim3_all(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};

class cl_tim3_l101: public cl_tim235
{
 public:
  cl_tim3_l101(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};


class cl_tim5_saf: public cl_tim235
{
 public:
  cl_tim5_saf(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim5_all: public cl_tim235
{
 public:
  cl_tim5_all(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
};


// Basic 4 and 6
class cl_tim46: public cl_tim
{
 public:
  cl_tim46(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim4_saf_a: public cl_tim46
{
 public:
  cl_tim4_saf_a(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim4_saf_b: public cl_tim46
{
 public:
  cl_tim4_saf_b(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim4_all: public cl_tim46
{
 public:
  cl_tim4_all(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim4_l101: public cl_tim46
{
 public:
  cl_tim4_l101(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};

class cl_tim6_saf: public cl_tim46
{
 public:
  cl_tim6_saf(class cl_uc *auc, int aid, t_addr abase);
  virtual int init(void);
  virtual bool get_dir() { return true; }
};


#endif

/* End of stm8.src/timercl.h */
