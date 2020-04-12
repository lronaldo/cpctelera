/*
   20021111-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Origin: PR c/8467 */

int aim_callhandler(int sess, int conn, unsigned short family, unsigned short type);

int aim_callhandler(int sess, int conn, unsigned short family, unsigned short type)
{
  static int i = 0;

  if (!conn)
    return 0;

  if (type == 0xffff)
    {
      return 0;
    }

  if (i >= 1)
    ASSERT (0);

  i++;
  return aim_callhandler(sess, conn, family, (unsigned short) 0xffff);
}

void testTortureExecute (void)
{
  aim_callhandler (0, 1, 0, 0);
  return;
}
