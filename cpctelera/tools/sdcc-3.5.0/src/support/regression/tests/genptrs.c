/* generic pointer comparison
*/

#include <testfwk.h>
#include <stdlib.h>

char eq(void * p1, void * p2)
{
	return (p1 == p2);
}

char neq(void * p1, void * p2)
{
	return (p1 != p2);
}

char smaller(void * p1, void * p2)
{
	return (p1 < p2);
}

char greater(void * p1, void * p2)
{
	return (p1 > p2);
}

char __xdata * xp0 = NULL;
char __xdata * xp1 = (char __xdata *)0x0001;
char __idata * ip0 = NULL;
char __idata * ip1 = (char __idata *)0x0001;
char __pdata * pp0 = NULL;
char __pdata * pp1 = (char __pdata *)0x0001;
char __code  * cp0 = NULL;
char __code  * cp1 = (char __code *)0x0001;
void (* fp0)(void) = NULL;
void (* fp1)(void) = (void (*)(void))0x0001;
#if defined (__SDCC_MODEL_HUGE)
void (* fpE)(void) = (void (*)(void))0x7E8000;	//SDCC assumes banked pointers have physical address != 0
void (* fpF)(void) = (void (*)(void))0x7F8000;	//choose the banks to be mapped to 0x8000 for the test
#endif
char         * gp0 = NULL;
char         * gp2 = (char __pdata *)0x0002;

void testPtrs(void)
{
#ifndef __SDCC_pic16
#if defined (__SDCC_MODEL_HUGE)
	char __code  * cp2 = (char __code *)0x0002;
	void (* fp2)(void) = (void (*)(void))0x0002;

	ASSERT (eq(cp2, fp2));
	ASSERT (smaller(fpE, fpF));
#endif

	ASSERT (xp0 == NULL);
	ASSERT (ip0 == NULL);
	ASSERT (pp0 == NULL);
	ASSERT (cp0 == NULL);
	ASSERT (fp0 == NULL);
	ASSERT (gp0 == NULL);

	ASSERT (xp1 != NULL);
	ASSERT (ip1 != NULL);
	ASSERT (pp1 != NULL);
	ASSERT (cp1 != NULL);
	ASSERT (fp1 != NULL);
	ASSERT (gp2 != NULL);

	ASSERT (eq(xp0, ip0));
	ASSERT (eq(xp0, pp0));
	ASSERT (eq(xp0, cp0));
	ASSERT (eq(xp0, fp0));
	ASSERT (eq(xp0, gp0));

#if defined(__SDCC_mcs51) || defined(__SDCC_ds390)
	ASSERT (neq(xp1, ip1));
	ASSERT (neq(xp1, pp1));
	ASSERT (neq(xp1, cp1));
	ASSERT (neq(xp1, fp1));
	ASSERT (neq(xp1, gp2));

	ASSERT (smaller(xp1, ip1) || greater(xp1, ip1));
	ASSERT (smaller(xp1, pp1) || greater(xp1, pp1));
	ASSERT (smaller(xp1, cp1) || greater(xp1, cp1));
	ASSERT (smaller(xp1, fp1) || greater(xp1, fp1));
	ASSERT (smaller(xp1, gp2) || greater(xp1, gp2));

	ASSERT (!smaller(xp0, ip0) && !greater(xp0, ip0));
	ASSERT (!smaller(xp0, pp0) && !greater(xp0, pp0));
	ASSERT (!smaller(xp0, cp0) && !greater(xp0, cp0));
	ASSERT (!smaller(xp0, fp0) && !greater(xp0, fp0));
	ASSERT (!smaller(xp0, gp0) && !greater(xp0, gp0));
#endif

	ASSERT (eq(cp1, fp1));
	ASSERT (smaller(pp1, gp2));
#endif
}
