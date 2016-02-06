/*
 * bug3188899.c
 */

#include <testfwk.h>

/* bug 3188899
   ?ASlink-Warning-Undefined Global '_s1' referenced by module
   _s1 should be _testBug3188899_s1_1_1 */
void * testBug3188899 (void)
{
  static char s1;
  static void * const __code s2 = &s1;
  return s2;
}
