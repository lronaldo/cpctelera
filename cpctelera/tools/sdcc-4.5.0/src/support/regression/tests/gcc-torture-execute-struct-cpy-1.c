/*
struct-cpy-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* powerpc64-linux gcc miscompiled this due to rs6000.c:expand_block_move
   not setting mem aliasing info correctly for the code implementing the
   structure assignment.  */

struct termios
{
  unsigned int a;
  unsigned int b;
  unsigned int c;
  unsigned int d;
  unsigned char pad[28];
};

struct tty_driver
{
  unsigned char pad1[38];
  struct termios t;
};

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
static struct termios zero_t;
static struct tty_driver pty;

void ini (void)
{
  pty.t = zero_t;
  pty.t.a = 1;
  pty.t.b = 2;
  pty.t.c = 3;
  pty.t.d = 4;
}
#endif

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
  ini ();
  if (pty.t.a != 1
      || pty.t.b != 2
      || pty.t.c != 3
      || pty.t.d != 4)
    ASSERT (0);
#endif
  return;
}
