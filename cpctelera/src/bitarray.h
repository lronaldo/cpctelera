#ifndef CPCT_BITARRAY_H
#define CPCT_BITARRAY_H

///
/// Function Declarations
///
extern char cpct_getBit(char *array, unsigned int pos);
extern char cpct_get2Bits(char *array, unsigned int pos);
extern char cpct_get4Bits(char *array, unsigned int pos);

extern void cpct_setBit(char *array, unsigned int pos, unsigned char value);
extern void cpct_set2Bits(char *array, unsigned int pos, unsigned int value);
extern void cpct_set4Bits(char *array, unsigned int pos, unsigned int value);

#endif