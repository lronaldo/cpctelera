/*
p18298.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* { dg-options "-fgnu89-inline" } */

#include <stdbool.h>
#include <stdlib.h>

#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_pic14) // Lack of memory
extern bool foo(const char *str); // extern declaration required by C standard, not present in GCC original, added to pass hosts regression tests everywhere.

int strcmp (const char*, const char*);
char s[2048] = "a";
inline bool foo(const char *str) {
  return !strcmp(s,str);
}
#endif
void
testTortureExecute (void) {
#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_pic14) // Lack of memory
int i = 0;
  while(!(foo(""))) {
    i ++;
    s[0] = '\0';
    if (i>2)
     ASSERT (0);
  }
#endif
  return;
}

