/* bug-2866.c
   A bug in rematerialization in the z80 backend.
 */

#include <testfwk.h>

typedef unsigned char uint8;
typedef int int16;

#define COL_MAX_HEIGHT 64
#define COL_TEX_HEIGHT 32

#ifndef __SDCC_pdk14 // Lack of memory
uint8 single_column[COL_MAX_HEIGHT];

void initSingleColumn(int16 height)
{
    int16 y;
    int16 y0;
    int16 y1;
    int16 dy;
    int16 dv;
    int16 v;

    y0 = (COL_MAX_HEIGHT - height) >> 1;
    y1 = COL_MAX_HEIGHT - y0;

    dy = y1 - y0 - 1;
    if (dy < 1) dy = 1;
    dv = (COL_TEX_HEIGHT << 8) / dy;
    v = 0;

    if (y0 < 0)
    {
        v -= y0 * dv;
        y0 = 0;
    }
    if (y1 > COL_MAX_HEIGHT) y1 = COL_MAX_HEIGHT;

    for (y = 0; y<y0; y++)
    {
        single_column[y] = 128;
        single_column[COL_MAX_HEIGHT - y - 1] = 64; // Bug resulted in wrong address calculation here.
    }

    for (y = y0; y < y1; y++)
    {
        if (v > (31 << 8)) v = 31 << 8;
        single_column[y] = (v >> 8);
        v+=dv;
    }
}
#endif

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
    initSingleColumn (0);

    ASSERT (single_column[0x00] == 0x80);
    ASSERT (single_column[0x1f] == 0x80);
    ASSERT (single_column[0x20] == 0x40);
    ASSERT (single_column[0x3f] == 0x40);
#endif
}

