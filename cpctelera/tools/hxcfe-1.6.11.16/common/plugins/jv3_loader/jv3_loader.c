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
// File : JV3_DiskFile.c
// Contains: JV3 TRS80 floppy image loader and plugins interfaces
//
// Written by:	Gustavo E A P A Batista using JV1 loader as template
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

#include "jv3_loader.h"

#include "../common/os_api.h"


unsigned char* compute_interleave_tab(unsigned char interleave,unsigned short numberofsector);


unsigned int gbn(int num, int mask) {
	num = num & mask;
	while (mask % 2 == 0) {
		num >>= 1;
		mask >>= 1;
	}
	return num;
}

int JV3_compare (const void * a, const void * b) {
	return ( ((JV3SectorsOffsets*)a)->key - ((JV3SectorsOffsets*)b)->key );
}

JV3SectorsOffsets *JV3_bsearch(unsigned int key, JV3SectorsOffsets *base, size_t num) {
	JV3SectorsOffsets aux;

	aux.key = key;
	return (JV3SectorsOffsets*) bsearch (&aux, base, num, sizeof(JV3SectorsOffsets), JV3_compare);
}

unsigned int JV3_disk_geometry(JV3SectorHeader JV3SH[], unsigned char *NumberofSides, unsigned short *SectorsperTrack, unsigned short *NumberofTracks, unsigned short *SectorSize, unsigned char *StartIdSector, unsigned short *NumberofEntries) {
	int i, total_data = 0;

	*StartIdSector = 255;
	*NumberofEntries = 0;
	*NumberofSides = 0;
	*SectorsperTrack = 0;
	*NumberofTracks = 0;
	*SectorSize = 0;
	for (i=0; i<JV3_HEADER_MAX; i++)
		if (JV3SH[i].track != JV3_FREE) {
			(*NumberofEntries)++;
			if (JV3SH[i].track > *NumberofTracks)
				*NumberofTracks = JV3SH[i].track;
			if (JV3SH[i].sector > *SectorsperTrack)
				*SectorsperTrack = JV3SH[i].sector;
			if (JV3SH[i].sector < *StartIdSector)
				*StartIdSector = JV3SH[i].sector;
			if (gbn(JV3SH[i].flags, JV3_SIDE) > *NumberofSides)
				*NumberofSides = gbn(JV3SH[i].flags, JV3_SIDE);
			switch (gbn(JV3SH[i].flags, JV3_SIZE)) {
				case JV3_SIZE_USED_256:
					if (*SectorSize == 256 || *SectorSize == 0) {
						*SectorSize = 256;
						total_data += 256;
					} else
						return 0;
					break;
				case JV3_SIZE_USED_128:
					if (*SectorSize == 128 || *SectorSize == 0) {
						*SectorSize = 128;
						total_data += 128;
					} else
						return 0;
					break;
				case JV3_SIZE_USED_1024:
					if (*SectorSize == 1024 || *SectorSize == 0) {
						*SectorSize = 1024;
						total_data += 1024;
					} else
						return 0;
					break;
				case JV3_SIZE_USED_512:
					if (*SectorSize == 512 || *SectorSize == 0) {
						*SectorSize = 512;
						total_data += 512;
					} else
						return 0;
					break;
			}
		}
	if (*StartIdSector == 0)
		(*SectorsperTrack)++;         /* Sectors numered 0..Sectors-1 */
	(*NumberofTracks)++;                  /* Tracks numbered 0..Tracks-1 */
	(*NumberofSides)++;                   /* Sides numbered 0 or 1 */
	return total_data;
}

