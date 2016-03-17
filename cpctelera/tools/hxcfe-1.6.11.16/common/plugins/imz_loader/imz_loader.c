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
// File : IMZ_DiskFile.c
// Contains: IMZ floppy image loader and plugins interfaces
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

#include "imz_loader.h"

#include "./libs/zlib/zlib.h"
#include "./libs/zlib/contrib/minizip/unzip.h"

#include "../common/os_api.h"

#define UNPACKBUFFER 128*1024

extern int pc_imggetfloppyconfig(unsigned char * img,unsigned int filesize,unsigned short *numberoftrack,unsigned char *numberofside,unsigned short *numberofsectorpertrack,unsigned char *gap3len,unsigned char *interleave,unsigned short *rpm, unsigned int *bitrate,unsigned short * ifmode);

int IMZ_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,err;
	char * filepath;
	unzFile uf;
	unz_file_info file_info;
	char filename_inzip[256];

	floppycontext->hxc_printf(MSG_DEBUG,"IMZ_libIsValidDiskFile %s",imgfile);
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
				
				if((strstr( filepath,".imz" )!=NULL))
				{

					uf=unzOpen (imgfile);
					if (!uf)
					{
						floppycontext->hxc_printf(MSG_ERROR,"unzOpen: Error while reading the file!");
						free(filepath);
						return LOADER_BADFILE;
					}

					err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
					if (err!=UNZ_OK)
					{
						unzClose(uf);
						return LOADER_BADFILE;
					}

					unzClose(uf);
					floppycontext->hxc_printf(MSG_DEBUG,"IMZ file : %s (%d bytes) !",filename_inzip,file_info.uncompressed_size);
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non IMZ file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int IMZ_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	unsigned int filesize;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned short sectorsize;
	unsigned char gap3len,skew,trackformat,interleave;
	char filename_inzip[256];
	char* flatimg;
	int err=UNZ_OK;
	unzFile uf;
	unz_file_info file_info;
	CYLINDER* currentcylinder;
	unsigned short rpm;
	
	floppycontext->hxc_printf(MSG_DEBUG,"IMZ_libLoad_DiskFile %s",imgfile);
	
	uf=unzOpen (imgfile);
	if (!uf)
	{
		floppycontext->hxc_printf(MSG_ERROR,"unzOpen: Error while reading the file!");
		return LOADER_BADFILE;
	}
	
	unzGoToFirstFile(uf);

    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
	if (err!=UNZ_OK)
    {
		unzClose(uf);
		return LOADER_BADFILE;
	}

	err=unzOpenCurrentFile(uf);
	if (err!=UNZ_OK)
    {
		unzClose(uf);
		return LOADER_BADFILE;
	}

	filesize=file_info.uncompressed_size;
	flatimg=(char*)malloc(filesize);
	if(!flatimg)
	{
		floppycontext->hxc_printf(MSG_ERROR,"Unpack error!");
		return LOADER_BADFILE;
	}	
	
	err=unzReadCurrentFile  (uf, flatimg, filesize);
	if (err<0)
	{
		floppycontext->hxc_printf(MSG_ERROR,"error %d with zipfile in unzReadCurrentFile",err);
		unzClose(uf);
		free(flatimg);
		return LOADER_BADFILE;
	}
	
	unzClose(uf);

	if(pc_imggetfloppyconfig(
			flatimg,
			filesize,
			&floppydisk->floppyNumberOfTrack,
			&floppydisk->floppyNumberOfSide,
			&floppydisk->floppySectorPerTrack,
			&gap3len,
			&interleave,
			&rpm,
			&floppydisk->floppyBitRate,
			&floppydisk->floppyiftype)==1
			)
	{		
		sectorsize=512;

		skew=0;
		trackformat=IBMFORMAT_DD;

		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
			
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
				file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
							(sectorsize*(floppydisk->floppySectorPerTrack)*i);

				currentcylinder->sides[i]=tg_generatetrack(&flatimg[file_offset],sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500,-11150);
			}
		}

		floppycontext->hxc_printf(MSG_INFO_1,"IMZ Loader : tracks file successfully loaded and encoded!");
		free(flatimg);
		return 0;
	}
	free(flatimg);
	return LOADER_BADFILE;
}

