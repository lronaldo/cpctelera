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
// File : ti99pc99_loader.c
// Contains: TI99 PC99 floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"

#include "../common/crc.h"
#include "../common/track_generator.h"

#include "ti99pc99_loader.h"

#include "../common/os_api.h"


int TI99PC99_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int filesize;
	FILE * f;

	floppycontext->hxc_printf(MSG_DEBUG,"TI99PC99_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{

		f=fopen(imgfile,"rb");
		if(f==NULL) 
		{
			floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
			return -1;
		}
		
		fseek (f , 0 , SEEK_END); 
		filesize=ftell(f);
		fseek (f , 0 , SEEK_SET); 
		
		fclose(f);
		
		if(filesize%3253 && filesize%6872)
		{
			floppycontext->hxc_printf(MSG_DEBUG,"non TI99 PC99 file !");
			return LOADER_BADFILE;
		}
			
		floppycontext->hxc_printf(MSG_DEBUG,"TI99 PC99 file !");
		return LOADER_ISVALID;
		
	}
	
	return LOADER_BADPARAMETER;
}

int patchtrackFM(unsigned char * trackdata, unsigned char * trackclk,int tracklen)
{
	int i,j,k,l;
	int sectorsize;
	int nbofsector;
	unsigned char crctable[32];
	unsigned char CRC16_High,CRC16_Low;
	
	nbofsector=0;
	i=0;
	do
	{
		if(trackdata[i]==0xFE)
		{
			trackclk[i]=0xC7;
			if( (trackdata[i+5]==0xF7) && (trackdata[i+6]==0xF7) )
			{
				// calculate header crc
				sectorsize=128<<trackdata[i+4];
				l=i;
				CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
				for(k=0;k<5;k++)
				{
					CRC16_Update(&CRC16_High,&CRC16_Low, trackdata[l],(unsigned char*)crctable );
					l++;
				}
				trackdata[i+5]=CRC16_High;
				trackdata[i+6]=CRC16_Low;


				j=12;
				do
				{
					if((trackdata[i+j]==0xFB) || (trackdata[i+j]==0xF8))
					{
						trackclk[i+j]=0xC7;

						if((trackdata[i+j+sectorsize+1]==0xF7) && (trackdata[i+j+sectorsize+2]==0xF7))
						{
							// calculate data crc
							//02 CRC The sector Header CRC
							CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
							for(k=0;k<sectorsize+1;k++)
							{
								CRC16_Update(&CRC16_High,&CRC16_Low, trackdata[i+j+k],(unsigned char*)crctable );
							}

							trackdata[i+j+sectorsize+1]=CRC16_High;
							trackdata[i+j+sectorsize+2]=CRC16_Low;
							i=i+j+sectorsize+3;
							j=32;
							nbofsector++;
						}

					}
		
					j++;
				}while(j<32);
			}
		}

		i++;
	}while(i<tracklen);

	return nbofsector;
}


int patchtrackMFM(unsigned char * trackdata, unsigned char * trackclk,int tracklen)
{
	int i,j,k,l;
	int sectorsize;
	int nbofsector;
	unsigned char crctable[32];
	unsigned char CRC16_High,CRC16_Low;
	
	nbofsector=0;
	i=0;
	do
	{

		if(trackdata[i]==0xA1 && trackdata[i+1]==0xA1 && trackdata[i+2]==0xA1 && trackdata[i+3]==0xFE)
		{
			l=i;
			trackclk[i]=0x0A;
			trackclk[i+1]=0x0A;
			trackclk[i+2]=0x0A;
			trackclk[i+3]=0xFF;
			i=i+3;

			if( (trackdata[i+5]==0xF7) && (trackdata[i+6]==0xF7) )
			{
				// calculate header crc
				sectorsize=128<<trackdata[i+4];

				CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
				for(k=0;k<8;k++)
				{
					CRC16_Update(&CRC16_High,&CRC16_Low, trackdata[l],(unsigned char*)crctable );
					l++;
				}
				trackdata[i+5]=CRC16_High;
				trackdata[i+6]=CRC16_Low;


				j=0;
				do
				{
					if(trackdata[i+j]==0xA1 && trackdata[i+j+1]==0xA1 && trackdata[i+j+2]==0xA1 && trackdata[i+j+3]==0xFB)
					{
						trackclk[i+j]=0x0A;
						trackclk[i+j+1]=0x0A;
						trackclk[i+j+2]=0x0A;
						trackclk[i+j+3]=0xFF;
						i=i+j+3+1;
						if((trackdata[i+sectorsize]==0xF7) && (trackdata[i+sectorsize+1]==0xF7))
						{
							// calculate data crc
							//02 CRC The sector Header CRC
							CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
							for(k=0;k<sectorsize+4;k++)
							{
								CRC16_Update(&CRC16_High,&CRC16_Low, trackdata[i+k-4],(unsigned char*)crctable );
							}

							trackdata[i+sectorsize]=CRC16_High;
							trackclk[i+sectorsize]=0xFF;
							trackdata[i+sectorsize+1]=CRC16_Low;
							trackclk[i+sectorsize+1]=0xFF;
							i=i+sectorsize+2;
							j=64;
							nbofsector++;
						}

					}
		
					j++;
				}while(j<64);

				

			}


		}

		i++;
	}while(i<tracklen);

	return nbofsector;
}

