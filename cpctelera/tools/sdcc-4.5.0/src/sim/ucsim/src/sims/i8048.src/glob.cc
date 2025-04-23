/*
 * Simulator of microcontrollers (glob.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include <stdio.h>

#include "glob.h"

struct dis_entry dis_tab20[]=
  {
#define GEN_DIS
#include "decode.h"
#undef GEN_DIS    
    { 0, 0, 0, 0, 0, 0 }
  };

i8_t tick_tab20[256];
  
struct cpu_entry cpus_8048[]=
  {
    {"I8021"	, CPU_I8021, 0		, "I8021", ""},
    {"8021"	, CPU_I8021, 0		, "I8021", ""},
    {"21"	, CPU_I8021, 0		, "I8021", ""},
    {"1"	, CPU_I8021, 0		, "I8021", ""},
    {"I8022"	, CPU_I8022, 0		, "I8022", ""},
    {"8022"	, CPU_I8022, 0		, "I8022", ""},
    {"22"	, CPU_I8022, 0		, "I8022", ""},
    {"2"	, CPU_I8022, 0		, "I8022", ""},

    {"I8035"	, CPU_I8035, 0		, "I8035", ""},
    {"8035"	, CPU_I8035, 0		, "I8035", ""},
    {"35"	, CPU_I8035, 0		, "I8035", ""},
    {"I8039"	, CPU_I8039, 0		, "I8039", ""},
    {"8039"	, CPU_I8039, 0		, "I8039", ""},
    {"39"	, CPU_I8039, 0		, "I8039", ""},
    {"I8040"	, CPU_I8040, 0		, "I8040", ""},
    {"8040"	, CPU_I8040, 0		, "I8040", ""},
    {"40"	, CPU_I8040, 0		, "I8040", ""},

    {"I8048"	, CPU_I8048, 0		, "I8048", ""},
    {"8048"	, CPU_I8048, 0		, "I8048", ""},
    {"48"	, CPU_I8048, 0		, "I8048", ""},
    {"8"	, CPU_I8048, 0		, "I8048", ""},
    {"I8049"	, CPU_I8049, 0		, "I8049", ""},
    {"8049"	, CPU_I8049, 0		, "I8049", ""},
    {"49"	, CPU_I8049, 0		, "I8049", ""},
    {"9"	, CPU_I8049, 0		, "I8049", ""},
    {"I8050"	, CPU_I8050, 0		, "I8050", ""},
    {"8050"	, CPU_I8050, 0		, "I8050", ""},
    {"50"	, CPU_I8050, 0		, "I8050", ""},
    //{"5"	, CPU_I8050, 0		, "I8050", ""},

    {"I8041"	, CPU_I8041, 0		, "I8041", ""},
    {"8041"	, CPU_I8041, 0		, "I8041", ""},
    {"41"	, CPU_I8041, 0		, "I8041", ""},
    {"I8041A"	, CPU_I8041A, 0		, "I8041A", ""},
    {"8041A"	, CPU_I8041A, 0		, "I8041A", ""},
    {"41A"	, CPU_I8041A, 0		, "I8041A", ""},

    {NULL, CPU_NONE, 0, "", ""}
  };

/* End of i8048.src/glob.cc */
