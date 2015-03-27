/* bug-983491.c
 */

#include <testfwk.h>

/*
 * test disabled since the fix was reverted 
 */

/*
code struct {
  char* b;
} c[2] = {
  {"abc"},
  {"abc"}
};
*/

void
testMergeStr(void)
{
/*  ASSERT(c[0].b == c[1].b); */
}
