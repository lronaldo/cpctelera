/*
   The new C2X keywords
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c23

static_assert(sizeof(nullptr) == sizeof(char *));
alignas(int) int i = alignof(int);
alignas(1) char c;
bool b = 0;
bool t = true;
bool f = false;
void *p = nullptr;

#endif

void
testC2Xkey(void)
{
#ifdef __SDCC
  ASSERT(!b);
  ASSERT(t);
  ASSERT(!f);
  ASSERT(!p);
#endif
}

