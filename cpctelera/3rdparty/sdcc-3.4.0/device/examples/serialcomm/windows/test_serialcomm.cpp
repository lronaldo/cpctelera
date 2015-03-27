// Test program for serial communication. Visual C / MFC Version
// Bela Torok / bela.torok@kssg.ch, March 2001

#include <afxwin.h>    // Defines the entry point for the console application.

#include <string.h>
#include <stdio.h>

#include "serial.h"

int main(int argc, char* argv[])
{
	HANDLE	hComPort;
	char buf[128];

	hComPort = SerialInit("com1", 1200, 8, 1, 'N', 'N', 5000, 5000, 2, 128);

	if( hComPort == 0) {
		printf("\n\nError initializing %s!\n", "com1");
		exit(1);
	}

	// write string to RS232
	SerialPuts(&hComPort, "\nInitialize\n");
//	sleep(5000);
	
	// read string from RS232
	printf("\nString received: %s\n", SerialGets(buf, sizeof (buf) - 1, hComPort));

	CloseHandle(hComPort);

	return 0;
}

