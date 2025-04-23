/*
 * Simulator of microcontrollers (mos65c02cl.h)
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

#ifndef MOS65C02CL_HEADER
#define MOS65C02CL_HEADER

#include "mos6502cl.h"

#include "decc02.h"

class cl_mos65c02: public cl_mos6502
{
 public:
  cl_mos65c02(class cl_sim *asim);
  virtual int init(void);
  virtual void reset(void);

  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual int inst_length(t_addr addr);

  virtual int accept_it(class it_level *il);
  
  virtual int nopft(int nuof_fetches, int nuof_ticks);
  virtual int tsb(class cl_cell8 &op);
  virtual int trb(class cl_cell8 &op);
  virtual int stz(class cl_cell8 &op);
  
  // New insts in column 0
  virtual int BRA(t_mem code) { return branch(true); tick(1); }
  // New insts in column 2
  virtual int ORAzi(t_mem code) { return ora(zind()); }
  virtual int ANDzi(t_mem code) { return And(zind()); }
  virtual int EORzi(t_mem code) { return eor(zind()); }
  virtual int ADCzi(t_mem code) { return adc(zind()); }
  virtual int STAzi(t_mem code) { return sta(dstzind()); }
  virtual int LDAzi(t_mem code) { return lda(zind()); }
  virtual int CMPzi(t_mem code) { return cmp(cA, zind()); }
  virtual int SBCzi(t_mem code) { return sbc(zind()); }
  // NOPs of column 2
  virtual int instruction_02(t_mem code) { return nopft(1,1); }
  virtual int instruction_22(t_mem code) { return nopft(1,1); }
  virtual int instruction_42(t_mem code) { return nopft(1,1); }
  virtual int instruction_62(t_mem code) { return nopft(1,1); }
  virtual int instruction_82(t_mem code) { return nopft(1,1); }
  virtual int instruction_c2(t_mem code) { return nopft(1,1); }
  virtual int instruction_e2(t_mem code) { return nopft(1,1); }
  // NOP 1,1 insts
  virtual int NOP11(t_mem code) { return nopft(0,0); }
  // New insts in column 4
  virtual int TSBz(t_mem code) { return tsb(rmwzpg()); }
  virtual int TRBz(t_mem code) { return trb(rmwzpg()); }
  virtual int BITzx(t_mem code) { return bit(zpgX()); }
  virtual int STZz(t_mem code) { return stz(dstzpg()); }
  virtual int STZzx(t_mem code) { return stz(dstzpgX()); }
  // NOPs of coulmn 4
  virtual int instruction_44(t_mem code) { return nopft(1,2); }
  virtual int instruction_54(t_mem code) { return nopft(1,3); }
  virtual int instruction_d4(t_mem code) { return nopft(1,3); }
  virtual int instruction_f4(t_mem code) { return nopft(1,3); }
  // New insts in column 8
  virtual int BIT8(t_mem code);
  // New insts in column 9
  virtual int STZa(t_mem code) { return stz(dstabs()); }
  virtual int STZax(t_mem code) { return stz(dstabsX()); }
  // New insts in column A
  virtual int INA(t_mem code);
  virtual int DEA(t_mem code);
  virtual int PHY(t_mem code);
  virtual int PLY(t_mem code);
  virtual int PHX(t_mem code);
  virtual int PLX(t_mem code);
  // New insts in column C
  virtual int TSBa(t_mem code) { return tsb(rmwabs()); }
  virtual int TRBa(t_mem code) { return trb(rmwabs()); }
  virtual int BITax(t_mem code) { return bit(absX()); }
  virtual int JMP7c(t_mem code);
  virtual int NOP5c(t_mem code) { return nopft(2,7); }
  virtual int NOPdc(t_mem code) { return nopft(2,3); }
  virtual int NOPfc(t_mem code) { return nopft(2,3); }
  // NOPs in columns 7 and F
  virtual int instruction_07(t_mem code) { return nopft(0,0); }
  virtual int instruction_17(t_mem code) { return nopft(0,0); }
  virtual int instruction_27(t_mem code) { return nopft(0,0); }
  virtual int instruction_37(t_mem code) { return nopft(0,0); }
  virtual int instruction_47(t_mem code) { return nopft(0,0); }
  virtual int instruction_57(t_mem code) { return nopft(0,0); }
  virtual int instruction_67(t_mem code) { return nopft(0,0); }
  virtual int instruction_77(t_mem code) { return nopft(0,0); }
  virtual int instruction_87(t_mem code) { return nopft(0,0); }
  virtual int instruction_97(t_mem code) { return nopft(0,0); }
  virtual int instruction_a7(t_mem code) { return nopft(0,0); }
  virtual int instruction_b7(t_mem code) { return nopft(0,0); }
  virtual int instruction_c7(t_mem code) { return nopft(0,0); }
  virtual int instruction_d7(t_mem code) { return nopft(0,0); }
  virtual int instruction_e7(t_mem code) { return nopft(0,0); }
  virtual int instruction_f7(t_mem code) { return nopft(0,0); }
  
  virtual int instruction_0f(t_mem code) { return nopft(0,0); }
  virtual int instruction_1f(t_mem code) { return nopft(0,0); }
  virtual int instruction_2f(t_mem code) { return nopft(0,0); }
  virtual int instruction_3f(t_mem code) { return nopft(0,0); }
  virtual int instruction_4f(t_mem code) { return nopft(0,0); }
  virtual int instruction_5f(t_mem code) { return nopft(0,0); }
  virtual int instruction_6f(t_mem code) { return nopft(0,0); }
  virtual int instruction_7f(t_mem code) { return nopft(0,0); }
  virtual int instruction_8f(t_mem code) { return nopft(0,0); }
  virtual int instruction_9f(t_mem code) { return nopft(0,0); }
  virtual int instruction_af(t_mem code) { return nopft(0,0); }
  virtual int instruction_bf(t_mem code) { return nopft(0,0); }
  virtual int instruction_cf(t_mem code) { return nopft(0,0); }
  virtual int instruction_df(t_mem code) { return nopft(0,0); }
  virtual int instruction_ef(t_mem code) { return nopft(0,0); }
  virtual int instruction_ff(t_mem code) { return nopft(0,0); }
};


#endif

/* End of mos6502.src/mos65c02.cc */
