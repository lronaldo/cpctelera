/** tests for support for iso-8859-1. Extracted from string.c
*/
#include <testfwk.h>
#include <string.h>

/* SDCC has limited support for source encodings other than utf-8. This is a small test for what is there. */

/** tests for multibyte character sets
 * related to bug #3506236
*/
static void
do_multibyte (void)
{
  const char *str = "ÔÂ";

  ASSERT (str[0] == '\xd4');
  ASSERT (str[1] == '\xc2');
}

static void
teststr (void)
{
  do_multibyte ();
}

