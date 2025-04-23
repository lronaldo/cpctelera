// dummy tm.h for sdcpp
#ifndef GCC_TM_H
#define GCC_TM_H

// misc stuff scraped together from gcc/config/$target
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG
#define BYTES_BIG_ENDIAN 0
// #define UNITS_PER_WORD 1
#ifndef LIBC_GLIBC
# define LIBC_GLIBC 1
#endif
#ifndef LIBC_UCLIBC
# define LIBC_UCLIBC 2
#endif
#ifndef LIBC_BIONIC
# define LIBC_BIONIC 3
#endif
#ifndef LIBC_MUSL
# define LIBC_MUSL 4
#endif
#ifndef DEFAULT_LIBC
# define DEFAULT_LIBC LIBC_GLIBC
#endif

#ifdef IN_GCC
# include "options.h"
# include "insn-constants.h"
#endif

#if defined IN_GCC && !defined GENERATOR_FILE && !defined USED_FOR_TARGET
# include "insn-flags.h"
# include "config/dummy/dummy.h"
#endif

// here?
#include "defaults.h"
#endif // guard
