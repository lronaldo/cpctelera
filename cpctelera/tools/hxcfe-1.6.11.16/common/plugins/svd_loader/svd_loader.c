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
// File : SVD_DiskFile.c
// Contains: SVD floppy image loader and plugins interfaces
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

#include "svd_loader.h"

#include "svd.h"

#include "../common/os_api.h"


int SVD_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	char   linebuffer[80];
	int major,minor;
	FILE * f;
	floppycontext->hxc_printf(MSG_DEBUG,"SVD_libIsValidDiskFile %s",imgfile);
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
				
				if(strstr( filepath,".svd" )!=NULL)
				{
					
					f=fopen(imgfile,"r");
					if(f)
					{
						fgets(linebuffer,sizeof(linebuffer),f);					
						fclose(f);

						if (sscanf(linebuffer,"%d.%d",&major,&minor) != 2) 
						{
							floppycontext->hxc_printf(MSG_DEBUG,"Bad code version !");
							return(LOADER_BADFILE);
						}
					
						if((major==2 && minor==0) ||(major==1 && minor==2) ||(major==1 && minor==5))
						{
							free(filepath);
							return LOADER_ISVALID;
						}
						else
						{
							floppycontext->hxc_printf(MSG_DEBUG,"Bad code version !");
							free(filepath);
							return LOADER_BADFILE;
						}
					}
					
					floppycontext->hxc_printf(MSG_DEBUG,"Access error !");
					free(filepath);
					return LOADER_ACCESSERROR;
					
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non SVD file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}




