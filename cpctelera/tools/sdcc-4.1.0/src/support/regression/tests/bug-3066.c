/* bug-3066.c
   Pointer in register overwritten in the z80 backend when using --reserve-regs-iy.
 */
 
#include <testfwk.h>

#include <stdint.h>

typedef uint16_t atom;

struct node {
    atom lhs;
    atom rhs;
};

#define MAX_NODES 10

static struct node nodes[MAX_NODES];
uint16_t node_freelist = 0;

atom alloc_node(atom lhsval, atom rhsval)
{
    atom a = node_freelist;
    node_freelist = nodes[a].lhs;
    nodes[a].lhs = lhsval;
    nodes[a].rhs = rhsval;
    return a;
}

void testBug(void)
{
    nodes[0].lhs = 23;

    atom a = alloc_node(0xa5a5, 0x5a5a);

    ASSERT (nodes[0].lhs == 0xa5a5);
    ASSERT (nodes[0].rhs == 0x5a5a);
    ASSERT (node_freelist == 23);
}

