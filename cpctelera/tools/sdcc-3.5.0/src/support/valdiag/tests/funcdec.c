
#ifdef TEST1
void foo(void);		/* IGNORE */
int foo(void) { }	/* ERROR */
#endif

#ifdef TEST2
void foo(void);		/* IGNORE */
void foo(int a) {a; }	/* ERROR */
#endif


#ifdef TEST3
void foo(int);		/* IGNORE */
void foo(int a, int b) {a;b; }	/* ERROR */
#endif

#ifdef TEST4
void foo(int, int);	/* IGNORE */
void foo(int a) {a; }	/* ERROR */
#endif

#if defined(SDCC) && !(defined(__z80) || defined(__gbz80))
#define REENTRANT __reentrant
#define HAS_REENTRANT 1
#else
#define REENTRANT
#define HAS_REENTRANT 0
#endif

#ifdef TEST5
void foo(int, int) REENTRANT;	/* IGNORE */
#if HAS_REENTRANT
void foo(int a, int b) {a; b;} /* ERROR(SDCC && !(__z80 || __gbz80 || SDCC_STACK_AUTO)) */
#endif
#endif

#ifdef TEST6
void foo(int a=1)		/* ERROR */
{
}
#endif

#ifdef TEST7
void foo(static int a)		/* ERROR */
{
}
#endif
