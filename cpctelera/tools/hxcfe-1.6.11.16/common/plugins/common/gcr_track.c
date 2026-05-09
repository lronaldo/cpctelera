/*
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Jean-François DEL NERO
//
// This file is part of HxCFloppyEmulator.
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : gcr_track.c
// Contains: gcr track builder/encoder
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "crc.h"
#include "gcr_track.h"

const unsigned char gcrencodingtable[16]=
			{
				0x0A,0x0B,0x12,0x13,
				0x0E,0x0F,0x16,0x17,
				0x09,0x19,0x1A,0x1B,
				0x0D,0x1D,0x1E,0x15
			};

// GCR encoder
unsigned char * BuildGCRCylinder(int * gcrtracksize,char * track,char * nongcrpart,int size)
{
	int i,j,k,l;

	unsigned char byte,quartet;
	unsigned char gcrcode;
	unsigned char nongcrcode;
	unsigned char shift,shift2;

	int finalsize;
	unsigned char * finalbuffer;

	finalsize=0;
	for(i=0;i<size;i++)
	{
		if(nongcrpart[i])
		{
			finalsize=finalsize+(4*2);	
		}
		else
		{
			finalsize=finalsize+(5*2);	
		}

	}

	finalsize=finalsize*2;
	finalsize=finalsize/8;

	finalbuffer=(unsigned char *)malloc(finalsize);
	*gcrtracksize=finalsize;

	// Clean up
	for(i=0;i<(finalsize);i++)
	{
		finalbuffer[i]=0x00;
	}

	// GCR Encoding
	shift=0;
	shift2=0;
	k=0;
	j=0;
	l=0;
	do
	{
		byte=track[l];
		nongcrcode=nongcrpart[l];


		if(!((nongcrcode>>shift)&0xF))
		{

			gcrcode=gcrencodingtable[(byte>>shift)&0xF];
		
			for(j=0;j<5;j++)
			{

				if(gcrcode & (0x10>>j))
				{
					finalbuffer[k]=finalbuffer[k] | (0x80>>shift2);
				}
				else
				{
					finalbuffer[k]=finalbuffer[k] & (~(0xC0>>shift2));
				}

				shift2=shift2+2;
				if(shift2==8)
				{	
					shift2=0;
					k++;

				}
			}
		}
		else
		{//non gcr - direct copy
			quartet=(byte>>shift)&0xF;
			for(j=0;j<4;j++)
			{

				if(quartet & (0x08>>j))
				{
					finalbuffer[k]=finalbuffer[k] | (0x80>>shift2);
				}
				else
				{
					finalbuffer[k]=finalbuffer[k] & (~(0xC0>>shift2));
				}

				shift2=shift2+2;
				if(shift2==8)
				{	
					shift2=0;
					k++;

				}
			}
		}

		shift=shift+4;
		if(shift==8)
		{
			shift=0;
			l++;
		}

	}while(l<size);

	return finalbuffer;

}

int GCRGetTrackSize(unsigned int numberofsector,unsigned int sectorsize)
{
	unsigned long finalsize;

	finalsize= ( 5 + 8 + 9 + 5 + 260 + 12) * numberofsector;

	return finalsize;

}

int BuildGCRTrack(unsigned int numberofsector,unsigned int sectorsize,unsigned int tracknumber,unsigned int sidenumber,char* datain,unsigned char * mfmdata,unsigned long * mfmsizebuffer)
{
	unsigned int i,j,k,l,t;
	unsigned char *tempdata;
	
	unsigned char *temptrack;
	int temptracksize;
	unsigned char *tempnongcr;
	unsigned long finalsize;
	unsigned long indexbuffer;
	unsigned long current_buffer_size;

	/*
	  Here's the layout of a standard low-level pattern on a 1541 disk. Use the
above sample to follow along.

   1. Header sync       FF FF FF FF FF (40 'on' bits, not GCR)
   2. Header info       52 54 B5 29 4B 7A 5E 95 55 55 (10 GCR bytes)
   3. Header gap        55 55 55 55 55 55 55 55 55 (9 bytes, never read)
   4. Data sync         FF FF FF FF FF (40 'on' bits, not GCR)
   5. Data block        55...4A (325 GCR bytes)
   6. Inter-sector gap  55 55 55 55...55 55 (4 to 12 bytes, never read)
   1. Header sync       (SYNC for the next sector)


  The 10 byte header info (#2) is GCR encoded and must be decoded  to  it's
normal 8 bytes to be understood. Once decoded, its breakdown is as follows:

   Byte    $00 - header block ID ($08)
            01 - header block checksum (EOR of $02-$05)
            02 - Sector
            03 - Track
            04 - Format ID byte #2
            05 - Format ID byte #1
         06-07 - $0F ("off" bytes)

  */

	finalsize= ( 5 + 8 + 9 + 5 + 260 + 12) * numberofsector;

	indexbuffer=0;
	current_buffer_size=(int)(*mfmsizebuffer * 0.8);

	if(finalsize<=current_buffer_size)
	{
		j=0;
		tempdata= (char *)malloc((int)((*mfmsizebuffer * 0.8)+1));
		memset(tempdata,0,(int)(*mfmsizebuffer * 0.8)+1);
		tempnongcr=(char *)malloc((int)(*mfmsizebuffer * 0.8)+1);
		memset(tempnongcr,0,(int)(*mfmsizebuffer * 0.8)+1);


		// sectors
		for(l=0;l<numberofsector;l++)
		{

				// sync
				for(k=0;k<5;k++)
				{
						tempdata[j]=0xFF;
						tempnongcr[j]=0xFF;
						j++;
				}

				tempdata[j]=0x08; // $00 - header block ID ($08)
				j++;
				tempdata[j]=0x00; // header block checksum (EOR of $02-$05)
				j++; 
				tempdata[j]=l;    // Sector
				j++;
				tempdata[j]=tracknumber;  // Track
				j++;
				tempdata[j]=0xA1;  // Format ID byte #2
				j++;
				tempdata[j]=0x1A;  // Format ID byte #1
				j++;
				
				tempdata[j]=0x0F;  // $0F ("off" bytes)
				j++;
				tempdata[j]=0x0F;  // $0F ("off" bytes)
				j++;
				tempdata[j-7]=tempdata[j-6] ^ tempdata[j-5] ^ tempdata[j-4] ^ tempdata[j-3];


				for(k=0;k<9;k++) // Header gap
				{
						tempdata[j]=0x55;
						tempnongcr[j]=0xFF;
						j++;
				}

				for(k=0;k<5;k++) // Data sync
				{
						tempdata[j]=0xFF;
						tempnongcr[j]=0xFF;
						j++;
				}
				 
				tempdata[j]=0x07;  // data block ID ($07)
				j++;
				t=j+256;
				for(i=0;i<sectorsize;i++) // data sector & checksum
				{
					tempdata[j]=datain[(sectorsize*l)+i];
					tempdata[t]=tempdata[t]^tempdata[j];
					j++;
				}
				j++; // data checksum (EOR of data sector)

				tempdata[j]=0x00;  // $00 ("off" bytes)
				j++;
				tempdata[j]=0x00;  // $00 ("off" bytes)
				j++;

				
				for(k=0;k<9;k++) // sector gap
				{
						tempdata[j]=0x55;
						tempnongcr[j]=0xFF;
						j++;
				}

		}

		temptrack=BuildGCRCylinder(&temptracksize,tempdata,tempnongcr,j);
		if(temptrack)
		{
		
			memset(mfmdata,0x22,*mfmsizebuffer);
			
			if(*mfmsizebuffer>=(unsigned long)temptracksize)
			{
				memcpy(mfmdata,temptrack,temptracksize);
			
			}
			
			free(temptrack);
		}

		free(tempdata);
		free(tempnongcr);
		return 0;
	 }
	 else
	 {
		return finalsize;
	 }

}
