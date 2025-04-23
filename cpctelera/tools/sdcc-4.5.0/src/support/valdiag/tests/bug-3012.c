/* bug-3009.c

   Infinite loop on unterminated string literal for lexers that return 0 at the end of file (e.g. flex used to return EOF in older versions, but return 0 now).
 */

#ifdef TEST1
" /* ERROR */
#endif /* IGNORE */

