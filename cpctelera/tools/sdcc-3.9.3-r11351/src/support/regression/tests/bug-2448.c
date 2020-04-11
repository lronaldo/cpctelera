/*
   bug-2448.c
*/

#include <testfwk.h>

typedef unsigned char BYTE;
typedef unsigned int WORD;

volatile BYTE MYDAT[8];

#define MAKEWORD(msb, lsb) (((WORD) (msb) << 8) | (lsb))

#define SETUP_VALUE() MAKEWORD (MYDAT[3], MYDAT[2])
#define SETUP_INDEX() MAKEWORD (MYDAT[5], MYDAT[4])
#define SETUP_LENGTH() MAKEWORD (MYDAT[7], MYDAT[6])

WORD badfunc (BYTE cmd)
{
    WORD val = SETUP_VALUE();
    WORD idx = SETUP_INDEX();
    WORD len = SETUP_LENGTH();
    volatile WORD r = val + idx + len; 

    if (cmd == 0)
      return len;
    else
      return 0;
}

void testBug (void)
{
  MYDAT[2] = 10; // val
  MYDAT[3] = 0;
  MYDAT[4] = 11; // idx
  MYDAT[5] = 0;
  MYDAT[6] = 12; // len
  MYDAT[7] = 0;

  ASSERT (badfunc (0) == 12);
  ASSERT (badfunc (1) == 0);
}
