/* bug-3010.c

   Segfault on invalid function declaration
 */

#ifdef TEST1
int f(int)_; /* ERROR */
#endif

