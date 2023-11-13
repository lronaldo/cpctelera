#include <stdio.h>
#include <unistd.h>

#include "fiocl.h"
#include "utils.h"
#include "charscl.h"


void
regular_ftest(const char *fn)
{
  int i;
  class cl_f *f;

  printf("Testing regular file access: %s\n", fn);
  printf("Write test\n");
  f= mk_io(fn, "w");
  f->init();
  f->write_str("proba\n");

  printf("Read test\n");
  f->open(fn, "r");
  while (f->input_avail())
    {
      char buf[100];
      i= f->read(buf, 99);
      printf("read=%d\n", i);
      if (i)
	buf[i]= 0;
      else
	break;
      printf("data:%s\n", buf);
    }
  f->close();

  delete f;
}

void
stdin_ftest()
{
  class cl_f *f= mk_io(NULL, chars(""));
  char buf[100];
  int i, p= 0, done= 0;

  printf("STDIN test\n");
  f->use_opened(0, "r");
  printf("istty= %d\n", f->tty);
  i= f->read(buf, 99);
  buf[i]= '\0';
  printf("Got: %s\n", buf);
  while (done<10)
    {
      p= 0;
      while (f->input_avail())
	{
	  char c;
	  i= f->read(&c, 1);
	  printf("read=%d\n", i);
	  if ((c == '\n') ||
	      (c == '\r'))
	    break;
	  if (i == 0)
	    break;
	  if (p<99)
	    buf[p++]= c;
	}
      buf[p]= '\0';
      printf("data:%s\n", buf);
      //done= strcmp(buf, "exit") == 0;
      done++;
    }
  delete f;
}

int
main(int argc, char *argv[])
{
  const char *fn;

  if (argc > 1)
    fn= argv[1];
  else
    fn= "ftest.txt";
  regular_ftest(fn);
  stdin_ftest();
  
  double last= dnow();
  int cnt= 0;
  while (1)
    {
      if (dnow() - last > 0.5)
	{
	  last= dnow();
	  printf("HUKK\n");
	  if (++cnt > 5)
	    return 0;
	}
    }
  return 0;
}
