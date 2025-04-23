/*
   20121108-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 196
#pragma disable_warning 84
#pragma disable_warning 85

#include <stdio.h>

char temp[] = "192.168.190.160";
unsigned result = (((((192u<<8)|168u)<<8)|190u)<<8)|160u;

int strtoul1(const char *a, char **b, int c);
int strtoul1(const char *a, char **b, int c)
{
  *b = a+3;
  if (a == temp)
    return 192;
  else if (a == temp+4)
    return 168;
  else if (a == temp+8)
    return 190;
  else if (a == temp+12)
    return 160;
  ASSERT (0);
}

int string_to_ip(const char *s);
int string_to_ip(const char *s)
{
        int addr;
        char *e;
        int i;

        if (s == 0)
                return(0);

        for (addr=0, i=0; i<4; ++i) {
                int val = s ? strtoul1(s, &e, 10) : 0;
                addr <<= 8;
                addr |= (val & 0xFF);
                if (s) {
                        s = (*e) ? e+1 : e;
                }
        }

        return addr;
}

void
testTortureExecute (void)
{
#if !defined( __SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  int t = string_to_ip (temp);
  printf ("%x\n", t);
  printf ("%x\n", result);
  if (t != result)
    ASSERT (0);
  printf ("WORKS.\n");
#endif
}

#if !defined(PORT_HOST)
int putchar(int c)
{
  c;
}
#endif

