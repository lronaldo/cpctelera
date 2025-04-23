/* Macro voodoo rewriting GCC-style attributes to C23-style attributes. */
/* Can be used via "--include gcc_attr.h" on the command line. */

#if __STDC_VERSION__ < 202311L

/* Not in C23 mode => remove GCC-style attributes */
#warning "Rewriting GCC-style to C23-style attributes requires C23. Ignoring all GCC-style attributes."
#define __attribute__(__GCC_ATTR_ARG)

#else

/* In C23 mode => translate to C23-style attributes */
/* e.g. from __attribute__((packed, foo(bar))) to [[packed, foo(bar)]] */
#warning "Rewriting GCC-style to C23-style attributes. Please check compatibility."
#define __GCC_ATTR_LIST_ARGS(...) __VA_ARGS__
#define __GCC_ATTR_UNWRAP_ARG(__GCC_ATTR_ARG) __GCC_ATTR_ARG
#define __attribute__(__GCC_ATTR_ARG) [[__GCC_ATTR_UNWRAP_ARG(__GCC_ATTR_LIST_ARGS __GCC_ATTR_ARG)]]

#endif
