/*
 * Simulator of microcontrollers (r5kcl.h)
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

#ifndef R5KCL_HEADER
#define R5KCL_HEADER

#include "r4kcl.h"


class cl_r5k: public cl_r4k
{
 public:
  cl_r5k(class cl_sim *asim);
  virtual const char *id_string(void);

  virtual void tick5p1(int n) { tick(n+1); }
  virtual void tick5p2(int n) { tick(n+2); }
  virtual void tick5p3(int n) { tick(n+3); }
  virtual void tick5p9(int n) { tick(n+9); }
  virtual void tick5p12(int n) { tick(n+12); }
  virtual void tick5m1(int n) { tick(n  ); }
  virtual void tick5m2(int n) { tick(n  ); }
};


#endif

/* End of rxk.src/r5kcl.h */
