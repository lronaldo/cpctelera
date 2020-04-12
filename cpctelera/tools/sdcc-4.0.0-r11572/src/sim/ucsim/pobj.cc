/*
 * Simulator of microcontrollers (pobj.cc)
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

#include <string.h>
#include <strings.h>
#include <unistd.h>

//#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>

#include "pstr.h"
/*#include "pobjt.h"*/
#include "pobjcl.h"


/*									    *
  ==========================================================================*
								    cl_base *
  ==========================================================================*
									    *
*/

/*
 * Initializing the object
 */

cl_base::cl_base(void)
{
  name= 0;
  parent= 0;
  children= 0;
}


/*
 * Destructing the object: calling hte virtual Done method
 */

cl_base::~cl_base(void)
{
  //if (name) free((void*)name);
  if (children)
    {
      int i;
      for (i= 0; i < children->count; i++)
	{
	}
      children->disconn_all();
      delete children;
    }
  parent= 0;
}

int cl_base::init(void) {return(0);}

const char *
cl_base::get_name(const char *def)
{
  if (!name)
    return(def);
  return(name);
}

const char *
cl_base::set_name(const char *new_name)
{
  /*
  if (name)
    free((void*)name);
  if (!new_name)
    name= 0;
  else if (*new_name)
    name= strdup(new_name);
  else
    name= strdup("");
  */
  name= new_name;
  return(name);
}

const char *
cl_base::set_name(const char *new_name, const char *def_name)
{
  char *def;

  if (!def_name ||
      *def_name == '\0')
    def= /*strdup*/cchars("");
  else
    def= /*strdup*/cchars(def_name);
  // if (name) free((void*)name);
  if (!new_name)
    name= def;
  else if (*new_name)
    name= /*strdup*/(new_name);
  else
    name= def;
  return(name);
}

bool
cl_base::is_named(const char *the_name)
{
  /*
  if (!name ||
      !*name ||
      !the_name ||
      !*the_name)
      return(false);*/
  return(/*strcmp(name, the_name) == 0*/name==the_name);
}

bool
cl_base::is_inamed(const char *the_name)
{
  if (/*!name ||
      !*name ||
      !the_name ||
      !*the_name*/name.empty())
    return(false);
  return(strcasecmp(name, the_name) == 0);
}


int
cl_base::nuof_children(void)
{
  if (!children)
    return(0);
  return(children->count);
}

void
cl_base::add_child(class cl_base *child)
{
  if (!children)
    {
      /*
      char *s;
      s= (char*)malloc(name.len()+100);
      sprintf(s, "children of %s", get_name("?"));
      */
      chars cs("", "children of %s", get_name("?"));
      children= new cl_list(1, 1, cs);
      //free(s);
    }
  if (child)
    {
      children->add(child);
      child->parent= this;
    }
}

void
cl_base::remove_child(class cl_base *child)
{
  if (child &&
      children)
    {
      child->unlink();
      children->disconn(child);
    }
}

void
cl_base::remove_from_chain(void)
{
  if (parent)
    parent->remove_child(this);
}

void
cl_base::unlink(void)
{
  parent= 0;
}

class cl_base *
cl_base::first_child(void)
{
  if (!children ||
      children->count == 0)
    return(0);
  return(dynamic_cast<class cl_base *>(children->object_at(0)));
}

class cl_base *
cl_base::next_child(class cl_base *child)
{
  if (!children ||
      !child)
    return(0);
  return((class cl_base *)(children->next(child)));
}


bool
cl_base::handle_event(class cl_event &event)
{
  return(pass_event_down(event));
}

bool
cl_base::pass_event_down(class cl_event &event)
{
  int i;
  if (!children)
    return(false);
  for (i= 0; i < children->count; i++)
    {
      class cl_base *child=
	dynamic_cast<class cl_base *>(children->object_at(i));
      if (child)
	{
	  child->handle_event(event);
	  if (event.is_handled())
	    return(true);
	}
    }
  return(false);
}


/*
 * Event
 */

cl_event::cl_event(enum event what_event):
  cl_base()
{
  handled= false;
  what= what_event;
}

cl_event::~cl_event(void)
{
}


/*									    *
  ==========================================================================*
								    cl_list *
  ==========================================================================*
									    *
*/

/*
 * Initializing a collection
 */

cl_list::cl_list(t_index alimit, t_index adelta, char *aname):
  cl_base()
{
  count= 0;
  Items= 0;
  Limit= 0;
  Delta= adelta;
  set_limit(alimit);
  set_name(aname, "unnamed list");
}

cl_list::cl_list(t_index alimit, t_index adelta, const char *aname):
  cl_base()
{
  count= 0;
  Items= 0;
  Limit= 0;
  Delta= adelta;
  set_limit(alimit);
  set_name(aname, "unnamed list");
}


/*
 * Disposing object's variables
 */

cl_list::~cl_list(void)
{
  //delete Items;
  free(Items);
}


/*
 * Get indexed item from the collection
 */

/*void *
cl_list::at(t_index index)
{
  if (index < 0 ||
      index >= count)
    error(1, index);
  return(Items[index]);
}*/