int TI99PC99_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned char* trackdata;
	unsigned char* trackclk;
	unsigned short rpm;
	int fmmode,tracklen,numberoftrack,numberofside;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	
	
	floppycontext->hxc_printf(MSG_DEBUG,"TI99PC99_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}

	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 


	if(filesize!=0)
	{		
			
		fmmode=0;

		if(filesize%3253 && filesize%6872)
		{
			floppycontext->hxc_printf(MSG_DEBUG,"non TI99 PC99 file !");
			return LOADER_BADFILE;
		}
		
		if(!(filesize%3253))
		{
			tracklen=3253;
			fmmode=1;
			floppycontext->hxc_printf(MSG_DEBUG,"FM TI99 PC99 file !");
			floppydisk->floppyBitRate=250000;
		}
		else
		{
			tracklen=6872;
			fmmode=0;
			floppycontext->hxc_printf(MSG_DEBUG,"MFM TI99 PC99 file !");
			floppydisk->floppyBitRate=250000;
		}

		switch(filesize/tracklen)
		{
		case 40:
			numberofside=1;
			numberoftrack=40;
			break;
		case 80:
			numberofside=2;
			numberoftrack=40;
			break;
		default:
			floppycontext->hxc_printf(MSG_ERROR,"Unsupported geometry!");
			fclose(f);
			return LOADER_BADFILE;
			break;
		}
			
		floppydisk->floppyNumberOfTrack=numberoftrack;
		floppydisk->floppyNumberOfSide=numberofside;
		floppydisk->floppySectorPerTrack=-1;
		floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
		rpm=300; // normal rpm
			
		floppycontext->hxc_printf(MSG_INFO_1,"filesize:%dkB, %d tracks, %d side(s), %d sectors/track, rpm:%d",filesize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,rpm);
				
		trackdata=(unsigned char*)malloc(tracklen);
		trackclk=(unsigned char*)malloc(tracklen);
			
		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
				
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
				
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
					
				if(fmmode)
				{	
					floppydisk->tracks[j]->sides[i]=tg_alloctrack(floppydisk->floppyBitRate,ISOIBM_FM_ENCODING ,currentcylinder->floppyRPM,tracklen*4*8,2500,-2500,0x00);

				}
				else
				{
					floppydisk->tracks[j]->sides[i]=tg_alloctrack(floppydisk->floppyBitRate,ISOIBM_MFM_ENCODING,currentcylinder->floppyRPM,tracklen*2*8,2500,-2500,0x00);
				}
				currentside=currentcylinder->sides[i];
				
				currentside->number_of_sector=floppydisk->floppySectorPerTrack;
					
				file_offset=(tracklen*j)+(tracklen*numberoftrack*(i&1));					
				fseek (f , file_offset , SEEK_SET);
					
				fread(trackdata,tracklen,1,f);
				memset(trackclk,0xFF,tracklen);


				if(fmmode)
				{
					patchtrackFM(trackdata,trackclk,tracklen);
					BuildFMCylinder(currentside->databuffer,currentside->tracklen/8,trackclk,trackdata,tracklen);
				}
				else
				{
					patchtrackMFM(trackdata,trackclk,tracklen);
					BuildCylinder(currentside->databuffer,currentside->tracklen/8,trackclk,trackdata,tracklen);
				}

			}

		}
		
		free(trackdata);
		
		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		
		fclose(f);
		return LOADER_NOERROR;
	}

	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}
