/*
 * Simulator of microcontrollers (mos65c02scl.h)
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

#ifndef MOS65C02SCL_HEADER
#define MOS65C02SCL_HEADER

#include "mos65c02cl.h"

#include "decc02s.h"

class cl_mos65c02s: public cl_mos65c02
{
public:
  cl_mos65c02s(class cl_sim *asim);
  virtual int init(void);

  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual int inst_length(t_addr addr);

  virtual int rmb(t_mem code, class cl_cell8 &op);
  virtual int smb(t_mem code, class cl_cell8 &op);
  virtual int bbr(t_mem code, class cl_cell8 &op);
  virtual int bbs(t_mem code, class cl_cell8 &op);

  virtual int WAI(t_mem code);
  virtual int STP(t_mem code);
  
  // New insts in column 7
  virtual int RMB0(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int RMB1(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int RMB2(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int RMB3(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int RMB4(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int RMB5(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int RMB6(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int RMB7(t_mem code) { return rmb(code, rmwzpg()); }
  virtual int SMB0(t_mem code) { return smb(code, rmwzpg()); }
  virtual int SMB1(t_mem code) { return smb(code, rmwzpg()); }
  virtual int SMB2(t_mem code) { return smb(code, rmwzpg()); }
  virtual int SMB3(t_mem code) { return smb(code, rmwzpg()); }
  virtual int SMB4(t_mem code) { return smb(code, rmwzpg()); }
  virtual int SMB5(t_mem code) { return smb(code, rmwzpg()); }
  virtual int SMB6(t_mem code) { return smb(code, rmwzpg()); }
  virtual int SMB7(t_mem code) { return smb(code, rmwzpg()); }
  // New insts in column F
  virtual int BBR0(t_mem code) { return bbr(code, zpg()); }
  virtual int BBR1(t_mem code) { return bbr(code, zpg()); }
  virtual int BBR2(t_mem code) { return bbr(code, zpg()); }
  virtual int BBR3(t_mem code) { return bbr(code, zpg()); }
  virtual int BBR4(t_mem code) { return bbr(code, zpg()); }
  virtual int BBR5(t_mem code) { return bbr(code, zpg()); }
  virtual int BBR6(t_mem code) { return bbr(code, zpg()); }
  virtual int BBR7(t_mem code) { return bbr(code, zpg()); }
  virtual int BBS0(t_mem code) { return bbs(code, zpg()); }
  virtual int BBS1(t_mem code) { return bbs(code, zpg()); }
  virtual int BBS2(t_mem code) { return bbs(code, zpg()); }
  virtual int BBS3(t_mem code) { return bbs(code, zpg()); }
  virtual int BBS4(t_mem code) { return bbs(code, zpg()); }
  virtual int BBS5(t_mem code) { return bbs(code, zpg()); }
  virtual int BBS6(t_mem code) { return bbs(code, zpg()); }
  virtual int BBS7(t_mem code) { return bbs(code, zpg()); }
};


#endif

/* End of mos6502.src/mos65c02s.cc */
