/** Tests sdcc internal macros.
 */
#include <testfwk.h>

/* test for bug #2805536: Assembly code comment cause Internal Error
   https://sourceforge.net/tracker/?func=detail&atid=100599&aid=2805536&group_id=599 */
static void
bug_2805536(void)
{
#ifndef PORT_HOST
__asm
; != <-- this should not cause a compiler internal error
__endasm;
#endif
  ASSERT(1);
}

static void
test_macro(void)
{
  bug_2805536();
}
