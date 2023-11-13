/* bug-2940.c

   Missing diagnostic on auto at file scope
 */

#ifdef TEST1
auto int i; /* ERROR */
#endif

