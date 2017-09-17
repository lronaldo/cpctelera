/*
   bug-2590.c
*/

#include <testfwk.h>

#if (defined __SDCC_z80 || defined __SDCC_z180 || defined __SDCC_gbz80 || defined __SDCC_r2k || defined __SDCC_r3ka || defined __SDCC_tlcs90 || defined __SDCC_stm8)

#pragma disable_warning 85

const unsigned char *letras_tiles01;

extern void msx_vfill(unsigned int addr, unsigned int value, unsigned int count) __smallc
{
}

extern void msx_vwrite_direct(void* source, unsigned int dest, unsigned int count) __smallc
{
}

void poner_texto_tiles(unsigned char *texto, unsigned int size, unsigned int posicion, unsigned char color)
{
      unsigned char i = 0;

      while (size)
      {

            if (texto[i] == ' ')
            {
                  msx_vfill((posicion << 3), 0x00, 8);
            }
            else
            {
                  msx_vwrite_direct(letras_tiles01 + ((*(texto + i) - 65) << 3), posicion << 3, 8);
            }

            size--;
        }
}

#endif

void testBug(void)
{
}
