/*
 * Simulator of microcontrollers (errorcl.h)
 *
 * Copyright (C) 2001,01 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/*
  This file is part of microcontroller simulator: ucsim.

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
  02111-1307, USA.
*/
/*@1@*/

#ifndef ERRORCL_HEADER
#define ERRORCL_HEADER

#include <string.h>

// prj
#include "pobjcl.h"
#include "stypes.h"

extern struct id_element error_on_off_names[];

enum error_on_off {
  ERROR_PARENT,
  ERROR_ON,
  ERROR_OFF
};

const int err_stop= (err_unknown|err_error);

class cl_error_class: public cl_base
{
protected:
  enum error_type type;
  //char *name;
  enum error_on_off on;
public:
  cl_error_class(enum error_type typ, const char *aname,
                 enum error_on_off be_on= ERROR_PARENT);
  cl_error_class(enum error_type typ, const char *aname,
                 class cl_error_class *parent,
                 enum error_on_off be_on= ERROR_PARENT);
  
  enum error_on_off get_on(void) { return(on); }
  void set_on(enum error_on_off val);
  bool is_on(void);
  enum error_type get_type(void);
  const char *get_type_name(void);
  //char *get_name(void);
};

class cl_error_registry
{
public:
  cl_error_registry(void);
  class cl_error_class *find(const char *type_name)
  {
    if (NIL == registered_errors)
      return NIL;
    return static_cast<class cl_error_class *>(registered_errors->first_that(compare, type_name));
  }
  static class cl_list *get_list(void)
  {
    return registered_errors;
  }

protected:
  class cl_error_class *register_error(class cl_error_class *error_class)
  {
    if (!registered_errors)
      registered_errors= new cl_list(2, 2, "registered errors");
    registered_errors->add(error_class);
    return error_class;
  }

private:
  static class cl_list *registered_errors;
  static int compare(void *obj1, const void *obj2)
  {
    return (static_cast<class cl_base *>(obj1))->is_named(static_cast<const char *>(obj2));
  }
};

class cl_commander_base; //forward

class cl_error: public cl_base
{
protected:
  class cl_error_class *classification;
public:
  bool inst;    // Occured during instruction execution
  t_addr PC;    // Address of the instruction
public:
  cl_error(void);
  virtual ~cl_error(void);
  virtual int init(void);

public:
  virtual enum error_type get_type(void);
  virtual enum error_on_off get_on(void);
  virtual bool is_on(void);
  virtual class cl_error_class *get_class(void) { return(classification); }

  virtual void print(class cl_commander_base *c);
  virtual const char *get_type_name();
};

#endif


/* End of sim.src/errorcl.h */
