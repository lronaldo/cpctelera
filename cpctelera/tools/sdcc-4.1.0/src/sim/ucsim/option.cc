/*
 * Simulator of microcontrollers (option.cc)
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

#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "i_string.h"

  // local, prj
#include "stypes.h"
#include "optioncl.h"
#include "globals.h"
#include "utils.h"

  // sim.src
#include "simcl.h"


/*struct id_element option_type_names[]= {
  { non_opt	, "non" },
  { integer_opt	, "integer" },
  { float_opt	, "float" },
  { bool_opt	, "boolean" },
  { string_opt	, "string" },
  { pointer_opt	, "pointer" },
  { 0, 0 }
  };*/

/*
 * Base class for option's objects
 *____________________________________________________________________________
 *
 */

cl_option::cl_option(class cl_base *the_creator, const char *aname, const char *Ihelp):
  cl_base()
{
  creator= the_creator;
  set_name(aname);
  help= strdup(Ihelp);
  char *s= (char*)malloc(strlen(aname)+100);
  sprintf(s, "users of option \"%s\"", aname);
  users= new cl_list(2, 2, s);
  free(s);
  memset(&value, 0, sizeof(value));
  show();
}

class cl_option &
cl_option::operator=(class cl_option &o)
{
  //memcpy(&value, &(o.value), sizeof(union option_value));
  //fprintf(stderr,"opt%p\"%s\"=%p\"%s\"\nold=%p\n",this,object_name(this),&o,object_name(&o),value.sval);
  value= o.value;
  //fprintf(stderr,"new=%p\n",value.sval);
  inform_users();
  return(*this);
}

cl_option::~cl_option(void)
{
  users->disconn_all();
  delete users;
  free(help);
}

void
cl_option::pre_remove(void)
{
  int i;

  for (i= 0; i < users->count; i++)
    {
      class cl_optref *user= (class cl_optref *)(users->at(i));
      user->option_removing();
    }
}

void
cl_option::new_reference(class cl_optref *ref)
{
  users->add(ref);
}

void
cl_option::del_reference(class cl_optref *ref)
{
  users->disconn(ref);
}

void
cl_option::inform_users(void)
{
  int i;

  for (i= 0; i < users->count; i++)
    {
      class cl_optref *user= (class cl_optref *)(users->at(i));
      //fprintf(stderr,"%p\"%s\" informs user %p\"%s\"\n",this,object_name(this),user,object_name(user));
      user->option_changed();
    }
}


void
cl_option::get_value(bool *val)
{
  if (val)
    *val= value.bval;
}

void
cl_option::set_value(bool opt)
{
  value.bval= opt;
  inform_users();
}


void
cl_option::get_value(char **val)
{
  if (val)
    {
      *val= value.sval;
    }
}

void
cl_option::set_value(const char *opt)
{
  //fprintf(stderr,"set_string_value (%s) to %p\"%s\"\n",opt,this,object_name(this));
  //fprintf(stderr,"old value=%p\"%s\"\n",value.sval,value.sval);
  if (value.sval)
    free(value.sval);
  if (opt &&
      *opt)
    value.sval= strdup(opt);
  else
    value.sval= strdup("");
  //fprintf(stderr,"new value=%p\"%s\"\n",value.sval,value.sval);
  inform_users();
}

void
cl_option::get_value(void **val)
{
  if (val)
    *val= value.pval;
}

void
cl_option::set_value(void *opt)
{
  value.pval= opt;
  inform_users();
}


void
cl_option::get_value(long *val)
{
  if (val)
    *val= value.ival;
}

void
cl_option::set_value(long opt)
{
  value.ival= opt;
  inform_users();
}


void
cl_option::get_value(double *val)
{
  if (val)
    *val= value.fval;
}

void
cl_option::set_value(double opt)
{
  value.fval= opt;
  inform_users();
}


/*
 * List of options
 */

const void *
cl_options::key_of(const void *item) const
{
  return ((const class cl_base *)item)->get_name();
}

int
cl_options::compare(const void *key1, const void *key2)
{
  const char *k1, *k2;

  k1= (const char *)key1;
  k2= (const char *)key2;
  return strcmp(k1, k2);
}

void
cl_options::new_option(class cl_option *opt)
{
  add(opt);
}

