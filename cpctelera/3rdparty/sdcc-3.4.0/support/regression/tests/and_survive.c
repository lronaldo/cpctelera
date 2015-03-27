/*
  This test is meant to test the backend's generation of
  non-destructive and available on some architectures.
  Also serves as a regression test for bug #3534833.
*/

#include <testfwk.h>

/* Some architectures have non-destructive and when one operand is a literal with exactly one bit set (e.g. all Z80-related ) */
int litbitchar (unsigned char a)
{
  register unsigned char b = a + 1; /* Suggest allocating b to accumulator */

  if (b & 0x01)
    return(0);
  else if (b & 0x04)
    return(1);
  else if (b & 0x20)
    return(2);
  else
    return(3);
}

/* Some architectures have non-destructive and when one operand is a literal with exactly one bit set, and the other operand can be in any register or even spilt (e.g. S08) */
int litbitchar2 (unsigned char a, unsigned char c, unsigned char e)
{
  unsigned char b = a + 1;
  unsigned char d = c + 1;

  if (b & 0x01)
    {
      if (d & 0x01)
        return(8);
      else if (d & 0x04)
        return(9);
      else
        return(10);
    }
  else if (b & 0x04)
    return(1);
  else if (b & 0x20)
    {
      if (e & 0x01)
        return(4);
      else if (e & 0x04)
        return(5);
      else
        return(6);
    }
  else
    return(3);
}

/* Some architectures have non-destructive and when one operand is a literal with at most one bit per byte set (e.g. Z80) */
int litbitint (unsigned int a)
{
  register unsigned int b = a + 1; /* Suggest allocating b to accumulator */

  if (b & 0x0001)
    return(0);
  else if (b & 0x0004)
    return(1);
  else if (b & 0x2010)
    return(2);
  else
    return(3);
}

/* Some architectures have non-destructive and when one operand is a one-byte literal (e.g. Z180) */
int litchar (unsigned char a)
{
  register unsigned char b = a + 1; /* Suggest allocating b to accumulator */

  if (b & 0x33)
    return(0);
  else if (b & 0x44)
    return(1);
  else if (b & 0x1f)
    return(2);
  else
    return(3);
}

/* Some architectures have non-destructive and when one operand is in a register (e.g. Z180) */
int regchar (unsigned char a, unsigned char c)
{
  register unsigned char b = a + 1; /* Suggest allocating b to accumulator */
  register unsigned char d = c + 1;

  if (b & 0x11)
    return(0);
  else if (b & d)
    return(1);
  else if (b & 0x33)
    return(2);
  else
    return(3);
}

void testAndSurvive (void)
{
  ASSERT (litbitchar (0x77u - 1) == 0);
  ASSERT (litbitchar (0x74u - 1) == 1);
  ASSERT (litbitchar (0x70u - 1) == 2);
  ASSERT (litbitchar (0x80u - 1) == 3);

  ASSERT (litbitchar2 (0x01u - 1, 0x01u - 1, 0x01u) == 8);
  ASSERT (litbitchar2 (0x01u - 1, 0x80u - 1, 0x01u) == 10);
  ASSERT (litbitchar2 (0x74u - 1, 0x01u - 1, 0x01u) == 1);
  ASSERT (litbitchar2 (0x80u - 1, 0x01u - 1, 0x01u) == 3);
  ASSERT (litbitchar2 (0x20u - 1, 0x01u - 1, 0x04u) == 5);

  ASSERT (litbitint (0x0001u - 1) == 0);
  ASSERT (litbitint (0x8fe8u - 1) == 3);
  ASSERT (litbitint (0x3030u - 1) == 2);

  ASSERT (litchar (0x77u - 1) == 0);
  ASSERT (litchar (0x88u - 1) == 2);
  ASSERT (litchar (0x48u - 1) == 1);
  ASSERT (litchar (0x80u - 1) == 3);

  ASSERT (regchar (0x77u - 1, 0x00u - 1) == 0);
  ASSERT (regchar (0x88u - 1, 0x08u - 1) == 1);
  ASSERT (regchar (0x80u - 1, 0x88u - 1) == 1);
  ASSERT (regchar (0x80u - 1, 0x08u - 1) == 3);
}

