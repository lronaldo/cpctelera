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


#ifndef INTERRUPTCL_HEADER
#define INTERRUPTCL_HEADER


#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"


class cl_interrupt: public cl_hw
{
public:
  bool interrupt_request;
  
  bool preserved_flag_c;
  bool preserved_flag_z;
public:
  cl_interrupt(class cl_uc *auc);
  virtual int init(void);
  virtual bool set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual void set_help(class cl_console_base *con);
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of pblaze.src/interruptcl.h */
