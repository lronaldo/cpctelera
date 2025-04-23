/* Tests escape sequences
 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_c11
#endif

#include <stddef.h> // For wchar_t
#if defined(__SDCC) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__APPLE__) && !defined(__OpenBSD__) // As of 2023, macOS and OpenBSD are still not fully C11-compliant: they lack uchar.h.
#include <uchar.h> // For char16_t, char32_t
#endif

void
testEscape(void)
{
  volatile char c;
#if defined(__SDCC) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__APPLE__) && !defined(__OpenBSD__) // As of 2023, macOS and OpenBSD are still not fully C11-compliant: they lack uchar.h.
  volatile char16_t u;
#endif

  ASSERT ('\x55' == 0x55);
  c = '\x55';
  ASSERT (c == 0x55);

#if defined(__SDCC) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__APPLE__) && !defined(__OpenBSD__) // As of 2023, macOS and OpenBSD are still not fully C11-compliant: they lack uchar.h.
  ASSERT (u'\777' == 0777);
  u = u'\777';
  ASSERT (u == 0777);

  ASSERT (u'\x55aa' == 0x55aau);
  u = u'\x55aa';
  ASSERT (u == 0x55aau);

  ASSERT (u'\u55aa' == 0x55aau);
  u = u'\u55aa';
  ASSERT (u == 0x55aau);
#endif
}

