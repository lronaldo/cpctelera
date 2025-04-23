/* Test the types of enum.
 */
#include <testfwk.h>

enum _SimpleEnum {
  SIMPLE_ZERO,
  SIMPLE_ONE,
  SIMPLE_TWO
};

enum _ValuesEnum {
  VALUES_ZERO,
  VALUES_FIVE = 5,
  VALUES_SIX,
  VALUES_SEVEN,
  VALUES_TWELVE = 12
};

enum _IndexedEnum {
  INDEXED_ZERO,
  INDEXED_ONE,
  INDEXED_ONE_ALIAS = INDEXED_ONE,
  INDEXED_TWO,
  INDEXED_THREE,
  INDEXED_ZERO_ALIAS = INDEXED_ZERO + 0,
  INDEXED_ONE_ALIAS2 = INDEXED_ZERO + 1,
  INDEXED_TWO_ALIAS = INDEXED_ZERO + 2,
  INDEXED_TWO_ALIAS2 = INDEXED_ONE_ALIAS2 + 1,
  INDEXED_MATH = INDEXED_ONE_ALIAS + INDEXED_TWO * INDEXED_THREE
};

enum _SignedEnum {
  SIGNED_POS = 1,
  SIGNED_ZERO = 0,
  SIGNED_NEG = -1
} se;

enum comma {
  COMMA_ZERO,
  COMMA_ONE,
  COMMA_TWO, // Trailing comma after enumerator list is a C99 feature.
}; 

volatile enum _IntEnum {
  INT_256 = 256,
  INT_ZERO = 0
} ie;

void
testSimpleEnum(void)
{
  ASSERT(SIMPLE_ZERO == 0);
  ASSERT(SIMPLE_ONE == 1);
  ASSERT(SIMPLE_TWO == 2);
}

void
testValuesEnum(void)
{
  ASSERT(VALUES_ZERO == 0);
  ASSERT(VALUES_FIVE == 5);
  ASSERT(VALUES_SIX == 6);
  ASSERT(VALUES_SEVEN == 7);
  ASSERT(VALUES_TWELVE == 12);
}

void
testIndexedEnum(void)
{
  ASSERT(INDEXED_ZERO == 0);
  ASSERT(INDEXED_ZERO_ALIAS == 0);
  ASSERT(INDEXED_ONE == 1);
  ASSERT(INDEXED_ONE_ALIAS == 1);
  ASSERT(INDEXED_ONE_ALIAS2 == 1);
  ASSERT(INDEXED_TWO == 2);
  ASSERT(INDEXED_TWO_ALIAS == 2);
  ASSERT(INDEXED_TWO_ALIAS2 == 2);
  ASSERT(INDEXED_MATH == 7);
}

void
testSignedEnum(void)
{
  ASSERT(SIGNED_POS == 1);
  ASSERT(SIGNED_ZERO == 0);
  ASSERT(SIGNED_NEG == -1);

  se = SIGNED_NEG;
  ASSERT(se == SIGNED_NEG);
}

void
testIntEnum(void)
{
  ASSERT(INT_ZERO == 0);
  ASSERT(INT_256 == 256);

  ie = INT_ZERO;
  ASSERT(ie == INT_ZERO);
  ie = INT_256;
  ASSERT(ie == INT_256);
}
