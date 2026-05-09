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
// File : D88_loader.c
// Contains: D88 floppy image loader.
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

#include "d88_loader.h"
#include "d88_format.h"

int D88_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;

	floppycontext->hxc_printf(MSG_DEBUG,"D88_libIsValidDiskFile %s",imgfile);

	if(imgfile)
	{
		pathlen=strlen(imgfile);
		if(pathlen!=0)
		{
			filepath=malloc(pathlen+1);
			if(filepath!=0)
			{
				sprintf(filepath,"%s",imgfile);
				strlwr(filepath);

				if(strstr( filepath,".d88" ) || strstr( filepath,".d77" ) || strstr( filepath,".88d" ) || strstr( filepath,".d8u" ) || strstr( filepath,".2d" )|| strstr( filepath,".d68" ) )
				{
					floppycontext->hxc_printf(MSG_DEBUG,"D88 file !");
					free(filepath);
					return LOADER_ISVALID;
				}
					
			}
			else
			{
				floppycontext->hxc_printf(MSG_DEBUG,"non D88 file !");
				free(filepath);
				return LOADER_BADFILE;
			}
		}
	}

	return LOADER_BADPARAMETER;
}

int D88_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	d88_fileheader fileheader;
	d88_sector sectorheader;
	int i,j;
	unsigned short rpm;
	unsigned char tracktype,side,interleave;
	SECTORCONFIG* sectorconfig;
	CYLINDER* currentcylinder;
	unsigned int bitrate;
	int indexfile;
	unsigned long tracklen;
	int number_of_track,number_of_sector;
	unsigned long track_offset;
	char * indexstr;
	char str_file[512];
	int basefileptr;
	int totalfilesize,truetotalfilesize,partcount;
	
	floppycontext->hxc_printf(MSG_DEBUG,"D88_libLoad_DiskFile %s",imgfile);

	indexfile=0;
	basefileptr=0;
	sprintf(str_file,"%s",imgfile);
	indexstr=strstr( str_file,".d88" );
	if(indexstr)
	{
		indexstr=strstr( indexstr," " );
		if(indexstr)
		{
			if(indexstr[1]>='0' && indexstr[1]<='9')
			{
				indexfile=indexstr[1]-'0';
				indexstr[0]=0;
			}
		}
	
	}


	f=fopen(str_file,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	//////////////////////////////////////////////////////
	// sanity check
	floppycontext->hxc_printf(MSG_INFO_1,"Floppy disk in this file :");
	partcount=0;

	fseek(f,0,SEEK_END);
	truetotalfilesize=ftell(f);
	fseek(f,0,SEEK_SET);
	
	totalfilesize=0;
	while((!feof(f)) && (partcount<256) && (totalfilesize<truetotalfilesize))
	{
		fread(&fileheader,sizeof(fileheader),1,f);
		if(fileheader.file_size)
		{
			floppycontext->hxc_printf(MSG_INFO_1,"%s",fileheader.name);

			fseek(f,fileheader.file_size-sizeof(fileheader),SEEK_CUR);
			totalfilesize=totalfilesize+fileheader.file_size;
			partcount++;
		}
		else
		{
			truetotalfilesize=totalfilesize;
			fseek(f,truetotalfilesize,SEEK_SET);
		}
	}

	if((totalfilesize!=ftell(f)) || partcount==256)
	{
		// bad total size ! 
		floppycontext->hxc_printf(MSG_ERROR,"Bad D88 file size !",imgfile);
		fclose(f);
		return LOADER_BADFILE;
	}
	floppycontext->hxc_printf(MSG_INFO_1,"%d floppy in this file.",partcount);
	fseek(f,0,SEEK_SET);
	//////////////////////////////////////////////////////
		
	//////////////////////////////////////////////////////
	// Floppy selection
	if(indexfile>=partcount)
	{
		floppycontext->hxc_printf(MSG_ERROR,"bad selection index (%d). there are only %d disk(s) in this file!",indexfile,partcount);
		fclose(f);
		return LOADER_ACCESSERROR;
	}
	
	while(indexfile)
	{
		fread(&fileheader,sizeof(fileheader),1,f);
		fseek(f,fileheader.file_size-sizeof(fileheader),SEEK_CUR);
		indexfile--;
	}

	basefileptr=ftell(f);
	//////////////////////////////////////////////////////

	//////////////////////////////////////////////////////
	// Header read
	fread(&fileheader,sizeof(fileheader),1,f);
	fileheader.reserved[0]=0;

	floppycontext->hxc_printf(MSG_INFO_1,"Opening %s (%d), part %s , part size:%d",imgfile,indexfile,fileheader.name,fileheader.file_size);
	switch(fileheader.media_flag)
	{
		case 0x00: // 2D
			floppycontext->hxc_printf(MSG_INFO_1,"2D disk");
			tracktype=IBMFORMAT_SD;
			bitrate=250000;
			side=2;
			break; 
		case 0x10: // 2DD
			floppycontext->hxc_printf(MSG_INFO_1,"2DD disk");
			tracktype=IBMFORMAT_DD;
			bitrate=250000;
			side=2;	
			break;
		case 0x20: // 2HD
			floppycontext->hxc_printf(MSG_INFO_1,"2HD disk");
			tracktype=IBMFORMAT_DD;
			bitrate=500000;
			side=2;	
			break;
		case 0x40: // 1DD
			floppycontext->hxc_printf(MSG_INFO_1,"1DD disk");
			tracktype=IBMFORMAT_DD;
			bitrate=250000;
			side=1;	
			break;

		default:
			side=2;
			floppycontext->hxc_printf(MSG_ERROR,"unknow disk: %.2X !",fileheader.media_flag);
			fclose(f);
			return LOADER_BADFILE;
			break;
	}

	if(fileheader.write_protect & 0x10)
	{
		floppycontext->hxc_printf(MSG_INFO_1,"write protected disk");
	}


	fseek(f,basefileptr+sizeof(d88_fileheader),SEEK_SET);
	fread(&track_offset,sizeof(unsigned long),1,f);

	number_of_track=(track_offset-sizeof(d88_fileheader))/sizeof(unsigned long);
	do
	{
		fseek(f,basefileptr+sizeof(d88_fileheader)+((number_of_track-1)*sizeof(unsigned long)),SEEK_SET);
		fread(&track_offset,1,sizeof(unsigned long),f);
		if(!track_offset)
		{
			number_of_track--;
		}
	}while(number_of_track && !track_offset);


	floppycontext->hxc_printf(MSG_INFO_1,"Number of track: %d",number_of_track);

	fseek(f,basefileptr+sizeof(d88_fileheader),SEEK_SET);

	fread(&track_offset,sizeof(unsigned long),1,f);
	floppycontext->hxc_printf(MSG_ERROR,"first track offset:%X",track_offset);


	floppydisk->floppyNumberOfTrack=number_of_track;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
	memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	floppydisk->floppyBitRate=bitrate;
	floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
	floppydisk->floppyNumberOfSide=side;
	floppydisk->floppySectorPerTrack=-1; // default value
	interleave=1;
	rpm=300;
	
	floppycontext->hxc_printf(MSG_INFO_1,"%d tracks, %d Side(s)\n",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide);
	
	if(side==1)
		number_of_track=number_of_track*2;

	i=0;
	do
	{
		if(track_offset)
		{
			fseek(f,track_offset+basefileptr,SEEK_SET);
			fread(&sectorheader,sizeof(d88_sector),1,f);
			
			number_of_sector=sectorheader.number_of_sectors;
			floppycontext->hxc_printf(MSG_INFO_1,"Number of sector: %d",number_of_sector);
			if(sectorheader.density&0x40)
			{
				tracktype=IBMFORMAT_SD;
			}
			else
			{
				tracktype=IBMFORMAT_DD;
			}

			sectorconfig=(SECTORCONFIG*)malloc(sizeof(SECTORCONFIG)*number_of_sector);
			memset(sectorconfig,0,sizeof(SECTORCONFIG)*number_of_sector);

			j=0;
			do
			{

				if(!sectorheader.sector_length)
				{
					sectorheader.sector_length=128 * (1 << sectorheader.sector_size);
				}

				floppycontext->hxc_printf(MSG_INFO_1,"Cylinder:%.3d, Size:%.1d, Sector ID:%.3d, Status:0x%.2x, File offset:0x%.6x",
					sectorheader.cylinder,
					sectorheader.sector_length,
					sectorheader.sector_id,
					sectorheader.sector_status,
					ftell(f)
					);

				sectorconfig[j].cylinder=sectorheader.cylinder;
				sectorconfig[j].head=sectorheader.head;
				sectorconfig[j].sector=sectorheader.sector_id;
				sectorconfig[j].sectorsize=sectorheader.sector_length;
				sectorconfig[j].gap3=255;
				sectorconfig[j].trackencoding=tracktype;
				sectorconfig[j].input_data=malloc(sectorheader.sector_length);
				sectorconfig[j].bitrate=floppydisk->floppyBitRate;
			
				fread(sectorconfig[j].input_data,sectorheader.sector_length,1,f); 
				
				if(sectorheader.sector_status)
				{
					sectorconfig[j].use_alternate_header_crc=0x1;
				}

				//	sectorconfig[j].baddatacrc=1;

				// fread datas
				fread(&sectorheader,sizeof(d88_sector),1,f); 
				
				j++;
			}while(j<number_of_sector);

			if(!floppydisk->tracks[i>>1])
			{
				floppydisk->tracks[i>>1]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
				currentcylinder=floppydisk->tracks[i>>1];
			}
	//
			currentcylinder->floppyRPM=rpm;
			currentcylinder->sides[i&1]=tg_generatetrackEx((unsigned short)number_of_sector,sectorconfig,interleave,0,floppydisk->floppyBitRate,rpm,tracktype,2500 | NO_SECTOR_UNDER_INDEX,-2500);
			
			for(j=0;j<number_of_sector;j++)
			{
				if(sectorconfig[j].input_data)
					free(sectorconfig[j].input_data);
			}

			free(sectorconfig);

		}
		else
		{
			floppycontext->hxc_printf(MSG_INFO_1,"Unformated track:%.3d",i);

			if(!floppydisk->tracks[i>>1])
			{
				floppydisk->tracks[i>>1]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
				currentcylinder=floppydisk->tracks[i>>1];
			}

			tracklen=((bitrate/(rpm/60))/4)*8;
			currentcylinder->floppyRPM=rpm;
			currentcylinder->sides[i&1]=tg_alloctrack(floppydisk->floppyBitRate,ISOIBM_MFM_ENCODING,currentcylinder->floppyRPM,tracklen,2500,-2500,TG_ALLOCTRACK_ALLOCFLAKEYBUFFER|TG_ALLOCTRACK_RANDOMIZEDATABUFFER|TG_ALLOCTRACK_UNFORMATEDBUFFER);
		}

		if(side==2)
		{
			i++;
			floppycontext->hxc_printf(MSG_DEBUG,"Track %d offset: 0x%X",i,track_offset);
			fseek(f,basefileptr + sizeof(d88_fileheader)  + (i * sizeof(unsigned long)),SEEK_SET);
		}
		else
		{
			i=i+2;
			floppycontext->hxc_printf(MSG_DEBUG,"Track %d offset: 0x%X",i>>1,track_offset);
			fseek(f,basefileptr + sizeof(d88_fileheader)  + ((i>>1) * sizeof(unsigned long)),SEEK_SET);
		}

		
		fread(&track_offset,sizeof(unsigned long),1,f);
	
	}while(i<number_of_track);

	if(side==2)
		floppydisk->floppyNumberOfTrack=i/2;

		
	fclose(f);	
	//floppycontext->hxc_printf(MSG_ERROR,"bad header");
	return LOADER_NOERROR;
}

