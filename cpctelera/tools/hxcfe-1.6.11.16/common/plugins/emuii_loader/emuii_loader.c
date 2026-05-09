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
// File : emuii_loader.c
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
#include "../common/emuii_track.h"
#include "../common/track_generator.h"

#include "emuii_loader.h"

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


int EMUII_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
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
				
				if(strstr( filepath,".eii" )!=NULL)
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


					floppycontext->hxc_printf(MSG_DEBUG,"EMUII file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non EMUII file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int EMUII_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f_eii,*f_os;
	unsigned int i;
	char os_filename[512];

	CYLINDER* currentcylinder;
	SIDE* currentside;
	unsigned char sector_data[0xE00];
	int tracknumber,sidenumber;
 

	floppycontext->hxc_printf(MSG_DEBUG,"EMUII_libLoad_DiskFile %s",imgfile);
	
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
	sprintf(&os_filename[i],"emuiios.emuiifd");

	f_os=fopen(os_filename,"rb");
	if(f_os==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open os file %s !",os_filename);
		return LOADER_ACCESSERROR;
	}
	
	f_eii=fopen(imgfile,"rb");
	if(f_eii==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	floppydisk->floppyNumberOfTrack=80;
	floppydisk->floppyNumberOfSide=2;
	floppydisk->floppyBitRate=DEFAULT_EMUII_BITRATE;
	floppydisk->floppySectorPerTrack=1;
	floppydisk->floppyiftype=EMU_SHUGART_FLOPPYMODE;

	floppycontext->hxc_printf(MSG_DEBUG,"EmuII File : %d track, %d side, %d bit/s, %d sectors, mode %d",
		floppydisk->floppyNumberOfTrack,
		floppydisk->floppyNumberOfSide,
		floppydisk->floppyBitRate,
		floppydisk->floppySectorPerTrack,
		floppydisk->floppyiftype);


	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
	memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	for(i=0;i<(unsigned int)(floppydisk->floppyNumberOfTrack*floppydisk->floppyNumberOfSide);i++)
	{			

		tracknumber=i>>1;
		sidenumber=i&1;
	
		if((i<22) || (i>=158))
		{
			fseek(f_os,i*0xE00,SEEK_SET);
			memset(&sector_data,0x00,0xE00);
			fread(&sector_data,0xE00,1,f_os);
		}
		else
		{
			memset(&sector_data,0xFF,0xE00);

			fseek(f_os,i*0xE00,SEEK_SET);
			fread(&sector_data,0xE00,1,f_os);

			fseek(f_eii,(i-22)*0xE00,SEEK_SET);
			fread(&sector_data,0xE00,1,f_eii);
		}

		if(!floppydisk->tracks[tracknumber])
		{
			floppydisk->tracks[tracknumber]=allocCylinderEntry(300,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[tracknumber];
		}
			

		floppycontext->hxc_printf(MSG_DEBUG,"track %d side %d at offset 0x%x (0x%x bytes)",
			tracknumber,
			sidenumber,
			(0xE00*tracknumber*2)+(sidenumber*0xE00),
			0xE00);

		currentcylinder->sides[sidenumber]=tg_alloctrack(floppydisk->floppyBitRate,EMU_FM_ENCODING,currentcylinder->floppyRPM,((floppydisk->floppyBitRate/5)*2),2000,-2000,0x00);
		currentside=currentcylinder->sides[sidenumber];					
		currentside->number_of_sector=floppydisk->floppySectorPerTrack;
			
		BuildEmuIITrack(floppycontext,tracknumber,sidenumber,sector_data,currentside->databuffer,&currentside->tracklen,2);
			
	}			
	
	fclose(f_eii);
	fclose(f_os);
	return LOADER_NOERROR;

}
