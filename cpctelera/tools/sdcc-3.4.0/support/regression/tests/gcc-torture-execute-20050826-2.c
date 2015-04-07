/*
   20050826-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

/* PR rtl-optimization/23560 */

struct rtattr
{
  unsigned short rta_len;
  unsigned short rta_type;
};

int inet_check_attr (void *r, struct rtattr **rta)
{
  int i;

  for (i = 1; i <= 14; i++)
    {
      struct rtattr *attr = rta[i - 1];
      if (attr)
	{
	  if (attr->rta_len - sizeof (struct rtattr) < 4)
	    return -22;
	  if (i != 9 && i != 8)
	    rta[i - 1] = attr + 1;
	}
    }
  return 0;
}

void
testTortureExecute (void)
{
#if 0 // TODO: Enable when struct can be assigned!
  struct rtattr rt[2];
  struct rtattr *rta[14];
  int i;

  rt[0].rta_len = sizeof (struct rtattr) + 8;
  rt[0].rta_type = 0;
  rt[1] = rt[0];
  for (i = 0; i < 14; i++)
    rta[i] = &rt[0];
  if (inet_check_attr (0, rta) != 0)
    ASSERT (0);
  for (i = 0; i < 14; i++)
    if (rta[i] != &rt[i != 7 && i != 8])
      ASSERT (0);
  for (i = 0; i < 14; i++)
    rta[i] = &rt[0];
  rta[1] = 0;
  rt[1].rta_len -= 8;
  rta[5] = &rt[1];
  if (inet_check_attr (0, rta) != -22)
    ASSERT (0);
  for (i = 0; i < 14; i++)
    if (i == 1 && rta[i] != 0)
      ASSERT (0);
    else if (i != 1 && i <= 5 && rta[i] != &rt[1])
      ASSERT (0);
    else if (i > 5 && rta[i] != &rt[0])
      ASSERT (0);
  return;
#endif
}
