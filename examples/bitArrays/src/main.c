#include <stdio.h>
#include "bitarray.h"
#include "utils.c"

#define N 2
unsigned char array[N];

void printArray()
{
    int i;
    for (i = 0; i < N*8; ++i)
        printf("%d", getBit(array, i));

    printf("\n");
    resetCursorX();
}

void printArrayCPCT()
{
    int i;
    for (i = 0; i < N*8; ++i)
        printf("%d", cpct_getBit(array, i));

    printf("\n");
    resetCursorX();
}

void printArray2()
{
    int i;
    for (i = 0; i < N*4; ++i)
        printf("%d", getBit2(array, i));

    printf("\n");
    resetCursorX(); 
}

void printArrayCPCT2()
{
    int i;
    for (i = 0; i < N*4; ++i)
        printf("%d", cpct_get2Bits(array, i));

    printf("\n");
    resetCursorX();
}

void printArray4()
{
    int i;
    for (i = 0; i < N*2; ++i)
        printf("%d", getBit4(array, i));

    printf("\n");
    resetCursorX();   
}

void printArrayCPCT4()
{
    int i;
    for (i = 0; i < N*2; ++i)
        printf("%d", cpct_get4Bits(array, i));

    printf("\n");
    resetCursorX();
}

main ()
{
    int i;
    for (i = 0; i < N; ++i)
        array[i] = 0;

    printf("Testing getBit and setBit\n");
    resetCursorX();

    for (i = 0; i < 16; ++i)
    {
        setBit(array, i, 1);
        printArray();
        setBit(array, i, 0);
    }

    printf("Testing cpct_getBit and setBit\n");
    resetCursorX();

    for (i = 0; i < 16; ++i)
    {
        setBit(array, i, 1);
        printArrayCPCT();
        setBit(array, i, 0);
    }


    printf("Testing getBit2 and setBit\n");
    resetCursorX();

    for (i = 0; i < 16; ++i)
    {
        setBit(array, i, 1);
        printArray2();
        setBit(array, i, 0);
    }

    printf("Testing cpct_getBit2 and setBit\n");
    resetCursorX();

    for (i = 0; i < 16; ++i)
    {
        setBit(array, i, 1);
        printArrayCPCT2();
        setBit(array, i, 0);
    }

    printf("Testing getBit4 and setBit\n");
    resetCursorX();

    for (i = 0; i < 16; ++i)
    {
        setBit(array, i, 1);
        printArray4();
        setBit(array, i, 0);
    }

    printf("Testing cpct_getBit4 and setBit\n");
    resetCursorX();

    for (i = 0; i < 16; ++i)
    {
        setBit(array, i, 1);
        printArrayCPCT4();
        setBit(array, i, 0);
    }

    printf("Testing getBit and cpct_setBit\n");
    resetCursorX();

    for (i = 0; i < 16; ++i)
    {
        cpct_setBit(array, i, 1);
        printArray();
        cpct_setBit(array, i, 0);
    }

    printf("Testing getBit and setBit2\n");
    resetCursorX();

    for (i = 0; i < 8; ++i)
    {
        setBit2(array, i, 3);
        printArray();
        setBit2(array, i, 0);
    }

    printf("Testing getBit and cpct_setBit2\n");
    resetCursorX();

    for (i = 0; i < 8; ++i)
    {
        cpct_set2Bits(array, i, 3);
        printArray();
        cpct_set2Bits(array, i, 0);
    }

    printf("Testing getBit and setBit4\n");
    resetCursorX();

    for (i = 0; i < 4; ++i)
    {
        setBit4(array, i, 15);
        printArray();
        setBit4(array, i, 0);
    }

    printf("Testing getBit and cpct_setBit4\n");
    resetCursorX();

    for (i = 0; i < 4; ++i)
    {
        cpct_set4Bits(array, i, 15);
        printArray();
        cpct_set4Bits(array, i, 0);
    }

    printf("----END\n");
    resetCursorX();

    while(1) { };
}