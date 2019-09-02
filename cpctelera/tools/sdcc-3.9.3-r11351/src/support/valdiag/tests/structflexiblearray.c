
#ifdef TEST1
struct tag {
  int good1;
  int flex[];		/* ERROR */
  int good2;
} badstruct;
#endif

#ifdef TEST2
struct tag {
  int flex[];		/* ERROR */
} badstruct;
#endif

#ifdef TEST3
struct tag {
  int good1;
  struct tag2 {
    int good2;
    int flex[];
  } nestedstruct;	/* ERROR(SDCC) */
} badstruct;
#endif
