/*
    bug 2435941
*/

#include <testfwk.h>
#include <stdint.h>

uint32_t sleep_timer_get(void)
{
  return 0x12345678;
}

int32_t remaining;
uint32_t updated;
uint32_t now;

// no need to call this, it generates compiler error:
//   error 9: FATAL Compiler Internal Error
static void
do_test (void) __reentrant
{
  while (1)
    {
      now = sleep_timer_get();

      remaining -= (now - updated) & 0xFF;
      updated = now;
    }
}

void
testBug (void)
{
  ASSERT(1);
}
