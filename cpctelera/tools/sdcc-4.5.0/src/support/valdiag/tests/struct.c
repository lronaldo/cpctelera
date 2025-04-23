
#ifdef TEST1
struct tag {
  int good1;
  register int bad;	/* ERROR */
  int good2;
} badstruct;		/* IGNORE */
#endif

#ifdef TEST2
struct tag {
  int good1;
  int bad;	/* IGNORE */
  int bad;	/* ERROR */
  int good2;
} badstruct;
#endif

#ifdef TEST3a
struct tag {
  int good1;
  int bad:255;	/* ERROR */
  int good2;
} badstruct;
#endif

#ifdef TEST3b
struct tag {
  int good1;
  float badtype1 : 5; /* ERROR */
  int good2;
  _Bool badwidth2 : 2; /* ERROR */
  int good3;
  int badwidth2 : 17; /* ERROR */
  int good4;
} badstruct;
#endif

#ifdef TEST4
struct tag {
  int good1;
  int good2;
} goodstruct1;

struct tag goodstruct2;
#endif

#ifdef TEST5a
struct tag {
  int good1;
  int good2;
} goodstruct1;

union tag badunion;	/* ERROR */
#endif

#ifdef TEST5b
union tag {
  int good1;
  int good2;
} goodunion1;

struct tag badstruct;	/* ERROR */
#endif


#ifdef TEST6
struct linklist {
  struct linklist *prev;
  struct linklist *next;
  int x;
} ll;
#endif

#ifdef TEST7a
union tag {
  struct tag *next;	/* ERROR */
  int x;
} ll;
#endif

#ifdef TEST7b
struct tag {
  union tag *next;	/* ERROR */
  int x;
} ll;
#endif

#ifdef TEST8a
struct tag {
  int a;		/* IGNORE */
  struct {
    int a;		/* ERROR(SDCC) */ /* IGNORE(GCC) */
    int b;
  };
} ll;  
#endif

#ifdef TEST8b
struct tag {
  int a;
  struct {
    int b;
    int c;
  };
} ll;  

void test(void)
{
  ll.a = 1;
  ll.b = 2;
  ll.c = 3;
}

#endif

/* bug 3086: SDCC had infinite loop on this error */
#ifdef TEST9
struct tag1 {
  union {
    struct tag2;	/* ERROR(SDCC) */ /* IGNORE(GCC) */
  } tag3;		/* IGNORE */
};
#endif

// C23 allows multiple compatible definitions for struct.
#ifdef TEST10
#ifdef __SDCC
#pragma std_c23
#endif

struct A {int x; int y;}; /* IGNORE */
struct A {int x; int y;}; /* IGNORE(GCC) */
struct A {int x; int z;}; /* ERROR */

#endif

