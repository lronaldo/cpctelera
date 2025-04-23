/*
  bug-3607.c
*/

#include <testfwk.h>

#include <stdint.h>
#include <stdbool.h>

//#ifdef __SDCC_mcs51

__xdata uint16_t g_arg_1 = 0;
#ifdef __SDCC_mcs51
__bit g_arg_2 = 0;
#else
_Bool g_arg_2 = 0;
#endif

#define sel_const1 (uint16_t)((g_arg_2 ? 0x1F09FFFFu : 0x101FFFFEu) >> 16)
#define sel_const2 (uint16_t)((g_arg_2 ? 0x101FFFF0u : 0x101FFFF1u) >> 16)

bool func_3 (void)
{
  return true;
}

__xdata uint8_t func_1_count = 0;
__xdata uint8_t func_2_count = 0;

void func_1 (void)
{
  ++func_1_count;
}

void func_2 (void)
{
  ++func_2_count;
}

void func (uint32_t test_arg)
{
  uint16_t t = test_arg >> 16;

  if (func_3 ())
  {
    // the bug was the constant being loaded wrongly into the registers
    // before the comparision, where one of the bytes got overwritten with
    // another.
    // this gets tested twice with different inputs so we get two mutually
    // exclusive false positives if there is a bug, or two good hits if
    // there is no bug.

    // when g_arg_2 = 0:
    // (t - g_arg_1 = 1020) > 101F   T
    // (t - g_arg_1 = 1020) > 1010   T (false positive)
    // (t - g_arg_1 = 1020) > 1F1F   F
    // (t - g_arg_1 = 1020) > 1F10   F

    // when g_arg_2 = 1:
    // (t - g_arg_1 = 1F10) > 1F09   T
    // (t - g_arg_1 = 1F10) > 1F1F   F
    // (t - g_arg_1 = 1F10) > 0909   T (false positive)
    // (t - g_arg_1 = 1F10) > 091F   F

    if (t - g_arg_1 > sel_const1)
      func_1 ();
  }
  else
  {
    if (t - g_arg_1 > sel_const2)
      func_2 ();
  }
}
//#endif

void testBug (void)
{
//#ifdef __SDCC_mcs51

  g_arg_1 = 0;
  g_arg_2 = 0;
  func (0x10200000ul);

  g_arg_1 = 0;
  g_arg_2 = 1;
  func (0x1F100000ul);

  ASSERT (func_1_count == 2);
  ASSERT (func_2_count == 0);

//#endif
}

