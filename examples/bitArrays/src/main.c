#include "cpctelera_all.h"

void printArray(unsigned char *array, unsigned char size)
{
    unsigned int i;
    unsigned char* video=(unsigned char*)0xC000;
    for (i = 0; i < size*8; ++i) {
       unsigned char c = cpct_getBit(array, i) ? '#' : '_'; 
       cpct_drawROMCharM2(video, 1, c);
       video++;
    }
}

void pause() {
   __asm
      HALT
      HALT
      HALT
      HALT
   __endasm;
}

void main (void)
{
   unsigned char i, array[10];

   cpct_disableFirmware();
   cpct_memset(array, 10, 0);
   cpct_setVideoMode(2);
   printArray(array, 10);

   while(1) {
      for (i = 0; i < 80; ++i) {
         cpct_setBit(array, i, 1);
         printArray(array, 10); pause();
         cpct_setBit(array, i, 0);
      }

      for (i = 0; i < 80; ++i) {
         cpct_setBit(array, i, 1);
         printArray(array, 10); pause();
      }
   }
}