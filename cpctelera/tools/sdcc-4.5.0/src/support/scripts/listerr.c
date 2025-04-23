/*
 * listerr.c - program to create the list of errors and warnings list from SDCCerr.c
 *
 * gcc -I ../../src listerr.c -o listerr
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* although this seems to be strange, this is the easiest way how to import the ErrTab without having to modify SDCCerr.c/h */
#include "SDCCerr.c"

// this is to make SDCCerr happy - simulate global SDCC variables
char *filename ;
int lineno ;
int fatalError ;


/* predefined names for errorlevels */
char *ErrTypeName[] = {
  "ALL   ",
  /** All warnings, including those considered 'reasonable to use,
    on occasion, in clean programs' (man 3 gcc). */
  "PEDANTIC",
  /** 'informational' warnings */
  "INFO  ",
  /** Most warnings. */
  "WARNING ",
  /** Errors only. */
  "ERROR   "
  };


/* some simple internal variables */
int i;
char s[256];
char *p;

int main(int argc, char *argv[])
{
  printf("Number  Type            Text\n"); /* output file header */
  printf("------------------------------------------------------------------------------\n");
  for (i = 0; i < MAX_ERROR_WARNING; i++)
    {
      if (ErrTab[i].errIndex == i)
        {
          strcpy(s, ErrTab[i].errText);
          for (p = s; NULL != (p = strchr(s, '\n')); )
            *p = ' '; /* replace all newlines by spaces */
          printf("%3d     %-16.16s%s\n", ErrTab[i].errIndex, ErrTypeName[ErrTab[i].errType], s);
        }
    }

  return 0;
}

