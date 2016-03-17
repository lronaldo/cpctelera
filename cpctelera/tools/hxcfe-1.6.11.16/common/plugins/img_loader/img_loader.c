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
// File : IMG_DiskFile.c
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

#include "img_loader.h"

#include "pcimgfileformat.h"

#include "../common/os_api.h"

int pc_imggetfloppyconfig(unsigned char * img,unsigned int filesize,unsigned short *numberoftrack,unsigned char *numberofside,unsigned short *numberofsectorpertrack,unsigned char *gap3len,unsigned char *interleave,unsigned short *rpm, unsigned int *bitrate,unsigned short * ifmode)
{
	int i;
	unsigned char * uimg;
	int conffound,numberofsector;
	
	conffound=0;
	uimg=(unsigned char *)img;
	
	if(uimg[0x18]<24 && uimg[0x18]>7)
	{
				
		*rpm=300;
		*numberofsectorpertrack=uimg[0x18];
		*numberofside=uimg[0x1A];
		if(*numberofsectorpertrack<=10)
		{
			*gap3len=84;
			*interleave=1;
			*bitrate=250000;
			*ifmode=IBMPC_DD_FLOPPYMODE;
		}
		else
		{

			if(*numberofsectorpertrack<=21)
			{
				*bitrate=500000;
				*gap3len=84;
				*interleave=1;
				*ifmode=IBMPC_HD_FLOPPYMODE;
				if(*numberofsectorpertrack>18)
				{
					*gap3len=14;
					*interleave=2;
				}
			}
			else
			{
				*bitrate=1000000;
				*gap3len=84;
				*interleave=1;
				*ifmode=IBMPC_ED_FLOPPYMODE;
			}


		}
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
			
			if(pcimgfileformats[i].filesize==filesize)
			{
				*numberoftrack=pcimgfileformats[i].numberoftrack;
				*numberofsectorpertrack=pcimgfileformats[i].sectorpertrack;
				*numberofside=pcimgfileformats[i].numberofside;
				*gap3len=pcimgfileformats[i].gap3len;
				*interleave=pcimgfileformats[i].interleave;
				*rpm=pcimgfileformats[i].RPM;
				*bitrate=pcimgfileformats[i].bitrate;
				*ifmode=pcimgfileformats[i].interface_mode;
				conffound=1;
			}
			i++;

		}while(pcimgfileformats[i].filesize!=0 && conffound==0);	
	}
	return conffound;
}


int IMG_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,i,conffound;
	unsigned int filesize;
	char * filepath;
	FILE *f;
	floppycontext->hxc_printf(MSG_DEBUG,"IMG_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".img" )!=NULL || strstr( filepath,".ima" )!=NULL)
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
					
					fclose(f);

					if(filesize&0x1FF)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non IMG file - bad file size !");
						return LOADER_BADFILE;
					}

					i=0;
					conffound=0;
					do
					{
						if(pcimgfileformats[i].filesize==filesize)
						{
							conffound=1;
						}
						i++;
					}while(pcimgfileformats[i].filesize!=0 && conffound==0);

					if(!conffound)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non IMG file - bad file size !");
						return LOADER_BADFILE;
					}

					floppycontext->hxc_printf(MSG_DEBUG,"IMG file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non IMG file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}



int IMG_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned char* trackdata;
	unsigned char gap3len,interleave;
	unsigned short rpm;
	unsigned short sectorsize;
	unsigned char trackformat;
	CYLINDER* currentcylinder;
	
	floppycontext->hxc_printf(MSG_DEBUG,"IMG_libLoad_DiskFile %s",imgfile);
	
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
		trackdata=(char*)malloc(sectorsize);
		fread(trackdata,sectorsize,1,f);
		if(pc_imggetfloppyconfig(
			trackdata,
			filesize,
			&floppydisk->floppyNumberOfTrack,
			&floppydisk->floppyNumberOfSide,
			&floppydisk->floppySectorPerTrack,
			&gap3len,
			&interleave,
			&rpm,
			&floppydisk->floppyBitRate,
			&floppydisk->floppyiftype)==1
			)
		{
		
			free(trackdata);
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
			floppycontext->hxc_printf(MSG_DEBUG,"rpm %d bitrate:%d track:%d side:%d sector:%d",rpm,floppydisk->floppyBitRate,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack);
			
			trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
			trackformat=IBMFORMAT_DD;

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
					
					currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(0),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500|REVERTED_INDEX,-2500);
				}
			}
			
			free(trackdata);

			floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		}
		fclose(f);
		return LOADER_NOERROR;
	}
	
	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}


