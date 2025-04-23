/** bug-3779.c: code generation for sm83 could store a temporary in l, then set up hl for stack access, overwriting the temporary.
*/

#include <testfwk.h>

struct State {
    unsigned short a;
    unsigned char b;
};
static struct State state;

static char glob_var;

#ifdef __SDCC_sm83
static __sfr __at(0xff00) reg; // Use LCD control register for this test, since all its bits are read/write.
#else
unsigned char reg;
#endif

void m(void)
{
    state.b = 0;

    if (glob_var) {
        if (glob_var) {
            state.a += 1;
        } 
        if (glob_var) {
            state.b = 0;
        }

        unsigned char foo = state.a >> 1;
        foo -= reg;
        reg = foo;
    }
}

void testBug(void)
{
    reg = 0x00;
    glob_var = 1;
    state.a = 1;
    m();
    ASSERT(reg == 1);
}

