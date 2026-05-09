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
// File : scl_DiskFile.c
// Contains: SCL floppy image loader and plugins interfaces
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

#include "scl_loader.h"

#include "../common/os_api.h"

unsigned char dir_entry[34] =
	{
		0x01, 0x16, 0x00, 0xF0,
		0x09, 0x10, 0x00, 0x00,
		0x20, 0x20, 0x20, 0x20,
		0x20, 0x20, 0x20, 0x20,
		0x20, 0x00, 0x00, 0x64,
		0x69, 0x73, 0x6B, 0x6E,
		0x61, 0x6D, 0x65, 0x00,
		0x00, 0x00, 0x46, 0x55
	};


int SCL_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	char sclsignature[8];
	FILE * f;
	floppycontext->hxc_printf(MSG_DEBUG,"SCL_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".scl" )!=NULL)
				{

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						free(filepath);
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open the file !");
						return LOADER_ACCESSERROR;
					}
					fread(&sclsignature,8,1,f);
					fclose(f);

					if(!strncmp(sclsignature, "SINCLAIR", 8))
					{
						floppycontext->hxc_printf(MSG_DEBUG,"Sinclair SCL file !");
						return LOADER_ISVALID;
					}
					else
					{

						floppycontext->hxc_printf(MSG_DEBUG,"non Sinclair SCL file !(bad header)");
						return LOADER_BADFILE;
					}
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non Sinclair SCL file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}

unsigned int lsb2ui(unsigned char *mem)
{
  return (mem[0] + (mem[1] * 256) + (mem[2] * 256 * 256)
          + (mem[3] * 256 * 256 * 256));
}


void ui2lsb(unsigned char *mem, unsigned int value)
{
  mem[0] = (value>>0 )&0xFF;
  mem[1] = (value>>8 )&0xFF;
  mem[2] = (value>>16)&0xFF;
  mem[3] = (value>>24)&0xFF;
}

int SCL_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned char  gap3len,interleave;
	unsigned short rpm;
	unsigned short sectorsize;
	int number_of_track,number_of_side,number_of_sectorpertrack;

	void *tmp;
	unsigned char size,skew;
	unsigned int *trd_free;
	unsigned char *trd_fsec;
	unsigned char *trd_ftrk;
	unsigned char *trd_files;
	char sclsignature[8];
	unsigned char number_of_blocks;
	char block_headers[256][14];
	unsigned char * trd_image;

	unsigned long left;
	unsigned long trd_offset;
	unsigned char trackformat;

	CYLINDER* currentcylinder;
	
	floppycontext->hxc_printf(MSG_DEBUG,"SCL_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fread(&sclsignature,8,1,f);
	if(strncmp(sclsignature, "SINCLAIR", 8))
	{
		floppycontext->hxc_printf(MSG_DEBUG,"non Sinclair SCL file !(bad header)");
		fclose(f);
		return LOADER_BADFILE;
	}

	fread(&number_of_blocks,1,1,f);
	floppycontext->hxc_printf(MSG_DEBUG,"%d block(s) in the file",number_of_blocks);
	
	for (i=0; i < number_of_blocks; i++)
	{
		fread(&(block_headers[i][0]),14,1,f);
	}

	// allocate and init a TR DOS disk.
	number_of_track=80;
	number_of_side=2;
	number_of_sectorpertrack=16;
	trd_image=(unsigned char*)malloc(number_of_track*number_of_side*number_of_sectorpertrack*256);
	if(!trd_image)
	{
		floppycontext->hxc_printf(MSG_ERROR,"Malloc error !");
		fclose(f);
		return LOADER_INTERNALERROR;
	}
	
	memset(trd_image,0,number_of_track*number_of_side*number_of_sectorpertrack*256);
	memcpy(&trd_image[0x08E2], dir_entry, 32);
    strncpy((char*)&trd_image[0x08F5], "HxCFE", 8);


	tmp = (char *) trd_image + 0x8E5;
	trd_free = (unsigned int *) tmp;
	trd_files = (unsigned char *) trd_image + 0x8E4;
	trd_fsec = (unsigned char *) trd_image + 0x8E1;
	trd_ftrk = (unsigned char *) trd_image + 0x8E2;

	// copy blocks to the trd disk
	for (i=0; i < number_of_blocks; i++)
	{
		size=block_headers[i][13];
		if (lsb2ui(tmp) < size) 
		{
			floppycontext->hxc_printf(MSG_ERROR,"File too long to fit in the image *trd_free=%u < size=%u",lsb2ui(tmp),size);
			fclose(f);
			free(trd_image);
			return LOADER_INTERNALERROR;
		}

		if (*trd_files > 127) 
		{
			floppycontext->hxc_printf(MSG_ERROR,"Image File full!");
			fclose(f);
			free(trd_image);
			return LOADER_INTERNALERROR;
		}

		memcpy((void *) ((char *) trd_image + *trd_files * 16),
			 (void *) block_headers[i], 14);

		memcpy((void *) ((char *) trd_image + *trd_files * 16 + 0x0E),
			 (void *) trd_fsec, 2);

		left = (unsigned long) ((unsigned char) block_headers[i][13]) * 256L;
		trd_offset = (*trd_ftrk) * 4096L + (*trd_fsec) * 256L;

		fread(&(trd_image[trd_offset]),left,1,f);
    
		(*trd_files)++;

		ui2lsb(tmp, lsb2ui(tmp) - size);

		while (size > 15) 
		{
		  (*trd_ftrk)++;
		  size = size - 16;
		}


		(*trd_fsec) += size;
		
		while ((*trd_fsec) > 15)
		{
			(*trd_fsec) -= 16;
			(*trd_ftrk)++;
		}

	}
	fclose(f);

	rpm=300;
	sectorsize=256; // SCL file support only 256bytes/sector floppies.
	gap3len=50;
	trackformat=IBMFORMAT_DD;
	interleave=1;
	skew=0;
	floppydisk->floppyBitRate=250000;
	floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
	floppydisk->floppyNumberOfTrack=number_of_track;
	floppydisk->floppyNumberOfSide=number_of_side;
	floppydisk->floppySectorPerTrack=number_of_sectorpertrack;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
	floppycontext->hxc_printf(MSG_DEBUG,"rpm %d bitrate:%d track:%d side:%d sector:%d",rpm,floppydisk->floppyBitRate,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack);

			
	for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
	{
				
		floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
		currentcylinder=floppydisk->tracks[j];	
				
		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{
			file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
						(sectorsize*(floppydisk->floppySectorPerTrack)*i);			
			currentcylinder->sides[i]=tg_generatetrack(&trd_image[file_offset],sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2000,-2000);
		}
	}
			
	free(trd_image);		
	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
	return LOADER_NOERROR;
}
			

