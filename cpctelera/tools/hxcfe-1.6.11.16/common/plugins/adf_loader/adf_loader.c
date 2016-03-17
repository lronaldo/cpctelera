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
// File : ADF_DiskFile.c
// Contains: ADF floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"

#include "../common/crc.h"
#include "../common/track_generator.h"


#include "adf_loader.h"

#include "../common/os_api.h"

int ADF_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	FILE * f;
	char * filepath;
	unsigned int filesize;

	floppycontext->hxc_printf(MSG_DEBUG,"ADF_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".adf" )!=NULL)
				{
					floppycontext->hxc_printf(MSG_DEBUG,"ADF file !");
					free(filepath);

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}

					fseek (f , 0 , SEEK_END); 
					filesize=ftell(f);
					fseek (f , 0 , SEEK_SET); 
					fclose(f);

					if(filesize%(512*11))
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non ADF file ! Bad file size!");
						return LOADER_BADFILE;
					}

					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non ADF file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}

int ADF_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	unsigned int filesize;
	unsigned int i,j;
	unsigned short rpm;
	unsigned int file_offset;
	char* trackdata;
	unsigned char gap3len,skew,trackformat,interleave;
	unsigned short sectorsize;
	CYLINDER* currentcylinder;
	
	floppycontext->hxc_printf(MSG_DEBUG,"ADF_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 
	
	if(!filesize)
	{
		floppycontext->hxc_printf(MSG_ERROR,"Bad file size : %d !",filesize);
		fclose(f);
		return LOADER_BADFILE;
	}
	
	if(filesize<100*11*2*512)
	{
		floppydisk->floppySectorPerTrack=11;
		rpm=DEFAULT_AMIGA_RPM;
	}
	else
	{
		floppydisk->floppySectorPerTrack=22;
		rpm=DEFAULT_AMIGA_RPM/2;
	}

	floppydisk->floppyNumberOfSide=2;

	if((filesize/(512*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))<80)
		floppydisk->floppyNumberOfTrack=80;
	else
		floppydisk->floppyNumberOfTrack=filesize/(512*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide);
	
	floppydisk->floppyBitRate=DEFAULT_AMIGA_BITRATE;
	floppydisk->floppyiftype=AMIGA_DD_FLOPPYMODE;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	sectorsize=512;
	interleave=1;
	gap3len=0;
	skew=0;
	trackformat=AMIGAFORMAT_DD;

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

			currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,0,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500,-11150);
		}
	}

	free(trackdata);
	
	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
	fclose(f);
	return LOADER_NOERROR;	
}


