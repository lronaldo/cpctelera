/*  Test for bugs in equality comparison that are sensitive to regoster allocation (as #2621 was)

    type: unsigned long, unsigned long long

 */

#include <testfwk.h>

#ifndef __SDCC_ds390

void t(unsigned int i)
{
	i;
}

/* Try to get lower 16 bits allocated to register x on stm8 */
{type} f( {type} p)
{
	register {type} v = p;

	if(v == 0x0000ffff)
		t((unsigned int)v);
	else if(v == 0x00010000)
		t((unsigned int)v + 1);
	else if(v == 0xffff0001)
		t((unsigned int)v + 42);
	else
		return(v);

	return(v);
}

/* Try to get lower 16 bits allocated to register y on stm8*/
{type} g({type} p)
{
	register {type} v = p;

	if(v == 0x0000ffff)
		t((unsigned int)(v >> 16));
	else if(v == 0x00010000)
		t((unsigned int)(v >> 16) + 1);
	else if(v == 0xffff0001)
		t((unsigned int)(v >> 16) + 42);
	else
		return(v);


	return(v >> 16);
}

/* Try to get lower 8 bits allocated to register a on stm8*/
{type} h( {type} p)
{
	register {type} v = p;

	if(v == 0x0000ffff)
		v |= 0x55;
	else if(v == 0x00010000)
		v &= 0x55;
	else if(v == 0xffff0001)
		v |= 0xaa;
	else
		return(v);

	return(v);
}

#endif

void testBug(void)
{
#ifndef __SDCC_ds390
	ASSERT(f(0x55aa55aa) == 0x55aa55aa);
	ASSERT(f(0x0000ffff) == 0x0000ffff);
	ASSERT(f(0xffff0001) == 0xffff0001);

	ASSERT(g(0x55aa55aa) == 0x55aa55aa);
	ASSERT(g(0x0000ffff) == 0x00000000);
	ASSERT(g(0xffff0001) == 0x0000ffff);

	ASSERT(h(0x55aa55aa) == 0x55aa55aa);
	ASSERT(h(0x0000ffff) == 0x0000ffff);
	ASSERT(h(0xffff0001) == 0xffff00ab);
#endif
}

