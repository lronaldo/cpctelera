/*
   const-addr-expr-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include        <stdio.h>
#include        <stdlib.h>

typedef struct foo
{
        int     uaattrid;
        const char    *name;
} FOO;

FOO     Upgrade_items[] =
{
        {1, "1"},
        {2, "2"},
        {0, NULL}
};

int     *Upgd_minor_ID = 
        (int *) &((Upgrade_items + 1)->uaattrid);

int     *Upgd_minor_ID1 = 
        (int *) &((Upgrade_items)->uaattrid);

void
testTortureExecute (void)
{
	ASSERT (*Upgd_minor_ID == 2);
	ASSERT (*Upgd_minor_ID1 == 1);
	return;
}