void
cl_options::del_option(class cl_option *opt)
{
  opt->pre_remove();
  disconn(opt);
  delete opt;
}

class cl_option *
cl_options::get_option(const char *the_name)
{
  t_index idx;

  if (search(the_name, idx))
    return((class cl_option *)(at(idx)));
  return(0);
}

class cl_option *
cl_options::get_option(const char *the_name, class cl_base *creator)
{
  t_index idx;
  class cl_option *o;

  if (!search(the_name, idx))
    return(0);
  if (idx > 0)
    {
      idx--;
      o= (class cl_option *)(at(idx));
      while (compare(the_name, key_of(o)) == 0 &&
	     idx > 0)
	{
	  idx--;
	  o= (class cl_option *)(at(idx));
	}
      if (compare(the_name, key_of(o)) != 0)
	idx++;
    }
  o= (class cl_option *)(at(idx));
  while (compare(the_name, key_of(o)) == 0 &&
	 o->get_creator() != creator &&
	 idx < count)
    {
      idx++;
      o= (class cl_option *)(at(idx));
      if (compare(the_name, key_of(o)) == 0 &&
	  o->get_creator() == creator)
	return(o);
    }
  if (compare(the_name, key_of(o)) == 0 &&
      o->get_creator() == creator)
    return(o);
  return(0);
}

class cl_option *
cl_options::get_option(const char *the_name, const char *creator)
{
  t_index idx;
  class cl_option *o;

  if (!search(the_name, idx))
    return(0);
  if (idx > 0)
    {
      idx--;
      o= (class cl_option *)(at(idx));
      while (compare(the_name, key_of(o)) == 0 &&
	     idx > 0)
	{
	  idx--;
	  o= (class cl_option *)(at(idx));
	}
      if (compare(the_name, key_of(o)) != 0)
	idx++;
    }
  o= (class cl_option *)(at(idx));
  while (compare(the_name, key_of(o)) == 0 &&
	 strcmp(object_name(o->get_creator()), creator) != 0 &&
	 idx < count)
    {
      idx++;
      o= (class cl_option *)(at(idx));
      if (compare(the_name, key_of(o)) == 0 &&
	  strcmp(object_name(o->get_creator()), creator) == 0)
	return(o);
    }
  if (compare(the_name, key_of(o)) == 0 &&
      strcmp(object_name(o->get_creator()), creator) == 0)
    return(o);
  return(0);
}

class cl_option *
cl_options::get_option(int idx)
{
  if (idx >= count)
    return(0);
  return((class cl_option *)(at(idx)));
}

int
cl_options::nuof_options(char *the_name)
{
  int i, n= 0;

  for (i= 0; i < count; i++)
    {
      class cl_option *o= (class cl_option *)(at(i));
      if (strcmp(the_name, o->get_name()) == 0)
	n++;
    }
  return(n);
}

int
cl_options::nuof_options(char *the_name, const char *creator)
{
  int i, n= 0;

  for (i= 0; i < count; i++)
    {
      class cl_option *o= (class cl_option *)(at(i));
      if (strcmp(the_name, o->get_name()) == 0 &&
	  strcmp(creator, object_name(o->get_creator())) == 0)
	n++;
    }
  return(n);
}

class cl_option *
cl_options::set_value(const char *the_name, cl_base *creator, bool value)
{
  class cl_option *o= get_option(the_name, creator);

  if (o)
    o->set_value(value);
  return(o);
}

class cl_option *
cl_options::set_value(const char *the_name, cl_base *creator, const char *value)
{
  class cl_option *o= get_option(the_name, creator);

  if (o)
    o->set_value(value);
  return(o);
}

class cl_option *
cl_options::set_value(const char *the_name, cl_base *creator, void *value)
{
  class cl_option *o= get_option(the_name, creator);

  if (o)
    o->set_value(value);
  return(o);
}

class cl_option *
cl_options::set_value(const char *the_name, cl_base *creator, long value)
{
  class cl_option *o= get_option(the_name, creator);

  if (o)
    o->set_value(value);
  return(o);
}

class cl_option *
cl_options::set_value(const char *the_name, cl_base *creator, double value)
{
  class cl_option *o= get_option(the_name, creator);

  if (o)
    o->set_value(value);
  return(o);
}


/*
 * Reference to an option
 */

