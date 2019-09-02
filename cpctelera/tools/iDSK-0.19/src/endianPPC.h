#ifndef __ENDIANPPC_H__
#define __ENDIANPPC_H__

/* macros convertion little endian convertion to big endian convertion */


#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )
#define SWAP_4(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )
#define FIX_SHORT(x) (*(unsigned short *)&(x) = SWAP_2(*(unsigned short *)&(x)))
#define FIX_INT(x)   (*(unsigned int *)&(x)   = SWAP_4(*(unsigned int *)&(x)))
#define FIX_FLOAT(x) FIX_INT(x)



/* endianness test function */

bool isBigEndian(void);

#endif 
