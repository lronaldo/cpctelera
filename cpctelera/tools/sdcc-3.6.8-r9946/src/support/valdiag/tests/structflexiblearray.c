
#ifdef TEST1
struct tag {
  int good1;
  int flex[];		/* ERROR(GCC) */
  int good2;
} badstruct;		/* ERROR(SDCC) */
#endif

#ifdef TEST2
struct tag {
  int flex[];		/* ERROR(GCC) */
} badstruct;		/* ERROR(SDCC) */
#endif

#ifdef TEST3
struct tag {
  int good1;
  struct tag2 {
    int good2;
    int flex[];
  } nestedstruct;	/* IGNORE(SDCC) */
} badstruct;		/* ERROR(SDCC) */
#endif
