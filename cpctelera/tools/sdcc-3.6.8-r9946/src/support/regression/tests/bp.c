/* Base pointer tests, specifically for the z80.
 */
#include <testfwk.h>
#include <string.h>

int
verifyBlock(char *p, char val, int len)
{
  while (len--) {
    if (*p++ != val) {
      return 0;
    }
  }
  return 1;
}

int
spoil(int a)
{
  return a;
}

#if defined(__SDCC_mcs51) || defined(__SDCC_pic16)

// test devices with much less memory
#define ABOVE_MEM_SIZE       30
#define ABOVE_MEM_TEST_SIZE  17
#define BELOW_MEM_SIZE       20
#define BELOW_MEM_TEST_SIZE   7

#else

#define ABOVE_MEM_SIZE      400
#define ABOVE_MEM_TEST_SIZE  17
#define BELOW_MEM_SIZE      200
#define BELOW_MEM_TEST_SIZE  74

#endif

void
testBP(void)
{
  char above[ABOVE_MEM_SIZE];
  int f;
  char below[BELOW_MEM_SIZE];

  memset(above, ABOVE_MEM_TEST_SIZE, sizeof(above));
  memset(below, BELOW_MEM_TEST_SIZE, sizeof(below));

  ASSERT(verifyBlock(above, ABOVE_MEM_TEST_SIZE, sizeof(above)));
  ASSERT(verifyBlock(below, BELOW_MEM_TEST_SIZE, sizeof(below)));

  f = spoil(-5);
  spoil(f);

  ASSERT(verifyBlock(above, ABOVE_MEM_TEST_SIZE, sizeof(above)));
  ASSERT(verifyBlock(below, BELOW_MEM_TEST_SIZE, sizeof(below)));
}

