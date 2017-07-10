/* ctype.c
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <ctype.h>
#include <stdio.h>

#ifndef __STDC_VERSION__
  #warning __STDC_VERSION__ not defined
#endif

void testCtype (void)
{
  char c = 'a';
  char d = '0';

  ASSERT (isalnum (++c));
  ASSERT (isalpha (++c));
  ASSERT (!iscntrl (++c));
  ASSERT (!isdigit (++c));
  ASSERT (isgraph (++c));
  ASSERT (islower (++c));
  ASSERT (isprint (++c));
  ASSERT (!ispunct (++c));
  ASSERT (!isspace (++c));
  ASSERT (!isupper (++c));
  ASSERT (!isxdigit (++c));

  ASSERT (c == 'a' + 11);

  ASSERT (isalnum (++d));
  ASSERT (!isalpha (++d));
  ASSERT (!iscntrl (++d));
  ASSERT (isdigit (++d));
  ASSERT (isgraph (++d));
  ASSERT (!islower (++d));
  ASSERT (isprint (++d));
  ASSERT (!ispunct (++d));
  ASSERT (!isspace (++d));
  ASSERT (!isupper (++d));
  ASSERT (!isxdigit (++d));

  ASSERT (isspace (' '));
  ASSERT (isxdigit ('a'));
  ASSERT (isxdigit ('F'));
  ASSERT (!isxdigit ('Z'));

  c = 'A';
  d = '0';

  ASSERT (tolower (c) == 'a');
  ASSERT (tolower (d) == '0');
  ASSERT (toupper (++c) == 'A' + 1);

  ASSERT (tolower (EOF) == EOF);
  ASSERT (toupper (EOF) == EOF);
}