class cl_base *
cl_list::object_at(t_index index)
{
  if (index < 0 ||
      index >= count)
    error(1, index);
  return((class cl_base *)(Items[index]));
}

/*void *
cl_list::operator[](t_index index)
{
  if (index < 0 ||
      index >= count)
    error(1, 0);
  return(Items[index]);
}*/


/*
 * Deleting the indexed item from the collection
 */

void
cl_list::disconn_at(t_index index)
{
  if (index < 0 ||
      index >= count)
    error(1, 0);
  count--;
  memmove(&Items[index], &Items[index+1], (count-index)*sizeof(void *));
}


/*
 * Deleting an item from the collection but not disposing it
 */

void
cl_list::disconn(void *item)
{
  t_index i;

  if (index_of(item, &i))
    disconn_at(i);
}


/*
 * Deleting all the items from the collection but not disposing them
 */

void
cl_list::disconn_all(void)
{
  count= 0;
}


/*
 * Deleting the indexed item from the collection and disposing it
 */

void
cl_list::free_at(t_index index)
{
  void *Item= at(index);

  disconn_at(index);
  free_item(Item);
}

void
cl_list::free_all(void)
{
  t_index i;

  if (count)
    {
      for (i= count-1; i; i--)
	free_at(i);
      free_at(0);
    }
}


/*
 * Inserting a new item to the exact position
 */

void
cl_list::add_at(t_index index, void *item)
{
  if (index < 0 )
    error(1, 0);
  if (count == Limit)
    set_limit(count + Delta);

  /*{
    char s[1000];
    s[0]='\0';
    sprintf(s, "%s add_at(%d,%p) PC=0x%x (count=%d)", get_name("?"), index, item,
	    application?
	    ((application->sim)?
	     ((application->sim->uc)?(application->sim->uc->PC):
	      -3):
	     -1):
	    -2, count);
    strcat(s,"\n");
    }*/
  memmove(&Items[index+1], &Items[index], (count-index)*sizeof(void *));
  count++;
  
  Items[index]= item;
}


/*
 * Put a new item to the collection. This function replaces an existing
 * item with a new one but it does not delete or dispose the old item!
 */

void
cl_list::put_at(t_index index, void *item)
{
  if (index >= count)
    error(1, 0);
  Items[index]= item;
}


/*
 * Action taken when an error occure
 */

void
cl_list::error(t_index code, t_index info)
{
  fprintf(stderr,
	  "Collection index error. Code= %d, Info= %d.\n",
	  code, info);
  exit(code);
}


/*
 * Iterator method. This function calls 'Test' using every items as Test's
 * argument until Test returns TRUE.
 */

void *
cl_list::first_that(match_func test, void *arg)
{
  for (t_index i= 0; i < count; i++)
    {
      if (test(Items[i], arg)) return(Items[i]);
    }
  return(0);
}


/*
 * Iterator method. This function calls 'Action' using every items as
 * Action's argument.
 */

void
cl_list::for_each(iterator_func action, void *arg)
{
  for(t_index i= 0; i < count; i++)
    action(Items[i], arg);
}


/*
 * Disposing an item.
 */

void
cl_list::free_item(void *item)
{
  delete (class cl_base*)item;
}


/*
 * Get the number of collected items.
 */

int
cl_list::get_count(void)
{
  return(count);
}

void *
cl_list::pop(void)
{
  void *i;

  if (!count)
    return(0);
  i= Items[0];
  disconn_at(0);
  return(i);
}

void *
cl_list::top(void)
{
  if (!count)
    return(0);
  return(Items[0]);
}


/*
 * Returning the index of an item.
 */

t_index
cl_list::index_of(void *item)
{
  for (t_index i= 0; i < count; i++)
    if (item == Items[i])
      return(i);
  error(1, 0);
  return(0);    /* Needed by Sun! */
}

bool
cl_list::index_of(void *item, t_index *idx)
{
  for (t_index i= 0; i < count; i++)
    if (item == Items[i])
      {
	if (idx)
	  *idx= i;
	return(true);
      }
  return(false);
}

void *
cl_list::next(void *item)
{
  for (t_index i= 0; i < count; i++)
    if (item == Items[i])
      {
	if (count >= 2 &&
	    i < count-1)
	  return(Items[i+1]);
      }
  return(0);
}


/*
 * Inserting a new item to the collection.
 */

t_index
cl_list::add(void *item)
{
  t_index loc= count;

  add_at(count, item);
  return(loc);
}

t_index
cl_list::add(class cl_base *item, class cl_base *parent)
{
  if (parent && item)
    parent->add_child(item);
  return(add(item));
}

void
cl_list::push(void *item)
{
  if (count)
    add_at(0, item);
  else
    add(item);
}


/*
 * Iterator method. This function calls 'Test' using every items
 * (in reversed order) as Test's argument until Test returns TRUE.
 */

void *
cl_list::last_that(match_func test, void *arg)
{
  for(t_index i= count; i > 0; i--)
    if (test(Items[i-1], arg))
      return(Items[i-1]);
  return(0);
}


