/* bug 1505811
 *  failed to compile due to incorrect calculation
 *  of offset for relative jump.
 */

#pragma disable_warning 85

#include <testfwk.h>

#include <stdint.h>

typedef enum CORE_STATE {
	CORE_START=0,
	CORE_INVALID,
	CORE_S1,
	CORE_S2,
	CORE_INT,
	CORE_FLOAT,
	CORE_EXPONENT,
	CORE_SCIENTIFIC,
	NUM_CORE_STATES
} core_state_e ;

uint16_t crcu32(uint32_t newval, uint16_t crc);

enum CORE_STATE core_state_transition(uint8_t **instr , uint32_t *transition_count)
{
	return 0;
}

void core_bench_state(uint32_t blksize, uint8_t *memblock, 
		int16_t seed1, int16_t step) 
{
	uint32_t final_counts[NUM_CORE_STATES];
	uint32_t track_counts[NUM_CORE_STATES];
	uint8_t *p=memblock;
	uint32_t i;

	for (i=0; i<NUM_CORE_STATES; i++) {
		final_counts[i]=track_counts[i]=0;
	}

	while (*p!=0) {
		enum CORE_STATE fstate=core_state_transition(&p,track_counts);
		final_counts[fstate]++;
	}

	p=memblock;
	while (p < (memblock+blksize)) {
		if (*p!=',')
			*p^=(uint8_t)seed1;
		p+=step;
	}
}

void
testBug(void)
{
}

