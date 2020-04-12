/** memory function test
*/
#include <testfwk.h>

#include <string.h>

#if defined(__SDCC_stm8) || defined(PORT_HOST) || defined(__SDCC_ds390) || \
	(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_LARGE) || defined(__SDCC_MODEL_HUGE))) || \
    defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r3ka)
#define TEST_LARGE
#endif

unsigned char destination[9];
const unsigned char source[9] = {0, 1, 2, 3};
int c;

void testmemory(void)
{
  volatile size_t zero = 0;
  volatile size_t one = 1;

  ASSERT(source[0] == 0);

  /* Test memset() */
  c = 8;
  memset(destination, c, one);
  ASSERT(destination[0] == 8);
  memset(destination, c, 3);
  ASSERT(destination[2] == 8);
  destination[3] = 23;
  memset(destination, 42, 3);
  ASSERT(destination[0] == 42);
  ASSERT(destination[2] == 42);
  ASSERT(destination[3] == 23);
  memset(destination, 23, 1);
  ASSERT(destination[0] == 23);
  ASSERT(destination[1] == 42);
  memset(destination, 42, 1);

  /* Test memcpy() */
  memcpy(destination, source, 0);
  ASSERT(destination[0] == 42);
  memcpy(destination, source, zero);
  ASSERT(destination[0] == 42);
  memcpy(destination + 1, source + 1, 2);
  ASSERT(destination[0] == 42);
  ASSERT(destination[2] == source[2]);
  ASSERT(destination[3] == 23);
  memcpy(destination, source, one);
  ASSERT(destination[0] == 0);
  memset(destination, 5, 9);
  memcpy(destination, source, 8);
  ASSERT(destination[7] == source[7]);
  ASSERT(destination[8] == 5);
  memset(destination, 5, 9);
  memcpy(destination, source, 3);
  ASSERT(destination[2] == source[2]);
  ASSERT(destination[3] == 5);

  /* Test memcmp() */
  memcpy(destination, source, 4);
  ASSERT(memcmp(destination, source, 4) == 0);
#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  /* Test memmove() */
  memcpy(destination, source, 4);
  memmove(destination, destination + 1, 3);
  ASSERT(destination[0] == source[1]);
  ASSERT(destination[2] == source[3]);
  ASSERT(destination[3] == source[3]);
  memcpy(destination, source, 4);
  memmove(destination + 1, destination, 3);
  ASSERT(destination[0] == source[0]);
  ASSERT(destination[1] == source[0]);
  ASSERT(destination[3] == source[2]);

  /* Test memchr() */
  memcpy(destination, source, 4);
  ASSERT(NULL == memchr(destination, 5, 4));
  ASSERT(destination == memchr(destination, 0, 4));
  ASSERT(destination + 3 == memchr(destination, 3, 4));

  /* Test strlen() */
  ASSERT(strlen("test") == 4);
  ASSERT(strlen("t") == 1);
  ASSERT(strlen("") == 0);

  destination[2] = 0;
  destination[3] = 0;
#ifndef PORT_HOST
  ASSERT(memccpy(destination, source, 2, 4) == destination + 3);
  ASSERT(destination[2] == 2);
  ASSERT(destination[3] == 0);
#endif
#endif
#endif
}

#ifdef TEST_LARGE
unsigned char largedest[1050];
unsigned char largesource[1050];
#endif

void testLarge(void)
{
#ifdef TEST_LARGE
  memset(largedest, 0, 1050);
  memset(largedest, 1, 4);
  memset(largesource, 2, 1050);

  memcpy(largedest + 1, largesource, 1024);

  ASSERT(largedest[0] == 1);
  ASSERT(largedest[1] == 2);
  ASSERT(largedest[1024] == 2);
  ASSERT(largedest[1025] == 0);
#endif
}
