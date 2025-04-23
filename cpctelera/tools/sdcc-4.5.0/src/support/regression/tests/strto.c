/** strto.c
*/
#include <testfwk.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

const char string0[] = "-2";
const char string1[] = "-9999999999999999999999";
const char string2[] = "9999999999999999999999";
const char string3[] = "-2test";
const char string4[] = "test";
const char string5[] = "023test";
const char string6[] = "-0x23test";

void
testStrto(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory
  char *e;

  ASSERT(strtoul("", 0, 10) == 0);
  ASSERT(strtoul("2", 0, 10) == 2);
  ASSERT(strtoul("3", 0, 10) == 3);
  ASSERT(strtoul("23", 0, 10) == 23);

  ASSERT(strtoul("23", 0, 0) == 23);
  ASSERT(strtoul("023", 0, 0) == 023);
  ASSERT(strtoul("0x23", 0, 0) == 0x23);
  
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202300L // C2X introduces binary prefix.
  ASSERT(strtoul("0b11", 0, 0) == 0b11);
  ASSERT(strtoul("0b11", 0, 2) == 0b11);
#endif

  ASSERT(strtoul("+23", 0, 0) == +23);
  ASSERT(strtoul("+023", 0, 0) == +023);
  ASSERT(strtoul("+0x23", 0, 0) == +0x23);

  ASSERT(strtol("-42", 0, 0) == -42);
  ASSERT(strtol("-042", 0, 0) == -042);
  ASSERT(strtol("-0x42", 0, 0) == -0x42);
  ASSERT(strtol("-0x42", 0, 16) == -0x42);

#if ULONG_MAX == 4294967295
  errno = 0;
  strtoul("4294967296", 0, 10);
  ASSERT(errno == ERANGE);
#endif

  errno = 0;
  ASSERT(strtol(string1, &e, 10) == LONG_MIN);
  ASSERT(errno == ERANGE);
  ASSERT(e == string1 + strlen(string1));

  errno = 0;
  ASSERT(strtol(string2, &e, 10) == LONG_MAX);
  ASSERT(errno == ERANGE);
  ASSERT(e == string2 + strlen(string2));

  errno = 0;
  ASSERT(strtoul(string0, &e, 10) == (unsigned long int)(-2));
  ASSERT(errno != ERANGE);
  ASSERT(e == string0 + strlen(string0));

  errno = 0;
  ASSERT(strtoul(string2, &e, 10) == ULONG_MAX);
  ASSERT(errno == ERANGE);
  ASSERT(e == string2 + strlen(string2));

  errno = 0;
  ASSERT(strtol(string3, &e, 10) == -2);
  ASSERT(errno != ERANGE);
  ASSERT(e == string3 + 2);

  errno = 0;
  ASSERT(strtol(string4, &e, 0) == 0);
  ASSERT(errno != ERANGE);
  ASSERT(e == string4);

  errno = 0;
  ASSERT(strtol(string5, &e, 0) == 023);
  ASSERT(errno != ERANGE);
  ASSERT(e == string5 + 3);

  errno = 0;
  ASSERT(strtol(string6, &e, 0) == -0x23);
  ASSERT(errno != ERANGE);
  ASSERT(e == string6 + 5);
#endif
}