JV3SectorsOffsets *JV3_offset(JV3SectorHeader JV3SH[], unsigned int NumberofSides, unsigned int SectorsperTrack, unsigned int NumberofTracks, unsigned int NumberofEntries, FILE *jv3_file) {
	JV3SectorsOffsets *SO;
	unsigned int i, offset;
	int pos;

	pos = 0;
	offset = ftell(jv3_file);
	SO = (JV3SectorsOffsets *) malloc(sizeof(JV3SectorsOffsets)*NumberofEntries);
	for (i=0; i<JV3_HEADER_MAX; i++) {
		if (JV3SH[i].track != JV3_FREE) 
		{
			SO[pos].key = JV3SH[i].track << 16 | JV3SH[i].sector << 8 | gbn(JV3SH[i].flags, JV3_SIDE);
			SO[pos].offset = offset;
		    	
			switch (gbn(JV3SH[i].flags, JV3_SIZE)) 
			{
				case JV3_SIZE_USED_256:
					SO[pos].size = 256;
					offset += 256;
					break;
				case JV3_SIZE_USED_128:
					SO[pos].size = 128;
					offset += 128;
					break;
				case JV3_SIZE_USED_1024:
					SO[pos].size = 1024;
					offset += 1024;
					break;
				case JV3_SIZE_USED_512:
					SO[pos].size = 512;
					offset += 512;
					break;
			}
			
			if (gbn(JV3SH[i].flags, JV3_DENSITY)) 
			{         		/* Double density */
				
				SO[pos].density=0xFF;

				switch (gbn(JV3SH[i].flags, JV3_DAM))
				{
					case JV3_DAM_FB_DD:
						SO[pos].DAM = 0xFB;
						break;
					case JV3_DAM_F8_DD:
						SO[pos].DAM = 0xF8;
						break;
				}
			} 
			else 
			{
				SO[pos].density=0x00;

				switch (gbn(JV3SH[i].flags, JV3_DAM)) /* Single density */
				{    	 	
					case JV3_DAM_FB_SD:
						SO[pos].DAM = 0xFB;
						break;
					case JV3_DAM_FA_SD:
						SO[pos].DAM = 0xFA;
						break;
					case JV3_DAM_F8_SD:
						SO[pos].DAM = 0xF8;
						break;
					case JV3_DAM_F9_SD:
						SO[pos].DAM = 0xF9;
						break;
				}
  			}
        } 
		else 
		{
			//SO[pos].density=0x00;

			switch (gbn(JV3SH[i].flags, JV3_SIZE)) {
				case JV3_SIZE_FREE_256:
					offset += 256;
					break;
				case JV3_SIZE_FREE_128:
					offset += 128;
					break;
				case JV3_SIZE_FREE_1024:
					offset += 1024;
					break;
				case JV3_SIZE_FREE_512:
					offset += 512;
					break;
			}
		}
		pos++;
	}
	return SO;
}

int JV3_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen, offset1, offset2;
	unsigned short SectorPerTrack, NumberOfTrack, SectorSize, NumberOfEntries;
	unsigned int   total_data;
	unsigned char  StartIdSector,NumberOfSide;
	char * filepath;
	FILE *f;
	JV3SectorHeader sh[JV3_HEADER_MAX];

	floppycontext->hxc_printf(MSG_DEBUG,"JV3_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".dsk" )!=NULL || strstr( filepath,".jv3" )!=NULL)
				{
					
					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
						return LOADER_ACCESSERROR;
					}					
					fread(sh, sizeof(JV3SectorHeader), JV3_HEADER_MAX, f);
       				if ((total_data = JV3_disk_geometry(sh, &NumberOfSide, &SectorPerTrack, &NumberOfTrack, &SectorSize, &StartIdSector, &NumberOfEntries)) != 0)
					{
						offset1 = ftell(f);
						fseek (f , 0 , SEEK_END);
						offset2 = ftell(f); 
						fclose(f);
						free(filepath);
						if (total_data == (unsigned int)(offset2 - offset1 -1)) {
							floppycontext->hxc_printf(MSG_DEBUG,"JV3 file !");
							return LOADER_ISVALID;
						} else {
							floppycontext->hxc_printf(MSG_DEBUG,"non JV3 file !");
							return LOADER_BADFILE;
						}
					}
					else
					{
						fclose(f);
						free(filepath);
						floppycontext->hxc_printf(MSG_DEBUG,"non JV3 file !");
						return LOADER_BADFILE;
					}
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non JV3 file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}

