/* universal2.c

   Test a diagnostic for universal character names, that cannot reside in the same file as other tests:
   A \u003a outside of string literal always results in a diagnostoc, even inside #if 0.
   
 */

#ifdef TEST1
// ':' via UCN in identifier
const char *string\u003a = u8":"; /* ERROR */
#endif

