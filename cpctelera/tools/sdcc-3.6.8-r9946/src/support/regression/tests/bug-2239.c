/*
  bug-2239.c
*/

#include <testfwk.h>

static unsigned short buf[] = {0x8765, 0xa5a5, 0xc3c3, 0xf0f0, 0xabcd};

void testBug(void)
{
  unsigned char *p = ((unsigned char *) buf) + 4;
  ASSERT (*((unsigned char *) (p + 2 - 4)) == 0xa5);
  ASSERT (*((unsigned char *) (p - 2)) == 0xa5);
  ASSERT (*((unsigned short *) (p + 3 - 7)) == 0x8765);
  ASSERT (*((unsigned short *) (p - 4)) == 0x8765);
  ASSERT (*((unsigned char *) (p - 2 + 4)) == 0xf0);
  ASSERT (*((unsigned char *) (p + 2)) == 0xf0);
  ASSERT (*((unsigned short *) (p - 3 + 7)) == 0xabcd);
  ASSERT (*((unsigned short *) (p + 4)) == 0xabcd);
}
