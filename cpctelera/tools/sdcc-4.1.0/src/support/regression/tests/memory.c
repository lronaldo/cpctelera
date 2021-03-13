/** memory function test
 builtins: on, off
 part: 1, 2, 3
*/
#include <testfwk.h>

#include <string.h>

#if defined(__SDCC_stm8) || defined(PORT_HOST) || defined(__SDCC_ds390) || \
	(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_LARGE) || defined(__SDCC_MODEL_HUGE))) || \
    defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka)
#define TEST_LARGE
#endif


#define BUILTINS_{builtins} 1
#if defined(BUILTINS_off) && !defined(__SDCC_pdk14)
#undef memchr
#undef memcmp
#undef memcpy
#undef memccpy
#undef memmove
#undef memset
#undef strlen
#endif

/* Note: the majority of library function calls purposefully do not assign or
 * test the return value. This is so that, on some ports (e.g. z80), the
 * built-in functions are used and tested. */

unsigned char destination[9];
const unsigned char source[9] = {0, 1, 2, 3};
unsigned char *ret;
int c;

// Some of these tests intentionally do not check the return value of functions:
// Some backends use built-ins when the return value is not used, but fall back
// to a library version otherwise. So we need tests with unused return value to
// test the built-in. Similarly, code paths for built-ns are different depending
// on the last parameter being a literal vs. not. So we need to test both with a
// literal and a volatile variable.
void testMemory(void)
{
#if {part} == 1
  /* Local stack variables */
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
  ret = memmove(destination, "abcdefgh", 9); // Full copy
  ASSERT(strcmp(destination, "abcdefgh") == 0);
  ASSERT(ret == destination);

  ret = memmove(destination + 1, "123", 3); // Sub copy with offset
  ASSERT(strcmp(destination, "a123efgh") == 0);
  ASSERT(ret == destination + 1);

  ret = memmove(destination, source, 0); // Zero count, changes nothing
  ASSERT(strcmp(destination, "a123efgh") == 0);
  ASSERT(ret == destination);

  ret = memmove(destination + 1, destination, 7); // Overlap to right
  ASSERT(strcmp(destination, "aa123efg") == 0);
  ASSERT(ret == destination + 1);

  ret = memmove(destination, destination + 1, 8); // Overlap to left
  ASSERT(strcmp(destination, "a123efg") == 0);
  ASSERT(ret == destination);

  ret = memmove(destination, destination, 9); // 100% overlap
  ASSERT(strcmp(destination, "a123efg") == 0);
  ASSERT(ret == destination);

  /* Test memchr() */
  memcpy(destination, source, 4);
  ASSERT(NULL == memchr(destination, 5, 4));
  ASSERT(destination == memchr(destination, 0, 4));
  ASSERT(destination + 3 == memchr(destination, 3, 4));

  /* Test strlen() */
  ASSERT(strlen("test") == 4);
  ASSERT(strlen("t") == 1);
  ASSERT(strlen("") == 0);

  /* Test memccpy() */
  destination[2] = 0;
  destination[3] = 0;
#ifndef PORT_HOST
  ASSERT(memccpy(destination, source, 2, 4) == destination + 3);
  ASSERT(destination[2] == 2);
  ASSERT(destination[3] == 0);
#endif

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
#if {part} == 2
#ifdef TEST_LARGE
  memset(largedest, 0, 1050);
  memset(largedest, 1, 4);
  memset(largesource, 2, 1050);

  memcpy(largedest + 1, largesource, 1024);

  ASSERT(largedest[0] == 1);
  ASSERT(largedest[1] == 2);
  ASSERT(largedest[1024] == 2);
  ASSERT(largedest[1025] == 0);

  /* Test strlen with large string */
  memset(largesource, 'x', 999);
  largesource[999] = '\0';
  ASSERT(strlen(largesource) == 999);

  /* Test memmove() with large (>255) counts */
  unsigned char *ls_a = largesource + 0;
  unsigned char *ls_b = largesource + 500;

  ret = memset(ls_a, 'a', 500);
  ASSERT(ret == ls_a);
  ret = memset(ls_b, 'b', 500);
  ASSERT(ret == ls_b);
  ASSERT(ls_a[0] == 'a' && ls_a[499] == 'a');
  ASSERT(ls_b[0] == 'b' && ls_b[499] == 'b');

  // => aa--
  ret = memmove(largedest, ls_a, 500); // put 'a' in first half of dest
  ASSERT(ret == largedest);
  ASSERT(memcmp(largedest, ls_a, 500) == 0);

  // => aaaa
  ret = memmove(largedest + 500, largedest, 500); // copy to last half (no overlap)
  ASSERT(ret == largedest + 500);
  ASSERT(memcmp(largedest + 500, ls_a, 500) == 0);

  // => abba
  ret = memmove(largedest + 250, ls_b, 500); // put 'b' in middle half
  ASSERT(ret == largedest + 250);
  ASSERT(memcmp(largedest, ls_a, 250) == 0);
  ASSERT(memcmp(largedest + 250, ls_b, 500) == 0);
  ASSERT(memcmp(largedest + 750, ls_a, 250) == 0);

  // => abaa
  ret = memmove(largedest + 250, largedest + 500, 500); // overlap to left
  ASSERT(ret == largedest + 250);
  ASSERT(memcmp(largedest, ls_a, 250) == 0);
  ASSERT(memcmp(largedest + 250, ls_b, 250) == 0);
  ASSERT(memcmp(largedest + 500, ls_a, 500) == 0);

  // => abba
  ret = memmove(largedest + 500, largedest + 250, 500); // overlap to right
  ASSERT(ret == largedest + 500);
  ASSERT(memcmp(largedest, ls_a, 250) == 0);
  ASSERT(memcmp(largedest + 250, ls_b, 500) == 0);
  ASSERT(memcmp(largedest + 750, ls_a, 250) == 0);
#endif
#endif
}

void testRetVal(void)
{
#if {part} == 3
  volatile size_t one = 1;
  volatile size_t zero = 0;
  ASSERT(destination == memcpy(destination, source, sizeof(source)));
  ASSERT(destination == memcpy(destination, source, 0));
  ASSERT(destination == memcpy(destination, source, zero));
  ASSERT(destination == memcpy(destination, source, one));
  ASSERT(destination == memset(destination, (int)one, zero));
  ASSERT(destination == memset(destination, (int)zero, 0));
  ASSERT(destination == memset(destination, (int)zero, one));
  ASSERT(destination == memset(destination, (int)one, sizeof(destination)));
  ASSERT(&one == memset(&one, 0, sizeof(one)));
  ASSERT(&zero == memcpy(&zero, &one, sizeof(one)));
#endif
}
