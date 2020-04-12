/* bug-2764.c
   A crash in the stm8 peephole optimizer.
 */

#include <testfwk.h>

#include <stdint.h>

typedef int16_t ee_s16;
typedef uint16_t ee_u16;
typedef int32_t ee_s32;
typedef uint8_t ee_u8;
typedef uint32_t ee_u32;

typedef struct MAT_PARAMS_S {
	int N;
} mat_params;

typedef struct RESULTS_S {
	ee_s16	seed1;
	ee_s16	seed2;
	void	*memblock[4];
	ee_u32	size;
	mat_params mat;
	ee_u16	crc;
	ee_u16	crcmatrix;
	ee_u16	crcstate;
} core_results;

#pragma disable_warning 85

#if !(defined(__SDCC_pic14) && !defined(__SDCC_PIC14_ENHANCED)) // Pseudo-stack size limit
ee_u16 core_bench_state(ee_u32 blksize, ee_u8 *memblock, ee_s16 seed1, ee_s16 seed2, ee_s16 step, ee_u16 crc)
{
	return 0x0a55;
}

#ifndef __SDCC_pdk14 // Lack of memory
ee_s16 calc_func(ee_s16 *pdata, core_results *res)
{
	ee_s16 data=*pdata;
	ee_s16 retval;
	ee_u8 optype=(data>>7) & 1;
	if (optype)
		return (data & 0x007f);
	else {
		ee_s16 flag=data & 0x7;
		ee_s16 dtype=((data>>3) & 0xf);
		dtype |= dtype << 4;
		switch (flag) {
			case 0:
				if (dtype<0x22)
					dtype=0x22;
				retval=core_bench_state(res->size,res->memblock[3],res->seed1,res->seed2,dtype,res->crc);
				if (res->crcstate==0)
					res->crcstate=retval;
				break;
			default:
				retval=data;
				break;
		}
		return retval;
	}
}
#endif
#endif

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
	ee_s16 data = 0;
	core_results res = {0};
#if !(defined(__SDCC_pic14) && !defined(__SDCC_PIC14_ENHANCED)) // Pseudo-stack size limit
	ASSERT(calc_func(&data, &res) == 0x0a55);
	ASSERT(res.crcstate ==  0x0a55);
#endif
#endif
}

