/*@1@*/

#include "varcl.h"


cl_var::cl_var(char *iname, class cl_address_space *ias, t_addr iaddr, chars adesc, int ibitnr):
  cl_base()
{
  as= ias;
  addr= iaddr;
  bitnr= ibitnr;
  desc= adesc;
  
  set_name(iname);
  
  cell= NULL;
}

int
cl_var::init(void)
{
  if (!as ||
      !as->is_address_space() ||
      !as->valid_address(addr))
    return 0;
  cell= as->get_cell(addr);
  if (cell && (bitnr < 0))
    cell->set_flag(CELL_VAR, true);
  return 0;
}


void
cl_var::print_info(cl_console_base *con)
{
  con->dd_printf("%s ", get_name("?"));
  if (cell)
    {
      t_mem v= cell->read();
      con->dd_printf("%s", as->get_name("?"));
      con->dd_printf("[");
      con->dd_printf(as->addr_format, addr);
      con->dd_printf("] ");
      if (bitnr >= 0)
	{
	  con->dd_printf(".%d", bitnr);
	  con->dd_printf("= %d", (v & (1<<bitnr))?1:0);
	}
      else
	{
	  con->dd_printf("= ");
	  con->dd_printf(as->data_format, v);
	}
    }
  con->dd_printf("\n");
  if (!desc.empty())
    con->dd_printf("  %s\n", (char*)desc);
}


void *
cl_var_list::key_of(void *item)
{
  class cl_var *v= (class cl_var *)item;
  return (void*)v->get_name();
}

int
cl_var_list::compare(void *key1, void *key2)
{
  char *k1, *k2;

  k1= (char*)key1;
  k2= (char*)key2;
  if (k1 && k2)
    return strcmp(k1, k2);
  return 0;
}


/* End of sim.src/var.cc */