cl_optref::cl_optref(class cl_base *the_owner)
{
  option =0;
  owner= the_owner;
}

cl_optref::cl_optref(class cl_base *the_owner, class cl_option *new_option)
{
  owner= the_owner;
  option= new_option;
  application->options->new_option(option);
  if (option)
    {
      option->new_reference(this);
      set_name(option->get_name());
    }
}

cl_optref::~cl_optref(void)
{
  if (option)
    {
      option->del_reference(this);
      if (option->get_creator() == owner)
	application->options->del_option(option);
    }
}

class cl_option *
cl_optref::create(class cl_base *creator,
		  enum option_type type,
		  const char *the_name, const char *help)
{
  if (option)
    option->del_reference(this);
  switch (type)
    {
    case non_opt:
      option= 0;
      break;
    case integer_opt:
      option= new cl_number_option(creator, the_name, help);
      break;
    case float_opt:
      option= new cl_float_option(creator, the_name, help);
      break;
    case bool_opt:
      option= new cl_bool_option(creator, the_name, help);
      break;
    case string_opt:
      option= new cl_string_option(creator, the_name, help);
      break;
    case pointer_opt:
      option= new cl_pointer_option(creator, the_name, help);
      break;
    default:
      option= 0;
      break;
    }
  if (option)
    {
      application->options->new_option(option);
      option->new_reference(this);
      set_name(option->get_name());
    }
  
  return(option);
}

void
cl_optref::default_option(const char *the_name)
{
  class cl_option *o= application->options->get_option(the_name, application);

  if (o &&
      option)
    {
      //memcpy(option->get_value(), o->get_value(), sizeof(union option_value));
      *option= *o;
      option->inform_users();
    }
  /*else
    fprintf(stderr,"can not set opt from default, option=%p, o=%p\n",option,o);*/
}

class cl_option *
cl_optref::use(void)
{
  if (option)
    {
      option->del_reference(this);
      option->new_reference(this);
    }
  return(option);
}

class cl_option *
cl_optref::use(const char *the_name)
{
  if (option)
    option->del_reference(this);
  option= application->options->get_option(the_name);
  if (option)
    {
      option->new_reference(this);
      set_name(option->get_name());
    }
  return(option);
}

void
cl_optref::option_removing(void)
{
  option= 0;
}

bool
cl_optref::get_value(bool)
{
  if (!option)
    {
      fprintf(stderr, "Warning: \"%s\" is dereferencing a non-existent "
	      "bool option: %s\n", object_name(owner), get_name());
      return(false);
    }
  else
    {
      bool v;
      option->get_value(&v);
      return(v);
    }
}

char *
cl_optref::get_value(const char *)
{
  if (!option)
    {
      const char *o= object_name(owner);
      const char *n= get_name();
      fprintf(stderr, "Warning: \"%s\" is dereferencing a non-existent "
	      "string option: %s\n", o, n?n:"?"); 
      return(0);
    }
  else
    {
      char *s= 0;
      option->get_value(&s);
      return(s);
    }
}

void *
cl_optref::get_value(void *)
{
  if (!option)
    {
      fprintf(stderr, "Warning: \"%s\" is dereferencing a non-existent "
	      "pointer option: %s\n", object_name(owner), get_name());
      return(0);
    }
  else
    {
      void *p= 0;
      option->get_value(&p);
      return(p);
    }
}

long
cl_optref::get_value(long)
{
  if (!option)
    {
      fprintf(stderr, "Warning: \"%s\" is dereferencing a non-existent "
	      "number option: %s\n", object_name(owner), get_name());
      return(0);
    }
  else
    {
      long l= 0;
      option->get_value(&l);
      return(l);
    }
}

double
cl_optref::get_value(double)
{
  if (!option)
    {
      fprintf(stderr, "Warning: \"%s\" is dereferencing a non-existent "
	      "float option: %s\n", object_name(owner), get_name());
      return(0);
    }
  else
    {
      double d= 0;
      option->get_value(&d);
      return(d);
    }
}


/*
 * BOOL type of option
 *____________________________________________________________________________
 *
 */

cl_bool_option::cl_bool_option(class cl_base *the_creator,
			       const char *aname, const char *Ihelp):
  cl_option(the_creator, aname, Ihelp)
{}

