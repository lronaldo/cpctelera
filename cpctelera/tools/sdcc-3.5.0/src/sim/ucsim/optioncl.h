/*
 * Simulator of microcontrollers (sim.src/optioncl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#ifndef SIM_OPTIONCL_HEADER
#define SIM_OPTIONCL_HEADER

#include "ddconfig.h"

#include <stdio.h>

#include "pobjcl.h"
#include "stypes.h"
  

enum option_type {
  non_opt,
  integer_opt,
  float_opt,
  bool_opt,
  string_opt,
  pointer_opt
};

// forward
class cl_optref;

union option_value {
  long ival;
  double fval;
  bool bval;
  char *sval;
  void *pval;
};

class cl_option: public cl_base
{
protected:
  //enum opt_type type;
  void *option;
  union option_value value;
  class cl_base *creator;
public:
  class cl_list *users; // cl_optref
  //char *name;
  char *help;
  bool hidden;

public:
  cl_option(class cl_base *the_creator, const char *aname, const char *Ihelp);
  virtual class cl_option &operator=(class cl_option &o);
  virtual ~cl_option(void);
  virtual void pre_remove(void);

  virtual class cl_base *get_creator(void) { return(creator); }
  virtual void hide(void) { hidden= DD_TRUE; }
  virtual void show(void) { hidden= DD_FALSE; }

  virtual void print(class cl_console_base *con) {}
  virtual const char *get_type_name(void) { return("non"); }

  virtual union option_value *get_value(void) { return(&value); }
  virtual void get_value(bool *val);
  virtual void get_value(char **val);
  virtual void get_value(void **val);
  virtual void get_value(long *val);
  virtual void get_value(double *val);
  virtual void set_value(bool opt);
  virtual void set_value(char *opt);
  virtual void set_value(void *opt);
  virtual void set_value(long opt);
  virtual void set_value(double opt);

  virtual void new_reference(class cl_optref *ref);
  virtual void del_reference(class cl_optref *ref);
  virtual void inform_users(void);
};


class cl_options: public cl_sorted_list
{
public:
  cl_options(void): cl_sorted_list(2, 2, "options") { Duplicates= DD_TRUE; }
  virtual const void *key_of(void *item);
  virtual int compare(const void *key1, const void *key2);
  virtual void new_option(class cl_option *opt);
  virtual void del_option(class cl_option *opt);
  virtual class cl_option *get_option(const char *the_name);
  virtual class cl_option *get_option(const char *the_name, class cl_base *creator);
  virtual class cl_option *get_option(const char *the_name, char *creator);
  virtual class cl_option *get_option(int idx);
  virtual int nuof_options(char *the_name);
  virtual int nuof_options(char *the_name, char *creator);

  virtual class cl_option *set_value(const char *the_name, cl_base *creator,
                                     bool value);
  virtual class cl_option *set_value(const char *the_name, cl_base *creator,
                                     char *value);
  virtual class cl_option *set_value(const char *the_name, cl_base *creator,
                                     void *value);
  virtual class cl_option *set_value(const char *the_name, cl_base *creator,
                                     long value);
  virtual class cl_option *set_value(const char *the_name, cl_base *creator,
                                     double value);
};


class cl_optref: public cl_base
{
protected:
  class cl_option *option;
  class cl_base *owner;
public:
  cl_optref(class cl_base *the_owner);
  cl_optref(class cl_base *the_owner, class cl_option *new_option);
  virtual ~cl_optref(void);

  virtual class cl_option *create(class cl_base *creator,
                                  enum option_type type,
                                  const char *the_name, const char *help);
  virtual void default_option(const char *the_name);
  virtual class cl_option *use(void);
  virtual class cl_option *use(const char *the_name);
  virtual void option_changed(void) {}
  virtual void option_removing(void);
  virtual class cl_base *get_owner(void) { return(owner); }

  virtual bool get_value(bool);
  virtual char *get_value(const char *);
  virtual void *get_value(void *);
  virtual long get_value(long);
  virtual double get_value(double);
};


class cl_bool_option: public cl_option
{
public:
  cl_bool_option(class cl_base *the_creator, const char *aname, const char *Ihelp);
  virtual void print(class cl_console_base *con);
  virtual const char *get_type_name(void) { return("boolean"); }
  virtual void set_value(char *s);
};


class cl_string_option: public cl_option
{
public:
  cl_string_option(class cl_base *the_creator, const char *aname, const char *Ihelp);
  virtual class cl_option &operator=(class cl_option &o);
  virtual void print(class cl_console_base *con);
  virtual const char *get_type_name(void) { return("string"); }
};


class cl_pointer_option: public cl_option
{
public:
  cl_pointer_option(class cl_base *the_creator, const char *aname, const char *Ihelp);
  virtual class cl_option &operator=(class cl_option &o);
  virtual void print(class cl_console_base *con);
  virtual const char *get_type_name(void) { return("pointer"); }
};


class cl_number_option: public cl_option
{
public:
  cl_number_option(class cl_base *the_creator, const char *aname, const char *Ihelp);
  virtual void print(class cl_console_base *con);
  virtual const char *get_type_name(void) { return("integer"); }
  virtual void set_value(char *s);
};


class cl_float_option: public cl_option
{
public:
  cl_float_option(class cl_base *the_creator, const char *aname, const char *Ihelp);
  virtual void print(class cl_console_base *con);
  virtual const char *get_type_name(void) { return("float"); }
  virtual void set_value(char *s);
};


/*class cl_cons_debug_opt: public cl_option
{
public:
  class cl_app *app;
public:
  cl_cons_debug_opt(class cl_app *the_app, char *Iid, char *Ihelp);

  virtual void print(class cl_console_base *con);

  virtual void get_value(bool *val);

  virtual void set_value(bool);
  virtual void set_value(char *s);
};*/


#endif

/* End of optioncl.h */
