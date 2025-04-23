/*
struct-ret-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#pragma disable_warning 93

#include <stdio.h>
#include <string.h>

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
char out[100];
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) // Todo: enable when struct parmeters are supported!

typedef struct { double d; int i[3]; } B;
typedef struct { char c[33],c1; } X;

char c1 = 'a';
char c2 = 127;
char c3 = (char)128;
char c4 = (char)255;
char c5 = -1;

double d1 = 0.1;
double d2 = 0.2;
double d3 = 0.3;
double d4 = 0.4;
double d5 = 0.5;
double d6 = 0.6;
double d7 = 0.7;
double d8 = 0.8;
double d9 = 0.9;

B B1 = {0.1,{1,2,3}};
B B2 = {0.2,{5,4,3}};
X X1 = {"abcdefghijklmnopqrstuvwxyzABCDEF", 'G'};
X X2 = {"123",'9'};
X X3 = {"return-return-return",'R'};

X f (B a, char b, double c, B d)
{
  static X xr = {"return val", 'R'};
  X r;
  r = xr;
  r.c1 = b;
  sprintf (out, "X f(B,char,double,B):({%g,{%d,%d,%d}},'%c',%g,{%g,{%d,%d,%d}})",
	   a.d, a.i[0], a.i[1], a.i[2], b, c, d.d, d.i[0], d.i[1], d.i[2]);
  return r;
}

X (*fp) (B, char, double, B) = &f;
#endif
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) // Todo: enable when struct parmeters are supported!
  X Xr;
  char tmp[100];

  Xr = f (B1, c2, d3, B2);
  strcpy (tmp, out);
  Xr.c[0] = Xr.c1 = '\0';
  Xr = (*fp) (B1, c2, d3, B2);
  if (strcmp (tmp, out))
    ASSERT (0);

  return;
#endif
#endif
}
