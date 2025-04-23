#pragma std_c11

#include <assert.h>

/* C11 static_assert */
#ifdef TEST1
#pragma std_c11
static_assert (1, "text");
static_assert (0, "test");	/* WARNING */
static_assert (1);	/* ERROR */
#endif

/* C23 static_assert */
#ifdef TEST2
#pragma std_c23
static_assert (1);
static_assert (0);	/* WARNING */
#endif

