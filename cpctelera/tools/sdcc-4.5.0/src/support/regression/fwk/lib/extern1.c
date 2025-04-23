/* needed by tests/inline.c */

#ifdef __SDCC
#pragma std_sdcc99
#endif

/*--------------
    bug 2591
    extern definition with parameters not in registers
	these parameters should only be allocated here
    the corresponding inline definition is in tests/inline.c
*/
extern long bug_2591 (long a, long b, long c)
{
  return a | b | c;
}

extern char inlined_function (void)
{
	return 2;
}

/* needed by tests/test-p99-conformance.c */
void
has_undefined_symbol2(void) {
  /* empty */
}

