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
// File : emax_loader.c
// Contains: Emax floppy image loader and plugins interfaces
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

#include "emax_loader.h"

#include "../common/os_api.h"


#define SCTR_SIZE 512
#define HEADS 2
#define SCTR_TRK 10
#define TRK_CYL 80
#define START_SCTR 1
#define END_SCTR 10

#define RWDISK_VERSION "1.1"
#define RWDISK_DATE "Fri Mar 19 13:31:05 1993" 
#define EMAXUTIL_HDR "emaxutil v%3s %24s\n"
#define EMAXUTIL_HDRLEN 39 	



#define BANK_LOW 368
#define BANK_HIGH 423
#define SAMPLE_LOW 440
#define SAMPLE_HIGH 1463
#define OS1_LOW 0
#define OS1_HIGH 367
#define OS2_LOW 424
#define OS2_HIGH 439
#define OS3_LOW 1464
#define OS3_HIGH 1599
#define TOTAL_BLKS ((BANK_HIGH-BANK_LOW)+(SAMPLE_HIGH-SAMPLE_LOW))
#define TOTAL_OS ((OS1_HIGH-OS1_LOW)+(OS2_HIGH-OS2_LOW)+(OS3_HIGH-OS3_LOW))


int EMAX_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,filesize;
	char * filepath;
	FILE * f;
	floppycontext->hxc_printf(MSG_DEBUG,"EMAX_libIsValidDiskFile %s",imgfile);
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
				
				if((strstr( filepath,".em1" )!=NULL) || (strstr( filepath,".em2" )!=NULL) || (strstr( filepath,".emx" )!=NULL))
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


					floppycontext->hxc_printf(MSG_DEBUG,"Emax file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non Emax file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int EMAX_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f,*f2;
	unsigned int filesize;
	unsigned int i,j,k;
	unsigned int file_offset;
	unsigned char* floppy_data;
	char os_filename[512];
	unsigned char  gap3len,interleave;
	unsigned short sectorsize,rpm;
	unsigned short numberofsector;
	unsigned char  trackformat,skew;

	CYLINDER* currentcylinder;
	SECTORCONFIG  sectorconfig[10];
    
	char hdr[EMAXUTIL_HDRLEN+1];
    char fhdr[EMAXUTIL_HDRLEN+1]; 

	floppycontext->hxc_printf(MSG_DEBUG,"EMAX_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 
	
	numberofsector=10;
	
	sprintf(os_filename,imgfile);
	i=strlen(os_filename)-1;
	while(i && (os_filename[i]!='\\') && (os_filename[i]!='/') )
	{
		
		i--;
	}
	if(i)
	{
		i++;
	}
	sprintf(&os_filename[i],"emaxos.emx");

	f2=fopen(os_filename,"rb");
	if(f2==NULL) 
	{	
		fclose(f);
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open os file %s !",os_filename);
		return LOADER_ACCESSERROR;
	}

	if(filesize!=0)
	{		
		
		sectorsize=512;
		gap3len=255;
		interleave=1;
		skew=2;
		trackformat=ISOFORMAT_DD;
		floppydisk->floppyNumberOfSide=2;
		floppydisk->floppySectorPerTrack=10;
		floppydisk->floppyNumberOfTrack=80;

		if(1)
		{
			
			floppydisk->floppyBitRate=250000;
			floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
			rpm=300; // normal rpm
			
			floppycontext->hxc_printf(MSG_INFO_1,"filesize:%dkB, %d tracks, %d side(s), %d sectors/track, gap3:%d, interleave:%d,rpm:%d",filesize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,gap3len,interleave,rpm);
				
			floppy_data= malloc((SCTR_SIZE * SCTR_TRK) * TRK_CYL * HEADS);
			memset(floppy_data,0xE6,(SCTR_SIZE * SCTR_TRK) * TRK_CYL * HEADS);

			sprintf (hdr, EMAXUTIL_HDR, RWDISK_VERSION, RWDISK_DATE); 
			hdr[EMAXUTIL_HDRLEN]=0;
			
			fread (fhdr, (unsigned int) EMAXUTIL_HDRLEN,1,f);
			fhdr[EMAXUTIL_HDRLEN]=0;


			if (strncmp(fhdr, hdr, EMAXUTIL_HDRLEN)!=0)
			{
				floppycontext->hxc_printf(MSG_ERROR,"Wrong version: disk says %s", fhdr);
				fclose(f);
				fclose(f2);
				return LOADER_BADFILE;
			} 


			for(i=OS1_LOW;i<=OS1_HIGH;i++)
			{
				fread(&floppy_data[i*512],512,1,f2);
			}

			for(i=OS2_LOW;i<=OS2_HIGH;i++)
			{
				fread(&floppy_data[i*512],512,1,f2);
			}

			for(i=OS3_LOW;i<=OS3_HIGH;i++)
			{
				fread(&floppy_data[i*512],512,1,f2);
			}

			for(i=BANK_LOW;i<=BANK_HIGH;i++)
			{
				fread(&floppy_data[i*512],512,1,f);
			}

			for(i=SAMPLE_LOW;i<=SAMPLE_HIGH;i++)
			{
				fread(&floppy_data[i*512],512,1,f);
			}

			for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
			{	
				floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
				currentcylinder=floppydisk->tracks[j];
				
				for(i=0;i<floppydisk->floppyNumberOfSide;i++)
				{
					
					file_offset=( (((numberofsector)*512)) * floppydisk->floppyNumberOfSide * j ) +
						        ( (((numberofsector)*512)) * i );

					memset(sectorconfig,0,sizeof(SECTORCONFIG)*10);
					for(k=0;k<10;k++)
					{
						sectorconfig[k].head=i;
						sectorconfig[k].cylinder=j;
						sectorconfig[k].sector=k+1;
						sectorconfig[k].sectorsize=512;
						sectorconfig[k].bitrate=floppydisk->floppyBitRate;
						sectorconfig[k].gap3=gap3len;
						sectorconfig[k].trackencoding=trackformat;
						sectorconfig[k].input_data=&floppy_data[file_offset+(k*512)];

					}

					currentcylinder->sides[i]=tg_generatetrackEx(floppydisk->floppySectorPerTrack,(SECTORCONFIG *)&sectorconfig,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,rpm,trackformat,2500|NO_SECTOR_UNDER_INDEX,-2500);
				}
			}

			free(floppy_data);
			
			floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		
			fclose(f);
			fclose(f2);
			return LOADER_NOERROR;

		}
		fclose(f);
		fclose(f2);
		return LOADER_FILECORRUPT;
	}
	
	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	fclose(f2);
	return LOADER_BADFILE;
}
