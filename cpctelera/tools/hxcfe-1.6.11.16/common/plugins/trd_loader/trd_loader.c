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
// File : trd_DiskFile.c
// Contains: IMG floppy image loader and plugins interfaces
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

#include "trd_loader.h"

#include "../common/os_api.h"

int TRD_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	floppycontext->hxc_printf(MSG_DEBUG,"TRD_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".trd" )!=NULL)
				{
					floppycontext->hxc_printf(MSG_DEBUG,"TRD file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non TRD file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}



int TRD_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned char* trackdata;
	unsigned char gap3len,interleave;
	unsigned short sectorsize,rpm;
	int number_of_track,number_of_side,number_of_sectorpertrack;
	unsigned char tempsector[256];
	unsigned char trackformat;
	unsigned char skew;

	CYLINDER* currentcylinder;
	
	floppycontext->hxc_printf(MSG_DEBUG,"TRD_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	
	fseek (f , 8*256 , SEEK_SET);				
	fread(tempsector,256,1,f);
				
	fseek (f , 0 , SEEK_SET); 

	switch(filesize)
	{
		case 16*256 * 40 * 1: // 40 track one side
			//35 track, no errors 
			number_of_track=40;
			number_of_side=1;
			number_of_sectorpertrack=16;
			break;

		case 16*256 * 40 * 2: // 40 track two side // 80 track one side
			
			//22: double-sided, 80 tracks
			//23: double-sided, 40 tracks
			//24: single-sided, 80 tracks
			//25: single-sided, 40 tracks

			switch(tempsector[0xE3])
			{
				case 22:
					number_of_track=80;
					number_of_side=2;
					number_of_sectorpertrack=16;
					break;
				case 23:
					number_of_track=40;
					number_of_side=2;
					number_of_sectorpertrack=16;
					break;
				case 24:
					number_of_track=80;
					number_of_side=1;
					number_of_sectorpertrack=16;
					break;
				case 25:
					number_of_track=40;
					number_of_side=1;
					number_of_sectorpertrack=16;
					break;

				default:
					floppycontext->hxc_printf(MSG_ERROR,"Unsupported TRD file size ! (%d Bytes)",filesize);
					fclose(f);
					return LOADER_UNSUPPORTEDFILE;
					break;
			}


			break;

		case 16*256 * 80 * 2: // 80 track two side
			number_of_track=80;
			number_of_side=2;
			number_of_sectorpertrack=16;
			break;

		default:
			switch(tempsector[0xE3])
			{
				case 22:
					number_of_track=80;
					number_of_side=2;
					number_of_sectorpertrack=16;
					break;
				case 23:
					number_of_track=40;
					number_of_side=2;
					number_of_sectorpertrack=16;
					break;
				case 24:
					number_of_track=80;
					number_of_side=1;
					number_of_sectorpertrack=16;
					break;
				case 25:
					number_of_track=40;
					number_of_side=1;
					number_of_sectorpertrack=16;
					break;

				default:
					// not supported !
					floppycontext->hxc_printf(MSG_ERROR,"Unsupported TRD file size ! (%d Bytes)",filesize);
					fclose(f);
					return LOADER_UNSUPPORTEDFILE;
					break;
			}
			break;
	}

	rpm=300;
	sectorsize=256; // TRD file support only 256bytes/sector floppies.
	gap3len=50;
	interleave=1;
	skew=0;
	floppydisk->floppyBitRate=250000;
	floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
	floppydisk->floppyNumberOfTrack=number_of_track;
	floppydisk->floppyNumberOfSide=number_of_side;
	floppydisk->floppySectorPerTrack=number_of_sectorpertrack;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
	trackformat=IBMFORMAT_DD;		
	floppycontext->hxc_printf(MSG_DEBUG,"rpm %d bitrate:%d track:%d side:%d sector:%d",rpm,floppydisk->floppyBitRate,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack);
			
	trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
	for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
	{	
		floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
		currentcylinder=floppydisk->tracks[j];
				
		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{
					
			file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
						(sectorsize*(floppydisk->floppySectorPerTrack)*i);			
					
			fseek (f , file_offset , SEEK_SET);
			fread(trackdata,sectorsize*floppydisk->floppySectorPerTrack,1,f);
					
			currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)0,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2000,-2000);
		}
	}
			
			
	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");

	fclose(f);
	return LOADER_NOERROR;
}
			

