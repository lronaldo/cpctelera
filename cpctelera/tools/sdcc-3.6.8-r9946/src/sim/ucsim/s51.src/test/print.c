#if defined __SDCC || defined SDCC
#include <stdio.h>
#elif defined __C51__
#include <stdio.h>
#endif

#include "print.h"

void
print(char *s) __reentrant
{
  if (s)
    for (; *s; putchar(*s++)) ;
}

void
print_form(char *s, long l, void *p) __reentrant
{
  if (s)
    for (; *s; s++)
      if (*s != '%')
	putchar(*s);
      else
	{
	  s++;
	  if (*s)
	    switch (*s)
	      {
	      case 'u': print_u(l); break;
	      case 'd': print_d(l); break;
	      case 'p': print_p(p); break;
	      case 'x': print_x(l); break;
	      case '2': print_cx(l); break;
	      case '4': print_lx(l); break;
	      }
	}
}

/* signed integer in decimal */

void
print_d(long i) __reentrant
{
  long x= 1000000000;
  char in= 0;
  if (i<0)
    {
      putchar('-');
      i= -i;
    }
  while (x)
    {
      int j= i/x;
      if (in || j || (x==1))
	putchar(j+'0');
      in= in || j;
      i%= x;
      x/= 10;
    }
}


/* unsigned integer in decimal */

void
print_u(unsigned int i) __reentrant
{
  int x= 10000;
  while (x)
    {
      int j= i/x;
      putchar(j+'0');
      i%= x;
      x/= 10;
    }
}


/* unsigned int in hex */

void
print_cx(unsigned char i)
{
  putchar((i/16)+(((i/16)<10)?'0':('A'-10)));
  putchar((i&15)+(((i&15)<10)?'0':('A'-10)));
}

void
print_x(unsigned int i) __reentrant
{
/*
  unsigned char j;
  for (j= 0; j<4; j++)
    {
      char v= (i&0xf000)>>12;
      putchar(v+((v<10)?'0':('A'-10)));
      i<<= 4;
    }
*/
  print_cx(i/256);
  print_cx(i&0xff);
}

void
print_lx(unsigned long i) __reentrant
{
  print_x(i >> 16);
  print_x(i & 0xffff);
}

void
print_p(void *p) __reentrant
{
  unsigned char t= ((long)p)/0x10000;
  if (t>=0x80)
    putchar('C');
  else if (t>=0x60)
    putchar('P');
  else if (t>=0x40)
    putchar('I');
  else
    putchar('X');
  putchar(':');
  print_x(((long)p)&0xffff);
}


void
print_c(char c)
{
  putchar(c);
}


void
term_cls()
{
  print("\033[2J");
}


void
term_xy(char x1, char y1)
{
  putchar('\033');
  putchar('[');
  print_u(y1);
  putchar(';');
  print_u(x1);
  putchar('H');
}


void
term_save()
{
  print("\033[s");
}


void
term_restore()
{
  print("\033[u");
}


void
term_hide()
{
  print("\033[?25l");
}


void
term_show()
{
  print("\033[?25h");
}


void
term_color(int bg, int fg) __reentrant
{
  if (bg >= 0)
    print_form("\033[%um", bg+40, NULL);
  if (fg >= 0)
    print_form("\033[%um", fg+30, NULL);
}
