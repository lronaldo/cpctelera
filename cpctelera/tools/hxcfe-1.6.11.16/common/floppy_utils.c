#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"

unsigned long us2index(unsigned long startindex,SIDE * track,unsigned long us,unsigned char fill,char fillorder)
{
	uint32_t time,freq;

	if(!fillorder)
	{
		if(track->bitrate==VARIABLEBITRATE)
		{
			time=0;
			do
			{

				if(fill)track->indexbuffer[startindex>>3]=0xFF;
				freq=track->timingbuffer[startindex>>3];
				startindex++;
				if(startindex>=track->tracklen) startindex=0;
				if(freq)
					time=time+(((1000000000/2)/freq)*1);
			}while(us>(time/1000));
			return startindex;
		}
		else
		{
			freq=track->bitrate;
			time=0;
			do
			{
				if(fill)track->indexbuffer[startindex>>3]=0xFF;
				startindex++;
				if(startindex>=track->tracklen) startindex=0;
				if(freq)
					time=time+(((1000000000/2)/freq)*1);
			}while(us>(time/1000));
			return startindex;
		}
	}
	else
	{
		if(track->bitrate==VARIABLEBITRATE)
		{
			time=0;
			do
			{

				if(fill)track->indexbuffer[startindex>>3]=0xFF;
				freq=track->timingbuffer[startindex>>3];

				if(startindex)
					startindex--;
				else
					startindex=track->tracklen-1;
				if(freq)
					time=time+(((1000000000/2)/freq)*1);
			}while(us>(time/1000));
			return startindex;
		}
		else
		{
			freq=track->bitrate;
			time=0;
			do
			{
				if(fill)track->indexbuffer[startindex>>3]=0xFF;

				if(startindex)
					startindex--;
				else
					startindex=track->tracklen-1;
				if(freq)
					time=time+(((1000000000/2)/freq)*1);
			}while(us>(time/1000));
			return startindex;
		}
	}
};

unsigned long fillindex(int startindex,SIDE * track,unsigned long us,unsigned char fill,char fillorder)
{
	int start_index;

	if(startindex>=0)
	{
		start_index=us2index(0,track,startindex,0,0);
	}
	else
	{
		start_index=us2index(0,track,-startindex,0,1);
	}


	return us2index(start_index,track,us&0xFFFFFF,fill,fillorder);
}

CYLINDER* allocCylinderEntry(unsigned short rpm,unsigned char number_of_side)
{
	CYLINDER* cyl;

	cyl=(CYLINDER*)malloc(sizeof(CYLINDER));
	cyl->floppyRPM=rpm;
	cyl->number_of_side=number_of_side;
	cyl->sides=(SIDE**)malloc(sizeof(SIDE*)*number_of_side);
	memset(cyl->sides,0,sizeof(SIDE*)*number_of_side);
	return cyl;
}

void savebuffer(unsigned char * name,unsigned char * buffer, int size)
{
	FILE * f;

	f=fopen(name,"w+b");
	if(f)
	{
		fwrite(buffer,size,1,f);
		fclose(f);
	}
}
