#include <string.h>
#include <stdlib.h>
#include "fatlib.h"

int initFATPartion(int fattype,int numberofsector,FATPARTITION * fatpart)
{
	switch(fattype)
	{
		case FATLIB_FAT12:
			break;
		case FATLIB_FAT16:
			return -1;
			break;
		case FATLIB_FAT32:
			return -1;
			break;
		default:
			return -1;
			break;
	}

	if(fatpart)
	{

		fatpart->FATType=fattype;
		fatpart->rawdata=(unsigned char*)malloc(numberofsector*512);
		memset(fatpart->rawdata,0xF6,numberofsector*512);




	}
	return 0;
}

