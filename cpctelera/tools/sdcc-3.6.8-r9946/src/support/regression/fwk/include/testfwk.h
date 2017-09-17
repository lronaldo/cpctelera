#ifndef __TESTFWK_H
#define __TESTFWK_H   1

extern int __numTests;
extern const int __numCases;

#ifndef NO_VARARGS
void __printf(const char *szFormat, ...);
#define LOG(_a)     __printf _a
#else
#define LOG(_a)     /* hollow log */
#endif

#ifdef __SDCC
 #include <sdcc-lib.h>
#else
 #define _AUTOMEM
 #define _STATMEM
#endif

#if defined(PORT_HOST) || defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r3ka) || defined(__SDCC_gbz80) || defined(__SDCC_stm8) || defined(__SDCC_tlcs90)
# define __data
# define __idata
# define __pdata
# define __xdata
# define __code
# define __near
# define __far
# define __reentrant
#endif

#if defined(PORT_HOST)
# define __at(x)
#endif

#if defined(__SDCC_hc08) || defined(__SDCC_s08)
# define __idata __data
# define __pdata __data
#endif

#if defined(__SDCC_pic16)
# define __idata __data
# define __xdata __data
# define __pdata __data
#endif

void __fail (__code const char *szMsg, __code const char *szCond, __code const char *szFile, int line);
void __prints (const char *s);
void __printn (int n);
__code const char *__getSuiteName (void);
void __runSuite (void);

#define ASSERT(_a)  (++__numTests, (_a) ? (void)0 : __fail ("Assertion failed", #_a, __FILE__, __LINE__))
#define ASSERTFALSE(_a)  ASSERT(!(_a))
#define FAIL()      FAILM("Failure")
#define FAILM(_a)   __fail(_a, #_a, __FILE__, __LINE__)

#define UNUSED(_a)  if (_a) { }

#endif //__TESTFWK_H
