/* bug 3040
   crash in pdk code generation when both operands of an additive operator are block-scope static const int.
   type: char, signed char, unsigned char, signed int, unsigned int, signed long, unsigned long, signed long long, unsigned long long
 */
#include <testfwk.h>

{type} f(void)
{
  static const {type} a;
  static const {type} b;
  return a + b;
}

{type} g(void)
{
  static const {type} a;
  static const {type} b;
  return a - b;
}

void testBug(void)
{
# if 0 // Bug #3073.
	ASSERT(!f());
#endif
}

