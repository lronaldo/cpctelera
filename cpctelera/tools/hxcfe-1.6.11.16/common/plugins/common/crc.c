/*************************************************************************
 * Example Table Driven CRC16 Routine using 4-bit message chunks
 *
 * By Ashley Roll
 * Digital Nemesis Pty Ltd
 * www.digitalnemesis.com
 *
 * The following is an example of implementing a restricted size CRC16
 * table lookup. No optimisation as been done so the code is clear and
 * easy to understand.
 *
 * Test Vector: "123456789" (character string, no quotes)
 * Generated CRC: 0x29B1
 *
 *************************************************************************/

#include "crc.h"

/*
 * Calculate the value to XOR into the shifted CRC register for the given index
 * NumBits should be the "width" of the chunk being operated on eg: 4 or 8. Poly
 * is the polynomial to use eg 0x1021
 */
unsigned short GenCRC16TableEntry( const unsigned short index, const short NumBits, const unsigned short Poly )
{
	int i;
	unsigned short Ret;
		
	// Prepare the initial setup of the register so the index is at the
	// top most bits.
	Ret = index;
	Ret <<= 16 - NumBits;

	for( i = 0; i < NumBits; i++ ) {
		if( Ret & 0x8000 )
			Ret = (Ret << 1) ^ Poly;
		else 
			Ret = Ret << 1;
	}
	
	return Ret;
}

/*
 * Before each message CRC is generated, the CRC register must be
 * initialised by calling this function
 */
void CRC16_Init( unsigned char *CRC16_High, unsigned char *CRC16_Low,unsigned char * crctable,unsigned short polynome,unsigned short initvalue)
{

	unsigned short i, Count, te;
	#define NUM_BITS 4			// Width of message chunk each iteration of the CRC algorithm
	
	// Setup the values to compute the table
	Count = 1 << NUM_BITS;		// Number of entries in the table
	
	for( i = 0; i < Count; i++ ) {
		te = GenCRC16TableEntry( i, NUM_BITS, polynome );
		crctable[i+Count]=te>>8;
		crctable[i]=te&0xFF;
	}

	// Initialise the CRC to 0xFFFF for the CCITT specification
	*CRC16_High = initvalue>>8;
	*CRC16_Low = initvalue&0xFF;
}

/*
 * Process 4 bits of the message to update the CRC Value.
 *
 * Note that the data must be in the low nibble of val.
 */
void CRC16_Update4Bits(unsigned char *CRC16_High, unsigned char *CRC16_Low, unsigned char val ,unsigned char * crctable )
{
	unsigned char	t;

	// Step one, extract the Most significant 4 bits of the CRC register
	t = *CRC16_High >> 4;

	// XOR in the Message Data into the extracted bits
	t = t ^ val;

	// Shift the CRC Register left 4 bits
	*CRC16_High = (*CRC16_High << 4) | (*CRC16_Low >> 4);
	*CRC16_Low = *CRC16_Low << 4;

	// Do the table lookups and XOR the result into the CRC Tables
	*CRC16_High = *CRC16_High ^ crctable[t+16];
	*CRC16_Low = *CRC16_Low ^ crctable[t];
}

/*
 * Process one Message Byte to update the current CRC Value
 */
void CRC16_Update(unsigned char *CRC16_High, unsigned char *CRC16_Low, unsigned char val,unsigned char * crctable)
{
	CRC16_Update4Bits(CRC16_High,CRC16_Low, (unsigned char)(val >> 4),crctable);		// High nibble first
	CRC16_Update4Bits(CRC16_High,CRC16_Low, (unsigned char)(val & 0x0F), crctable);	// Low nibble
}

