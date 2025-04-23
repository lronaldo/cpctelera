/* bug-3031.c

   Initialization of file-scope variables by an expression that contains function calls resulted in a segfault in codegen.
 */

#ifdef TEST1
float f(int);

int i;
float d = f(i); /* ERROR */

#endif

