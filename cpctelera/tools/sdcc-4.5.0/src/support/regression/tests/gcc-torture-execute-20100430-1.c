/*
20100430-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* This used to generate unaligned accesses at -O2 because of IVOPTS.  */

#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM) || defined(__SDCC_STACK_AUTO))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_pic14) // Lack of memory
struct packed_struct
{
  struct packed_struct1
  {
    unsigned char cc11;
    unsigned char cc12;
  } pst1;
  struct packed_struct2
  {
    unsigned char cc21;
    unsigned char cc22;
    unsigned short ss[104];
    unsigned char cc23[13];
  } pst2[4];
};

typedef struct
{
  int ii;
  struct packed_struct buf;
} info_t;

static unsigned short g;

static void
dummy (unsigned short s)
{
  g = s;
}

static int
foo (info_t *info)
{
  int i, j;

  for (i = 0; i < info->buf.pst1.cc11; i++)
    for (j = 0; j < info->buf.pst2[i].cc22; j++)
      dummy (info->buf.pst2[i].ss[j]);

  return 0;
}

int
wrapper (void)
{
  info_t info;
  info.buf.pst1.cc11 = 2;
  info.buf.pst2[0].cc22 = info.buf.pst2[1].cc22 = 8;
  return foo (&info);
}
#endif

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM) || defined(__SDCC_STACK_AUTO))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_pic14) // Lack of memory
  wrapper();
#endif
}

