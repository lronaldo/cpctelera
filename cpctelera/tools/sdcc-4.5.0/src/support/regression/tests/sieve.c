/*
   sieve.c

   This variant of the sieve of Eratosthenes was, following its 1981 publication in Byte Magazine,
   a popular benchmark for array acces throughout the 1980s and 1990s.
*/

#include <testfwk.h>

#if !defined (__SDCC_mcs51) && !defined (__SDCC_sm83) && !defined (__SDCC_pdk13) && !defined (__SDCC_pdk14) && !defined (__SDCC_pdk15) && !defined (__SDCC_pdk16) && !defined (__SDCC_stm8) && !defined (__SDCC_f8) // Lack of data memory
#define true 1
#define false 0
#define size 8190
#define sizepl 8191
char flags[sizepl];
#endif
void testSieve (void) {
#if !defined (__SDCC_mcs51) && !defined (__SDCC_sm83) && !defined (__SDCC_pdk13) && !defined (__SDCC_pdk14) && !defined (__SDCC_pdk15) && !defined (__SDCC_pdk16) && !defined (__SDCC_stm8) && !defined (__SDCC_f8) // Lack of data memory
    int i, prime, k, count, iter; 
    for (iter = 1; iter <= 10; iter ++) {
        count=0 ; 
        for (i = 0; i <= size; i++)
            flags[i] = true; 
        for (i = 0; i <= size; i++) { 
            if (flags[i]) {
                prime = i + i + 3; 
                k = i + prime; 
                while (k <= size) { 
                    flags[k] = false; 
                    k += prime; 
                }
                count = count + 1;
            }
        }
    }
    ASSERT (count == 1899);
#endif
}

