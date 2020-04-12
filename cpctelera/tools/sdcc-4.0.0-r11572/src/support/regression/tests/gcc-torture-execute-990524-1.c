/*
   990524-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

char a[] = "12345";
char b[] = "12345";

void loop (char * pz, char * pzDta)
{
    for (;;) {
        switch (*(pz++) = *(pzDta++)) {
        case 0:
  	    goto loopDone2;

	case '"':
	case '\\':
	    pz[-1]  = '\\';
            *(pz++) = pzDta[-1];
	}
    } loopDone2:;

  if (a - pz != b - pzDta)
    ASSERT (0);
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pic16
  loop (a, b);
  return;
#endif
}

