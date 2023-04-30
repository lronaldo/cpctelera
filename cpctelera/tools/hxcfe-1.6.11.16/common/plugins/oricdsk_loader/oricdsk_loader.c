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
// File : OricDSK_DiskFile.c
// Contains: OricDSK floppy image loader and plugins interfaces
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

#include "oricdsk_loader.h"
#include "oricdsk_format.h"

#include "../common/os_api.h"

int OricDSK_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	FILE *f;
	unsigned char fileheader[10];
	floppycontext->hxc_printf(MSG_DEBUG,"OricDSK_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".dsk" )!=NULL)
				{
					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						return LOADER_ACCESSERROR;
					}
					fread(fileheader,10,1,f);
					fclose(f);
					
					free(filepath);
					fileheader[8]=0;
					if( !strcmp(fileheader,"MFM_DISK") ||
					    !strcmp(fileheader,"ORICDISK"))
					{
						floppycontext->hxc_printf(MSG_DEBUG,"OricDSK file !");
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non OricDSK file (bad header)!");
						return LOADER_BADFILE;
					}

				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non OricDSK file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}




typedef struct mfmformatsect_
{
	unsigned char idam1[3]; // 0xA1 * 3
	unsigned char idam2;    // 0xFE
	unsigned char track;
	unsigned char head;
	unsigned char sector;
	unsigned char bytes;
	unsigned char crc[2];
	unsigned char gap2[22]; // 0x4E
	unsigned char sync[12]; // 0x00
	unsigned char dataam1[3]; // 0xA1 * 3
	unsigned char dataam2;    // 0xFB
}mfmformatsect;


typedef struct sectorlist_
{
	unsigned short sectorsize;
	unsigned char sectorid;
	unsigned char trackid;
	unsigned char headid;
	unsigned char * data;
}sectorlist;

int issector(mfmformatsect * sector)
{
	int i;

	i=0;
	do
	{
		if(sector->idam1[i]!=0xA1)
		{	
			return 0;
		}
			
		i++;
	}while(i<3);


	i=0;
	do
	{
		if(sector->dataam1[i]!=0xA1)
		{	
			return 0;
		}
		i++;
	}while(i<3);


	if(sector->idam2!=0xFE)
	{
		return 0;
	}

	if((sector->dataam2!=0xFB) && (sector->dataam2!=0xF8))
	{
		return 0;
	}

	return 1;

}

SECTORCONFIG * extractsector(HXCFLOPPYEMULATOR* floppycontext,unsigned char *data,unsigned char * inputtrack,int tracksize,int * numberofsector)
{
	int i,j,k;
	int sectorsize;
	SECTORCONFIG * tabsector;

	mfmformatsect * testsector;

	tabsector=(SECTORCONFIG *)malloc(sizeof(SECTORCONFIG)*32);
	memset(tabsector,0,sizeof(SECTORCONFIG)*32);

	i=0;
	j=0;
	k=0;
	do	
	{
		testsector=(mfmformatsect *)(inputtrack+i);
		if(!issector(testsector))
		{
			i++;
		}
		else
		{
		
			floppycontext->hxc_printf(MSG_DEBUG,"Sector found ! %.4x",i);
			sectorsize=128<<(testsector->bytes&7);

			tabsector[j].head=testsector->head;
			tabsector[j].sector=testsector->sector;
			tabsector[j].sectorsize=sectorsize;
			tabsector[j].cylinder=testsector->track;
			tabsector[j].use_alternate_data_crc=0;
			tabsector[j].use_alternate_header_crc=0;
			tabsector[j].header_crc=0;
			tabsector[j].missingdataaddressmark=0;
			tabsector[j].startdataindex=0;
			tabsector[j].startsectorindex=0;
			
			tabsector[j].use_alternate_datamark=0xFF;
			tabsector[j].alternate_datamark=testsector->dataam2;

			tabsector[j].use_alternate_addressmark=0;
			tabsector[j].gap3=255;
			tabsector[j].bitrate=DEFAULT_DD_BITRATE;
			tabsector[j].trackencoding=IBMFORMAT_DD;
			
			memcpy(&data[k],&inputtrack[i+sizeof(mfmformatsect)],sectorsize);
			tabsector[j].input_data=&data[k];
			k=k+sectorsize;
			j++;

			i=i+sectorsize+sizeof(mfmformatsect)+2;

			floppycontext->hxc_printf(MSG_DEBUG,"%d bytes, track %d, sector %d, side %d",sectorsize,testsector->track,testsector->sector,testsector->head);


		}

	}while(i<tracksize);

	*numberofsector=j;

	return tabsector;
}




