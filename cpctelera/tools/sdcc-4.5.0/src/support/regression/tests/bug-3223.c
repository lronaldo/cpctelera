/*
   bug-3223.c
   A bug in tracking of register pair hl over ldir
 */

#include <testfwk.h>

typedef unsigned long   uint32;
typedef unsigned short  addr16;
typedef unsigned short  uint16;
typedef unsigned char   uint8;

uint32* memory = 0;

void memWrite(uint32 sectorNumber){
    uint16 value = 200;
    uint32* byte51202 = memory + 02;
    *byte51202 = sectorNumber;
    memory[3]=100;
    memory[0]=value;
}

void
testBug (void)
{
    uint32 buffer[4];
    memory = buffer;
    memWrite(42);
    ASSERT(buffer[2] == 42);
    ASSERT(buffer[3] == 100);
    ASSERT(buffer[0] == 200);
}

