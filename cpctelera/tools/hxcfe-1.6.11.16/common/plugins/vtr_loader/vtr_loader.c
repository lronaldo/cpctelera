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
// File : vtr_loader.c
// Contains: VTR (VTrucco file format) floppy image loader and plugins interfaces
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

#include "vtr_loader.h"
#include "vtrucco_file_writer.h"

#include "../common/os_api.h"

extern unsigned char bit_inverter[];



int VTR_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	FILE *f;
	vtrucco_picfileformatheader header;

	floppycontext->hxc_printf(MSG_DEBUG,"VTR_libIsValidDiskFile %s",imgfile);

	if(imgfile)
	{
		pathlen=strlen(imgfile);
		if(pathlen!=0)
		{
			f=fopen(imgfile,"rb");
			if(f==NULL) 
			{
				return LOADER_ACCESSERROR;
			}
			fread(&header,sizeof(header),1,f);
			fclose(f);

			if( !strncmp(header.HEADERSIGNATURE,"VTrucco",7))
			{
				floppycontext->hxc_printf(MSG_DEBUG,"VTrucco file !");
				return LOADER_ISVALID;
			}
			else
			{
				floppycontext->hxc_printf(MSG_DEBUG,"non VTrucco file !");
				return LOADER_BADFILE;
			}				
		}
	}
	
	return LOADER_BADPARAMETER;
}

int VTR_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	vtrucco_picfileformatheader header;
	unsigned int i,j,k,l,offset,offset2;
	CYLINDER* currentcylinder;
	SIDE* currentside;
    vtrucco_pictrack* trackoffsetlist;
    unsigned int tracks_base;
    unsigned char * hfetrack;
	unsigned int nbofblock,tracklen;


	floppycontext->hxc_printf(MSG_DEBUG,"VTR_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fread(&header,sizeof(header),1,f);

	if(!strncmp(header.HEADERSIGNATURE,"VTrucco",7))
	{

		floppydisk->floppyNumberOfTrack=header.number_of_track;
		floppydisk->floppyNumberOfSide=header.number_of_side;
		floppydisk->floppyBitRate=header.bitRate*1000;
		floppydisk->floppySectorPerTrack=-1;
		floppydisk->floppyiftype=header.floppyinterfacemode;


		floppycontext->hxc_printf(MSG_DEBUG,"VTrucco File : %d track, %d side, %d bit/s, %d sectors, mode %d",
			floppydisk->floppyNumberOfTrack,
			floppydisk->floppyNumberOfSide,
			floppydisk->floppyBitRate,
			floppydisk->floppySectorPerTrack,
			floppydisk->floppyiftype);

        trackoffsetlist=(vtrucco_pictrack*)malloc(sizeof(vtrucco_pictrack)* header.number_of_track);
        memset(trackoffsetlist,0,sizeof(vtrucco_pictrack)* header.number_of_track);
        fseek( f,512,SEEK_SET);
        fread( trackoffsetlist,sizeof(vtrucco_pictrack)* header.number_of_track,1,f);

        tracks_base= 512+( (((sizeof(vtrucco_pictrack)* header.number_of_track)/512)+1)*512);
        fseek( f,tracks_base,SEEK_SET);

		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
		memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);


		for(i=0;i<floppydisk->floppyNumberOfTrack;i++)
		{			

			fseek(f,(trackoffsetlist[i].offset*512),SEEK_SET);
			if(trackoffsetlist[i].track_len&0x1FF)
			{
				tracklen=(trackoffsetlist[i].track_len&(~0x1FF))+0x200;
			}
			else
			{
				tracklen=trackoffsetlist[i].track_len;
			}

			hfetrack=(unsigned char*)malloc( tracklen );
			
			fread( hfetrack,tracklen,1,f);

			floppydisk->tracks[i]=allocCylinderEntry(300,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[i];
			

		/*	floppycontext->hxc_printf(MSG_DEBUG,"read track %d side %d at offset 0x%x (0x%x bytes)",
			trackdesc.track_number,
			trackdesc.side_number,
			trackdesc.mfmtrackoffset,
			trackdesc.mfmtracksize);
*/
			for(j=0;j<currentcylinder->number_of_side;j++)
			{
				currentcylinder->sides[j]=malloc(sizeof(SIDE));
				memset(currentcylinder->sides[j],0,sizeof(SIDE));
				currentside=currentcylinder->sides[j];

				currentside->number_of_sector=floppydisk->floppySectorPerTrack;
				currentside->tracklen=tracklen/2;

				currentside->databuffer=malloc(currentside->tracklen);
				memset(currentside->databuffer,0,currentside->tracklen);
						
				currentside->flakybitsbuffer=0;
							
				currentside->indexbuffer=malloc(currentside->tracklen);
				memset(currentside->indexbuffer,0,currentside->tracklen);
							
				for(k=0;k<256;k++)
				{
					currentside->indexbuffer[k]=0xFF;
				}
						
				currentside->timingbuffer=0;
				currentside->bitrate=floppydisk->floppyBitRate;
				currentside->track_encoding=UNKNOWN_ENCODING;

				nbofblock=(currentside->tracklen/256);
				for(k=0;k<nbofblock;k++)
				{
					for(l=0;l<256;l++)
					{
						offset=(k*256)+l;
						offset2=(k*512)+l+(256*j);
						currentside->databuffer[offset]=bit_inverter[hfetrack[offset2]];
					}
				}

				currentside->tracklen=currentside->tracklen*8;
			}
		
			free(hfetrack);
		}
		
		free(trackoffsetlist);
	
		fclose(f);
		return LOADER_NOERROR;
	}	
	
	fclose(f);	
	floppycontext->hxc_printf(MSG_ERROR,"bad header");
	return LOADER_BADFILE;
}

