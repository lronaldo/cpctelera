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

#if defined(__SDCC_pic14) || defined(__SDCC_pdk14)

// test devices with much less memory
#define ABOVE_MEM_SIZE       20
#define ABOVE_MEM_TEST_SIZE  17
#define BELOW_MEM_SIZE       10
#define BELOW_MEM_TEST_SIZE   7

#elif defined(__SDCC_mcs51) || defined(__SDCC_pic16) || defined(__SDCC_pdk15) || defined(__SDCC_STACK_AUTO)

// test devices with much less
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

