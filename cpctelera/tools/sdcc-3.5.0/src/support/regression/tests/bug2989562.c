/*
   bug2989562.c
 */

#include <testfwk.h>

unsigned int  adc_samples[8];
unsigned char sample_count = 8;

/* bug 2989562: push inside, but pop outside loop
   resulting in stack corruption */
unsigned int average(void)
{
    unsigned long sum;              // DWORD for the accumulated sum of elements
    unsigned char index;

    if (!sample_count) return 0;    // Avoid a divide by zero
    index = sample_count;           // Copy the size for a loop counter
    sum = sample_count / 2;         // Add half the size for averaging
    do {
        --index;
        sum += adc_samples[index];  // Accumulate a sum of elements
    } while (index & 0x07);

    return sum/sample_count;        // Return sum divided by number-of-elements
}

/**********************************************************/

unsigned char mult(unsigned char x, unsigned char pol)
{
    return x * pol;
}

__data unsigned char str[4];

__code unsigned char MIXCON[16] = {
  0x02, 0x03, 0x01, 0x01,
  0x01, 0x02, 0x03, 0x01,
  0x01, 0x01, 0x02, 0x03,
  0x03, 0x01, 0x01, 0x02
};

/* bug 2995824: push inside, but pop outside inner loop
   resulting in stack corruption */
void mix_columns(unsigned char *dbuf)
{
    unsigned char byte, i, col, mixi;

    byte= 0;

    for (col=0; col<4; col++)
    {
        mixi= 0;

        for (i=0; i<4; i++)
        {
            dbuf[byte]= mult(MIXCON[mixi+3], str[3]);
            byte++;
            mixi= mixi+4;
        }
    }
}

unsigned char buf[20];

void testBug(void)
{
    mix_columns(buf);
    average();
    ASSERT(1);
}
