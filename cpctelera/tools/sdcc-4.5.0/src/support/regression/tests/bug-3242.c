/*
   bug-3240.c. A bug in z80 code generation regarding handling of register pair hl.
 */

#include <testfwk.h>

typedef unsigned short uint16;

uint16 x, y;

char flag;

static uint16 printHello() {
    flag = 1;
    return 1;
}

static uint16 hello(uint16 a, uint16 b) {
    uint16 result = 0;
    if(a + b > x - y){
        result = printHello();
    }
    return result;
}

void
testBug(void) {
    x = 2000;
    y = 1000;
    hello(600,600);
    ASSERT (flag);
}

