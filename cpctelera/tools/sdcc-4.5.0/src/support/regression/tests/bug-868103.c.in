/** Bug 868103 tests.

    storage1: __near, __far, __code,
    storage2: __near, __far,
*/

#ifndef STORAGE1
#define STORAGE1 {storage1}
#endif

#ifndef STORAGE2
#define STORAGE2 {storage2}
#endif

#include <testfwk.h>

typedef struct {
  STORAGE1 char * bar[2];
} foo;

STORAGE1 char c = 'x';
STORAGE2 foo f;

void
testBug868103 (void)
{
  f.bar[1] = &c;
  ASSERT (f.bar[1] == &c);
}
