/* bool.c
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdbool.h>
#include <stdint.h>

  bool ret_true(void)
  {
    return(true);
  }

  bool ret_false(void)
  {
    return(false);
  }

  volatile bool E;

  bool (* const pa[])(void) = {&ret_true, &ret_false};

  struct s
  {
    bool b;
  };

  struct
  {
    bool b : 1;
  } s2;

void
testBug2233(void)
{
#ifndef __SDCC_pic16
	bool result;
	volatile char test = 0;

	result = ret_true();

	if (result == 1)
		test = 1;
	ASSERT(test);
#endif
}

void
testBool(void)
{
#ifndef __SDCC_pic16
	volatile unsigned char z = 2;

	const char *BOOL = "George Boole";

	ASSERT(true);
	ASSERT((*(pa[0]))() == true);
	ASSERT((*(pa[1]))() == false);

	s2.b = (z & 2);
	ASSERT(s2.b);
	s2.b = (bool)(z & 2);
	ASSERT(s2.b);

	E = true;
	ASSERT((E ? 1 : 0) == (!(!E)));
#ifndef __SDCC_ds390
	ASSERT((E += 2) == 1);
#endif
	ASSERT((--E, --E, E) == E);

	E = false;
#ifndef __SDCC_ds390
	ASSERT((E ? 1 : 0) == (!(!E)));
	ASSERT((E += 2) == 1);
	ASSERT((--E, --E, E) == E);
#endif

	E = 0;   ASSERT(!E); // sets E to 0
	E = 1;   ASSERT(E);  // sets E to 1
	E = 4;   ASSERT(E);  // sets E to 1
	E = 0.5; ASSERT(E);  // sets E to 1
	E = false;
	E++;     ASSERT(E);  // sets E to 1
	E = true;
	E++;     ASSERT(E);  // sets E to 1
	E = false;
	E--;     ASSERT(E);  // sets E to 1-E
	E = true;
	E--;     ASSERT(!E); // sets E to 1-E
#endif
}
