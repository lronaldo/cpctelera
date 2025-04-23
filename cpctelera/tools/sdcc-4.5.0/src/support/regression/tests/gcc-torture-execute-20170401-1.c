/*
   20170401-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR45070 */

struct packed_ushort {
    unsigned short ucs;
};

struct source {
    int pos, length;
};

static int flag;

static void fetch(struct source *p)
{
    p->length = 128;
}
#if 0 // Enable when sdcc accepts struct initialization as below "struct packed_ushort rv = next(&s);"
static struct packed_ushort next(struct source *p)
{
    struct packed_ushort rv;

    if (p->pos >= p->length) {
	if (flag) {
	    flag = 0;
	    fetch(p);
	    return next(p);
	}
	flag = 1;
	rv.ucs = 0xffff;
	return rv;
    }
    rv.ucs = 0;
    return rv;
}
#endif
void
testTortureExecute (void)
{
#if 0
    struct source s;
    int i;

    s.pos = 0;
    s.length = 0;
    flag = 0;

    for (i = 0; i < 16; i++) {
	struct packed_ushort rv = next(&s);
	if ((i == 0 && rv.ucs != 0xffff)
	    || (i > 0 && rv.ucs != 0))
	    ASSERT(0);
    }
#endif
}
