/* Tests escape sequences
 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_c11
#endif

#include <stddef.h> // For wchar_t
#ifndef PORT_HOST // Too many old host compilers out there
#include <uchar.h> // For char16_t, char32_t
#endif

void
testEscape(void)
{
  volatile char c;
#ifndef PORT_HOST // Too many old host compilers out there
  volatile char16_t u;
#endif

  ASSERT ('\x55' == 0x55);
  c = '\x55';
  ASSERT (c == 0x55);

#ifndef PORT_HOST // Too many old host compilers out there
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

