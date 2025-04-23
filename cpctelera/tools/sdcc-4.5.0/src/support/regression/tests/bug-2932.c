/*
   bug-932.c

   For a basic block that consists of only a call two al __smallc function with at least two parameters,
   the sch pointer of a basic block could be wrong, resulting in incorrect live-range for some corner cases,
   which in turn for some corner cases could result in live-range separation incorrectly assuming an
   iCode to be dead, which resulted in changes to the iCode that triggered a segfault in the z80 register allocator.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#endif

int fg(void *fp)
{
    volatile int i = 1;
    return i;
}

extern int fp(int c, void *fp) __smallc
{
    return -1;
}

void *infile, *outfile;
static unsigned long int textsize = 0;

int error;

static void Error(const char *message)
{
    error++;
}

#define THRESHOLD   2

static int DecodeChar(void)
{
    volatile int i = 255;
    return i;
}

static void Decode(void)
{
    int j, k, c;
    unsigned long int  count;

    textsize = fg(infile);

    for (count = 0; count < textsize; ) {
        c = DecodeChar();
        if (c < 256) {
            if (fp(c, outfile) == -1) {
                Error("");
            }
            count++;
        } else {
            j = c - 255 + THRESHOLD;
            for (k = 0; k < j; k++) {
                count++;
            }
        }
    }
}

void testBug(void)
{
    error = 0;
    Decode();
    ASSERT (error == 1);
}

