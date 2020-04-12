#ifndef MDU517_HEADER
#define MDU517_HEADER

#include <stdint.h>

extern uint8_t mdu_32udiv16(uint32_t op1, uint16_t op2,
			    uint32_t *res, uint16_t *rem)
  __reentrant;
extern uint8_t mdu_16udiv16(uint16_t op1, uint16_t op2,
			    uint16_t *res, uint16_t *rem)
    __reentrant;
extern uint8_t mdu_16umul16(uint16_t op1, uint16_t op2,
			    uint32_t *res)
    __reentrant;
extern uint8_t mdu_norm(uint32_t op,
			uint32_t *res, uint8_t *nuof_shifts)
    __reentrant;
extern uint8_t mdu_lshift(uint32_t op, uint8_t shifts, uint8_t right,
			  uint32_t *res)
    __reentrant;
extern uint8_t mdu_lshift_left(uint32_t op, uint8_t shifts,
			       uint32_t *res)
    __reentrant;
extern uint8_t mdu_lshift_right(uint32_t op, uint8_t shifts,
				uint32_t *res)
    __reentrant;

#endif
