/*
   bug-3240.c. A problem in the interaction between pointers to __code (implicit in this case) and plain pointers resulted in a false error.
 */

#include <testfwk.h>

#pragma disable_warning 283

const char *const data[] = {"a", "b", "c"};

char ptrs(const char *const * ptr);

void ptrs1()
{
   ptrs(data); // False error on about incompatible types this line
}

void testBug(void)
{
  ptrs1();
  ASSERT (ptrs(data) == 'b');
}

char ptrs(const char *const * ptr)
{
	return ptr[1][0];
}

