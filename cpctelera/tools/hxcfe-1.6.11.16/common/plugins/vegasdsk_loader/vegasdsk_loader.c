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
// File : vegasdsk_loader.c
// Contains: vegas floppy image loader and plugins interfaces
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

#include "vegasdsk_loader.h"

#include "../common/os_api.h"

int VEGASDSK_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,filesize;
	char * filepath;
	unsigned char buffer[256];
	FILE * f;
	floppycontext->hxc_printf(MSG_DEBUG,"EMAX_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{
		pathlen=strlen(imgfile);
		if(pathlen!=0)
		{
			filepath=malloc(pathlen+1);
			if(filepath!=0)
			{
				sprintf(filepath,"%s",imgfile);
				strlower(filepath);
				
				if((strstr( filepath,".vegasdsk" )!=NULL) || (strstr( filepath,".veg" )!=NULL))
				{

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}
					
					fseek (f , 0 , SEEK_END);
					filesize=ftell(f);
					fseek (f , 256*(3-1) , SEEK_SET);
					fread(buffer,256,1,f);

					fclose(f);

					floppycontext->hxc_printf(MSG_DEBUG,"Vegas DSK file ! %d tracks %d sectors/tracks",buffer[0x26]+1,buffer[0x27]+1);
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non Vegas DSK file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int VEGASDSK_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j,k;
	unsigned int file_offset,offset;
	unsigned char* floppy_data;
	unsigned char  gap3len,interleave;
	unsigned short sectorsize,rpm;
	unsigned short bootnumberofsector;
	unsigned char  trackformat;
	unsigned char  buffer[256];
	unsigned char  ddmode;


	CYLINDER* currentcylinder;
	SECTORCONFIG  sectorconfig[30];

	floppycontext->hxc_printf(MSG_DEBUG,"VEGASDSK_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	
	fseek (f , 256*(3-1) , SEEK_SET);
	fread(buffer,256,1,f);

	fseek (f , 0 , SEEK_SET); 
	
	gap3len=255;
	
	
	switch(buffer[0x27])
	{
		case 10:
			floppydisk->floppySectorPerTrack=10;
			bootnumberofsector=10;
			sectorsize=256;
			floppydisk->floppyNumberOfSide=1;
			interleave=5;
			trackformat=ISOFORMAT_SD;
			break;
		case 20:
			floppydisk->floppySectorPerTrack=10;
			bootnumberofsector=10;
			sectorsize=256;
			floppydisk->floppyNumberOfSide=2;
			interleave=5;
			trackformat=ISOFORMAT_SD;
			break;
		case 18:
			floppydisk->floppySectorPerTrack=18;
			gap3len=20;
			bootnumberofsector=10;
			sectorsize=256;
			ddmode=0xFF;
			floppydisk->floppyNumberOfSide=1;
			interleave=9;
			trackformat=ISOFORMAT_DD;
			break;
		case 36:
			floppydisk->floppySectorPerTrack=18;
			gap3len=20;
			bootnumberofsector=10;
			sectorsize=256;
			ddmode=0xFF;
			floppydisk->floppyNumberOfSide=2;
			interleave=9;
			trackformat=ISOFORMAT_DD;
			break;
		default:
			floppydisk->floppySectorPerTrack=10;
			bootnumberofsector=10;
			sectorsize=256;
			floppydisk->floppyNumberOfSide=1;
			interleave=5;
			trackformat=ISOFORMAT_SD;
			break;
	}

	floppydisk->floppyNumberOfTrack=buffer[0x26]+1;


			
		
	floppydisk->floppyBitRate=250000;
	floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
	rpm=300; // normal rpm
			
	floppycontext->hxc_printf(MSG_INFO_1,"filesize:%dkB, %d tracks, %d side(s), %d sectors/track, gap3:%d, interleave:%d,rpm:%d",filesize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,gap3len,interleave,rpm);
	
	j=0;
	floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
	currentcylinder=floppydisk->tracks[j];
	for(i=0;i<floppydisk->floppyNumberOfSide;i++)
	{
		file_offset=( (((bootnumberofsector)*256)) * i );
		memset(sectorconfig,0,sizeof(SECTORCONFIG)*10);
		for(k=0;k<10;k++)
		{
			sectorconfig[k].head=i;
			sectorconfig[k].cylinder=j;
			sectorconfig[k].sector=k+1;//+ (numberofsector
			sectorconfig[k].sectorsize=256;
			sectorconfig[k].bitrate=floppydisk->floppyBitRate;
			sectorconfig[k].gap3=255;
			sectorconfig[k].trackencoding=ISOFORMAT_SD;
			sectorconfig[k].input_data=malloc(sectorconfig[k].sectorsize);
			fread(sectorconfig[k].input_data,256,1,f);
			
		}	

		currentcylinder->sides[i]=tg_generatetrackEx(10,(SECTORCONFIG *)&sectorconfig,5,0,floppydisk->floppyBitRate,rpm,ISOFORMAT_SD,2500,-2500);

		for(k=0;k<10;k++)
		{
			free(sectorconfig[k].input_data);
		}
	}

	offset=ftell(f);

	floppy_data=malloc(floppydisk->floppySectorPerTrack*sectorsize);

	for(j=1;j<floppydisk->floppyNumberOfTrack;j++)
	{	
		floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
		currentcylinder=floppydisk->tracks[j];
				
		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{
					
			file_offset=offset+ ( (((floppydisk->floppySectorPerTrack)*256)) * floppydisk->floppyNumberOfSide * (j-1) ) +
				                ( (((floppydisk->floppySectorPerTrack)*256)) * i );
			fread(floppy_data,floppydisk->floppySectorPerTrack*sectorsize,1,f);

			memset(sectorconfig,0,sizeof(SECTORCONFIG)*floppydisk->floppySectorPerTrack);
			for(k=0;k<floppydisk->floppySectorPerTrack;k++)
			{
				sectorconfig[k].head=i;
				sectorconfig[k].cylinder=j;
				sectorconfig[k].sector=k+1 + (floppydisk->floppySectorPerTrack * i);
				sectorconfig[k].sectorsize=sectorsize;
				sectorconfig[k].bitrate=floppydisk->floppyBitRate;
				sectorconfig[k].gap3=gap3len;
				sectorconfig[k].trackencoding=trackformat;
				sectorconfig[k].input_data=&floppy_data[k*256];
			}

			currentcylinder->sides[i]=tg_generatetrackEx(floppydisk->floppySectorPerTrack,(SECTORCONFIG *)&sectorconfig,interleave,0,floppydisk->floppyBitRate,rpm,trackformat,2500,-2500);
		}
	}

	free(floppy_data);

	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		
	fclose(f);
	return LOADER_NOERROR;
}
