/*
 * Simulator of microcontrollers (r4kwrap.h)
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

#ifndef R4KWRAP_HEADER
#define R4KWRAP_HEADER

#include "r4kcl.h"

extern int instruction_wrapper_4knone(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4k45(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k48(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k4c(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k4d(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4k50(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k51(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k54(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k55(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4k60(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k61(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k62(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k63(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k65(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k6d(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4k7f(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4k81(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k82(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k83(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k84(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k85(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k86(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k87(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k89(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k8a(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k8b(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k8c(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k8d(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k8e(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k8f(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4k91(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k92(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k93(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k94(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k95(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k96(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k97(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k98(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k99(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k9a(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k9b(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k9c(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k9d(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k9e(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4k9f(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4ka0(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka1(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka2(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka3(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka4(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka5(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka6(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka7(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka8(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4ka9(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kaa(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kab(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kac(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kad(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kae(class cl_uc *uc, t_mem code);

extern int instruction_wrapper_4kb0(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb1(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb2(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb3(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb4(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb5(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb6(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb8(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kb9(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kba(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kbb(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kbc(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kbd(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kbe(class cl_uc *uc, t_mem code);
extern int instruction_wrapper_4kbf(class cl_uc *uc, t_mem code);

#endif

/* End of rxk.src/r4kwrap.h */
