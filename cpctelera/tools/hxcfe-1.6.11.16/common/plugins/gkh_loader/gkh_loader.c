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
// File : gkh_loader.c
// Contains: Ensoniq GKH floppy image loader and plugins interfaces
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

#include "gkh_loader.h"
#include "gkh_format.h"

#include "../common/os_api.h"



int GKH_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	gkh_header header;
	FILE * f;

	floppycontext->hxc_printf(MSG_DEBUG,"GKH_libIsValidDiskFile %s",imgfile);
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
				
				if(strstr( filepath,".gkh" )!=NULL)
				{
					free(filepath);
					
					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}

					fread(&header,sizeof(header),1,f);

					fclose(f);
					if(!memcmp(&header.header_tag,"TDDFI",5))
					{
						floppycontext->hxc_printf(MSG_DEBUG,"GKH file !");
					}
					else
					{
						floppycontext->hxc_printf(MSG_ERROR,"Bad header !!");
						return LOADER_BADFILE;
					}

					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non GKH file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int GKH_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int i,j;
	unsigned char gap3len,interleave,startid;
	unsigned short rpm,sectorsize;
	CYLINDER* currentcylinder;
	int data_offset;
	unsigned char trackformat;
	unsigned char skew;	
	gkh_header header;
	unsigned char tagbuffer[10];
	image_type_tag *img_type_tag;
	image_location_tag *img_location_tag;
	unsigned char * trackdata;
	int file_offset;

	floppycontext->hxc_printf(MSG_DEBUG,"GKH_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	data_offset=58;
	fread(&header,sizeof(header),1,f);

	if(!memcmp(&header.header_tag,"TDDFI",5) && header.version==1)
	{

		i=0;
		do
		{
			fread(&tagbuffer,10,1,f);
			switch(tagbuffer[0])
			{
				case 0x0A:
					img_type_tag=(image_type_tag *)&tagbuffer;
					sectorsize=img_type_tag->sectorsize;
					floppydisk->floppyNumberOfTrack=img_type_tag->nboftrack;
					floppydisk->floppyNumberOfSide=(unsigned char)img_type_tag->nbofheads;
					floppydisk->floppySectorPerTrack=img_type_tag->nbofsectors;
				break;

				case 0x0B:
					img_location_tag=(image_location_tag *)&tagbuffer;
					data_offset=img_location_tag->fileoffset;
				break;
			}
			i++;
		}while(i<header.numberoftags);

		floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
		floppydisk->floppyBitRate=250000;
		rpm=300;
		
		trackformat=IBMFORMAT_DD;
		
		skew=2;
		startid=0;
		gap3len=255;
		interleave=1;

		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);	
		floppycontext->hxc_printf(MSG_INFO_1,"%d tracks, %d side(s), %d sectors/track, gap3:%d, interleave:%d,rpm:%d",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,gap3len,interleave,rpm);

		trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
				
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
							
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
					
				file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
					        (sectorsize*(floppydisk->floppySectorPerTrack)*i)+
							data_offset;

				fseek (f , file_offset , SEEK_SET);
				fread(trackdata,sectorsize*floppydisk->floppySectorPerTrack,1,f);

				currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,startid,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500|NO_SECTOR_UNDER_INDEX,-2500);
			}
		}

		free(trackdata);

		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		fclose(f);
		
		return LOADER_NOERROR;	
	}

	floppycontext->hxc_printf(MSG_ERROR,"BAD GKH file!");
	fclose(f);
	return LOADER_BADFILE;
}
