/** Tests declarations after statements within the same block (C99).
 */
#include <testfwk.h>

void
testDeclAfterStmt(void)
{
  int a = 0;
  a++;
  int b = 1;
  {
    int a = 0;
  }
  int c = a + b;
  ASSERT(c == 2);
}
