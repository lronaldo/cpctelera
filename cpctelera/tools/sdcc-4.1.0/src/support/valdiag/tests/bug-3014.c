/* bug-3014.c

   Crash on incomplete semi-K&R function.
 */

#ifdef TEST1
void *f(a) /* ERROR */
#endif /* IGNORE */

