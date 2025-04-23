/*
 * Simulator of microcontrollers (r3kacl.h)
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

#ifndef R3KACL_HEADER
#define R3KACL_HEADER

#include "dpedm3a.h"

#include "r3kcl.h"


class cl_r3ka: public cl_r3k
{
public:
  u8_t rSU;
  u8_t edmr;
  class cl_cell8 cSU;
public:
  cl_r3ka(class cl_sim *asim);
  virtual int init();
  virtual const char *id_string(void);
  virtual void reset(void);

  virtual struct dis_entry *dis_entry(t_addr addr);
  virtual char *disassc(t_addr addr, chars *comment= NULL);

  virtual void print_regs(class cl_console_base *con);

  virtual int LDxSR(int dif);
  virtual int LSxDR(int dif);
  virtual int LSxR(int dif);
  virtual int UMx(bool add);
  
  virtual int PUSH_SU(t_mem code);
  virtual int POP_SU(t_mem code);
  virtual int SETUSR(t_mem code);
  virtual int SURES(t_mem code);
  virtual int RDMODE(t_mem code);
  virtual int SYSCALL(t_mem code);
  virtual int LDDSR(t_mem code) { return LDxSR(-1); }
  virtual int LDISR(t_mem code) { return LDxSR(+1); }
  virtual int LSDDR(t_mem code) { return LSxDR(-1); }
  virtual int LSIDR(t_mem code) { return LSxDR(+1); }
  virtual int LSDR(t_mem code) { return LSxR(-1); }
  virtual int LSIR(t_mem code) { return LSxR(+1); }
  virtual int UMA(t_mem code) { return UMx(true); }
  virtual int UMS(t_mem code) { return UMx(false); }

  virtual int instruction_5b(t_mem code);
};


#endif

/* End of rxk.src/r3kacl.h */
