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
// File : emui_raw_loader.c
// Contains: EmuII floppy image loader and plugins interfaces
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

#include "../common/emuii_track.h"
#include "emui_raw_loader.h"
#include "../common/track_generator.h"

#include "../common/os_api.h"

int EMUI_RAW_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,filesize;
	char * filepath;
	FILE *f;

	floppycontext->hxc_printf(MSG_DEBUG,"EMUI_RAW_libIsValidDiskFile %s",imgfile);

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

				if(strstr( filepath,".emufd" )!=NULL)
				{

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						return LOADER_ACCESSERROR;
					}

					fseek (f , 0 , SEEK_END); 
					filesize=ftell(f);
					fseek (f , 0 , SEEK_SET); 
					
					fclose(f);
					
					if(filesize==(0xE00*35))
					{
						floppycontext->hxc_printf(MSG_DEBUG,"EmuI raw file !");
						free(filepath);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non EmuI raw file !");
						free(filepath);
						return LOADER_BADFILE;
					}
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non EmuI raw file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}

int EMUI_RAW_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	unsigned int i;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	unsigned char sector_data[0xE00];
	int tracknumber,sidenumber;
	
	
	floppycontext->hxc_printf(MSG_DEBUG,"EMUI_RAW_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	floppydisk->floppyNumberOfTrack=35;
	floppydisk->floppyNumberOfSide=1;
	floppydisk->floppyBitRate=DEFAULT_EMUII_BITRATE;
	floppydisk->floppySectorPerTrack=1;
	floppydisk->floppyiftype=EMU_SHUGART_FLOPPYMODE;

	floppycontext->hxc_printf(MSG_DEBUG,"EmuI File : %d track, %d side, %d bit/s, %d sectors, mode %d",
		floppydisk->floppyNumberOfTrack,
		floppydisk->floppyNumberOfSide,
		floppydisk->floppyBitRate,
		floppydisk->floppySectorPerTrack,
		floppydisk->floppyiftype);


	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
	memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	for(i=0;i<floppydisk->floppyNumberOfTrack;i++)
	{			

		tracknumber=i;
		sidenumber=0;
	
		fseek(f,i*0xE00,SEEK_SET);
		fread(&sector_data,0xE00,1,f);

		if(!floppydisk->tracks[tracknumber])
		{
			floppydisk->tracks[tracknumber]=allocCylinderEntry(300,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[tracknumber];
		}
			

		floppycontext->hxc_printf(MSG_DEBUG,"read track %d side %d at offset 0x%x (0x%x bytes)",
			tracknumber,
			sidenumber,
			(0xE00*tracknumber*2)+(sidenumber*0xE00),
			0xE00);


			currentcylinder->sides[sidenumber]=tg_alloctrack(floppydisk->floppyBitRate,EMU_FM_ENCODING,currentcylinder->floppyRPM,((floppydisk->floppyBitRate/5)*2),2000,-2000,0x00);
			currentside=currentcylinder->sides[sidenumber];					
			currentside->number_of_sector=floppydisk->floppySectorPerTrack;
			
			BuildEmuIITrack(floppycontext,tracknumber,sidenumber,sector_data,currentside->databuffer,&currentside->tracklen,1);
	}			
	
	fclose(f);
	return LOADER_NOERROR;
}

