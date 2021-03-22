/*
   20000314-3.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

static char arg0[] = "arg0";
static char arg1[] = "arg1";

static void attr_rtx            (char *, char *);
static char *attr_string        (char *);
static void attr_eq             (char *, char *);

static void
attr_rtx (char *varg0, char *varg1)
{
  ASSERT (varg0 == arg0);
  ASSERT (varg1 == arg1);
  return;
}

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Bug #2878
static void
attr_eq (char *name, char *value)
{
  return attr_rtx (attr_string (name), attr_string (value));
}

static char *
attr_string (char *str)
{
  return str;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
  attr_eq (arg0, arg1);
  return;
#endif
}
