/*@1@*/

#ifndef SIM_VARCL_HEADER
#define SIM_VARCL_HEADER


#include "pobjcl.h"

#include "newcmdcl.h"

#include "memcl.h"


class cl_var: public cl_base
{
 public:
  class cl_address_space *as; // reference
  t_addr addr;
  int bitnr;
  chars desc;
 protected:
  class cl_memory_cell *cell;
 public:
  cl_var(char *iname, class cl_address_space *ias, t_addr iaddr, chars adesc, int ibitnr= -1);
  virtual int init(void);
  virtual class cl_memory_cell *get_cell(void) { return cell; }
  
  virtual void print_info(cl_console_base *con);
};


class cl_var_list: public cl_sorted_list
{
 public:
 cl_var_list(): cl_sorted_list(10, 10, "symlist") {}
 public:
  virtual void *key_of(void *item);
  virtual int compare(void *key1, void *key2);
};


#endif

/* End of sim.src/varcl.h */
