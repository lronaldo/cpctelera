
#ifdef TEST0
int a;
int a;
#endif

#ifdef TEST1
int a;		/* IGNORE */
char a;		/* ERROR */
#endif

#ifdef TEST2
int a;		/* IGNORE */
int *a;		/* ERROR */
#endif

#ifdef TEST3
int *a;		/* IGNORE */
int a[5];	 /* ERROR */
#endif

/* array size must match */

#ifdef TEST4
int a[4];	/* IGNORE */
int a[5];	/* ERROR */
#endif

/* but it is legal to clarify */
/* an incomplete type */

#ifdef TEST4b
int a[];
int a[5];
#endif

/* type qualifier must match */

#ifdef TEST5
int a;		/* IGNORE */
volatile int a; /* ERROR */
#endif

#ifdef TEST6
int a;		/* IGNORE */
const int a;	/* ERROR */
#endif

#ifdef TEST7
int a=1;	/* IGNORE */
int a=2;	 /* ERROR */
#endif

#ifdef TEST7a
int a=1;
int a;
#endif

#ifdef TEST8
int a=1;	/* IGNORE */
int a=1;	/* ERROR */
#endif

#if defined(__SDCC)
#define AT(x) __at x
#else
#define AT(x)
#endif

#if defined(__has_xdata)
#define XDATA __xdata
#define DATA __data
#else
#define XDATA
#define DATA
#endif

#ifdef TEST9
#ifdef __SDCC
XDATA int a;	/* IGNORE */
DATA int a;	/* IGNORE(SDCC && __has_xdata) */ // Should be error, see bug #2735.
#endif
#endif

#ifdef TEST9b
#ifdef __SDCC
DATA int a;	/* IGNORE */
XDATA int a;	/* ERROR(SDCC && __has_xdata) */
#endif
#endif

#ifdef TEST9c
#ifdef __SDCC
extern DATA int a;
DATA int a;
#endif
#endif

#ifdef TEST9d
#ifdef __SDCC
extern XDATA int a;
XDATA int a;
#endif
#endif

#ifdef TEST9e
#ifdef __SDCC
extern XDATA int a; /* IGNORE */
DATA int a;	/* ERROR(SDCC && __has_xdata) */
#endif
#endif

#ifdef TEST9f
#ifdef __SDCC
extern DATA int a; /* IGNORE */
XDATA int a;	/* ERROR(SDCC && __has_xdata) */
#endif
#endif

#ifdef TEST10
#if defined(__SDCC) && defined(__has_xdata)
extern volatile XDATA AT(0) int a; /* IGNORE */
volatile XDATA int a;	/* ERROR(SDCC && __has_xdata) */
#endif
#endif

int validDeclaraction;  /* to make sure this is never an empty source file */
