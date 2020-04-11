#include <string.h>
#include "hw.h"

#include "serial.h"
#include "print.h"

__xdata char *simif;

int cnt;

void process(char *cmd)
{
  if (strstr(cmd, "test") == cmd)
    {
      cnt++;
      print_c('_');print(cmd);print_c('_');
      print_f("This is a test %d\n", cnt);
    }
  else if (strstr(cmd, "dump") == cmd)
    {
      int i;
      print_c('_');print(cmd);print_c('_');
      for (i= 0; i<100; i++)
	{
	  print_cx(i);
	  print(" ");
	}
      print("\n----\n");
    }
  else
    {
      print("Unknown command: \"");print(cmd);print("\"\n");
    }
}

__xdata char cmd[40];
char ptr;

void main(void)
{
  simif= (__xdata char *)0xffff;
  serial_init(9600);
  cmd[ptr=0]= 0;
  print("Hello World!\n");
  P1= 0;
  while (1)
    {
      if (serial_received())
	{
	  char c= serial_receive();
	  if ((c == '\n') ||
	      (c == '\r'))
	    {
	      process(cmd);
	      cmd[ptr=0]= 0;
	    }
	  else if (ptr < 39)
	    {
	      cmd[ptr++]= c;
	      cmd[ptr]= 0;
	      print("cmd=");print(cmd);print_c('\n');
	    }
	}
    }
}
