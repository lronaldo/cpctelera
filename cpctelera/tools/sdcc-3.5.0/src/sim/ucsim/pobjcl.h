/*
 * Simulator of microcontrollers (pobjcl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

#ifndef POBJ_HEADER
#define POBJ_HEADER

#include "ddconfig.h"

#include "pobjt.h"

#include "eventcl.h"


/*									    #
  ==========================================================================#
								    cl_base #
  ==========================================================================#
									    #
*/

class cl_list;
class cl_event;

class cl_base
{
private:
  char *name;
  class cl_base *parent;
  class cl_list *children;
public:
  cl_base(void);
  virtual ~cl_base(void);

  virtual int init(void);
  virtual const char *get_name(void) { return(name); }
  virtual const char *get_name(const char *def);
  virtual bool have_name(void) { return(name != 0); }
  virtual bool have_real_name(void) { return(name != 0 && *name != '\0'); }
  const char *set_name(const char *new_name);
  const char *set_name(const char *new_name, const char *def_name);
  bool is_named(const char *the_name);
  bool is_inamed(const char *the_name);

  class cl_base *get_parent(void) { return(parent); }
  int nuof_children(void);

  virtual void add_child(class cl_base *child);
  virtual void remove_child(class cl_base *child);
  virtual void remove_from_chain(void);
  virtual void unlink(void);
  virtual class cl_base *first_child(void);
  virtual class cl_base *next_child(class cl_base *child);

  virtual bool handle_event(class cl_event &event);
  virtual bool pass_event_down(class cl_event &event);
};


/*
 * Event
 */

class cl_event: public cl_base
{
protected:
  bool handled;
public:
  enum event what;
public:
  cl_event(enum event what_event);
  virtual ~cl_event(void);

  bool is_handled(void) { return(handled); }
  virtual void handle(void) { handled= DD_TRUE; }
};


/*									    #
  ==========================================================================#
								    cl_list #
  ==========================================================================#
									    #
*/

class cl_list: public cl_base
{
public:
  t_index	   count;
protected:
  void		   **Items;
  t_index	   Limit;
  t_index	   Delta;

public:
  cl_list(t_index alimit, t_index adelta, const char *aname);
  virtual ~cl_list(void);

  inline  void	   *at(t_index index)
  {
    if (index < 0 || index >= count)
      error(1, index);
    return (Items[index]);
  }
  class cl_base *object_at(t_index index);
  virtual t_index  index_of(void *item);
  virtual bool     index_of(void *item, t_index *idx);
  virtual void     *next(void *item);
  	  int	   get_count(void);
  virtual void     *pop(void);
  virtual void     *top(void);

  //void	   pack(void);
  virtual void	   set_limit(t_index alimit);

	  void	   free_at(t_index index);
          void     free_all(void);
	  void	   disconn_at(t_index index);
	  void	   disconn(void *item);
	  void	   disconn_all(void);

	  void	   add_at(t_index index, void *item);
	  void	   put_at(t_index index, void *item);
  virtual t_index  add(void *item);
  virtual t_index  add(class cl_base *item, class cl_base *parent);
  virtual void     push(void *item);

	  void	   *first_that(match_func test, const void *arg);
	  void	   *last_that(match_func test, void *arg);
	  void	   for_each(iterator_func action, void *arg);

	  void	   error(t_index code, t_index info);
private:
  virtual void	   free_item(void *item);
};


/*									    #
  ==========================================================================#
							     cl_sorted_list #
  ==========================================================================#
									    #
*/

class cl_sorted_list: public cl_list
{
public:
  bool		   Duplicates;
public:
  cl_sorted_list(t_index alimit, t_index adelta, const char *aname);
  virtual ~cl_sorted_list(void);

  virtual bool	   search(const void *key, t_index& index);
  virtual t_index  index_of(void *item);
  virtual t_index  add(void *item);
  virtual const void *key_of(void *item);
private:
  virtual int	   compare(const void *key1, const void *key2)= 0;
};


/*									    #
  ==========================================================================#
							         cl_strings #
  ==========================================================================#
									    #
*/

class cl_strings: public cl_sorted_list
{
public:
  cl_strings(t_index alimit, t_index adelta, const char *aname);
  virtual ~cl_strings(void);

private:
  virtual int	   compare(const void *key1, const void *key2);
  virtual void	   free_item(void *item);
};


/*									    #
  ==========================================================================#
							        cl_ustrings #
  ==========================================================================#
									    #
*/

class cl_ustrings: public cl_strings
{
public:
  cl_ustrings(t_index alimit, t_index adelta, const char *aname);
  virtual ~cl_ustrings(void);

private:
  virtual int	   compare(const void *key1, const void *key2);
  virtual bool	   search(const void *key, t_index &index);
};


#endif

/* End of pobj.h */
