/*
   bug2889032.c
 */

#include <testfwk.h>

struct timetable_timestamp {
  const char *id;
};

struct timetable {
  struct timetable_timestamp *timestamps;
  unsigned int * const ptr;
};

// no need to call this, it generates compiler error:
//   Assertion failed: 0, file sdcc/src/SDCCopt.c, line 707
int timetable_timediff(struct timetable *t, const char *id1)
{
  int i;

  for(i = *t->ptr - 1; i >= 0; --i)
  {
    if(t->timestamps[i].id == id1)
    {
      return i;
    }
  }

  return 0;
}

void
testBug (void)
{
	ASSERT (1);
}
