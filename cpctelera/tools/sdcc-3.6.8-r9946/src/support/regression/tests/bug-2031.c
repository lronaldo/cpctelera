/*
   bug-2031.c
 */

#include <testfwk.h>

struct bugtest {
  unsigned int i;
  unsigned char c;
  unsigned long l;
};

#ifdef __SDCC
unsigned long gv;
unsigned long *gp = &gv;
unsigned int gi = (unsigned int) &gv;
unsigned long gl = (unsigned long) &gv;
unsigned char gc = (unsigned char) &gv;

struct bugtest gs = {
  (unsigned int) &gv,
  (unsigned char) &gv,
#ifndef __SDCC_gbz80
  (unsigned long) &gv,
#endif // __SDCC_gbz80
};
#endif // __SDCC

void testBug(void)
{
#ifdef __SDCC
  unsigned long lv;
  unsigned long *lp = &lv;
  unsigned char lc = (unsigned char) &lv;
  unsigned int li = (unsigned int) &lv;
  unsigned long ll = (unsigned long) &lv;
  struct bugtest ls = {(unsigned int) &lv, (unsigned char) &lv, (unsigned long) &lv};

  ASSERT (gc == (unsigned char) gp);
  ASSERT (gi == (unsigned int) gp);
  ASSERT (gl == (unsigned long) gp);

  ASSERT (gs.i == (unsigned int) gp);
  ASSERT (gs.c == (unsigned char) gp);
#ifndef __SDCC_gbz80
  ASSERT (gs.l == (unsigned long) gp);
#endif // __SDCC_gbz80
  
  ASSERT (lc == (unsigned char) lp);
  ASSERT (li == (unsigned int) lp);
  ASSERT (ll == (unsigned long) lp);  

  ASSERT (ls.i == (unsigned int) lp);
  ASSERT (ls.c == (unsigned char) lp);
  ASSERT (ls.l == (unsigned long) lp);
#endif // __SDCC
}