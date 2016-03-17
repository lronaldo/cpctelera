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
// File : HDM_DiskFile.c
// Contains: HDM X68000 floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois / VTrucco
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

#include "hdm_loader.h"

#include "../common/os_api.h"



int HDM_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	floppycontext->hxc_printf(MSG_DEBUG,"HDM_libIsValidDiskFile %s",imgfile);
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

				if((strstr( filepath,".hdm" )!=NULL) || (strstr( filepath,".xdf" )!=NULL))
				{
					floppycontext->hxc_printf(MSG_DEBUG,"HDM file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non HDM file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}



int HDM_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{

	FILE * f;
	unsigned int filesize;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned char* trackdata;
	unsigned char trackformat;
	unsigned char skew;
	int tracklen;
	unsigned char gap3len,interleave;
	unsigned short rpm;
	unsigned short sectorsize;
	unsigned int bitrate;

	CYLINDER* currentcylinder;

	floppycontext->hxc_printf(MSG_DEBUG,"HDM_libLoad_DiskFile %s",imgfile);

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
			
			sectorsize=1024; 

            bitrate=500000;
            rpm=360;
            interleave=1;
			skew=0;
            gap3len=116;
			trackformat=ISOFORMAT_DD;
            floppydisk->floppyNumberOfSide=2;
            floppydisk->floppySectorPerTrack=8;
			floppydisk->floppyNumberOfTrack=77;

			floppydisk->floppyBitRate=bitrate;
			floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

			floppycontext->hxc_printf(MSG_DEBUG,"rpm %d bitrate:%d track:%d side:%d sector:%d",rpm,bitrate,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack);
					
			tracklen=(bitrate/(rpm/60))/4;
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
					
					currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500,-2500);
				}
			}

			floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");

			fclose(f);
			return LOADER_NOERROR;
	}

	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}


