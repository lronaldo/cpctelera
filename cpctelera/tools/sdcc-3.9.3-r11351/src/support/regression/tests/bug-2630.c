/*
   bug-2630.c
*/

#include <testfwk.h>

#define NUM_CORE_STATES 4

int core_state_transition(unsigned char **instr , unsigned long *transition_count)
{
	static unsigned long i;

	transition_count;

	if (i >= NUM_CORE_STATES * 2)
		**instr=0;

	return i++ % NUM_CORE_STATES;
}

unsigned int crcu32(unsigned long newval, unsigned int crc)
{
	return (newval>>16)+crc;
}

#ifndef __SDCC_pdk14 // Lack of memory
unsigned int core_bench_state(unsigned long blksize, unsigned char *memblock, int seed2, int step, unsigned int crc) 
{
	unsigned long final_counts[NUM_CORE_STATES];
	unsigned long track_counts[NUM_CORE_STATES];
	unsigned char *p=memblock;
	unsigned long i;

	for (i=0; i<NUM_CORE_STATES; i++) {
		final_counts[i]=track_counts[i]=(1ul << 16) - 2 + i;
	}

	p=memblock;

	while (*p!=0) {
		int fstate=core_state_transition(&p,track_counts);
		final_counts[fstate]++; // Wrong code generated for increment here.
	}

	p=memblock;
	while (p < (memblock+blksize)) {
		if (*p!=',')
			*p^=(unsigned char)seed2;
		p+=step;
	}

	for (i=0; i<NUM_CORE_STATES; i++) {
		crc=crcu32(final_counts[i],crc);
		crc=crcu32(track_counts[i],crc);
	}

	return crc;
}
#endif

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
	unsigned char c = 1;
	ASSERT(core_bench_state(0, &c, 0x5a0a, 0x5a0a, 0) == 6);
#endif
}

