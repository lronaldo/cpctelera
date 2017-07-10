#ifndef MDU88X_HEADER
#define MDU88X_HEADER

#include <stdint.h>

/* unsigned OPs */

extern uint8_t mdu_32udiv16(uint32_t op1, uint16_t op2,
			    uint32_t *res, uint16_t *rem)
  __reentrant;
extern uint8_t mdu_16udiv16(uint16_t op1, uint16_t op2,
			    uint16_t *res, uint16_t *rem)
    __reentrant;
extern uint8_t mdu_16umul16(uint16_t op1, uint16_t op2,
			    uint32_t *res)
    __reentrant;

/* signed OPs */

extern uint8_t mdu_32sdiv16(int32_t op1, int16_t op2,
			    int32_t *res, int16_t *rem)
  __reentrant;
extern uint8_t mdu_16sdiv16(int16_t op1, int16_t op2,
			    int16_t *res, int16_t *rem)
    __reentrant;
extern uint8_t mdu_16smul16(int16_t op1, int16_t op2,
			    int32_t *res)
    __reentrant;

/* normalize */

extern uint8_t mdu_norm(uint32_t op,
			uint32_t *res, uint8_t *nuof_shifts)
    __reentrant;

/* logical shifts */

extern uint8_t mdu_lshift(uint32_t op, uint8_t shifts, uint8_t right,
			  uint32_t *res)
    __reentrant;
extern uint8_t mdu_lshift_left(uint32_t op, uint8_t shifts,
			       uint32_t *res)
    __reentrant;
extern uint8_t mdu_lshift_right(uint32_t op, uint8_t shifts,
				uint32_t *res)
    __reentrant;

/* arithmetic shifts */

extern uint8_t mdu_ashift(int32_t op, int8_t shifts, int8_t right,
			  int32_t *res)
    __reentrant;
extern uint8_t mdu_ashift_left(int32_t op, int8_t shifts,
			       int32_t *res)
    __reentrant;
extern uint8_t mdu_ashift_right(int32_t op, int8_t shifts,
				int32_t *res)
    __reentrant;

#endif
