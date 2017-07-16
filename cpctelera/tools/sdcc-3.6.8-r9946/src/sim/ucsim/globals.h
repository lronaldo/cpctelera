/*
 * Simulator of microcontrollers (globals.h)
 *
 * Copyright (C) 1997,16 Drotos Daniel, Talker Bt.
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

#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER

#include "ddconfig.h"

// prj
#include "stypes.h"
#include "appcl.h"


extern class cl_app *application;

extern char delimiters[];

extern struct id_element mem_ids[];
extern struct id_element mem_classes[];
extern struct id_element cpu_states[];
extern struct id_element error_type_names[];
//extern char *case_string(enum letter_case lcase, const char *str);

extern char *warranty;
extern char *copying;

extern struct cpu_entry *cpus;
extern struct cpu_entry cpus_51[];
extern struct cpu_entry cpus_z80[];
extern struct cpu_entry cpus_hc08[];
extern struct cpu_entry cpus_stm8[];


#endif

/* End of globals.h */
