/** tests for wcsnlen
*/

#include <testfwk.h>

#include <string.h>
#include <wchar.h>

const wchar_t* hello1 = L"hello1";
const wchar_t hello2[] = L"hello2";
wchar_t hello3[] = L"hello3";

void
testStr(void)
{
#if  defined(__SDCC) || (_POSIX_C_SOURCE >= 200809L) || (__STDC_VERSION_STRING_H__ > 202311L) // wcsnlen is a C2Y function previously vailable in POSIX.
#ifndef __SDCC_pdk14 // lack of RAM
  const wchar_t hello4[] = L"hello4";
  const wchar_t hello5[7];

  memcpy (hello5, hello4, 7 * sizeof(wchar_t));

  ASSERT(wcsnlen(L"hello0", 23) == wcslen(L"hello0"));
  ASSERT(wcsnlen(hello1, 23) == wcslen(hello1));
  ASSERT(wcsnlen(hello2, 23) == wcslen(hello2));
  ASSERT(wcsnlen(hello3, 23) == wcslen(hello3));
  ASSERT(wcsnlen(hello4, 23) == wcslen(hello4));
  ASSERT(wcsnlen(hello5, 23) == wcslen(hello5));

  ASSERT(wcsnlen(L"hello0", 3) == 3);
  ASSERT(wcsnlen(hello1, 3) == 3);
  ASSERT(wcsnlen(hello2, 3) == 3);
  ASSERT(wcsnlen(hello3, 3) == 3);
  ASSERT(wcsnlen(hello4, 3) == 3);
  ASSERT(wcsnlen(hello5, 3) == 3);

#if !(defined(__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // lack of code memory
  ASSERT(wcsnlen(L"hello0", 6) == 6);
  ASSERT(wcsnlen(hello1, 6) == 6);
  ASSERT(wcsnlen(hello2, 6) == 6);
  ASSERT(wcsnlen(hello3, 6) == 6);
  ASSERT(wcsnlen(hello4, 6) == 6);
  ASSERT(wcsnlen(hello5, 6) == 6);

  ASSERT(wcsnlen(L"hello0", 7) == 6);
  ASSERT(wcsnlen(hello1, 7) == 6);
  ASSERT(wcsnlen(hello2, 7) == 6);
  ASSERT(wcsnlen(hello3, 7) == 6);
  ASSERT(wcsnlen(hello4, 7) == 6);
  ASSERT(wcsnlen(hello5, 7) == 6);
#endif
#endif
#endif
}

