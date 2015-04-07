/*
   980707-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <stdlib.h>
#include <string.h>

char **
buildargv (char *input)
{
#if !defined(__SDCC_mcs51)
  static char *arglist[256];
#else
  static char *arglist[8];
#endif
  int numargs = 0;

  while (1)
    {
      while (*input == ' ')
	    input++;
      if (*input == 0)
	    break;
      arglist [numargs++] = input;
      while (*input != ' ' && *input != 0)
	    input++;
      if (*input == 0)
	    break;
      *(input++) = 0;
    }
  arglist [numargs] = NULL;
  return arglist;
}

void
testTortureExecute (void)
{
  char **args;
#if !defined(__SDCC_mcs51)
  char input[256];
#else
  char input[8];
#endif
  int i;

  strcpy(input, " a b");
  args = buildargv(input);

  if (strcmp (args[0], "a"))
    ASSERT (0);
  if (strcmp (args[1], "b"))
    ASSERT (0);
  if (args[2] != NULL)
    ASSERT (0);
  
  return;
}

