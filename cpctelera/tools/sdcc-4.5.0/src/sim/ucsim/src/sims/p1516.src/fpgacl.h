/*
 * Simulator of microcontrollers (fpgacl.h)
 *
 * Copyright (C) 2020 Drotos Daniel
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

#ifndef FPGACL_HEADER
#define FPGACL_HEADER

#include "hwcl.h"


class cl_fpga;

class cl_led: public cl_base
{
public:
  class cl_fpga *fpga;
  int x, y;
  u32_t mask;
  u32_t last;
public:
  cl_led(class cl_fpga *the_fpga, int ax, int ay, u32_t amask);
  virtual void refresh(bool force);
  virtual void draw(void) {}
};


class cl_rgb: public cl_led
{
public:
  cl_rgb(class cl_fpga *the_fpga, int ax, int ay, u32_t amask);
  virtual void refresh(bool force);
};


class cl_seg: public cl_led
{
public:
  int digit;
  u32_t last_what, last_ctrl;
public:
  cl_seg(class cl_fpga *the_fpga, int ax, int ay, int adigit);
  virtual void refresh(bool force);
  virtual void draw(void);
};


class cl_ibit: public cl_base
{
public:
  cl_fpga *fpga;
  int x, y;
  int mask;
  char key;
  u32_t last;
public:
  cl_ibit(class cl_fpga *the_fpga, int ax, int ay, int amask, char akey);
  virtual void refresh(bool force) {}
  virtual void draw(void) {}
  virtual bool handle_input(int c) { return false; }
};


class cl_sw: public cl_ibit
{
public:
  cl_sw(class cl_fpga *the_fpga, int ax, int ay, int amask, char akey);
  virtual void refresh(bool force);
  virtual void draw(void);
  virtual bool handle_input(int c);
};


class cl_btn: public cl_ibit
{
public:
  cl_btn(class cl_fpga *the_fpga, int ax, int ay, int amask, char akey);
  virtual void refresh(bool force);
  virtual void draw(void);
  virtual bool handle_input(int c);
};


class cl_fpga: public cl_hw
{
public:
  cl_cell32 *pa, *pb, *pc, *pd;
  cl_cell32 *pi, *pj;
  cl_cell32 *bc;
  cl_memory_cell *pip, *pjp;
  class cl_led *leds[16];
  class cl_seg *segs[8];
  class cl_sw *sws[16];
  class cl_btn *btns[8];
  int basey;
  chars board;
  int d2c_b;
public:
  cl_fpga(class cl_uc *auc, int aid, chars aid_string);
  virtual int init(void);
  virtual void mk_leds(void) {}
  virtual void mk_segs(void) {}
  virtual void mk_sws(void) {}
  virtual void mk_btns(void) {}
  virtual void make_io(void);
  virtual void new_io(class cl_f *f_in, class cl_f *f_out);
  virtual bool handle_input(int c);
  virtual void refresh_leds(bool force);
  virtual void refresh_segs(bool force);
  virtual void refresh_sws(bool force);
  virtual void refresh_btns(bool force);
  virtual void refresh_display(bool force);
  virtual void draw_display(void);
  virtual void draw_fpga(void) {}
  
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
};


class cl_n4: public cl_fpga
{
public:
  cl_n4(class cl_uc *auc, int aid, chars aid_string);
  virtual void mk_leds(void);
  virtual void mk_segs(void);
  virtual void mk_btns(void);
  virtual void mk_sws(void);
  virtual void draw_fpga(void);
};


class cl_bool: public cl_n4
{
public:
  cl_bool(class cl_uc *auc, int aid, chars aid_string);
  virtual void draw_fpga(void);
  virtual void mk_btns(void);
};


class cl_logsys: public cl_fpga
{
public:
  cl_logsys(class cl_uc *auc, int aid, chars aid_string);
  virtual void mk_leds(void);
  virtual void mk_segs(void);
  virtual void draw_fpga(void);
  virtual void mk_btns(void);
  virtual void mk_sws(void);
};


#endif

/* End of p1516.src/fpgacl.h */
