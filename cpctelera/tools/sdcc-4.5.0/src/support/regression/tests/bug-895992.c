/* bug-895992.c

   Life Range problem with
   - uninitialized variable
   - loop
   - conditional block

   LR problem hits all ports, but this test is mcs51 specific
 */
#include <testfwk.h>

char p0 = 2;
unsigned short loops;

static void
wait (void)
{
  long i, j;

  /* just clobber all registers: */
  for (i = 0; i < 2; ++i)
    for (j = 0; j < 2; ++j)
      ;
}

#if !defined(PORT_HOST)
#  pragma disable_warning 84
#endif

static void
testLR (void)
{
  unsigned char number;
  unsigned char start = 1;
  unsigned char i;

  do
    {
      for (i = 1; i > 0 ; i--)
        wait ();        /* destroys all registers */
      if (start)
        {
          number = p0;
          start = 0;
        }
      number--;         /* 'number' might be used before initialization     */
                        /* the life range of 'number' must be extended to   */
                        /* the whole loop _including_ the conditional block */
      ++loops;
    }
  while (number != 0);

  ASSERT (loops == p0);
}
