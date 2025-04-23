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


#ifndef PORTIDCL_HEADER
#define PORTIDCL_HEADER


#include "ddconfig.h"
#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"


class cl_port_id: public cl_hw
{
public:
  u8_t value;

  cl_port_id(class cl_uc *auc);
  /*virtual int init(void);*/
  virtual bool set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual void set_help(class cl_console_base *con);
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of pblaze.src/portidcl.h */
