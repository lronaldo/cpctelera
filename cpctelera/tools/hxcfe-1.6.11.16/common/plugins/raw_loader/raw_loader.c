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
// File : RAW_DiskFile.c
// Contains: RAW floppy image loader and plugins interfaces
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

#include "raw_loader.h"



int RAW_libIsValidFormat(HXCFLOPPYEMULATOR* floppycontext,cfgrawfile * imgformatcfg)
{
	unsigned int tracktype;
	int tracklen;
	int gap3len,interleave,rpm,bitrate;
	int sectorsize;
	int tracklendiv;
	

	sectorsize=128<<(imgformatcfg->sectorsize&7);
	
	if(imgformatcfg->autogap3)
		gap3len=255;
	else
		gap3len=imgformatcfg->gap3;

	rpm=imgformatcfg->rpm;
	bitrate=imgformatcfg->bitrate;
	interleave=imgformatcfg->interleave;

	switch(imgformatcfg->tracktype)
	{
		case FM_TRACK_TYPE:
			tracktype=ISOFORMAT_SD;
			tracklendiv=16;
		break;

		case FMIBM_TRACK_TYPE:
			tracktype=IBMFORMAT_SD;
			tracklendiv=16;
		break;

		case MFM_TRACK_TYPE:
			tracktype=ISOFORMAT_DD;
			tracklendiv=8;
		break;

		case MFMIBM_TRACK_TYPE:
			tracktype=IBMFORMAT_DD;
			tracklendiv=8;
		break;

		case GCR_TRACK_TYPE:
			tracktype=0;
		break;
	};

	if(rpm==0) 
		return LOADER_BADPARAMETER;


	tracklen=((bitrate*60)/rpm)/tracklendiv;

	//finaltracksize=ISOIBMGetTrackSize(tracktype,imgformatcfg->sectorpertrack,sectorsize,gap3len,0);
	
	//if(finaltracksize<=tracklen)
	{
		return LOADER_ISVALID;
	}
	//else
	{
	//	return LOADER_BADPARAMETER;
	}
}



int RAW_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,cfgrawfile * imgformatcfg)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j,fileside,bitrate;
	unsigned int file_offset;
	unsigned char* trackdata;
	unsigned char gap3len,interleave,skew,curskew,tracktype,firstsectorid;
	unsigned short sectorsize,rpm;
	
	f=0;
	
	if(imgfile)
	{
		floppycontext->hxc_printf(MSG_DEBUG,"RAW_libLoad_DiskFile %s",imgfile);
		
		f=fopen(imgfile,"rb");
		if(f==NULL) 
		{
			floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
			return -1;
		}
		
		fseek (f , 0 , SEEK_END); 
		filesize=ftell(f);
		fseek (f , 0 , SEEK_SET); 
	}
	else
	{
		floppycontext->hxc_printf(MSG_DEBUG,"RAW_libLoad_DiskFile empty floppy");
	}	

	
	sectorsize=128<<(imgformatcfg->sectorsize&7);

	if(!imgformatcfg->autogap3)
		gap3len=(unsigned char)imgformatcfg->gap3;
	else
		gap3len=255;

	rpm=(unsigned short)imgformatcfg->rpm;
	bitrate=imgformatcfg->bitrate;
	interleave=imgformatcfg->interleave;
	skew=imgformatcfg->skew;

	floppydisk->floppyNumberOfTrack=(unsigned short)imgformatcfg->numberoftrack;
	
	if((imgformatcfg->sidecfg&TWOSIDESFLOPPY) || (imgformatcfg->sidecfg&SIDE_INVERTED))
	{
		floppydisk->floppyNumberOfSide=2;
	}
	else
	{
		floppydisk->floppyNumberOfSide=1;
	}
	
	floppydisk->floppySectorPerTrack=imgformatcfg->sectorpertrack;

	floppydisk->floppyBitRate=bitrate;
	floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	floppycontext->hxc_printf(MSG_DEBUG,"%d bytes sectors, %d sectors/tracks,interleaving %d, skew %d, %d tracks, %d side(s), gap3 %d, %d rpm, %d bits/s",
		sectorsize,
		floppydisk->floppySectorPerTrack,
		interleave,
		skew,
		floppydisk->floppyNumberOfTrack,
		floppydisk->floppyNumberOfSide,
		gap3len,
		rpm,
		bitrate
		);

	switch(imgformatcfg->tracktype)
	{
		case FM_TRACK_TYPE:
			tracktype=ISOFORMAT_SD;
			floppycontext->hxc_printf(MSG_DEBUG,"FM ISO tracks format");
		break;

		case FMIBM_TRACK_TYPE:
			tracktype=IBMFORMAT_SD;
			floppycontext->hxc_printf(MSG_DEBUG,"FM IBM tracks format");
		break;

		case MFM_TRACK_TYPE:
			tracktype=ISOFORMAT_DD;
			floppycontext->hxc_printf(MSG_DEBUG,"MFM ISO tracks format");
		break;

		case MFMIBM_TRACK_TYPE:
			tracktype=IBMFORMAT_DD;
			floppycontext->hxc_printf(MSG_DEBUG,"MFM IBM tracks format");
		break;

		case GCR_TRACK_TYPE:
			tracktype=0;
			floppycontext->hxc_printf(MSG_DEBUG,"GCR tracks format");
		break;
	};

	trackdata=(unsigned char*)malloc(sectorsize*floppydisk->floppySectorPerTrack);
			
	for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
	{
				
		floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
				
		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{
					

			if(imgformatcfg->sidecfg&TWOSIDESFLOPPY)
			{
				if(imgformatcfg->sidecfg&SIDE_INVERTED)
				{
					fileside=i^0x1;
				}
				else
				{
					fileside=i;
				}
			}
			else
			{
				fileside=0;
			}

			if(imgformatcfg->sidecfg&SIDE0_FIRST)
			{
				file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack))+
							(sectorsize*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfTrack*fileside);
			}
			else
			{
				file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
							(sectorsize*(floppydisk->floppySectorPerTrack)*fileside);
			}
			
			if(f)
			{
				fseek (f , file_offset , SEEK_SET);
					
				floppycontext->hxc_printf(MSG_DEBUG,"Track %d Head %d : Reading %d bytes at %.8X",j,i,sectorsize*floppydisk->floppySectorPerTrack,file_offset);
				fread(trackdata,sectorsize*floppydisk->floppySectorPerTrack,1,f);
				
			}
			else
			{
				memset(trackdata,imgformatcfg->fillvalue,sectorsize*floppydisk->floppySectorPerTrack);
			}
					
			firstsectorid=imgformatcfg->firstidsector;
			if(imgformatcfg->intersidesectornumbering)
			{
				if(i)
				{
					firstsectorid=firstsectorid+floppydisk->floppySectorPerTrack;
				}
			}

			if(tracktype)
			{
				if(imgformatcfg->sideskew)
					curskew=(unsigned char)(((j<<1)|(i&1))*skew);
				else
					curskew=(unsigned char)(j*skew);

				floppydisk->tracks[j]->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,(unsigned char)firstsectorid,interleave,(unsigned char)(curskew),floppydisk->floppyBitRate,rpm,tracktype,gap3len,2500|NO_SECTOR_UNDER_INDEX,-2500);
			}
		}
	}

	free(trackdata);
	if(f) fclose(f);

	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");



	return LOADER_NOERROR;
}
