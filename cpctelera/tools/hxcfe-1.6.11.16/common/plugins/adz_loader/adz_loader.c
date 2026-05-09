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
// File : ADZ_DiskFile.c
// Contains: ADZ floppy image loader and plugins interfaces
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

#include "../common/track_generator.h"

#include "adz_loader.h"

#include "./libs/zlib/zlib.h"

#include "../common/os_api.h"

#define UNPACKBUFFER 128*1024

int ADZ_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	floppycontext->hxc_printf(MSG_DEBUG,"ADZ_libIsValidDiskFile %s",imgfile);
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
				
				if((strstr( filepath,".adz" )!=NULL) || (strstr( filepath,".adf.gz" )!=NULL))
				{
					floppycontext->hxc_printf(MSG_DEBUG,"ADZ file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non ADZ file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int ADZ_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	unsigned int filesize;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned short sectorsize;
	unsigned char gap3len,skew,trackformat,interleave;
	char* flatimg;
	gzFile file;
	int err;
	
	CYLINDER* currentcylinder;
	
	floppycontext->hxc_printf(MSG_DEBUG,"ADZ_libLoad_DiskFile %s",imgfile);
	
	file = gzopen(imgfile, "rb");
	if (!file)
	{
		floppycontext->hxc_printf(MSG_ERROR,"gzopen: Error while reading the file!");
		return -1;
	}
	
	i=0;
	filesize=0;
	flatimg=(char*)malloc(UNPACKBUFFER);
	do
	{
		err=gzread(file, flatimg+filesize,UNPACKBUFFER );
		filesize=filesize+err;
		flatimg=(char *)realloc(flatimg,filesize+UNPACKBUFFER);
		i++;
	}while(err>0);
	
	gzclose(file);

	if(!flatimg)
	{
		floppycontext->hxc_printf(MSG_ERROR,"Unpack error!");
		return LOADER_BADFILE;
	}	
	
	if(flatimg)
	{		
		sectorsize=512;
		interleave=1;
		gap3len=0;
		skew=0;
		trackformat=AMIGAFORMAT_DD;

		floppydisk->floppySectorPerTrack=11;
		floppydisk->floppyNumberOfSide=2;
		floppydisk->floppyNumberOfTrack=(filesize/(512*2*floppydisk->floppySectorPerTrack));
		floppydisk->floppyBitRate=DEFAULT_AMIGA_BITRATE;
		floppydisk->floppyiftype=AMIGA_DD_FLOPPYMODE;
		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
		

		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
			floppydisk->tracks[j]=allocCylinderEntry(DEFAULT_AMIGA_RPM,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
			
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
								
				file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
							(sectorsize*(floppydisk->floppySectorPerTrack)*i);
				
				currentcylinder->sides[i]=tg_generatetrack(&flatimg[file_offset],sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,0,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500,-11150);
			}
		}
		
		floppycontext->hxc_printf(MSG_INFO_1,"ADZ Loader : tracks file successfully loaded and encoded!");
		return 0;
	}

	return LOADER_BADFILE;
}

