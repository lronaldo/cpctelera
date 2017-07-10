#include <tinibios.h>
#include <stdio.h>

void main(void)
{
    int i, j;
    
    printf("Hello world!\n");
    j = 0;
    
    while (1)
    {
	printf("Loop %d\n", j++);

	
	/* Make LED go blink. */
	P5 |= 4;
	for (i = 0; i < 32767; i++)
	{
	    
	    ;
	}
	for (i = 0; i < 32767; i++)
	{
	    
	    ;
	}	
	
	P5 &= ~4;
	for (i = 0; i < 32767; i++)
	{
	    
	    ;
	}
    }
    
}
