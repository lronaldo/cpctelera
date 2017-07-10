/* Tests character constants.
 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_c11
#endif

#include <stddef.h> // For wchar_t

#if defined(__SDCC) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <uchar.h> // For char16_t, char32_t

char16_t uc = u'c';
char32_t Uc = U'c';

#endif

char c = 'c';
wchar_t wc = L'c';

void
testCharConst(void)
{
#if defined(__SDCC) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  ASSERT (_Generic('c', default: 1, int: 0) == 0);
  ASSERT (_Generic(L'c', default: 1, wchar_t: 0) == 0);
  ASSERT (_Generic(u'c', default: 1, char16_t: 0) == 0);
  ASSERT (_Generic(U'c', default: 1, char32_t: 0) == 0);
#endif
}