int SVD_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j,k,skew;
	unsigned int file_offset;
	unsigned char* trackdata;
	unsigned char  gap3len,interleave;
	unsigned short sectorsize,rpm;
	unsigned char Sector_attribute_flag;
	CYLINDER* currentcylinder;
	unsigned char trackformat;
	int major,minor;
	int sectorpertrack,numberoftrack,numberofside,sectsize,wprot;
	unsigned char	blockbuf[256];
	char linebuffer[80];
	int	blanks,indexptr,sector;
	SECTORCONFIG * sectorconfig;
	int sectorindex;

	floppycontext->hxc_printf(MSG_DEBUG,"JVC_libLoad_DiskFile %s",imgfile);
	
	gap3len=255;
	interleave=1;
	skew=1;
	file_offset=0;
	trackdata=0;
	sectorsize=512;
	trackformat=0;

	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 
	
	// code version
	fgets(linebuffer,sizeof(linebuffer),f);					
	if (sscanf(linebuffer,"%d.%d",&major,&minor) != 2) 
	{
		floppycontext->hxc_printf(MSG_DEBUG,"Bad code version !");
		return(LOADER_BADFILE);
	}

	// number of sector per track
	fgets(linebuffer,sizeof(linebuffer),f);					
	if (sscanf(linebuffer,"%d",&sectorpertrack) != 1) 
		return(LOADER_BADFILE);
	
	// number of tracks
	fgets(linebuffer,sizeof(linebuffer),f);					
	if (sscanf(linebuffer,"%d",&numberoftrack) != 1) 
		return(LOADER_BADFILE);
	
	// number of sides
	fgets(linebuffer,sizeof(linebuffer),f);					
	if (sscanf(linebuffer,"%d",&numberofside) != 1) 
		return(LOADER_BADFILE);

	// sector size
	fgets(linebuffer,sizeof(linebuffer),f);					
	if (sscanf(linebuffer,"%d",&sectsize) != 1) 
		return(LOADER_BADFILE);

	// write protect
	fgets(linebuffer,sizeof(linebuffer),f);					
	if (sscanf(linebuffer,"%d",&wprot) != 1) 
		return(LOADER_BADFILE);

	floppydisk->floppyNumberOfTrack=numberoftrack;
	floppydisk->floppyNumberOfSide=numberofside;
	floppydisk->floppySectorPerTrack=sectorpertrack;
	floppydisk->floppyBitRate=DEFAULT_DD_BITRATE;
	floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
	rpm=300;

	if(filesize!=0)
	{
			
		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
				
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
				
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{

				fread(blockbuf,256,1,f);


				blanks=0;
				indexptr = 0;
				sectorindex=0;

				sectorconfig=malloc(sizeof(SECTORCONFIG) * sectorpertrack);
				memset(sectorconfig,0,sizeof(SECTORCONFIG) * sectorpertrack);

				for (sector = 0; sector < sectorpertrack; sector++) 
				{
					int	sectortype = blockbuf[indexptr++];

					//THIS_SECTOR.encoding = SVD_SECTOR_TYPE(sectortype);

					if (SVD_SECTOR_TYPE(sectortype) == BLANK_SECTOR) {
						/* blanks are always at the end...so if we see a blank, we're done	*/
						/* with real data...however, we need to read in the blanks' data	*/
						blanks++;
					}
					else
					{

						/* there is a "standard" amount of data per header field in the	*/
						/* track header...each format needs to advance the indexpr right.*/

						switch(SVD_SECTOR_TYPE(sectortype)) 
						{
							case WD_FM:
							case WD_MFM:
								;
								sectorconfig[sectorindex].sector = blockbuf[indexptr++]; // id
								sectorconfig[sectorindex].head  = blockbuf[indexptr++]; // side
								//THIS_SECTOR.sector = blockbuf[indexptr++];
								sectorconfig[sectorindex].alternate_sector_size_id = blockbuf[indexptr++];
								sectorconfig[sectorindex].use_alternate_sector_size_id=0xFF;
								sectorconfig[sectorindex].header_crc=blockbuf[indexptr++];
								sectorconfig[sectorindex].header_crc +=blockbuf[indexptr++]*256;
								sectorconfig[sectorindex].use_alternate_header_crc=0xFF;
								sectorconfig[sectorindex].alternate_datamark=blockbuf[indexptr++];
								sectorconfig[sectorindex].use_alternate_datamark=0xFF;
								sectorconfig[sectorindex].data_crc = blockbuf[indexptr++];
								sectorconfig[sectorindex].data_crc += (int)(blockbuf[indexptr++]) * 256;
								sectorconfig[sectorindex].use_alternate_data_crc=0xFF;
								sectorconfig[sectorindex].sectorsize=256;
							break;

							case H17_HSFM:
								indexptr += 19;
								if (sector == sectorpertrack - 1) {
									indexptr += 5;		/* last sector has 5 extra		*/
								}
								//THIS_SECTOR.sizecode = reflect(blockbuf[indexptr++]); /* sets the volume */
								//THIS_SECTOR.id = reflect(blockbuf[indexptr++]);
								//THIS_SECTOR.sector = reflect(blockbuf[indexptr++]);
								//THIS_SECTOR.headCRC = reflect(blockbuf[indexptr++]);
								//THIS_SECTOR.dataCRC = reflect(blockbuf[indexptr++]);
								//if (THIS_SECTOR.id != 0) {
								//	floppy->volume = THIS_SECTOR.sizecode;
								//}
								//THIS_SECTOR.size = 256;
							break;

							case RNIB:
								/* no information is used with the RNIB format, but it is written/read	*/
								/* here just to allow the tools to work better (arguable).		*/

								//THIS_SECTOR.id = blockbuf[indexptr++];
								//THIS_SECTOR.side = blockbuf[indexptr++];
								//THIS_SECTOR.sector = blockbuf[indexptr++];
								//THIS_SECTOR.sizecode = blockbuf[indexptr++];

								//if (sector != 0) {
								//	indexptr += 5;
								//}
							break;

							case AGCR6x2:
							case AGCR5x3:
							{
								//int i;
/*
								for (i=0; i < 3; i++) {
								THIS_SECTOR.hdrprolog[i] = blockbuf[indexptr++];
								}
								THIS_SECTOR.volume = blockbuf[indexptr++];
								THIS_SECTOR.id = blockbuf[indexptr++];
								THIS_SECTOR.sector = blockbuf[indexptr++];
								THIS_SECTOR.hdrchecksum = blockbuf[indexptr++];
*/
								/* ONLY 2 BYTES OF EPILOG - THIRD IS SVD STATIC	*/

/*								for (i=0; i < 2; i++) {
								THIS_SECTOR.hdrepilog[i] = blockbuf[indexptr++];
								}

								for (i=0; i < 3; i++) {
								THIS_SECTOR.dataprolog[i] = blockbuf[indexptr++];
								}
								THIS_SECTOR.preload = blockbuf[indexptr++];
								THIS_SECTOR.datachecksum = blockbuf[indexptr++];
*/
								/* standard data for AGCR	*/

//								THIS_SECTOR.size = 256;
							}
							break;

							case RX02:
							case BLANK_SECTOR:
							case UNKNOWN_ENCODING:
							indexptr += 9;
							break;
						}

						sectorindex++;
					}
				}

				fseek (f , file_offset , SEEK_SET);
						
				Sector_attribute_flag=0;
				for(k=0;k<11;k++)
				{

					fread(&trackdata[k*sectorsize],sectorsize,1,f);
				}

				currentcylinder->sides[i]=tg_generatetrack(trackdata,sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500 | NO_SECTOR_UNDER_INDEX,-2500);
			}
		}

		free(trackdata);
			
		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		
		fclose(f);
		return LOADER_NOERROR;

	}
	
	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}
