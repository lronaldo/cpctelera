/** wcsto.c
*/
#include <testfwk.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <wchar.h>

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory
wchar_t string0[] = L"-2";
wchar_t string1[] = L"-9999999999999999999999";
wchar_t string2[] = L"9999999999999999999999";
wchar_t string3[] = L"-2test";
wchar_t string4[] = L"test";
wchar_t string5[] = L"023test";
wchar_t string6[] = L"-0x23test";
#endif

void
testWcsto(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory
  wchar_t *e;

  ASSERT(wcstoul(L"", 0, 10) == 0);
  ASSERT(wcstoul(L"2", 0, 10) == 2);
  ASSERT(wcstoul(L"3", 0, 10) == 3);
  ASSERT(wcstoul(L"23", 0, 10) == 23);

  ASSERT(wcstoul(L"23", 0, 0) == 23);
  ASSERT(wcstoul(L"023", 0, 0) == 023);
  ASSERT(wcstoul(L"0x23", 0, 0) == 0x23);

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202300L // C2X introduces binary prefix.
  ASSERT(wcstoul(L"0b11", 0, 0) == 0b11);
  ASSERT(wcstoul(L"0b11", 0, 2) == 0b11);
#endif

  ASSERT(wcstoul(L"+23", 0, 0) == +23);
  ASSERT(wcstoul(L"+023", 0, 0) == +023);
  ASSERT(wcstoul(L"+0x23", 0, 0) == +0x23);

  ASSERT(wcstol(L"-42", 0, 0) == -42);
  ASSERT(wcstol(L"-042", 0, 0) == -042);
  ASSERT(wcstol(L"-0x42", 0, 0) == -0x42);
  ASSERT(wcstol(L"-0x42", 0, 16) == -0x42);

#if ULONG_MAX == 4294967295
  errno = 0;
  wcstoul(L"4294967296", 0, 10);
  ASSERT(errno == ERANGE);
#endif

  errno = 0;
  ASSERT(wcstol(string1, &e, 10) == LONG_MIN);
  ASSERT(errno == ERANGE);
  ASSERT(e == string1 + wcslen(string1));

  errno = 0;
  ASSERT(wcstol(string2, &e, 10) == LONG_MAX);
  ASSERT(errno == ERANGE);
  ASSERT(e == string2 + wcslen(string2));

  errno = 0;
  ASSERT(wcstoul(string0, &e, 10) == (unsigned long int)(-2));
  ASSERT(errno != ERANGE);
  ASSERT(e == string0 + wcslen(string0));

  errno = 0;
  ASSERT(wcstoul(string2, &e, 10) == ULONG_MAX);
  ASSERT(errno == ERANGE);
  ASSERT(e == string2 + wcslen(string2));

  errno = 0;
  ASSERT(wcstol(string3, &e, 10) == -2);
  ASSERT(errno != ERANGE);
  ASSERT(e == string3 + 2);

  errno = 0;
  ASSERT(wcstol(string4, &e, 0) == 0);
  ASSERT(errno != ERANGE);
  ASSERT(e == string4);

  errno = 0;
  ASSERT(wcstol(string5, &e, 0) == 023);
  ASSERT(errno != ERANGE);
  ASSERT(e == string5 + 3);

  errno = 0;
  ASSERT(wcstol(string6, &e, 0) == -0x23);
  ASSERT(errno != ERANGE);
  ASSERT(e == string6 + 5);
#endif
}