int JV3_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{

	FILE * f;
	unsigned int filesize;
	unsigned int i,j,k,bitrate;
	unsigned short SectorSize, NumberofEntries;
	unsigned char  gap3len,interleave,StartIdSector;
	unsigned short rpm;
	unsigned char  trackformat;
	unsigned short sector_found;

	SECTORCONFIG*	sectorconfig;
	CYLINDER*		currentcylinder;

	JV3SectorHeader sh[JV3_HEADER_MAX];
	JV3SectorsOffsets *pOffset, *SectorsOffsets;
	unsigned char write_protected;
	unsigned int inc;

	floppycontext->hxc_printf(MSG_DEBUG,"JV3_libLoad_DiskFile %s",imgfile);

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
		fread(sh, sizeof(JV3SectorHeader), JV3_HEADER_MAX, f);

		JV3_disk_geometry(sh, &floppydisk->floppyNumberOfSide, &floppydisk->floppySectorPerTrack, &floppydisk->floppyNumberOfTrack, &SectorSize, &StartIdSector, &NumberofEntries);
		
		fread(&write_protected, sizeof(write_protected), 1, f);                                                                         // just to jump this infomation


		SectorsOffsets = JV3_offset(sh, floppydisk->floppyNumberOfSide, floppydisk->floppySectorPerTrack, floppydisk->floppyNumberOfTrack, NumberofEntries, f);

		qsort(SectorsOffsets, NumberofEntries, sizeof(JV3SectorsOffsets), JV3_compare);

		bitrate=250000;
		rpm=300;
		interleave=3;
		gap3len=255;
		trackformat=IBMFORMAT_SD;

		floppydisk->floppyBitRate=bitrate;
		floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

		floppycontext->hxc_printf(MSG_DEBUG,"rpm %d bitrate:%d track:%d side:%d sector:%d",rpm,bitrate,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack);

		sectorconfig=(SECTORCONFIG*)malloc(sizeof(SECTORCONFIG)*floppydisk->floppySectorPerTrack);
		memset(sectorconfig,0,sizeof(SECTORCONFIG)*floppydisk->floppySectorPerTrack);

		
		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{

			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];

			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{
				inc = 0;                                    // used to build track data
				memset(sectorconfig,0,sizeof(SECTORCONFIG)*floppydisk->floppySectorPerTrack);
				sector_found=0;
				for(k=0;k<floppydisk->floppySectorPerTrack;k++)
				{
					pOffset = JV3_bsearch(j<<16|(k+StartIdSector)<<8|i, SectorsOffsets, NumberofEntries);
				
		    		if (pOffset == NULL) 
					{
						inc += SectorSize;
					} 
					else 
					{
						
						sectorconfig[sector_found].sectorsize=pOffset->size;
						sectorconfig[sector_found].input_data=malloc(sectorconfig[k].sectorsize);
						memset(sectorconfig[sector_found].input_data,0,sectorconfig[k].sectorsize);

						fseek(f, pOffset->offset, SEEK_SET);
						fread(sectorconfig[sector_found].input_data,pOffset->size,1,f);
						
						inc += pOffset->size;
						
						if (pOffset->DAM != 0xFB) 
						{
							sectorconfig[sector_found].use_alternate_datamark=1;
							sectorconfig[sector_found].alternate_datamark=pOffset->DAM;
						}
						
						if(pOffset->density)
						{
							sectorconfig[sector_found].trackencoding=IBMFORMAT_DD;
							if(!k) trackformat=IBMFORMAT_DD;
						}
						else
						{
							sectorconfig[sector_found].trackencoding=IBMFORMAT_SD;
							if(!k) trackformat=IBMFORMAT_SD;
						}

						sectorconfig[sector_found].cylinder=j;
						sectorconfig[sector_found].head=i;
						sectorconfig[sector_found].sector=k+StartIdSector;
						sectorconfig[sector_found].bitrate=floppydisk->floppyBitRate;
						sectorconfig[sector_found].gap3=gap3len;

						sector_found++;

					}


				}
				currentcylinder->sides[i]=tg_generatetrackEx(sector_found,sectorconfig,interleave,0,floppydisk->floppyBitRate,rpm,trackformat,2500|NO_SECTOR_UNDER_INDEX,-2500);

				for(k=0;k<floppydisk->floppySectorPerTrack;k++)
				{
					free(sectorconfig[k].input_data);
				}

			}
		}

		free(sectorconfig);
		free(SectorsOffsets);
		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");

		fclose(f);
		return LOADER_NOERROR;
	}

	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}


