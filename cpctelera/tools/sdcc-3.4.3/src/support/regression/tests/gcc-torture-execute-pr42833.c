/*
   pr42833.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdint.h>
#include <string.h>
#include <limits.h>

/* If SSIZE_MAX is defined in limits.h, then we are running in a POSIX    */
/* environment that already has a ssize_t definition (which may have been */
/* included indirectly via string.h). In this case, use the guaranteed    */
/* definition in sys/types.h, otherwise assume int compatible and hope    */
/* for the best. */
#ifdef SSIZE_MAX
#include <sys/types.h>
#else
typedef int ssize_t;
#endif

typedef struct { int8_t v1; int8_t v2; int8_t v3; int8_t v4; } neon_s8;

uint32_t helper_neon_rshl_s8 (uint32_t arg1, uint32_t arg2);

uint32_t
helper_neon_rshl_s8 (uint32_t arg1, uint32_t arg2)
{
  uint32_t res;
  neon_s8 vsrc1;
  neon_s8 vsrc2;
  neon_s8 vdest;
  do
    {
      union
      {
	neon_s8 v;
	uint32_t i;
      } conv_u;
      conv_u.i = (arg1);
      /*vsrc1 = conv_u.v; not in sdcc */
      memcpy (&vsrc1, &(conv_u.v), sizeof (neon_s8));
    }
  while (0);
  do
    {
      union
      {
	neon_s8 v;
	uint32_t i;
      } conv_u;
      conv_u.i = (arg2);
      /*vsrc2 = conv_u.v; not in sdcc*/
      memcpy (&vsrc2, &(conv_u.v), sizeof (neon_s8));
    }
  while (0);
  do
    {
      int8_t tmp;
      tmp = (int8_t) vsrc2.v1;
      if (tmp >= (ssize_t) sizeof (vsrc1.v1) * 8)
	{
	  vdest.v1 = 0;
	}
      else if (tmp < -(ssize_t) sizeof (vsrc1.v1) * 8)
	{
	  vdest.v1 = vsrc1.v1 >> (sizeof (vsrc1.v1) * 8 - 1);
	}
      else if (tmp == -(ssize_t) sizeof (vsrc1.v1) * 8)
	{
	  vdest.v1 = vsrc1.v1 >> (tmp - 1);
	  vdest.v1++;
	  vdest.v1 >>= 1;
	}
      else if (tmp < 0)
	{
	  vdest.v1 = (vsrc1.v1 + (1 << (-1 - tmp))) >> -tmp;
	}
      else
	{
	  vdest.v1 = vsrc1.v1 << tmp;
	}
    }
  while (0);
  do
    {
      int8_t tmp;
      tmp = (int8_t) vsrc2.v2;
      if (tmp >= (ssize_t) sizeof (vsrc1.v2) * 8)
	{
	  vdest.v2 = 0;
	}
      else if (tmp < -(ssize_t) sizeof (vsrc1.v2) * 8)
	{
	  vdest.v2 = vsrc1.v2 >> (sizeof (vsrc1.v2) * 8 - 1);
	}
      else if (tmp == -(ssize_t) sizeof (vsrc1.v2) * 8)
	{
	  vdest.v2 = vsrc1.v2 >> (tmp - 1);
	  vdest.v2++;
	  vdest.v2 >>= 1;
	}
      else if (tmp < 0)
	{
	  vdest.v2 = (vsrc1.v2 + (1 << (-1 - tmp))) >> -tmp;
	}
      else
	{
	  vdest.v2 = vsrc1.v2 << tmp;
	}
    }
  while (0);
  do
    {
      int8_t tmp;
      tmp = (int8_t) vsrc2.v3;
      if (tmp >= (ssize_t) sizeof (vsrc1.v3) * 8)
	{
	  vdest.v3 = 0;
	}
      else if (tmp < -(ssize_t) sizeof (vsrc1.v3) * 8)
	{
	  vdest.v3 = vsrc1.v3 >> (sizeof (vsrc1.v3) * 8 - 1);
	}
      else if (tmp == -(ssize_t) sizeof (vsrc1.v3) * 8)
	{
	  vdest.v3 = vsrc1.v3 >> (tmp - 1);
	  vdest.v3++;
	  vdest.v3 >>= 1;
	}
      else if (tmp < 0)
	{
	  vdest.v3 = (vsrc1.v3 + (1 << (-1 - tmp))) >> -tmp;
	}
      else
	{
	  vdest.v3 = vsrc1.v3 << tmp;
	}
    }
  while (0);
  do
    {
      int8_t tmp;
      tmp = (int8_t) vsrc2.v4;
      if (tmp >= (ssize_t) sizeof (vsrc1.v4) * 8)
	{
	  vdest.v4 = 0;
	}
      else if (tmp < -(ssize_t) sizeof (vsrc1.v4) * 8)
	{
	  vdest.v4 = vsrc1.v4 >> (sizeof (vsrc1.v4) * 8 - 1);
	}
      else if (tmp == -(ssize_t) sizeof (vsrc1.v4) * 8)
	{
	  vdest.v4 = vsrc1.v4 >> (tmp - 1);
	  vdest.v4++;
	  vdest.v4 >>= 1;
	}
      else if (tmp < 0)
	{
	  vdest.v4 = (vsrc1.v4 + (1 << (-1 - tmp))) >> -tmp;
	}
      else
	{
	  vdest.v4 = vsrc1.v4 << tmp;
	}
    }
  while (0);;
  do
    {
      union
      {
	neon_s8 v;
	uint32_t i;
      } conv_u;
      /*conv_u.v = (vdest) not in sdcc */;
      memcpy (&conv_u.v, &(vdest), sizeof (neon_s8));
      res = conv_u.i;
    }
  while (0);
  return res;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && __GNUC__ < 5)
  uint32_t r = helper_neon_rshl_s8 (0x05050505, 0x01010101);
  if (r != 0x0a0a0a0a)
    ASSERT (0);
  return;
#endif
}

