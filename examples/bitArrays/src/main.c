#include "cpctelera_all.h"

typedef enum {
   f_getbit,
   f_get2bits,
   f_get4bits
} TFunc;

void printArray(unsigned char* video, void *array, unsigned char numelems, TFunc thefunction)
{
    unsigned int i;
    unsigned char out;

    for (i = 0; i < numelems; ++i) {
       unsigned char c;
       switch(thefunction) {
          case f_getbit:   c = cpct_getBit  (array, i)         ? '1'       : '_';  break;
          case f_get2bits: c = (out = cpct_get2Bits(array, i)) ? '0' + out : '_';  break;
          case f_get4bits: c = (out = cpct_get4Bits(array, i)) ? '0' + out : '_';  break;
       }
       cpct_drawROMCharM2(video, 1, c);
       video++;
    }
}

void main (void)
{
   unsigned char i, j=0, array[10], array2[20], array3[40];

   cpct_disableFirmware();
   cpct_setVideoMode(2);

   while(1) {
      cpct_memset(array,  10, 0);
      cpct_memset(array2, 20, 0);
      cpct_memset(array3, 40, 0);

      for (i = 0; i < 80; ++i) {
         cpct_setBit  (array,  i,     1);
         printArray((void*)0xC000, array,  80,   f_getbit); 
         cpct_setBit  (array,  i,     0);
      }

      for (j = 0; j < 16; j++) { 
         for (i = 0; i < 80; ++i) {
            cpct_set4Bits(array3, i, (i & 0x0F) + j);
            printArray((void*)0xC140, array3, 80, f_get4bits);
         }
      }

      for (j = 3; j > 0; --j) { 
         for (i = 0; i < 80; ++i) {
            cpct_set2Bits(array2, i, j);
            printArray((void*)0xC0A0, array2, 80, f_get2bits);
            cpct_set2Bits(array2, i, 0);
         }
      }


      for (i = 0; i < 80; ++i) {
         cpct_setBit(array, i, 1);
         printArray((void*)0xC000, array,  80,   f_getbit); 
      }

      for (j = 3; j > 0; --j) { 
         for (i = 0; i < 80; ++i) {
            cpct_set2Bits(array2, i, j);
            printArray((void*)0xC0A0, array2, 80, f_get2bits); 
         }
      }
   }
}