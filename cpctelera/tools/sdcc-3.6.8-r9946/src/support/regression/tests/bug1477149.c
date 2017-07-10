/*  bug1477149.c
    multiple definition of local symbol both normal and debug
    second module is fwk/lib/statics.c
    type: long, float
*/

#include <testfwk.h>

static {type} {type}_1 = 2;

static {type} s_get_{type}_1(void)
{
	{type} alfa = {type}_1;
	{type} beta = {type}_1 + alfa;
	{type} gamma = {type}_1 + beta;
	return alfa + beta + gamma;
}

/* bug 3038028 */
static char s_get_indexed(char index, char *msg)
{
	/* "float" will put _s_get_indexed_PARM_2 in DSEG,
	 * "long" will put _s_get_indexed_PARM_2 in OSEG
	 */
	{type} idx = index;
	return msg[(char)(idx+1)];
}

{type} get_{type}_1(void);
char get_indexed(char index, char *msg);

void testBug(void)
{
	ASSERT (s_get_{type}_1() == 12);
	ASSERT (get_{type}_1() == 6);
	ASSERT (s_get_indexed(1, "One") == 'e');
	ASSERT (get_indexed(1, "One") == 'n');
}
