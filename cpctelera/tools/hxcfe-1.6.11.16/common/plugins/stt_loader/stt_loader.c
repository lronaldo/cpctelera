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
// File : stt_loader.c
// Contains: STT floppy image loader and plugins interfaces
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

#include "stt_loader.h"

#include "../common/os_api.h"

#include "sttfileformat.h"


int STT_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,filesize;
	char * filepath;
	FILE * f;
	stt_header STTHEADER;
	floppycontext->hxc_printf(MSG_DEBUG,"STT_libIsValidDiskFile %s",imgfile);
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
				
				if(strstr( filepath,".stt" )!=NULL)
				{

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}
					
					fseek (f , 0 , SEEK_END); 
					filesize=ftell(f);
					fseek (f , 0 , SEEK_SET); 
					
					STTHEADER.stt_signature=0;
					fread(&STTHEADER,sizeof(stt_header),1,f);

					fclose(f);
					
					
					if(STTHEADER.stt_signature!=0x4D455453) //"STEM"
					{
						free(filepath);
						floppycontext->hxc_printf(MSG_DEBUG,"non STT IMG file - bad signature !");
						return LOADER_BADFILE;
					}

					floppycontext->hxc_printf(MSG_DEBUG,"STT file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non STT file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int STT_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j,k;
	unsigned int file_offset;
	unsigned char gap3len,interleave;
	unsigned short sectorsize,rpm;
	unsigned char trackformat;
	unsigned long file_track_list_offset;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	SECTORCONFIG* sectorconfig;
	stt_header STTHEADER;
	stt_track_offset STTTRACKOFFSET;
	stt_track_header STTTRACKHEADER;
	stt_sector       STTSECTOR;
	
	floppycontext->hxc_printf(MSG_DEBUG,"STT_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 

	STTHEADER.stt_signature=0;
	fread(&STTHEADER,sizeof(stt_header),1,f);
				
	if(STTHEADER.stt_signature!=0x4D455453) //"STEM"
	{
		floppycontext->hxc_printf(MSG_DEBUG,"non STT IMG file - bad signature !");
		fclose(f);

		return LOADER_BADFILE;
	}

	file_track_list_offset=ftell(f);
	
	floppydisk->floppyNumberOfTrack=STTHEADER.number_of_tracks;
	floppydisk->floppyNumberOfSide=(unsigned char)STTHEADER.number_of_sides;
	
	floppydisk->floppyBitRate=250000;
	floppydisk->floppyiftype=ATARIST_DD_FLOPPYMODE;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
	memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
	rpm=300; // normal rpm
	
	interleave=1;
	gap3len=80;
	sectorsize=512;
	
	floppycontext->hxc_printf(MSG_INFO_1,"filesize:%dkB, %d tracks, %d side(s), %d sectors/track, gap3:%d, interleave:%d,rpm:%d",filesize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,gap3len,interleave,rpm);

	for(i=0;i<floppydisk->floppyNumberOfSide;i++)
	{

		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
			if(!floppydisk->tracks[j])
			{
				floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
				currentcylinder=floppydisk->tracks[j];
			}

			currentcylinder=floppydisk->tracks[j];

			currentcylinder->floppyRPM=rpm;
		
			fseek (f, file_track_list_offset, SEEK_SET); 
			fread((void*)&STTTRACKOFFSET,sizeof(stt_track_offset),1,f);
			file_track_list_offset=file_track_list_offset+sizeof(stt_track_offset);

			floppycontext->hxc_printf(MSG_DEBUG,"Current Track Offset : 0x%.8X, Size: 0x%.8X, Next File Track List Offset : 0x%.8X",STTTRACKOFFSET.track_offset,STTTRACKOFFSET.track_size,file_track_list_offset);
			
			fseek (f, STTTRACKOFFSET.track_offset, SEEK_SET); 

			fread((void*)&STTTRACKHEADER,sizeof(stt_track_header),1,f);

			if(!memcmp(&STTTRACKHEADER.stt_track_signature,"TRCK",4))
			{

				floppycontext->hxc_printf(MSG_INFO_1,"Track: %d, Side: %d, Number of sector: %d, Tracks Flags:0x%.8x, Sector Flags:0x%.8x",j,i,STTTRACKHEADER.number_of_sectors,STTTRACKHEADER.tracks_flags,STTTRACKHEADER.sectors_flags);

				trackformat=ISOFORMAT_DD;
				if(STTTRACKHEADER.number_of_sectors==11 || STTTRACKHEADER.number_of_sectors==12)
				{
					trackformat=ISOFORMAT_DD11S;
				}

				sectorconfig=malloc(sizeof(SECTORCONFIG)*STTTRACKHEADER.number_of_sectors);
				memset(sectorconfig,0,sizeof(SECTORCONFIG)*STTTRACKHEADER.number_of_sectors);
				for(k=0;k<STTTRACKHEADER.number_of_sectors;k++)
				{
					fread((void*)&STTSECTOR,sizeof(stt_sector),1,f);
				
					floppycontext->hxc_printf(MSG_INFO_1,"Sector id: %d, Side id: %d, Track id: %d, Sector size:%d",STTSECTOR.sector_nb_id,STTSECTOR.side_nb_id,STTSECTOR.track_nb_id,STTSECTOR.data_len);
	
					sectorconfig[k].sector=STTSECTOR.sector_nb_id;
					sectorconfig[k].head=STTSECTOR.side_nb_id;
					sectorconfig[k].sectorsize=STTSECTOR.data_len;
					sectorconfig[k].use_alternate_sector_size_id=1;
					sectorconfig[k].alternate_sector_size_id=STTSECTOR.sector_len_code;
					sectorconfig[k].cylinder=STTSECTOR.track_nb_id;
					//sectorconfig[k].use_alternate_header_crc=0x2;
					sectorconfig[k].header_crc=(STTSECTOR.crc_byte_1<<8) | STTSECTOR.crc_byte_2;
					sectorconfig[k].trackencoding=trackformat;
					sectorconfig[k].gap3=255;
					sectorconfig[k].bitrate=floppydisk->floppyBitRate;
									
					file_offset=ftell(f);
					
					sectorconfig[k].input_data=malloc(STTSECTOR.data_len);
					fseek (f, STTSECTOR.data_offset + STTTRACKOFFSET.track_offset, SEEK_SET); 
					fread(sectorconfig[k].input_data,STTSECTOR.data_len,1,f);
			
					fseek (f, file_offset, SEEK_SET); 

				}
					
				currentside=tg_generatetrackEx((unsigned short)STTTRACKHEADER.number_of_sectors,sectorconfig,interleave,0,floppydisk->floppyBitRate,rpm,trackformat,2500 | NO_SECTOR_UNDER_INDEX,-2500);
				currentcylinder->sides[i]=currentside;

				currentside->bitrate=(long)(250000*(float)((float)(currentside->tracklen/2)/(float)50000));


				for(k=0;k<STTTRACKHEADER.number_of_sectors;k++)
				{
					if(sectorconfig[k].input_data)
						free(sectorconfig[k].input_data);
				}

				free(sectorconfig);
			}
			else
			{				
				floppycontext->hxc_printf(MSG_INFO_1,"Bad track Header !? : Track: %d, Side: %d",j,i);
				trackformat=ISOFORMAT_DD;

				STTTRACKHEADER.number_of_sectors=0;

				sectorconfig=malloc(sizeof(SECTORCONFIG));
				memset(sectorconfig,0,sizeof(SECTORCONFIG));
				currentside=tg_generatetrackEx((unsigned short)STTTRACKHEADER.number_of_sectors,sectorconfig,interleave,0,floppydisk->floppyBitRate,rpm,trackformat,2500 | NO_SECTOR_UNDER_INDEX,-2500);														
		
				currentcylinder->sides[i]=currentside;

				free(sectorconfig);
			}
		}
	}
			
	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		
	fclose(f);
	return LOADER_NOERROR;

}
