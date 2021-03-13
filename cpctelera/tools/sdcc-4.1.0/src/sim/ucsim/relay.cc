//#include <stdio.h>
#include <unistd.h>

#include "fiocl.h"

class cl_f *single_in, *single_out;
class cl_f *srv;

struct multi_size_r {
  class cl_f *i, *o;
};

struct multi_size_r multies[2];
int nuof_multies= 0;

void
proc_single_input()
{
  if (!single_in->eof())
    {
      char c;
      int i;
      c= single_in->get_c();
      for (i= 0; i < nuof_multies; i++)
	{
	  multies[i].o->write(&c, 1);
	}
    }
}

void
check_multies()
{
  int i;
  for (i= 0; i < nuof_multies; i++)
    {
      if (multies[i].i->input_avail())
	{
	  if (!multies[i].i->eof())
	    {
	      char c= multies[i].i->get_c();
	      single_out->write(&c, 1);
	    }
	}
    }
}

int
main(int argc, char *argv[])
{
  single_in= cp_io(0, "r");
  single_out= cp_io(1, "w");
  single_in->raw();
  single_out->raw();
  
  srv= mk_srv(5678);

  while (1)
    {
      if (single_in->input_avail())
	proc_single_input();
      if (nuof_multies < 2)
	{
	  if (srv->input_avail())
	    {
	      class cl_f *si, *so;
	      srv_accept(srv, &si, &so);
	      si->raw();
	      multies[nuof_multies].i= si;
	      multies[nuof_multies].o= so;
	      nuof_multies++;
	    }
	}
      check_multies();
    }
  return 0;
}
