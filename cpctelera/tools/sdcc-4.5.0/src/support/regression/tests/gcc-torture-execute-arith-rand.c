/*
arith-rand.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
long
simple_rand ()
{
  static unsigned long seed = 47114711;
  unsigned long this = seed * 1103515245 + 12345;
  seed = this;
  return this >> 8;
}

unsigned long int
random_bitstring ()
{
  unsigned long int x;
  int n_bits;
  long ran;
  int tot_bits = 0;

  x = 0;
  for (;;)
    {
      ran = simple_rand ();
      n_bits = (ran >> 1) % 16;
      tot_bits += n_bits;

      if (n_bits == 0)
        return x;
      else
        {
          x <<= n_bits;
          if (ran & 1)
            x |= (1 << n_bits) - 1;

          if (tot_bits > 8 * sizeof (long) + 6)
            return x;
        }
    }
}
#endif

#define ABS(x) ((x) >= 0 ? (x) : -(x))

void
testTortureExecute (void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
  long int i;

  for (i = 0; i < 40 /* Was 1000 in GCC, reduced to speed up regression testing */; i++)
    {
      unsigned long x, y;
      x = random_bitstring ();
      y = random_bitstring ();

      if (sizeof (int) == sizeof (long))
        goto save_time;

      { unsigned long xx = x, yy = y, r1, r2;
        if (yy == 0) continue;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (r2 < yy);
        ASSERT (r1 * yy + r2 == xx);
      }
      { signed long xx = x, yy = y, r1, r2;
        if ((unsigned long) xx << 1 == 0 && yy == -1)
          continue;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (ABS (r2) < (unsigned long) ABS (yy));
        ASSERT ((signed long) (r1 * yy + r2) == xx);
      }
    save_time:
      { unsigned int xx = x, yy = y, r1, r2;
        if (yy == 0) continue;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (r2 < yy);
        ASSERT (r1 * yy + r2 == xx);
      }
      { signed int xx = x, yy = y, r1, r2;
        if ((unsigned int) xx << 1 == 0 && yy == -1)
          continue;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (ABS (r2) < (unsigned int) ABS (yy));
        ASSERT ((signed int) (r1 * yy + r2) == xx);
      }
      { unsigned short xx = x, yy = y, r1, r2;
        if (yy == 0) continue;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (r2 < yy);
        ASSERT (r1 * yy + r2 == xx);
      }
      { signed short xx = x, yy = y, r1, r2;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (ABS (r2) < (unsigned short) ABS (yy));
        ASSERT ((signed short) (r1 * yy + r2) == xx);
      }
      { unsigned char xx = x, yy = y, r1, r2;
        if (yy == 0) continue;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (r2 < yy);
        ASSERT (r1 * yy + r2 == xx);
      }
      { signed char xx = x, yy = y, r1, r2;
        r1 = xx / yy;
        r2 = xx % yy;
        ASSERT (ABS (r2) < (unsigned char) ABS (yy));
        ASSERT ((signed char) (r1 * yy + r2) == xx);
      }
    }

  return;
#endif
}
