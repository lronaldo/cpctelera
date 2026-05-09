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
// File : JVC_DiskFile.c
// Contains: JVC floppy image loader and plugins interfaces
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

#include "jvc_loader.h"

#include "../common/os_api.h"


int JVC_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;

	floppycontext->hxc_printf(MSG_DEBUG,"JVC_libIsValidDiskFile %s",imgfile);
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
				
				if(strstr( filepath,".jvc" )!=NULL)
				{
					floppycontext->hxc_printf(MSG_DEBUG,"JVC file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non JVC file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}

typedef struct jvc_header_
{
  unsigned char Setors_per_track;
  unsigned char Side_count;
  unsigned char Sector_size_code;
  unsigned char First_sector_ID;
  unsigned char Sector_attribute_flag;
}jvc_header;


int JVC_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j,k,skew;
	unsigned int file_offset;
	unsigned char* trackdata;
	int headerSize;
	unsigned char  gap3len,interleave;
	unsigned short sectorsize,rpm;
	jvc_header jvc_h;
	unsigned char Sector_attribute_flag;
	CYLINDER* currentcylinder;
	unsigned char trackformat;
	floppycontext->hxc_printf(MSG_DEBUG,"JVC_libLoad_DiskFile %s",imgfile);
	
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
		headerSize = filesize % 256;

		// default values
		jvc_h.Setors_per_track=18;
		jvc_h.Side_count=1;
		jvc_h.Sector_size_code=1;
		jvc_h.First_sector_ID=1;
		jvc_h.Sector_attribute_flag=0;

		fread(&jvc_h, headerSize,1,f);

		sectorsize=128<<jvc_h.Sector_size_code;

		floppydisk->floppySectorPerTrack=jvc_h.Setors_per_track;
		floppydisk->floppyNumberOfSide= jvc_h.Side_count;
		if( jvc_h.Sector_attribute_flag)
		{
			floppydisk->floppyNumberOfTrack=(filesize - headerSize) / (jvc_h.Setors_per_track * ((128 << jvc_h.Sector_size_code) + 1) ) / jvc_h.Side_count;
		}
		else
		{
			floppydisk->floppyNumberOfTrack=(filesize - headerSize) / (jvc_h.Setors_per_track * (128 << jvc_h.Sector_size_code)) / jvc_h.Side_count;
		}


		floppydisk->floppyBitRate=DEFAULT_DD_BITRATE;
		floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
		
		trackformat=ISOFORMAT_DD;
		skew=0;
		interleave=1;
		gap3len=255;

		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
		rpm=300; // normal rpm

		floppycontext->hxc_printf(MSG_INFO_1,"filesize:%dkB, %d tracks, %d side(s), %d sectors/track, sector size: %dB , gap3:%d, interleave:%d,rpm:%d bitrate:%d",filesize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,sectorsize,gap3len,interleave,rpm,floppydisk->floppyBitRate);

		trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
				
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
				
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
				if(jvc_h.Sector_attribute_flag)
					file_offset=((sectorsize+1)*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
						((sectorsize+1)*(floppydisk->floppySectorPerTrack)*i);
				else
					file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
						(sectorsize*(floppydisk->floppySectorPerTrack)*i);
					
				fseek (f , file_offset , SEEK_SET);
						
				Sector_attribute_flag=0;
				for(k=0;k<jvc_h.Setors_per_track;k++)
				{
					if(jvc_h.Sector_attribute_flag)
					{

						fread(&Sector_attribute_flag,1,1,f);
					}

					fread(&trackdata[k*sectorsize],sectorsize,1,f);
				}

				currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500 | NO_SECTOR_UNDER_INDEX,-2500);
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
