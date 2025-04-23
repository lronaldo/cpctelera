#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include "memcl.h"
#include "hwcl.h"

#include "newcmdcl.h"

static int go;

static void
alarmed(int sig)
{
  go= 0;
  signal(sig, alarmed);
}

class cl_hw_test: public cl_hw
{
public:
  cl_hw_test(void): cl_hw(0, HW_PORT, 0, "0") {}
  virtual t_mem r(class cl_cell *cell, t_addr addr);
  virtual void write(class cl_mem *mem, t_addr addr, t_mem *val);
};

t_mem
cl_hw_test::r(class cl_cell *cell, t_addr addr)
{
  return(cell->get());
}

void
cl_hw_test::write(class cl_mem *mem, t_addr addr, t_mem *val)
{
}

double
do_rw_test(class cl_mem *mem, int time)
{
  double counter;
  t_addr a;
  t_mem d;

  go= 1;
  counter= 0;
  alarm(time);
  while (go)
    for (a= 0; go && a < mem->size; a++)
      {
	t_mem d2;
	for (d2= 0; go && d2 <= 255; d2++)
	  {
	    d2= mem->write(a, d2);
	    d= mem->read(a);
	    if (d != d2)
	      printf("%d written to mem and %d read back!\n", (int)d2, (int)d);
	    counter+= 1;
	  }
      }
  return(counter);
}

int
main(void)
{
  int i;
  class cl_mem *mem;
  class cl_m *m2;
  class cl_console *con;

  signal(SIGALRM, alarmed);
  con= new cl_console(stdin, stdout, 0);

  mem= new cl_mem(MEM_SFR, "egy", 0x10000, 8, 0);
  mem->init();
  printf("%g operations on classic memory within 5 sec\n",
	 do_rw_test(mem, 5));
  //mem->dump(con);

  m2= new cl_m(MEM_TYPES, "test", 0x10000, 8, 0);
  m2->init();
  printf("%g operations on new memory within 5 sec\n",
	 do_rw_test(m2, 5));

  class cl_hw_test *hw= new cl_hw_test();
  for (i= 0; i < 0x10000; i++)
    {
      class cl_cell *c= m2->get_cell(i);
      int dummy;
      if (c)
	c->add_hw(hw, &dummy);
    }
  printf("%g operations on new memory within 5 sec with hw read\n",
	 do_rw_test(m2, 5));
  //m2->dump(con);

  return(0);
}
