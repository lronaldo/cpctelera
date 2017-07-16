/* Test ast_constant_folding() in SDCCast.c
 */
#include <testfwk.h>

volatile unsigned i;

unsigned
foo_aa (void)
{
  return i + 5 + i - i + 12;
}

unsigned
foo_asr (void)
{
  return i - 5 + i + i + 12;
}

unsigned
foo_asl (void)
{
  return 5 - i + i + 12;
}

unsigned
foo_ssr (void)
{
  return i - 5 - i - i - 12;
}

unsigned
foo_ssl (void)
{
  return 5 - i - i - 12;
}
unsigned
foo_sa (void)
{
  return i + 5 - i - i - 12;
}

unsigned
foo_mul (void)
{
  return 5 * i * i * 12;
}

unsigned
foo_div (void)
{
  return 33971u / i / 5 / i / i / 12;
}

unsigned
foo_or (void)
{
  return 5 | i | i | 12;
}

unsigned
foo_and (void)
{
  return 5 & i & i & 12;
}

void
test_ast_cf(void)
{
  i = 30; ASSERT(foo_aa () ==    47);
          ASSERT(foo_asr() ==    97);
          ASSERT(foo_asl() ==    17);
          ASSERT(foo_ssr() ==   -47);
          ASSERT(foo_ssl() ==   -67);
          ASSERT(foo_sa () ==   -37);
          ASSERT(foo_mul() == 54000);
  i =  3; ASSERT(foo_div() ==    20);
  i =  3; ASSERT(foo_or () ==    15);
  i =  7; ASSERT(foo_and() ==     4);
}
