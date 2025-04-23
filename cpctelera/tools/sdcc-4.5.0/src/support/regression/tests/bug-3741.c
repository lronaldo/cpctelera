/** bug-3741.c: A segfault at compile time that only happens on some systems?
*/

#include <testfwk.h>

#define PRN_TILE_WIDTH          20

#if defined(__SDCC_z80) || defined(SDCC_sm83)
#define PR __preserves_regs(b, c, h, l)
#else
#define PR
#define __banked
#endif

void mymemset(void *s, unsigned char c, unsigned int n);
unsigned char * get_xy(unsigned char x, unsigned char y);
unsigned char get_byte(unsigned char * addr) PR;
void vmemcpy(unsigned char *dest, unsigned char *sour, unsigned int len);

unsigned char * _VRAM8000 = (__xdata unsigned char *)0x8000;
unsigned char * _VRAM9000 = (__xdata unsigned char *)0x9000;

unsigned char some_func(unsigned char sx, unsigned char sy, unsigned char sw, unsigned char sh, unsigned char centered) __banked {

    unsigned char tile_data[16], rows = ((sh & 0x01) ? (sh + 1) : sh), x_ofs = (centered) ? ((PRN_TILE_WIDTH - sw) >> 1) : 0;

    for (unsigned char y = 0; y != rows; y++) {
        unsigned char * map_addr = get_xy(sx, y + sy);
        for (unsigned char x = 0; x != PRN_TILE_WIDTH; x++) {
            if ((x >= x_ofs) && (x < (x_ofs + sw)) && (y < sh))  {
                unsigned char tile = get_byte(map_addr++);
                unsigned char * source = (((y + sy) > 11) || (tile > 127)) ? _VRAM8000 : _VRAM9000;
                vmemcpy(tile_data, source + ((unsigned int)tile << 4), sizeof(tile_data));
            }
            else mymemset(tile_data, 0x00, sizeof(tile_data));
        }
    }
    return 0x01;
}

void
testBug (void)
{
}

void mymemset(void *s, unsigned char c, unsigned int n)
{
    (void)s;
    (void)c;
    (void)n;
}

unsigned char * get_xy(unsigned char x, unsigned char y)
{
    (void)x;
    (void)y;
    return(0);
}

unsigned char get_byte(unsigned char * addr) PR
{
    (void)addr;
    return(0);
}

void vmemcpy(unsigned char *dest, unsigned char *sour, unsigned int len)
{
    (void)dest;
    (void)sour;
    (void)len;
}