/*
 * ???
 */

/*void
cl_list::pack(void)
{
  void **CurDst= Items;
  void **CurSrc= Items;
  void **Last  = Items + count;

  while (CurSrc < Last)
    {
      if (*CurSrc != 0)
	*CurDst++= *CurSrc;
      *CurSrc++;
    }
}*/


/*
 * Setting up the maximum number of items. This function may expand
 * the size of the collection.
 */

void
cl_list::set_limit(t_index alimit)
{
  void **AItems;

  if (alimit < count)
    alimit= count;
  if (alimit > (int)max_list_size)
    alimit= max_list_size;
  if (alimit != Limit)
    {
      if (alimit == 0)
	AItems= 0;
      else
	{
	  //AItems = new void *[alimit];
	  int i= alimit*(sizeof(void *));
	  AItems= (void **)malloc(i);
	  if (count)
	    memcpy(AItems, Items, count*sizeof(void *));
	}
      //delete Items;
      if (Items)
	free(Items);
      Items= AItems;
      Limit= alimit;
    }
}


/*									    *
  ==========================================================================*
							     cl_sorted_list *
  ==========================================================================*
									    *
*/

/*
 * Initilizing the sorted collection
 */

cl_sorted_list::cl_sorted_list(t_index alimit, t_index adelta, char *aname):
  cl_list(alimit, adelta, aname)
{
  Duplicates= false;
}

cl_sorted_list::cl_sorted_list(t_index alimit, t_index adelta, const char *aname):
  cl_list(alimit, adelta, aname)
{
  Duplicates= false;
}


cl_sorted_list::~cl_sorted_list(void) {}


/*
 * Get the address of the key field in an item.
 */

void *
cl_sorted_list::key_of(void *item)
{
  return(item);
}


/*
 * Get index of an item.
 */

t_index
cl_sorted_list::index_of(void *item)
{
  t_index	i;

  if (search(key_of(item), i) == 0)
    return(ccNotFound);
  else
    {
      if (Duplicates)
	while (i < count &&
	       item != Items[i])
	  i++;
      if (i < count)
	return(i);
      else
	return(ccNotFound);
    }
}


/*
 * Inserting a new item to the collection
 */

t_index
cl_sorted_list::add(void *item)
{
  t_index i;

  if (search(key_of(item), i) == 0 ||
      Duplicates)   				// order dependency!
    add_at(i, item);				// must do Search
						// before calling
						// AtInsert
  return(i);
}


/*
 * Searching an item using binary search.
 */

bool
cl_sorted_list::search(void *key, t_index &index)
{
  t_index l  = 0;
  t_index h  = count - 1;
  bool    res= false;

  while (l <= h)
    {
      t_index i= (l + h) >> 1;
      t_index c= compare(key_of(Items[i]), key);
      if (c < 0) l= i + 1;
      else
	{
	  h= i - 1;
	  if (c == 0)
	    {
	      res= true;
	      if (!Duplicates)
		l= i;
	    }
	}
    }
  index= l;
  return(res);
}


/*									    *
  ==========================================================================*
							         cl_strings *
  ==========================================================================*
									    *
*/

/*
 * Initilizing the string collection
 */

cl_strings::cl_strings(t_index alimit, t_index adelta, char *aname):
  cl_sorted_list(alimit, adelta, aname)
{
  Duplicates= true;
}

cl_strings::cl_strings(t_index alimit, t_index adelta, const char *aname):
  cl_sorted_list(alimit, adelta, aname)
{
  Duplicates= true;
}


cl_strings::~cl_strings(void) {}


/*
 * Comapare two string from the collection
 */

int
cl_strings::compare(void *key1, void *key2)
{
  return(strcmp((char *)key1, (char *)key2));
}


/*
 * Deallocate string item of the collection
 */

void
cl_strings::free_item(void* item)
{
  delete (char*)item;
}


/*									    *
  ==========================================================================*
							        cl_ustrings *
  ==========================================================================*
									    *
*/

/*
 * Initilizing the unsorted string collection
 */

cl_ustrings::cl_ustrings(t_index alimit, t_index adelta, char *aname):
  cl_strings(alimit, adelta, aname)
{}

cl_ustrings::cl_ustrings(t_index alimit, t_index adelta, const char *aname):
  cl_strings(alimit, adelta, aname)
{}

cl_ustrings::~cl_ustrings(void) {}


/*
 * Comapare two string from the collection
 */

int
cl_ustrings::compare(void *key1, void *key2)
{
  return(-1);
}


/*
 * Searching an item using linear search.
 */

bool
cl_ustrings::search(void *key, t_index& index)
{
  t_index i    = 0;
  bool    found= false;
  void    *Actual;

  if ((count) && key)
    {
      while (!found && (i < count))
	{
	  Actual= key_of(at(i));
	  found = (Actual != 0) &&
		  (compare(key, Actual) == 0);
	  i++;
	}
    }
  if (found)
    index= i-1;
  else
    index= count;

  return(found);
}

/* End of pobj.cc */
