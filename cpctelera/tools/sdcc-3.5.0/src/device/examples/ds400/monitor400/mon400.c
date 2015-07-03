#include <tinibios.h>
#include <ds400rom.h>

#include <stdio.h>
#include <string.h>

#define BUF_LEN 80

void usage(void)
{
    puts("Available commands:\n");
    puts("ledon: turns LED on.");
    puts("ledoff: turns LED off.");
    puts("clock: reports millisecond timer.");
    puts("sleep: sleeps for 10 seconds (or forever if you didn't startclock first).");
}

void blinker(void)
{
    int i, j;
    
    while (1)
    {
	P5 |= 4;
	for (j = 0; j < 10; j++)
	{
	    for (i = 0; i < 32767; i++)
	    {
		;
	    }
	}
	
	P5 &= ~4;
	
	for (j = 0; j < 10; j++)
	{
	    for (i = 0; i < 32767; i++)
	    {
		;
	    }
	}	
    }
}


void main(void)
{
    char buffer[80];
    
    // At this stage, the rom isn't initalized. We do have polled serial I/O, but that's
    // about the only functional library service.
    printf("TINIm400 monitor rev 0.0\n");

    romInit(1, SPEED_2X);

    // Now we're cooking with gas.
    
    while (1)
    {
	// monitor prompt.
        printf("-> ");
	
	gets(buffer); // unsafe, of course, should use some equivalent of fgets.
	
	if (!strcmp(buffer, "ledon"))
	{
	    P5 &= ~4; // LED on.
	    printf("LED on.\n");
	}
	else if (!strcmp(buffer, "ledoff"))
	{
	    P5 |= 4;
	    printf("LED off.\n");
	}
	else if (!strcmp(buffer, "clock"))
	{
	    printf("Clock: %ld\n", ClockTicks());
	}
	else if (!strcmp(buffer, "thread"))
	{
	    printf("Thread ID: %d\n", (int)task_getthreadID());
	}
	else if (!strcmp(buffer, "sleep"))
	{
	    printf("Sleeping for 10 seconds...\n");
	    
	    ClockMilliSecondsDelay(10 * 1000);
	    
	    printf("Back.\n");
	}
	else if (!strcmp(buffer, "pmr"))
	{
	    printf("PMR: %x\n", PMR);
	}
	else if (!strcmp(buffer, "exif"))
	{
	    printf("EXIF: %x\n", EXIF);
	}
	else if (!strcmp(buffer, "blink"))
	{
	    blinker();
	}
	else if (!strcmp(buffer, "t0"))
	{
	    printf("TH0:TL0 %x:%x\n", TH0, TL0);
	}
	else if (!strcmp(buffer, "t2"))
	{
	    printf("TH2:TL2 %x:%x\n", TH2, TH2);
	}	
	else if (!strcmp(buffer, "faster"))
	{
	    printf("going really fast...\n");
	    P5 |= 4; // LED off.

	    PMR = 0x82; 
	    PMR = 0x8a; // 8a for REAL fast
	    PMR = 0x9a; // 9a for REAL fast.
	    
	    while (!(EXIF & 8))
		;

	    PMR = 0x1a; // 1a for REAL fast.
	    
_asm
		nop
_endasm;		
	    
	    P5 &= ~5; // LED on.
	}
	else if (buffer[0])
	{
	    printf("Unknown command \"%s\".\n", buffer);
	    usage();
	}
    }
}