void
cl_bool_option::print(class cl_console_base *con)
{
  if (/**(bool *)option*/value.bval)
    con->dd_printf("TRUE");
  else
    con->dd_printf("FALSE");
}

void
cl_bool_option::set_value(const char *s)
{
  char c;

  if (s)
    {
      c= toupper(*s);
      if (c == '1' ||
	  c == 'T' ||
	  c == 'Y')
	/**(bool *)option=*/ value.bval= true;
      else
	/**(bool *)option=*/ value.bval= false;
    }
  inform_users();
}


/*
 * STRING type of option
 *____________________________________________________________________________
 *
 */

cl_string_option::cl_string_option(class cl_base *the_creator,
				   const char *aname, const char *Ihelp):
  cl_option(the_creator, aname, Ihelp)
{}

class cl_option &
cl_string_option::operator=(class cl_option &o)
{
  //fprintf(stderr,"string=otheropt%p\"%s\"\nold=%p\"%s\"\n",&o,object_name(&o),value.sval,value.sval);
  set_value((o.get_value())->sval);
  //fprintf(stderr,"new=%p\"%s\"\n",value.sval,value.sval);
  return(*this);
}

void
cl_string_option::print(class cl_console_base *con)
{
  if (/**(bool *)option*/value.sval)
    con->dd_printf("\"%s\"", value.sval);
  else
    con->dd_printf("(null)");
}


/*
 * PONITER type of option
 *____________________________________________________________________________
 *
 */

cl_pointer_option::cl_pointer_option(class cl_base *the_creator,
				     const char *aname, const char *Ihelp):
  cl_option(the_creator, aname, Ihelp)
{}

class cl_option &
cl_pointer_option::operator=(class cl_option &o)
{
  set_value((o.get_value())->pval);
  return(*this);
}

void
cl_pointer_option::print(class cl_console_base *con)
{
  if (value.pval)
    con->dd_printf("\"%p\"", value.pval);
  else
    con->dd_printf("(null)");
}


/*
 * Debug on console
 */
/*
cl_cons_debug_opt::cl_cons_debug_opt(class cl_app *the_app,
				     char *Iid,
				     const char *Ihelp):
  cl_option(0, Iid, Ihelp)
{
  app= the_app;
}

void
cl_cons_debug_opt::print(class cl_console_base *con)
{
  if (con->flags & CONS_DEBUG)
    con->dd_printf("TRUE");
  else
    con->dd_printf("FALSE");
}

void
cl_cons_debug_opt::get_value(bool *val)
{
  if (val)
    *val= app->get_commander()->actual_console?
      (app->get_commander()->actual_console->flags & CONS_DEBUG):0;
}

void
cl_cons_debug_opt::set_value(bool opt)
{
  if (app->get_commander()->actual_console)
    {
      if (opt)
	app->get_commander()->actual_console->flags|= CONS_DEBUG;
      else
	app->get_commander()->actual_console->flags&= ~CONS_DEBUG;
    }
  inform_users();
}

void
cl_cons_debug_opt::set_value(const char *s)
{
  char c;

  if (s &&
      app->get_commander()->actual_console)
    {
      c= toupper(*s);
      if (c == '1' ||
	  c == 'T' ||
	  c == 'Y')
	set_value(1);
      else
	set_value(0);
    }
}
*/

/*
 * NUMBER type of option
 *____________________________________________________________________________
 *
 */

cl_number_option::cl_number_option(class cl_base *the_creator,
				   const char *aname, const char *Ihelp):
  cl_option(the_creator, aname, Ihelp)
{}

void
cl_number_option::print(class cl_console_base *con)
{
  con->dd_printf("%ld", value.ival);
}

void
cl_number_option::set_value(const char *s)
{
  if (s)
    value.ival= strtol(s, 0, 0);
  inform_users();
}


/*
 * FLOAT type of option
 *____________________________________________________________________________
 *
 */

cl_float_option::cl_float_option(class cl_base *the_creator,
				 const char *aname, const char *Ihelp):
  cl_option(the_creator, aname, Ihelp)
{}

void
cl_float_option::print(class cl_console_base *con)
{
  con->dd_printf("%.3f", value.fval);
}

void
cl_float_option::set_value(const char *s)
{
  if (s)
    value.fval= strtod(s, 0);
  inform_users();
}


/* End of option.cc */
