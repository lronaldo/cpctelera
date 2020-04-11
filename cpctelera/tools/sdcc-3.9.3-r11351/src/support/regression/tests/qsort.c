/** qsort.c - test sorting

	type: signed int, signed long
*/

#include <testfwk.h>

#include <stdlib.h>
#include <string.h>

#define NUM_ELEM 20

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
{type} unsorted[NUM_ELEM] = {120, 110, 90, 100, 190, 190, 190, 130, 125, 80, 132, -8, 20, 40, 60, -10, 20, 30, 40, 50};

const {type} sorted[NUM_ELEM] = {-10, -8, 20, 20, 30, 40, 40, 50, 60, 80, 90, 100, 110, 120, 125, 130, 132, 190, 190, 190};
#endif

int cmp(const void *lp, const void *rp) __reentrant
{
	{type} l = *((const {type} *)lp);
	{type} r = *((const {type} *)rp);

	if(l < r)
		return(-1);
	else if (l == r)
		return(0);
	else
		return(1);
}

void testSort(void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
	qsort(unsorted, NUM_ELEM, sizeof({type}), &cmp);

	ASSERT(!memcmp(unsorted, sorted, sizeof({type}) * NUM_ELEM));
#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL)) // Not enough RAM
	{
		const {type} e95 = 95;
		const {type} e35 = 35;
		const {type} e10 = -10;
		const {type} e20 = 20;
		const {type} e60 = 60;
		const {type} e190 = 190;

		ASSERT(bsearch(&e95, sorted, NUM_ELEM, sizeof({type}), &cmp) == 0);
		ASSERT(bsearch(&e35, sorted, NUM_ELEM, sizeof({type}), &cmp) == 0);
		ASSERT(*(const {type} *)(bsearch(&e10, sorted, NUM_ELEM, sizeof({type}), &cmp)) == -10);
		ASSERT(*(const {type} *)(bsearch(&e20, sorted, NUM_ELEM, sizeof({type}), &cmp)) == 20);
		ASSERT(*(const {type} *)(bsearch(&e60, sorted, NUM_ELEM, sizeof({type}), &cmp)) == 60);
		ASSERT(*(const {type} *)(bsearch(&e190, sorted, NUM_ELEM, sizeof({type}), &cmp)) == 190);
	}
#endif
#endif
}