int OricDSK_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{ 
	FILE * f;
	unsigned int filesize;
	unsigned int i,j,mfmformat,geometrie;
	unsigned int file_offset;
	char* trackdata;
	unsigned char* trackdatatab;
	int tracklen;
	int tracksize;
	int numberofsector;
	unsigned short sectorsize,rpm;
	unsigned char interleave,gap3len;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	SECTORCONFIG * sectlist;
	oricdsk_fileheader * fileheader;
	
	floppycontext->hxc_printf(MSG_DEBUG,"OricDSK_libLoad_DiskFile %s",imgfile);
	
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
		mfmformat=0;
		fileheader=(oricdsk_fileheader *) malloc(256);
		fread(fileheader,256,1,f);
		sectorsize=256; // OricDSK file support only 256bytes/sector floppies.
		gap3len=255;
		interleave=1;	
		rpm=300; // normal rpm
		
		
		floppydisk->floppyNumberOfTrack=(unsigned short)fileheader->number_of_tracks;
		floppydisk->floppyNumberOfSide=(unsigned char)fileheader->number_of_side;
		floppydisk->floppyBitRate=250000;
		floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
		
		floppycontext->hxc_printf(MSG_DEBUG,"%d tracks, %d side",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide);
		
		
		if( !strncmp(fileheader->headertag,"MFM_DISK",8))
		{
			mfmformat=1;
			geometrie=fileheader->number_of_sectors_geometrie;
			floppydisk->floppySectorPerTrack=-1;
			floppycontext->hxc_printf(MSG_DEBUG,"OricDSK_libLoad_DiskFile MFM_DISK %d tracks %d sides geometrie %d",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,geometrie);
		}
		else
		{
			if(	!strncmp(fileheader->headertag,"ORICDISK",8))
			{
				mfmformat=0;
				geometrie=0;
				floppydisk->floppySectorPerTrack=(unsigned short)fileheader->number_of_sectors_geometrie;
				floppycontext->hxc_printf(MSG_DEBUG,"OricDSK_libLoad_DiskFile ORICDISK %d tracks %d sides %d",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack);
			}
			else
			{
				return LOADER_BADFILE;
			}
		}
		
		
		
		tracklen=(floppydisk->floppyBitRate/((rpm)/60))/4;
		
		switch(mfmformat)
		{
			
		case 0:// "OLD" format
			floppycontext->hxc_printf(MSG_DEBUG,"ORICDISK format !");
			
			trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
				for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
				{
					
					if(!floppydisk->tracks[j])
					{
						floppydisk->tracks[j]=(CYLINDER*)malloc(sizeof(CYLINDER));
						currentcylinder=floppydisk->tracks[j];
						currentcylinder->number_of_side=floppydisk->floppyNumberOfSide;
						currentcylinder->floppyRPM=rpm;
					}

					file_offset=((j*(sectorsize*floppydisk->floppySectorPerTrack))+
						(sectorsize*(floppydisk->floppySectorPerTrack)*floppydisk->floppyNumberOfTrack*i))+256;
					
					fseek (f , file_offset , SEEK_SET); 
					fread(trackdata,sectorsize*floppydisk->floppySectorPerTrack,1,f);
					
					currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,1,0,floppydisk->floppyBitRate,currentcylinder->floppyRPM,IBMFORMAT_DD,gap3len,2500|NO_SECTOR_UNDER_INDEX,-2500);
					currentside=currentcylinder->sides[i];					
				}
				
			}
			free(trackdata);
			break;
			
		case 1:// "New" format	
			floppycontext->hxc_printf(MSG_DEBUG,"MFM_DISK format !");
			
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
			
			for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
			{
				floppydisk->tracks[j]=(CYLINDER*)malloc(sizeof(CYLINDER));
				currentcylinder=floppydisk->tracks[j];
				currentcylinder->number_of_side=(unsigned char)fileheader->number_of_side;
				currentcylinder->floppyRPM=rpm;
				
				for(i=0;i<floppydisk->floppyNumberOfSide;i++)
				{
					currentcylinder->sides=(SIDE**)malloc(sizeof(SIDE*)*currentcylinder->number_of_side);
					memset(currentcylinder->sides,0,sizeof(SIDE*)*currentcylinder->number_of_side);
				}
			}
			
			
			switch(geometrie)
			{
			case 0:
				break;
			case 1:
				floppycontext->hxc_printf(MSG_DEBUG,"Geometrie 1:  11111..000000!");
				tracksize=(((filesize-256)/fileheader->number_of_side)/fileheader->number_of_tracks);
				for(i=0;i<floppydisk->floppyNumberOfSide;i++)
				{
					for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"------------------ Track %d, side %d -------------------",j,i);	
						currentcylinder=floppydisk->tracks[j];
						
						file_offset=(i*floppydisk->floppyNumberOfTrack*tracksize)+(j*tracksize)+256;
						
						trackdata=(unsigned char *) malloc(tracksize);						
						fseek (f , file_offset , SEEK_SET); 
						fread(trackdata,tracksize,1,f);
												
						trackdatatab=(unsigned char*)malloc(256*1024);
						sectlist=extractsector(floppycontext,trackdatatab,trackdata,tracksize,&numberofsector);
						
						currentcylinder->sides[i]=tg_generatetrackEx((unsigned short)numberofsector,sectlist,interleave,(unsigned char)(j*2),DEFAULT_DD_BITRATE,rpm,IBMFORMAT_DD,2500|NO_SECTOR_UNDER_INDEX,-2500);
						currentside=currentcylinder->sides[i];
						
						free(trackdatatab);
						free(sectlist);
						free(trackdata);
					}
				}
				break;
				
			case 2:
				floppycontext->hxc_printf(MSG_DEBUG,"Geometrie 2:  01010101010...!");
				tracksize=(((filesize-256)/fileheader->number_of_side)/fileheader->number_of_tracks);
				for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
				{
					floppydisk->tracks[j]=(CYLINDER*)malloc(sizeof(CYLINDER));
					currentcylinder=floppydisk->tracks[j];
					currentcylinder->number_of_side=floppydisk->floppyNumberOfSide;
					currentcylinder->floppyRPM=rpm;
				}
				
				for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
				{
					currentcylinder=floppydisk->tracks[j];
					
					for(i=0;i<floppydisk->floppyNumberOfSide;i++)
					{	
						floppycontext->hxc_printf(MSG_DEBUG,"------------------ Track %d, side %d -------------------",j,i);	
						
						file_offset=(i*floppydisk->floppyNumberOfTrack*tracksize)+(j*tracksize)+256;
						
						trackdata=(unsigned char *) malloc(tracksize);
						
						fseek (f , file_offset , SEEK_SET); 
						fread(trackdata,tracksize,1,f);
						
						trackdatatab=(unsigned char*)malloc(256*1024);
						sectlist=extractsector(floppycontext,trackdatatab,trackdata,tracksize,&numberofsector);

						currentcylinder->sides[i]=tg_generatetrackEx((unsigned short)numberofsector,sectlist,interleave,(unsigned char)(j*2),DEFAULT_DD_BITRATE,rpm,IBMFORMAT_DD,2500|NO_SECTOR_UNDER_INDEX,-2500);

						free(trackdatatab);
						free(sectlist);
						free(trackdata);
					}
				}	
				break;
				
			default:
				break;
			}			
		break;
		}
		
		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		fclose(f);
		
		return LOADER_NOERROR;
		
	}

	fclose(f);
	
	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	return LOADER_BADFILE;
}

