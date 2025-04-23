/*
 * Simulator of microcontrollers (@@F@@)
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

#ifndef RMEMCL_HEADER
#define RMEMCL_HEADER

#include "rtypes.h"
#include "memcl.h"


class cl_ras: public cl_address_space
{
public:
  class cl_memory_chip *chip;
protected:
  RP(xpc,lxpc,hxpc,xpc);
protected:
  u8_t segsize;
  u16_t dataseg, stackseg;
public:
  cl_ras(chars id, class cl_memory_chip *achip);
  virtual int init(void);
public:
  virtual t_addr log2phy(t_addr log);
  virtual t_addr px2phy(u32_t px);
  virtual t_mem read(t_addr addr);
  virtual t_mem phread(t_addr phaddr) { return phget(phaddr); }
  virtual t_mem pxread(t_addr pxaddr);
  virtual t_mem get(t_addr addr);
  virtual t_mem phget(t_addr phaddr);
  virtual t_mem write(t_addr addr, t_mem val);
  virtual t_mem phwrite(t_addr phaddr, t_mem val) { set(phaddr, val); return val; }
  virtual t_mem pxwrite(t_addr pxaddr, t_mem val);
  virtual void set(t_addr addr, t_mem val);
  virtual void phset(t_addr phaddr, t_mem val);
  virtual void download(t_addr phaddr, t_mem val);

  virtual void re_decode(void);
  virtual u8_t *aof_xpc(void) { return &(xpc.r.xpc); }
  virtual u16_t *aof_lxpc(void) { return &(xpc.lxpc); }
  virtual u16_t get_xpc() { return (xpc.lxpc); }
  virtual void set_xpc(u8_t val);
  virtual void set_lxpc(u16_t val);
  virtual void set_segsize(u8_t val);
  virtual void set_dataseg(u16_t val);
  virtual void set_stackseg(u16_t val);
  virtual u16_t get_dataseg() { return dataseg; }
  virtual u16_t get_stackseg() { return stackseg; }
  virtual u8_t get_segsize() { return segsize; }
};

#endif

/* End of rxk.src/rmemcl.h */
