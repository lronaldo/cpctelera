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
// File : ede_DiskFile.c
// Contains: ede floppy image loader and plugins interfaces
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

#include "ede_loader.h"

#include "../common/os_api.h"



int EDE_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	unsigned char header_buffer[512];
	FILE * f;

	floppycontext->hxc_printf(MSG_DEBUG,"EDE_libIsValidDiskFile %s",imgfile);
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
				
				if(strstr( filepath,".ed" )!=NULL)
				{
					free(filepath);
					
					floppycontext->hxc_printf(MSG_DEBUG,"EDE file !");

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}


					fread(header_buffer,0x200,1,f);

					fclose(f);
					if((header_buffer[0]==0x0D) && (header_buffer[1]==0x0A))
					{

	
						switch(header_buffer[0x1FF])
						{

							case 0x01:
								floppycontext->hxc_printf(MSG_INFO_0,"Mirage (DD) format");
							break;

							case 0x02:
								floppycontext->hxc_printf(MSG_INFO_0,"SQ-80 (DD) format");
							break;

							case 0x03:
								floppycontext->hxc_printf(MSG_INFO_0,"EPS (DD) format");
								break;

							case 0x04:
								floppycontext->hxc_printf(MSG_INFO_0,"VFX-SD (DD) format");
							break;

							case 0xcb:
								floppycontext->hxc_printf(MSG_INFO_0,"ASR-10 HD format");
								break;

							case 0xcc: 
								floppycontext->hxc_printf(MSG_INFO_0,"TS-10/12 HD format");
								break;

							case 0x07:
								floppycontext->hxc_printf(MSG_INFO_0,"TS-10/12 DD format");
								break;

							default:
								floppycontext->hxc_printf(MSG_ERROR,"Unknow format : %x !",header_buffer[0x1FF]);
								return LOADER_BADFILE;
								break;
						}
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
					floppycontext->hxc_printf(MSG_DEBUG,"non EDE file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int EDE_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j,l;
	int k;
	unsigned char gap3len,interleave;
	unsigned short rpm,sectorsize;
	CYLINDER* currentcylinder;
	unsigned char header_buffer[512];
	int header_offset;
	int blocknum;
	int number_of_block;
	unsigned char bitmask;
	int floppy_buffer_index;
	unsigned char trackformat;
	unsigned char skew;
	SECTORCONFIG  * sectorconfig;
	unsigned int sectorsizelayout[32];
	unsigned int sectoridlayout[32];
	
	floppycontext->hxc_printf(MSG_DEBUG,"EDE_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 
	

	fread(header_buffer,0x200,1,f);

	if((filesize!=0) && (header_buffer[0]==0x0D) && (header_buffer[1]==0x0A))
	{

		floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
		sectorsize=512; 
		rpm=300;
		trackformat=ISOFORMAT_DD;
		skew=0;
		switch(header_buffer[0x1FF])
		{

			case 0x01:
				floppycontext->hxc_printf(MSG_INFO_0,"Mirage (DD) format");
				header_offset=0xA0;
				sectorsize=1024; 
				floppydisk->floppyBitRate=250000;
				floppydisk->floppyNumberOfTrack=80;
				floppydisk->floppyNumberOfSide=1;
				floppydisk->floppySectorPerTrack=6;
				gap3len=255;
				interleave=1;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectorsizelayout[k]=sectorsize;
				for(k=1;k<floppydisk->floppySectorPerTrack;k++)sectoridlayout[k]=k-1;
				sectoridlayout[0]=5;
				sectorsizelayout[0]=512;
				break;

			case 0x02:
				floppycontext->hxc_printf(MSG_INFO_0,"SQ-80 (DD) format");
				header_offset=0xA0;
				sectorsize=1024; 
				floppydisk->floppyBitRate=250000;
				floppydisk->floppyNumberOfTrack=80;
				floppydisk->floppyNumberOfSide=2;
				floppydisk->floppySectorPerTrack=6;
				gap3len=255;
				interleave=1;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectorsizelayout[k]=sectorsize;
				for(k=1;k<floppydisk->floppySectorPerTrack;k++)sectoridlayout[k]=k-1;
				sectoridlayout[0]=5;
				sectorsizelayout[0]=512;
				break;

			case 0x03:
				floppycontext->hxc_printf(MSG_INFO_0,"EPS (DD) format");
				header_offset=0xA0;
				floppydisk->floppyBitRate=250000;
				floppydisk->floppyNumberOfTrack=80;
				floppydisk->floppyNumberOfSide=2;
				floppydisk->floppySectorPerTrack=10;
				gap3len=40;
				interleave=1;
				skew=2;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectorsizelayout[k]=sectorsize;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectoridlayout[k]=k;
				break;

			case 0x04:
				floppycontext->hxc_printf(MSG_INFO_0,"VFX-SD (DD) format");
				header_offset=0xA0;
				floppydisk->floppyBitRate=250000;
				floppydisk->floppyNumberOfTrack=80;
				floppydisk->floppyNumberOfSide=2;
				floppydisk->floppySectorPerTrack=10;
				gap3len=40;
				interleave=1;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectorsizelayout[k]=sectorsize;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectoridlayout[k]=k;
				break;

			case 0xcb:
				floppycontext->hxc_printf(MSG_INFO_0,"ASR-10 HD format");
				floppydisk->floppyiftype=IBMPC_HD_FLOPPYMODE;
				header_offset=0x60;
				floppydisk->floppyBitRate=500000;
				floppydisk->floppyNumberOfTrack=80;
				floppydisk->floppyNumberOfSide=2;
				floppydisk->floppySectorPerTrack=20;
				gap3len=40;
				interleave=1;
				skew=2;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectorsizelayout[k]=sectorsize;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectoridlayout[k]=k;
				break;

			case 0xcc: 
				floppycontext->hxc_printf(MSG_INFO_0,"TS-10/12 HD format");
				floppydisk->floppyiftype=IBMPC_HD_FLOPPYMODE;
				header_offset=0x60;
				floppydisk->floppyBitRate=500000;
				floppydisk->floppyNumberOfTrack=80;
				floppydisk->floppyNumberOfSide=2;
				floppydisk->floppySectorPerTrack=20;
				gap3len=40;
				interleave=1;
				skew=2;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectorsizelayout[k]=sectorsize;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectoridlayout[k]=k;
				break;

			case 0x07:
				floppydisk->floppyiftype=IBMPC_DD_FLOPPYMODE;
				floppycontext->hxc_printf(MSG_INFO_0,"TS-10/12 DD format");
				header_offset=0xA0;
				floppydisk->floppyBitRate=250000;
				floppydisk->floppyNumberOfTrack=80;
				floppydisk->floppyNumberOfSide=2;
				floppydisk->floppySectorPerTrack=10;
				gap3len=40;
				interleave=1;
				skew=2;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectorsizelayout[k]=sectorsize;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)sectoridlayout[k]=k;
				break;

			default:
				floppycontext->hxc_printf(MSG_ERROR,"Unknow format : %x !",header_buffer[0x1FF]);
				fclose(f);
				return LOADER_BADFILE;
			break;
		}

		number_of_block=floppydisk->floppyNumberOfTrack*floppydisk->floppyNumberOfSide*floppydisk->floppySectorPerTrack;

		sectorconfig=malloc(sizeof(SECTORCONFIG) * floppydisk->floppySectorPerTrack);
		memset(sectorconfig,0,sizeof(SECTORCONFIG) * floppydisk->floppySectorPerTrack);

		floppy_buffer_index=0;
		blocknum=0;
		bitmask=0x80;

		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
		
		floppycontext->hxc_printf(MSG_INFO_1,"%d tracks, %d side(s), %d sectors/track, gap3:%d, interleave:%d,rpm:%d",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,gap3len,interleave,rpm);

		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
				
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
				
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{

				memset(sectorconfig,0,sizeof(SECTORCONFIG)*floppydisk->floppySectorPerTrack);
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)
				{
					sectorconfig[k].head=i;
					sectorconfig[k].cylinder=j;
					sectorconfig[k].sector=sectoridlayout[k];
					sectorconfig[k].sectorsize=sectorsizelayout[k];
					sectorconfig[k].bitrate=floppydisk->floppyBitRate;
					sectorconfig[k].gap3=gap3len;
					sectorconfig[k].trackencoding=trackformat;
					sectorconfig[k].input_data=malloc(sectorsize);
					memset(sectorconfig[k].input_data,0,sectorsize);

					if(blocknum<number_of_block)
					{
						if(!(header_buffer[header_offset]&bitmask))
						{
							floppycontext->hxc_printf(MSG_DEBUG,"T:%.3d S:%d Sector:%.2d Size:%.4d File offset: 0x%.8x",j,i,sectorconfig[k].sector,sectorconfig[k].sectorsize,ftell(f));
							fread(sectorconfig[k].input_data,sectorsize,1,f);
						}
						else
						{
							floppycontext->hxc_printf(MSG_DEBUG,"T:%.3d S:%d Sector:%.2d Size:%.4d File offset: ----------",j,i,sectorconfig[k].sector,sectorconfig[k].sectorsize);
							for(l=0;l<(sectorconfig[k].sectorsize/2);l++)
							{
								sectorconfig[k].input_data[(l*2)]=0x6D;
								sectorconfig[k].input_data[(l*2)+1]=0xB6;
							}
						}
						bitmask=bitmask>>1;
						if(!bitmask)
						{
							header_offset++;
							bitmask=0x80;
						}
						blocknum++;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"T:%.3d S:%d Sector:%.2d Size:%.4d",j,i,sectorconfig[k].sector,sectorconfig[k].sectorsize);
						for(l=0;l<(sectorconfig[k].sectorsize/2);l++)
						{
							sectorconfig[k].input_data[(l*2)]=0x6D;
							sectorconfig[k].input_data[(l*2)+1]=0xB6;
						}
					}
				}
					
				currentcylinder->sides[i]=tg_generatetrackEx(floppydisk->floppySectorPerTrack,sectorconfig,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,rpm,trackformat,2500,-2500);

				for(k=0;k<floppydisk->floppySectorPerTrack;k++)
				{
					free(sectorconfig[k].input_data);
				}
			}
		}

		free(sectorconfig);

		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		fclose(f);
		
		return LOADER_NOERROR;	
	}

	floppycontext->hxc_printf(MSG_ERROR,"BAD EDE file!");
	fclose(f);
	return LOADER_BADFILE;
}
