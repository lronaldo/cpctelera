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
// File : fei_loader.c
// Contains: FEI floppy image loader and plugins interfaces
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

#include "fei_loader.h"

#include "../common/os_api.h"

#include "../common/track_generator.h"

extern unsigned char bit_inverter[];

int FEI_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,filesize;
	char * filepath;
	FILE *f;

	floppycontext->hxc_printf(MSG_DEBUG,"FEI_libIsValidDiskFile %s",imgfile);

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

				if(strstr( filepath,".fei" )!=NULL)
				{
					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						return LOADER_ACCESSERROR;
					}
					fseek (f , 0 , SEEK_END); 
					filesize=ftell(f);
					fseek (f , 0 , SEEK_SET); 

					fclose(f);

					floppycontext->hxc_printf(MSG_DEBUG,"FEI file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non FEI file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}

int FEI_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	unsigned int i,j,k,filesize;
	unsigned short rpm;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	unsigned int tracksize;
	
	
	floppycontext->hxc_printf(MSG_DEBUG,"FEI_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 

	if(!(filesize%25000))
	{
		floppydisk->floppyNumberOfTrack=(filesize/2) / 25000;
		floppydisk->floppyNumberOfSide=2;
		floppydisk->floppyBitRate=500000;
		floppydisk->floppySectorPerTrack=-1;
		floppydisk->floppyiftype=ATARIST_DD_FLOPPYMODE;	
		tracksize=25000;
		rpm=300;
	}
	else
	{

		if(!(filesize%12500))
		{
			floppydisk->floppyNumberOfTrack=(filesize/2) / 12500;
			floppydisk->floppyNumberOfSide=2;
			floppydisk->floppyBitRate=250000;
			floppydisk->floppySectorPerTrack=-1;
			floppydisk->floppyiftype=ATARIST_DD_FLOPPYMODE;
			tracksize=12500;
			rpm=300;
		}
	}
	
	

	floppycontext->hxc_printf(MSG_DEBUG,"FEI File : %d track, %d side, %d bit/s, %d sectors, mode %d",
		floppydisk->floppyNumberOfTrack,
		floppydisk->floppyNumberOfSide,
		floppydisk->floppyBitRate,
		floppydisk->floppySectorPerTrack,
		floppydisk->floppyiftype);


	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
	memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	for(i=0;i<floppydisk->floppyNumberOfTrack;i++)
	{			

		for(j=0;j<floppydisk->floppyNumberOfSide;j++)
		{
		
			if(!floppydisk->tracks[i])
			{
				floppydisk->tracks[i]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
				currentcylinder=floppydisk->tracks[i];
			}
				
			currentcylinder->sides[j]=tg_alloctrack(floppydisk->floppyBitRate,UNKNOWN_ENCODING,currentcylinder->floppyRPM,tracksize*8,2500,-2500,0x00);
			currentside=currentcylinder->sides[j];
			currentside->number_of_sector=floppydisk->floppySectorPerTrack;
			
			fseek(f,(tracksize*i)+(tracksize*floppydisk->floppyNumberOfTrack*j),SEEK_SET);

			floppycontext->hxc_printf(MSG_DEBUG,"read track %d side %d at offset 0x%x (0x%x bytes)",
												i,
												j,
												ftell(f),
												tracksize);

			fread(currentside->databuffer,tracksize,1,f);

			for(k=0;k<tracksize;k++)
			{
				currentside->databuffer[k]=bit_inverter[currentside->databuffer[k]];
			}
		}
	}			
	
	fclose(f);
	return LOADER_NOERROR;	
}

