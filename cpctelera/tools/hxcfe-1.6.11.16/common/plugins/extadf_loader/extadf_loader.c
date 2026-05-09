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
// File : extadf_loader.c
// Contains: Extended ADF floppy image loader and plugins interfaces
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
#include "extadf_loader.h"

#include "../common/os_api.h"

int EXTADF_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	FILE * f;
	char * filepath;
	unsigned int filesize;
	unsigned char header[12];

	floppycontext->hxc_printf(MSG_DEBUG,"EXTADF_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".adf" )!=NULL)
				{
					free(filepath);

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}

					fseek (f , 0 , SEEK_END); 
					filesize=ftell(f);
					fseek (f , 0 , SEEK_SET); 

					fread(header,12,1,f);
					fclose(f);
					header[8]=0;
					if(!strcmp(header,"UAE-1ADF"))
					{
						floppycontext->hxc_printf(MSG_DEBUG,"Extended ADF file (new version)!");
						return LOADER_ISVALID;

					}

					return LOADER_BADFILE;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non Extended ADF file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}




int EXTADF_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	unsigned int filesize;
	unsigned int i,j;
	char* trackdata;
	int	tracklen;
	CYLINDER* currentcylinder;
	unsigned int numberoftrack;
	
	unsigned char header[12];
	unsigned char * tracktable;
	unsigned int trackindex,tracksize;

	unsigned char gap3len,skew,trackformat,interleave;
	unsigned short sectorsize;
	
	floppycontext->hxc_printf(MSG_DEBUG,"EXTADF_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 

	if(!filesize)
	{
		floppycontext->hxc_printf(MSG_ERROR,"Bad file size : %d !",filesize);
		fclose(f);
		return LOADER_BADFILE;
	}
	

	fread(header,12,1,f);
	
	numberoftrack=0;
	header[8]=0;
	if(!strcmp(header,"UAE-1ADF"))
	{
		numberoftrack=header[0xB]+(header[0xA]*256);
		tracktable=malloc(12*numberoftrack);
		memset(tracktable,0,12*numberoftrack);

		fread(tracktable,12*numberoftrack,1,f);
	}
	trackindex=0;

	floppydisk->floppyNumberOfTrack=numberoftrack>>1;
	if(numberoftrack&1) floppydisk->floppyNumberOfTrack++;

	sectorsize=512;
	interleave=1;
	gap3len=0;
	skew=0;
	trackformat=AMIGAFORMAT_DD;

	floppydisk->floppySectorPerTrack=-1;
	floppydisk->floppyNumberOfSide=2;
	floppydisk->floppyBitRate=DEFAULT_AMIGA_BITRATE;
	floppydisk->floppyiftype=AMIGA_DD_FLOPPYMODE;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
		
	tracklen=(DEFAULT_AMIGA_BITRATE/(DEFAULT_AMIGA_RPM/60))/4;
		
	floppycontext->hxc_printf(MSG_DEBUG,"Extended ADF : %x tracks",numberoftrack);
		
	for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
	{
			
		floppydisk->tracks[j]=allocCylinderEntry(DEFAULT_AMIGA_RPM,floppydisk->floppyNumberOfSide);
		currentcylinder=floppydisk->tracks[j];
		
		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{	

			if(trackindex<numberoftrack)
			{

				tracksize=tracktable[(12*trackindex)+4] * 0x1000000 + \
					      tracktable[(12*trackindex)+5] * 0x10000   + \
						  tracktable[(12*trackindex)+6] * 0x100     + \
						  tracktable[(12*trackindex)+7];
				
				if(tracksize)
				{

					if(tracktable[(12*trackindex)+3]==1)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"[%.3d:%.1X] Reading Non-DOS track at 0x%.8x, Size : 0x%.8x",j,i,ftell(f),tracksize);

						currentcylinder->sides[i]=tg_alloctrack(DEFAULT_AMIGA_BITRATE,AMIGA_MFM_ENCODING,DEFAULT_AMIGA_RPM,(tracksize)*8,2500,-100,0x00);
						fread(currentcylinder->sides[i]->databuffer,tracksize,1,f);
						currentcylinder->sides[i]->number_of_sector=floppydisk->floppySectorPerTrack;

					}
					else
					{
						tracksize=tracktable[(12*trackindex)+4] * 0x1000000 + \
								  tracktable[(12*trackindex)+5] * 0x10000   + \
								  tracktable[(12*trackindex)+6] * 0x100     + \
								  tracktable[(12*trackindex)+7];
						
						trackdata=(unsigned char*)malloc(tracksize);

						floppycontext->hxc_printf(MSG_DEBUG,"[%.3d:%.1X] Reading DOS track at 0x%.8x, Size : 0x%.8x",j,i,ftell(f),tracksize);

						fread(trackdata,tracksize,1,f);

						currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,(unsigned short)(tracksize/sectorsize),(unsigned char)j,(unsigned char)i,0,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500,-11150);

						free(trackdata);

					}
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"[%.3d:%.1X] Null Size track!",j,i);
					currentcylinder->sides[i]=tg_alloctrack(DEFAULT_AMIGA_BITRATE,AMIGA_MFM_ENCODING,DEFAULT_AMIGA_RPM,(tracklen)*8,2500,-11360,0x00);
				}

			}
			else
			{
				floppycontext->hxc_printf(MSG_DEBUG,"[%.3d:%.1X] No track!",j,i);
				currentcylinder->sides[i]=tg_alloctrack(DEFAULT_AMIGA_BITRATE,AMIGA_MFM_ENCODING,DEFAULT_AMIGA_RPM,(tracklen)*8,2500,-11360,0x00);			
			}
									
			trackindex++;
		}
	}

	if(tracktable)	free(tracktable);
	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
	fclose(f);

	return LOADER_NOERROR;	
}


