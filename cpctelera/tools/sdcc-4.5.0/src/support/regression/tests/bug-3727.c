/* bug-3727.c
   ?
 */

#include <testfwk.h>

typedef unsigned char u8;

void VDP_Poke_16K( u8 dest );

int bad = 1;

u8 n1;

void fake (void)
{
    n1 = 32;
}

void SatUpdate(void)
{
    n1=0;

    if ( bad )
    {
        for (u8 j=0; j<1; j++)
        {
            fake();
        }
    }
    VDP_Poke_16K( 2*n1 );
}

void testBug(void)
{
    SatUpdate();
}


void VDP_Poke_16K( u8 dest )
{
#if 0 // Bug not yet fixed
    ASSERT(dest);
#else
    (void)dest;
#endif
}

