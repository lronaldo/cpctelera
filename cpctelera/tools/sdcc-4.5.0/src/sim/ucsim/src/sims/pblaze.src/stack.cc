/*
 * Copyright (C) 2012-2013 Jiří Šimek
 * Copyright (C) 2013 Zbyněk Křivka <krivka@fit.vutbr.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
/*
#include "ddconfig.h"
#include "glob.h"*/
#include "pblazecl.h"
//#include "regspblaze.h"


/* returns true if stack overflows */
bool
cl_pblaze::stack_push(u32_t value) {
  cl_memory_cell *sp = sfr->get_cell(SP);
  int sp_value = sp->get();

  stack->set(sp_value, value);

  sp_value++;
  sp->set(sp_value % 31);

  if (sp_value > 30)
    return true;
  else
    return false;
}

/* returns true if stack underflows */
bool
cl_pblaze::stack_pop(u32_t *value) {
  cl_memory_cell *sp = sfr->get_cell(SP);
  int sp_value = sp->get();

  if (sp_value == 0)
    sp_value = 30;
  else
    sp_value--;

  sp->set(sp_value);

  *value = stack->get(sp_value);

  if (sp_value == 30)
    return true;
  else
    return false;
}

/* End of pblaze.src/stack.cc */
