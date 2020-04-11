/*
 * Simulator of microcontrollers (error.cc)
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

#include <stdlib.h>
#include "i_string.h"

// prj (local)
#include "errorcl.h"
#include "globals.h"
#include "utils.h"

// cmd.src
#include "newcmdcl.h"

struct id_element error_on_off_names[]= {
  { ERROR_PARENT, "unset" },
  { ERROR_ON	, "on" },
  { ERROR_OFF	, "off" },
  { 0		, 0 }
};

static class cl_error_registry error_registry;

class cl_list *cl_error_registry::registered_errors= 0;

/*
 */

cl_error_class::cl_error_class(enum error_type typ, const char *aname,
			       enum error_on_off be_on/* = ERROR_PARENT*/):
  cl_base()
{
  type= typ;
  on= be_on;
  set_name(aname, "not-known");
}

cl_error_class::cl_error_class(enum error_type typ, const char *aname,
			       class cl_error_class *parent,
			       enum error_on_off be_on/* = ERROR_PARENT*/):
  cl_base()
{
  type= typ;
  on= be_on;
  set_name(aname, "not-known");
  if (parent)
    parent->add_child(this);
}

void
cl_error_class::set_on(enum error_on_off val)
{
  if (!get_parent() &&
      val == ERROR_PARENT)
    return;
  on= val;
}

bool
cl_error_class::is_on(void)
{
  if (on == ERROR_PARENT)
    {
      if (!get_parent())
	return(true);
      class cl_error_class *p=
	dynamic_cast<class cl_error_class *>(get_parent());
      return(p->is_on());
    }
  else
    return(on == ERROR_ON);
}

enum error_type
cl_error_class::get_type(void)
{
  return(type);
}

/*char *
cl_error_class::get_name(void)
{
  return(name);
}*/

char *
cl_error_class::get_type_name()
{
  return(get_id_string(error_type_names, type, /*cchars*/((char*)"untyped")));
  /*switch (type)
    {
    case err_unknown: return("unclassified"); break;
    case err_error: return("error"); break;
    case err_warning: return("warning"); break;
    }
    return("untyped");*/
}


/*
 */

cl_error::cl_error(void):
  cl_base()
{
  classification= error_registry.find(/*cchars*/("non-classified"));

}

cl_error::~cl_error(void)
{}

int
cl_error::init(void)
{
  //type= get_type();
  return(0);
}

enum error_type
cl_error::get_type(void)
{
  if (classification)
    return(classification->get_type());
  return(err_unknown);
}

enum error_on_off
cl_error::get_on(void)
{
  if (!classification)
    return(ERROR_ON);
  return(classification->get_on());
}

bool
cl_error::is_on(void)
{
  if (!classification)
    return(true);
  return(classification->is_on());
}

void
cl_error::print(class cl_commander_base *c)
{
  c->dd_printf(cchars("%s\n"), get_type_name());
}

char *
cl_error::get_type_name()
{
  enum error_type type= get_type();
  return(get_id_string(error_type_names, type, /*cchars*/((char*)"untyped")));
}

cl_error_registry::cl_error_registry(void)
{
  if (NULL == error_registry.find(/*cchars*/("non-classified")))
    register_error(new cl_error_class(err_error, /*cchars*/("non-classified"), ERROR_ON));
}

/* End of sim.src/error.cc */
