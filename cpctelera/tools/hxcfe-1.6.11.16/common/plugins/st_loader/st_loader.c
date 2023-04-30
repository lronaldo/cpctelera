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
// File : ST_DiskFile.c
// Contains: ST floppy image loader and plugins interfaces
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

#include "st_loader.h"

#include "../common/os_api.h"

#include "stfileformat.h"

int getfloppyconfig(unsigned char * img,unsigned int filesize,unsigned short *numberoftrack,unsigned char *numberofside,unsigned short *numberofsectorpertrack,unsigned char *gap3len,unsigned char *interleave)
{
	int i;
	unsigned char  nb_of_side,nb_of_track,nb_of_sector;
	unsigned short numberofsector;
	unsigned char  * uimg;
	unsigned char  conffound;
	
	uimg=(unsigned char *)img;
	
	conffound=0;
	if(uimg[0x18]<24 && uimg[0x18]>8)
	{
		
		*numberofsectorpertrack=uimg[0x18];
		*numberofside=uimg[0x1A];

		*gap3len=84;
		*interleave=1;

		switch(*numberofsectorpertrack)
		{
			case 9:
				*gap3len=84;
				*interleave=1;
			break;
			case 10:
				*gap3len=30;
				*interleave=1;
			break;
			case 11:
				*interleave=2;
				*gap3len=3;
			break;
		}
	
		numberofsector=uimg[0x13]+(uimg[0x14]*256);
		if(*numberofsectorpertrack && *numberofside )
		{
			*numberoftrack=(numberofsector/(*numberofsectorpertrack*(*numberofside)));
			
			if((unsigned int)((*numberofsectorpertrack) * (*numberoftrack) * (*numberofside) *512)==filesize)
			{
				conffound=1;
			}
		}
		
	}
	
	if(!conffound)
	{
		i=0;
		do
		{
			
			if(stfileformats[i].filesize==filesize)
			{
				*numberoftrack=stfileformats[i].numberoftrack;
				*numberofsectorpertrack=stfileformats[i].sectorpertrack;
				*numberofside=stfileformats[i].numberofside;
				*gap3len=stfileformats[i].gap3len;
				*interleave=stfileformats[i].interleave;
				conffound=1;
			}
			i++;
			
		}while(stfileformats[i].filesize!=0 && conffound==0);

		if(!conffound)
		{

			for(nb_of_side=1;nb_of_side<=2;nb_of_side++)
			{
				for(nb_of_track=0;nb_of_track<85;nb_of_track++)
				{
					for(nb_of_sector=8;nb_of_sector<=11;nb_of_sector++)
					{
						if(filesize==(unsigned int)(nb_of_side*nb_of_track*nb_of_sector*512))
						{
							*numberoftrack=nb_of_track;
							*numberofsectorpertrack=nb_of_sector;
							*numberofside=nb_of_side;

							*gap3len=84;
							*interleave=1;

							switch(*numberofsectorpertrack)
							{
								case 9:
									*gap3len=84;
									*interleave=1;
								break;
								case 10:
									*gap3len=30;
									*interleave=1;
								break;
								case 11:
									*interleave=2;
									*gap3len=3;
								break;
							}
							
							conffound=1;


							return conffound;
						}
					}					
				}
			}
		}
	}
	return conffound;
}


int ST_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,filesize;
	char * filepath;
	FILE * f;
	floppycontext->hxc_printf(MSG_DEBUG,"ST_libIsValidDiskFile %s",imgfile);
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
				
				if(strstr( filepath,".st" )!=NULL)
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
						floppycontext->hxc_printf(MSG_DEBUG,"non ST IMG file - bad file size !");
						return LOADER_BADFILE;
					}

					floppycontext->hxc_printf(MSG_DEBUG,"ST file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non ST file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int ST_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int file_offset;

	unsigned short i,j;
	unsigned char* trackdata;
	unsigned char  gap3len,interleave,skew,trackformat;
	unsigned short rpm;
	unsigned short sectorsize;
	CYLINDER* currentcylinder;
	
	floppycontext->hxc_printf(MSG_DEBUG,"ST_libLoad_DiskFile %s",imgfile);
	
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
		
		sectorsize=512; // st file support only 512bytes/sector floppies.
		// read the first sector
		trackdata=(unsigned char*)malloc(sectorsize);
		fread(trackdata,sectorsize,1,f);
		if(getfloppyconfig(
			trackdata,
			filesize,
			&floppydisk->floppyNumberOfTrack,
			&floppydisk->floppyNumberOfSide,
			&floppydisk->floppySectorPerTrack,
			&gap3len,
			&interleave)==1
			)
		{
			
			free(trackdata);

			if(floppydisk->floppySectorPerTrack<15)
			{
				floppydisk->floppyiftype=ATARIST_DD_FLOPPYMODE;
				floppydisk->floppyBitRate=DEFAULT_DD_BITRATE;
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
			}
			
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
			rpm=300; // normal rpm
			
			floppycontext->hxc_printf(MSG_INFO_1,"filesize:%dkB, %d tracks, %d side(s), %d sectors/track, gap3:%d, interleave:%d,rpm:%d bitrate:%d",filesize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,gap3len,interleave,rpm,floppydisk->floppyBitRate);	
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
	
	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}
