/* bug-2984.c

   Missing diagnostic and crash on incomplete type in offsetof
 */

#ifdef TEST1
#include <stddef.h>

volatile unsigned char test1=0;
volatile unsigned char test2=0;

int f(void) {

  test1=offsetof(struct test_t,v1); /* ERROR */
  test2=offsetof(int,v1);           /* ERROR */

  return 0;
}
#endif

