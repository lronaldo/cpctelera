/** Code generation for f8 could overwrite the lower byte of a still-needed pointer value in register x.
*/

#include <testfwk.h>

#include <string.h>

#ifdef __SDCC_pdk14
#define GUARDSIZE 4
#else
#define GUARDSIZE 16
#endif

struct a { unsigned int bitfield : 1; };

unsigned int x;

void
f (void)
{
  struct a a = {0};    // Here the lower byte of a pointer to a was overwritten.
  x = 0xbeef;
  a.bitfield |= x;     // So this 1 was stored to the wrong location.
  if (a.bitfield != 1) // But this cheak read fom the wrong location, too, and thus got the correct value.
    ASSERT (0);
  return;
}

void h(const char *c0, const char *c1)
{
  ASSERT (!memcmp (c0, c1, GUARDSIZE));
}

void
f2 (void)
{
  char c0 [GUARDSIZE] = {0};
  struct a a = {0};
  char c1 [GUARDSIZE] = {0};
  x = 0xbeef;
  a.bitfield |= x;
  if (a.bitfield != 1)
    ASSERT (0);
  h (c0, c1);
  return;
}

void
f3 (void)
{
  char c0 [GUARDSIZE] = {0};
  char c1 [GUARDSIZE] = {0};
  f ();
  f2 ();
  h (c0, c1);
}

void
testBug(void)
{
  f ();
  f2 ();
  f3 ();
}

