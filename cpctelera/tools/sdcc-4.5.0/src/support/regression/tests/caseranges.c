/** Test case for case range expressions
 */

#include <testfwk.h>

#ifdef __SDCC
_Pragma("save")
// case range expressions are a C2y feature
_Pragma("std_c2y")
// we know that some of the case ranges are empty
_Pragma("disable_warning 300")
#endif

// case range expressions were originally a GNU extension => test GNU-like host compilers, too
#if defined(__SDCC) || defined(__GNUC__)
void
testCaseRanges (void)
{
  for (int n = 0; n <= 11; n++)
  {
    // switch statement adapted from n3370
    switch (n)
    {
      case 1:
        ASSERT (n == 1);
        break;
      // case 4 : // error, overlaps 2 ... 5
      //   foo ();
      //   break;
      case 2 ... 5:
        ASSERT (n >= 2 && n <= 5);
        break;
      case 6 ... 6: // OK (but questionable)
        ASSERT (n == 6);
        break;
      case 8 ... 7: // not a GCC error
        ASSERT (0);
        break;
      case 10 ... 4: // not a GCC error despite overlap
        ASSERT (0);
        break;
      default:
        ASSERT (n < 1 || n > 6);
        break;
    }
  }
}
#endif

#ifdef __SDCC
_Pragma("restore")
#endif
