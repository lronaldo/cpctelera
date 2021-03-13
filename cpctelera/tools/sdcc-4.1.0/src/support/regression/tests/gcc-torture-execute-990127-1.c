/*
   990127-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

void
testTortureExecute (void)
{
    int a,b,c;
    int *pa, *pb, *pc;
    int **ppa, **ppb, **ppc;
    int i,j,k,x,y,z;

    a = 10;
    b = 20;
    c = 30;
    pa = &a; pb = &b; pc = &c;
    ppa = &pa; ppb = &pb; ppc = &pc;
    x = 0; y = 0; z = 0;

    for(i=0;i<10;i++){
        if( pa == &a ) pa = &b;
        else pa = &a;
        while( (*pa)-- ){
            x++;
            if( (*pa) < 3 ) break;
            else pa = &b;
        }
        x++;
        pa = &b;
    }

    if ((*pa) != -5 || (*pb) != -5 || x != 43)
      ASSERT (0);

    return;
}

