#include <iostream>
#include <stdlib.h>
using namespace std;
#include "endianPPC.h"

bool isBigEndian(void)
{
	/* byte order array */
	char byte[8] = { (char)0x12, (char)0x36, (char)0x48, (char)0x59,(char)0x01, (char)0x23, (char)0x45, (char)0x67 };
	int *intp = (int *)byte;
	if (intp[0] == 0x12364859) { return true; }
	else
		if (intp[0] == 0x59483612) { return false; }
		else 
		{
			cerr << "Endian error , Unknown int Byte Order "<<endl;
			exit(-2);
		}
}
