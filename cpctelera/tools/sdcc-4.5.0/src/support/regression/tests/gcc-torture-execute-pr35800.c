/*
   pr35800.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 84
#endif

int stab_xcoff_builtin_type (int typenum)
{
  const char *name;
  if (typenum >= 0 || typenum < -34)
    {
      return 0;
    }
  switch (-typenum)
    {
    case 1:
      name = "int";
      break;
    case 2:
      name = "char";
    case 3:
      name = "short";
      break;
    case 4:
      name = "long";
    case 5:
      name = "unsigned char";
    case 6:
      name = "signed char";
    case 7:
      name = "unsigned short";
    case 8:
      name = "unsigned int";
    case 9:
      name = "unsigned";
    case 10:
      name = "unsigned long";
    case 11:
      name = "void";
    case 12:
      name = "float";
    case 13:
      name = "double";
    case 14:
      name = "long double";
    case 15:
      name = "integer";
    case 16:
      name = "boolean";
    case 17:
      name = "short real";
    case 18:
      name = "real";
    case 19:
      name = "stringptr";
    case 20:
      name = "character";
    case 21:
      name = "logical*1";
    case 22:
      name = "logical*2";
    case 23:
      name = "logical*4";
    case 24:
      name = "logical";
    case 25:
      name = "complex";
    case 26:
      name = "double complex";
    case 27:
      name = "integer*1";
    case 28:
      name = "integer*2";
    case 29:
      name = "integer*4";
    case 30:
      name = "wchar";
    case 31:
      name = "long long";
    case 32:
      name = "unsigned long long";
    case 33:
      name = "logical*8";
    case 34:
      name = "integer*8";
    }
  return name[0];
}

void
testTortureExecute (void)
{
  int i;
  if (stab_xcoff_builtin_type(0) != 0)
    ASSERT (0);
  if (stab_xcoff_builtin_type(-1) != 'i')
    ASSERT (0);
  if (stab_xcoff_builtin_type(-2) != 's')
    ASSERT (0);
  if (stab_xcoff_builtin_type(-3) != 's')
    ASSERT (0);
  for (i = -4; i >= -34; --i)
    if (stab_xcoff_builtin_type(i) != 'i')
      ASSERT (0);
  if (stab_xcoff_builtin_type(-35) != 0)
    ASSERT (0);
  return;
}
