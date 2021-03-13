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

#if defined(__SDCC_stm8) ||defined(PORT_HOST)
#define __data
#define __idata
#define __pdata
#define __xdata
#define __code
#define __near
#define __far
#define __reentrant
#endif

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka) || defined(__SDCC_gbz80) || defined(__SDCC_tlcs90) || defined(__SDCC_ez80_z80) || defined(__SDCC_z80n)
#define __data
#define __idata
#define __pdata
#define __xdata
#define __code
#define __near
#define __far
#define __reentrant
#else
#define __smallc
#define __z88dk_callee
#define __z88dk_fastcall
#endif

#ifdef __SDCC_gbz80
#define __z88dk_fastcall
#endif

#if defined(__SDCC_pdk13) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15)
# define __data
# define __idata
# define __pdata
# define __xdata
# define __code // TODO: __code will be supported in the future.
# define __near
# define __far
#endif

#if defined(PORT_HOST)
# define __at(x)
#endif

#if defined(__SDCC_hc08) || defined(__SDCC_s08)
# define __idata __data
# define __pdata __data
#endif

#if defined(__SDCC_pic14)
# define __idata __data
# define __xdata __data
# define __pdata __data
#endif

#if defined(__SDCC_pic16)
# define __idata __data
# define __xdata __data
# define __pdata __data
#endif

void __fail (__code const char *szMsg, __code const char *szCond, __code const char *szFile, int line);
void __prints (const char *s);
void __printd (int n);
void __printu (unsigned int n);
__code const char *__getSuiteName (void);
void __runSuite (void);

#define ASSERT(_a)  (++__numTests, (_a) ? (void)0 : __fail ("Assertion failed", #_a, __FILE__, __LINE__))
#define ASSERTFALSE(_a)  ASSERT(!(_a))
#define FAIL()      FAILM("Failure")
#define FAILM(_a)   __fail(_a, #_a, __FILE__, __LINE__)

#define UNUSED(_a)  if (_a) { }

#endif //__TESTFWK_H
