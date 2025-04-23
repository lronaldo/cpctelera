/* bug-3626.c
   Compatibility check of array declarations incorrectly depended on order of declarations.
*/

#include <testfwk.h>

const int foo[] = {42,43,44,45,46,47}; // Initialized to 6 elements, completing the array type.
extern const int foo[]; // Redeclaration uses incomplete type.

void
testBug (void)
{
  return;
}

