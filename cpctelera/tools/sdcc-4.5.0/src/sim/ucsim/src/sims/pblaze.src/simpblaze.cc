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

#include "simpblazecl.h"
#include "pblazecl.h"
#include "glob.h"

/**
 * Constructor
 */
cl_simpblaze::cl_simpblaze(class cl_app *the_app):
  cl_sim(the_app)
{}

/**
 * Creates new microcontroller instance
 */
class cl_uc *
cl_simpblaze::mk_controller(void)
{
  int i = 0;
  const char *type= NULL;
  class cl_optref type_option(this);

  type_option.init();
  type_option.use("cpu_type");

  if ((type = type_option.get_value(type)) == NULL)
    type = "KCPSM3"; // default cpu type

  while ((cpus_pblaze[i].type_str != NULL) && (strcmp(type, cpus_pblaze[i].type_str) != 0))
    i++;

  if (cpus_pblaze[i].type_str == NULL)
  {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");

      return(NULL);
  }

  return(new cl_pblaze(&cpus_pblaze[i], this));
}

/* End of pblaze.src/simpblaze.cc */
