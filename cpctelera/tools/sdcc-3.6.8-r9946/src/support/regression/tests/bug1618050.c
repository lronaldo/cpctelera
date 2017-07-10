/** bug 1618050
 *    global variable px in function buggy, is marked rematr..
 *    and is erronously assumed to retain its value "x" across
 *    the function call to fiddle_px
 */

#include <testfwk.h>

_STATMEM char * __data px;
_STATMEM char x[2] = {0,42};

void
fiddle_px (_STATMEM char * unused)
{
  (volatile char) unused[0]; // shut up
  px++;
}

char
buggy (void)
{
  px = x;
  fiddle_px (x);
  return *px;
}

void
testBug (void)
{
  ASSERT (buggy () == 42);
}
