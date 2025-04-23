/* bug-3547.c
   Codegen for sm83 could generate an "ex de, hl" (a Z80 instruction not supported on SM83) when passing a struct as function argument while register hl holds another variable.
   This would then result in an error message from the assembler.
 */

#include <testfwk.h>

#pragma disable_warning 85

#if !defined(__SDCC_pdk14) && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) // Lack of memory

#define LINE_SIZE 32
#define POINT_AMOUNT 14
#define SIDE_AMOUNT 22

typedef struct
{
    int  x;
    int  y;
    int  z;
} Point3D;

void render(Point3D p1, Point3D p2)
{
}

void f(void)
{
    unsigned int  angles[3] = {47, 10, 0};
    const unsigned char sides[][2] = {
        {7, 8},
        {8, 9},
        {8, 10},
        {10, 11},
        {11, 9},
        {9, 6},
        {6, 0},
        {0, 1},
        {1, 7},
        {7, 4},
        {6, 5},
        {0, 3},
        {1, 2},
        {4, 5},
        {5, 12},
        {12, 4},
        {13, 2},
        {13, 3},
        {13, 12},
        {5, 3},
        {3, 2},
        {2, 4}};

    Point3D points[POINT_AMOUNT] = {
        {LINE_SIZE, LINE_SIZE, 0}, //0
        {0, LINE_SIZE, 0}, //1
        {0, LINE_SIZE, LINE_SIZE}, //2
        {LINE_SIZE, LINE_SIZE, LINE_SIZE}, //3
        {0, 0, LINE_SIZE}, //4
        {LINE_SIZE, 0, LINE_SIZE}, //5
        {LINE_SIZE, 0, 0}, //6
        {0, 0, 0}, //7
        {LINE_SIZE / 4, 0, 0}, //8
        {(LINE_SIZE / 4) + (LINE_SIZE /4), 0, 0}, //9
        {LINE_SIZE / 4, 0, LINE_SIZE/2},//10
        {(LINE_SIZE / 4) + (LINE_SIZE /4), 0, LINE_SIZE/2},//11
        {(LINE_SIZE / 2), 0, LINE_SIZE + (LINE_SIZE/2)}, //12
        {(LINE_SIZE / 2), LINE_SIZE, LINE_SIZE + (LINE_SIZE/2)}}; //13

	while (1)
	{
    	for (unsigned char i = 0; i < SIDE_AMOUNT; i++)
	    {
	        render(points[sides[i][0]], points[sides[i][1]]); // Invalid assembler instruction generated here.
	    }
	}
}
#endif

void
testBug (void)
{
}

