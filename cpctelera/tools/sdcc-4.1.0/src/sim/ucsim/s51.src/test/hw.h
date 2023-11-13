#ifndef HW_HEADER
#define HW_HEADER


#if defined __SDCC || defined SDCC
#include <mcs51/8052.h>
#elif defined __C51__
#include <reg52.h>
#else /* IAR4 */
#include <io51.h>
#endif


#if defined __SDCC || defined SDCC
#define bit __bit
#define CODE_PTR(TYPE) __code TYPE *
#define XRAM_PTR(TYPE) __xdata TYPE *
#elif defined __C51__
#define __bit bit
#define CODE_PTR(TYPE) TYPE code *
#define XRAM_PTR(TYPE) TYPE xdata *
#else /* IAR4 */
#define __bit bit
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define XRAM_SIZE (0x1000)
#define PAGE_SIZE (0x0200)
#define RESERVED_SPACE (2*PAGE_SIZE)

#define LOADER_START (0xe000)
#define LOADER_SIZE (0x10000 - LOADER_START)

#define INFO_BLOCK_SIZE (0x0040)

#define BOARD_INFO ((LOADER_START - RESERVED_SPACE - PAGE_SIZE - INFO_BLOCK_SIZE) + LOADER_SIZE)


#endif

/* End of hw.h */
