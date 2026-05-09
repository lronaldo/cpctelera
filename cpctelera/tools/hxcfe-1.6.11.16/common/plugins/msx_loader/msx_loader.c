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
// File : MSX_DiskFile.c
// Contains: IMG floppy image loader and plugins interfaces
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

#include "msx_loader.h"

#include "msxfileformat.h"

#include "../common/os_api.h"

int msx_imggetfloppyconfig(unsigned char * filename,unsigned char * img,unsigned int filesize,unsigned short  *numberoftrack,unsigned char *numberofside,unsigned short *numberofsectorpertrack,unsigned short *sectorsize,unsigned char *gap3len,unsigned char *interleave,unsigned short *rpm, unsigned int *bitrate)
{
	int i;
	int imgmode,nbofside_img;
	//unsigned int numberofsector;
	unsigned char * uimg;
	int conffound,numberofsector;
	
	uimg=(unsigned char *)img;
	imgmode=0;
	conffound=0;
	uimg=(unsigned char *)img;
	
	
	if(!strstr( filename,".img" ))
	{
		nbofside_img=img[0];
		imgmode=1;
		img++;
	}
	
	conffound=0;
	if(uimg[0x18]<24 && uimg[0x18]>7)
	{
		
		*rpm=300;
		*numberofsectorpertrack=uimg[0x18];
		*numberofside=uimg[0x1A];
		*gap3len=60;
		*interleave=1;
		*bitrate=250000;
		*sectorsize=512;		
		
		numberofsector=uimg[0x13]+(uimg[0x14]*256);
		*numberoftrack=(numberofsector/(*numberofsectorpertrack*(*numberofside)));
		
		//	if((unsigned int)((*numberofsectorpertrack) * (*numberoftrack) * (*numberofside) *512)==filesize)
		{
			conffound=1;
		}

	}
	
	if(conffound==0)
	{
		i=0;
		do
		{
			
			if(msxfileformats[i].filesize==filesize)
			{
				*numberoftrack=msxfileformats[i].numberoftrack;
				*numberofsectorpertrack=msxfileformats[i].sectorpertrack;
				*numberofside=msxfileformats[i].numberofside;
				*gap3len=msxfileformats[i].gap3len;
				*interleave=msxfileformats[i].interleave;
				*rpm=msxfileformats[i].RPM;
				*bitrate=msxfileformats[i].bitrate;
				*sectorsize=msxfileformats[i].sectorsize;
				conffound=1;
			}
			i++;

		}while(msxfileformats[i].filesize!=0 && conffound==0);
		
	}
	return conffound;
}


int MSX_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,conffound,i;
	unsigned int filesize;
	char * filepath;
	FILE * f;
	
	floppycontext->hxc_printf(MSG_DEBUG,"MSX_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".img" )!=NULL || strstr( filepath,".dsk" )!=NULL)
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
					
					fclose(f);
					

					if(filesize&0x1FF)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non MSX IMG file - bad file size !");
						return LOADER_BADFILE;
					}


					i=0;
					conffound=0;
					do
					{
							
						if(msxfileformats[i].filesize==filesize)
						{
							conffound=1;
						}
						i++;

					}while(msxfileformats[i].filesize!=0 && conffound==0);

					if(conffound)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"MSX IMG file !");
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non MSX IMG file - bad file size !");
						return LOADER_BADFILE;
					}
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non MSX IMG file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}



int MSX_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int   filesize;
	unsigned int   i,j;
	unsigned int   file_offset;
	unsigned char* trackdata;
	unsigned char  gap3len,interleave;
	unsigned short rpm;
	unsigned short sectorsize;
	unsigned int   bitrate;
	unsigned char  trackformat,skew;

	CYLINDER* currentcylinder;
	
	floppycontext->hxc_printf(MSG_DEBUG,"MSX_libLoad_DiskFile %s",imgfile);
	
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
		
		sectorsize=512; // IMG file support only 512bytes/sector floppies.
		// read the first sector
		trackdata=(char*)malloc(sectorsize+1);
		fread(trackdata,sectorsize+1,1,f);
		if(msx_imggetfloppyconfig(
			imgfile,
			trackdata,
			filesize,
			&floppydisk->floppyNumberOfTrack,
			&floppydisk->floppyNumberOfSide,
			&floppydisk->floppySectorPerTrack,
			&sectorsize,
			&gap3len,
			&interleave,
			&rpm,
			&bitrate)==1
			)
		{
			
			free(trackdata);
			trackformat=IBMFORMAT_DD;
			floppydisk->floppyBitRate=bitrate;
			floppydisk->floppyiftype=MSX2_DD_FLOPPYMODE;
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
			floppycontext->hxc_printf(MSG_DEBUG,"rpm %d bitrate:%d track:%d side:%d sector:%d",rpm,bitrate,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack);
			
			skew=0;			
			trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
			for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
			{
				
				floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
				currentcylinder=floppydisk->tracks[j];
				
				for(i=0;i<floppydisk->floppyNumberOfSide;i++)
				{						
					file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
								(sectorsize*(floppydisk->floppySectorPerTrack)*i);
					
					fseek (f , file_offset , SEEK_SET);
					
					fread(trackdata,sectorsize*floppydisk->floppySectorPerTrack,1,f);
					
					currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500|NO_SECTOR_UNDER_INDEX,-2500);
				}
			}
			
			
			floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		}
		
		fclose(f);
		return LOADER_NOERROR;
	}
	
	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}


