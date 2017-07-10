/*  bug 2188
    Function miscompiled when inline selected
    temporal variable for function return value
    is forced to int type even that type is already
    resolved because variable name is hidden in
    the internal scope of inline function.
*/

#include <testfwk.h>
#include <stdint.h>

volatile int32_t glob_var32_a = 0x12345678;
volatile int32_t glob_var32_b = 0x0abcdef0;

inline
int32_t inline_fnc(char sel)
{
  return sel? glob_var32_b: glob_var32_a;
}

int32_t notinlined_fnc(int32_t val)
{
  return val;
}

inline
int32_t *inline_ptr_fnc(char sel)
{
  return sel? &glob_var32_b: &glob_var32_a;
}

int32_t notinlined_ptr_fnc(int32_t *pval)
{
  return *pval;
}

void
testBug (void)
{
  int32_t var32 = notinlined_fnc(inline_fnc(0));
  ASSERT(var32 == glob_var32_a);
  var32 = notinlined_fnc(inline_fnc(1));
  ASSERT(var32 == glob_var32_b);

  var32 = notinlined_ptr_fnc(inline_ptr_fnc(0));
  ASSERT(var32 == glob_var32_a);
  var32 = notinlined_ptr_fnc(inline_ptr_fnc(1));
  ASSERT(var32 == glob_var32_b);
}

extern inline
int32_t inline_fnc(char sel);

extern inline
int32_t *inline_ptr_fnc(char sel);

