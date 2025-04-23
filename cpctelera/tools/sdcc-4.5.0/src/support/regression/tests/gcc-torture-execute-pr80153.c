/*
   pr80153.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <string.h>

/* PR tree-optimization/80153 */

void check (int, int, int);
void check (int c, int c2, int val)
{
  (void)c;
  (void)c2;
  ASSERT (val);
}

static const char *buf;
static int l, i;

void _fputs(const char *str);
void _fputs(const char *str)
{
  buf = str;
  i = 0;
  l = strlen(buf);
}

char _fgetc();
char _fgetc()
{
  char val = buf[i];
  i++;
  if (i > l)
    return -1;
  else
    return val;
}

static const char *string = "oops!\n";

void
testTortureExecute (void)
{
  int i;
  int c;

  _fputs(string);

  for (i = 0; i < strlen(string); i++) {
    c = _fgetc();
    check(c, string[i], c == string[i]);
  }

  return;
}
