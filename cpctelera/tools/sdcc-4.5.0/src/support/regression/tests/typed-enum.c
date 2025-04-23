/* Test enums with explicit underlying type
 */
#include <testfwk.h>

#ifdef __SDCC
_Pragma("std_c23")

enum typed_enum_bool : bool {
  TYPED_ENUM_BOOL_VAL1
};

enum typed_enum_char : char {
  TYPED_ENUM_CHAR_VAL1
};

enum typed_enum_int : int {
  TYPED_ENUM_INT_VAL1
};

enum typed_enum_long : long {
  TYPED_ENUM_LONG_VAL1
};

enum typed_enum_longlong : long long {
  TYPED_ENUM_LONGLONG_VAL1
};
#endif

void
testTypedEnum(void)
{
#ifdef __SDCC
  ASSERT(sizeof(enum typed_enum_bool) == sizeof(bool));
  ASSERT(sizeof(enum typed_enum_char) == sizeof(char));
  ASSERT(sizeof(enum typed_enum_int) == sizeof(int));
  ASSERT(sizeof(enum typed_enum_long) == sizeof(long));
  ASSERT(sizeof(enum typed_enum_longlong) == sizeof(long long));
#endif
}
