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
// File : dim_loader.c
// Contains: DIM floppy image loader and plugins interfaces
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

#include "dim_loader.h"

#include "../common/os_api.h"


#pragma pack(1)
typedef struct dim_header_
{
	unsigned short id_header;        // 0x0000 Word ID Header (0x4242('BB')) 
	unsigned char  unused1;
	unsigned char  used_sector_only; // 0x0003 Byte Image contains all sectors (0) or used sectors (1) 
	unsigned short unused2;
	unsigned char  side;             // 0x0006 Byte Sides (0 or 1; add 1 to this to get correct number of sides) 
	unsigned char  unused3;
	unsigned char  nbsector;         // 0x0008 Byte Sectors per track
    unsigned char  unused4;
	unsigned char  start_track;      // 0x000A Byte Start Track (0 based) 
    unsigned char  unused5;
    unsigned char  end_track;        // 0x000C Byte Ending Track (0 based)
    unsigned char  density;			 // 0x000D Byte Double-Density(0) or High-Density (1) 
    unsigned char  sectorsizeh;       // sector size (bytes) 
    unsigned char  sectorsizel;       // sector size (bytes) 
}dim_header;
#pragma pack()



int DIM_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	FILE * f;
	dim_header header;
	floppycontext->hxc_printf(MSG_DEBUG,"DIM_libIsValidDiskFile %s",imgfile);
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
				
				if(strstr( filepath,".dim" )!=NULL)
				{

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}
					
					fread(&header,sizeof(dim_header),1,f);

					fclose(f);
					

					if(	header.id_header==0x4242)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"DIM file !");
						free(filepath);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non DIM file ! Bad header!");
						free(filepath);
						return LOADER_BADFILE;
					}
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non DIM file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int DIM_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned char  i,j,skew;
	unsigned int   file_offset;
	unsigned char  trackformat;
	unsigned char* trackdata;
	unsigned char  gap3len,interleave;
	unsigned short rpm;
	unsigned short sectorsize;
	CYLINDER* currentcylinder;
	dim_header header;
	
	floppycontext->hxc_printf(MSG_DEBUG,"DIM_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fread(&header,sizeof(dim_header),1,f);
	
	
	if(header.id_header==0x4242)
	{		
		
		sectorsize=(header.sectorsizeh*256)+header.sectorsizel; // st file support only 512bytes/sector floppies.
		if(!sectorsize || (sectorsize&0xFF)) sectorsize=512;
		// read the first sector
		interleave=1;
		gap3len=30;
		floppydisk->floppyNumberOfTrack=header.end_track+1;
		floppydisk->floppyNumberOfSide=header.side+1;
		floppydisk->floppySectorPerTrack=header.nbsector;

		if(!header.density)
		{
			floppydisk->floppyBitRate=DEFAULT_DD_BITRATE;
			floppydisk->floppyiftype=ATARIST_DD_FLOPPYMODE;
			skew=2;
		}
		else
		{
			floppydisk->floppyiftype=ATARIST_HD_FLOPPYMODE;
			floppydisk->floppyBitRate=DEFAULT_HD_BITRATE;
			skew=4;
		}

		trackformat=ISOFORMAT_DD;
		if(floppydisk->floppySectorPerTrack==11)
		{
			gap3len=3;
			trackformat=ISOFORMAT_DD11S;
			interleave=2;
		}
					
		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
		rpm=300; // normal rpm
			
		floppycontext->hxc_printf(MSG_INFO_1,"%d tracks, %d side(s), %d sectors/track, gap3:%d, interleave:%d,rpm:%d bitrate:%d",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,gap3len,interleave,rpm,floppydisk->floppyBitRate);
				
		trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
				
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
				
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
				file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
							(sectorsize*(floppydisk->floppySectorPerTrack)*i)+0x20;
					
				fseek (f , file_offset , SEEK_SET);
				fread(trackdata,sectorsize*floppydisk->floppySectorPerTrack,1,f);
					
				currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500,-2500);
			}
		}

		free(trackdata);
		
		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
	
		fclose(f);
		return LOADER_NOERROR;

	}
	fclose(f);
	
	return LOADER_FILECORRUPT;
}
