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
// File : FAT12FLOPPY_DiskFile.c
// Contains: FAT12FLOPPY floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "types.h"
#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"

#include "../common/crc.h"
#include "../common/track_generator.h"

#include "fat12floppy_loader.h"

#include "os_api.h"

#include "fat12.h"
#include "fat12formats.h"

#include "fatlib.h"



int FAT12FLOPPY_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	int i;
	struct stat staterep;
	
	floppycontext->hxc_printf(MSG_DEBUG,"FAT12FLOPPY_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{
		stat(imgfile,&staterep);
		
		if(staterep.st_mode&S_IFDIR)
		{
			pathlen=strlen(imgfile);
			if(pathlen!=0)
			{
				
				filepath=malloc(pathlen+1);
				if(filepath!=0)
				{
					sprintf(filepath,"%s",imgfile);
					strlower(filepath);
					
					i=0;
					while( (strlen(configlist[i].dirext) && ( !strstr( &filepath[strlen(filepath)-(strlen(configlist[i].dirext)+1)],configlist[i].dirext)) || (strlen(configlist[i].dirext) && !configlist[i].dir)))
					{
						i++;
					}
					
					if(strlen(configlist[i].dirext))				
					{
						floppycontext->hxc_printf(MSG_DEBUG,"FAT12FLOPPY file ! (Dir , %s)",configlist[i].dirext);
						free(filepath);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non FAT12FLOPPY file !");
						free(filepath);
						return LOADER_BADFILE;
					}
				}
			}		
		}
		else
		{
			pathlen=strlen(imgfile);
			if(pathlen!=0)
			{
				
				filepath=malloc(pathlen+1);
				if(filepath!=0)
				{
					sprintf(filepath,"%s",imgfile);
					strlower(filepath);
					
					i=0;
					while((strlen(configlist[i].dirext) && ( !strstr(configlist[i].dirext, &filepath[strlen(filepath)-(strlen(configlist[i].dirext)+1)])) || (strlen(configlist[i].dirext) && configlist[i].dir)))
					{
						i++;
					}
					
					if(strlen(configlist[i].dirext))				
					{
						floppycontext->hxc_printf(MSG_DEBUG,"FAT12FLOPPY file ! (File , %s)",configlist[i].dirext);
						free(filepath);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non FAT12FLOPPY file !");
						free(filepath);
						return LOADER_BADFILE;
					}
				}
			}
		}
	}
	
	return LOADER_BADPARAMETER;
}



int FAT12FLOPPY_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	unsigned int i,j;
	unsigned int file_offset;
	unsigned char * flatimg;
	unsigned char   gap3len,interleave,dirmode;
	unsigned short  sectorsize,rpm;
	int numberofcluster;
	unsigned long   fatposition;
	unsigned long   rootposition;
	unsigned long   dataposition;
	int dksize;
	unsigned char media_type;
	unsigned char tracktype;
	FATCONFIG fatconfig;
	CYLINDER* currentcylinder;
	struct stat staterep;

	//	FILE * f;

	fat_boot_sector * fatbs;

	floppycontext->hxc_printf(MSG_DEBUG,"FAT12FLOPPY_libLoad_DiskFile %s",imgfile);

	if(!parameters)
	{
		i=0;
		while(!strstr( &imgfile[strlen(imgfile)-(strlen(configlist[i].dirext)+1)],configlist[i].dirext) && strlen(configlist[i].dirext))
		{
			i++;
		}
	}
	else
	{
		i=0;
		while(!strstr( (char*)parameters,configlist[i].dirext) && strlen(configlist[i].dirext))
		{
			i++;
		}
	}
		
	dirmode=configlist[i].dir;
	floppydisk->floppyNumberOfTrack=configlist[i].number_of_track;
	floppydisk->floppyNumberOfSide=configlist[i].number_of_side;
	floppydisk->floppySectorPerTrack=configlist[i].number_of_sectorpertrack;
	floppydisk->floppyBitRate=configlist[i].bitrate;
	floppydisk->floppyiftype=configlist[i].interface_mode;
	tracktype=configlist[i].tracktype;
	interleave=configlist[i].interleave;
	gap3len=configlist[i].gap3;
	media_type=configlist[i].BPB_media;

	rpm=configlist[i].rpm;

	dksize=floppydisk->floppyNumberOfTrack*
			(floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide*configlist[i].sectorsize);


	floppycontext->hxc_printf(MSG_INFO_1,"floppy size:%dkB, %d tracks, %d side(s), %d sectors/track, rpm:%d, bitrate:%d, gap3: %d ",dksize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,rpm,floppydisk->floppyBitRate,gap3len);
	
	flatimg=(char*)malloc(dksize);
	if(flatimg!=NULL)
	{
		memset(flatimg,0xE6,dksize);
		if(configlist[i].bootsector)
			memcpy(flatimg,configlist[i].bootsector,512);
		else
			memset(flatimg,0x00,512);

		for(i=0;i<4;i++)
		{
			flatimg[i+0x27]=rand();
		}
		fatconfig.sectorsize=configlist[i].sectorsize;
		fatconfig.clustersize=configlist[i].clustersize;
		fatconfig.reservedsector=configlist[i].reserved_sector;
		fatconfig.numberoffat=2;

		fatconfig.numberofrootentries=configlist[i].root_dir_entries;
		fatconfig.nbofsector=(floppydisk->floppyNumberOfTrack*
								(floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide));
		fatconfig.nbofsectorperfat=((fatconfig.nbofsector-(fatconfig.reservedsector+(fatconfig.numberofrootentries/32)))/((fatconfig.sectorsize*8)/12))+1;
		//sprintf(&flatimg[CHSTOADR(0,0,0)+3],"HXC.EMU");

		fatbs=(fat_boot_sector*)flatimg;

		fatbs->BPB_BytsPerSec=fatconfig.sectorsize; //Nombre d'octets par secteur;
		fatbs->BPB_SecPerClus=fatconfig.clustersize; //Nombre de secteurs par cluster (1, 2, 4, 8, 16, 32, 64 ou 128).
		fatbs->BPB_RsvdSecCnt=fatconfig.reservedsector; //Nombre de secteur réservé en comptant le secteur de boot (32 par défaut pour FAT32, 1 par défaut pour FAT12/16).
		fatbs->BPB_NumFATs=fatconfig.numberoffat; //Nombre de FATs sur le disque (2 par défaut).
		fatbs->BPB_RootEntCnt=fatconfig.numberofrootentries; //Taille du répertoire racine (0 par défaut pour FAT32).
		fatbs->BPB_TotSec16=fatconfig.nbofsector; //Nombre total de secteur 16-bit (0 par défaut pour FAT32).
		fatbs->BPB_Media=media_type; //Type de disque

		fatbs->BPB_FATSz16=fatconfig.nbofsectorperfat; //Taille d'une FAT en secteurs (0 par défaut pour FAT32).
		fatbs->BPB_SecPerTrk=floppydisk->floppySectorPerTrack; //Sectors per track
		fatbs->BPB_NumHeads=floppydisk->floppyNumberOfSide; //Number of heads.
		
		*( (unsigned short*) &flatimg[0x1FE])=0xAA55;//End of sector marker (0x55 0xAA)		

		fatposition=fatconfig.sectorsize*fatconfig.reservedsector;
		memset(&flatimg[fatposition],0x00,fatconfig.numberoffat*fatconfig.nbofsectorperfat*512);

		rootposition=((fatconfig.reservedsector)+(fatconfig.numberoffat*fatconfig.nbofsectorperfat))*fatconfig.sectorsize;
		memset(&flatimg[rootposition],0x00,fatconfig.numberofrootentries*32);

		dataposition=(((fatconfig.reservedsector)+(fatconfig.numberoffat*fatconfig.nbofsectorperfat))*fatconfig.sectorsize)+(32*fatconfig.numberofrootentries);		
		
		numberofcluster=(fatconfig.nbofsector-(dataposition/fatconfig.sectorsize))/fatconfig.clustersize;

		stat(imgfile,&staterep);
		if(!(staterep.st_mode&S_IFDIR))
				dirmode=0x00;

		if(dirmode)
		{
			if(ScanFileAndAddToFAT(floppycontext,imgfile,"*.*",&flatimg[fatposition],&flatimg[rootposition],&flatimg[dataposition],0,&fatconfig,numberofcluster))
			{
				return LOADER_BADFILE;
			}
		}
		else
		{
			if(ScanFileAndAddToFAT(floppycontext,imgfile,0    ,&flatimg[fatposition],&flatimg[rootposition],&flatimg[dataposition],0,&fatconfig,numberofcluster))
			{
				return LOADER_BADFILE;
			}
		}

		flatimg[fatposition+0]=fatbs->BPB_Media;
		flatimg[fatposition+1]=0xFF;
		flatimg[fatposition+2]=0xFF;

		memcpy(&flatimg[((fatconfig.reservedsector)+(fatconfig.nbofsectorperfat))*fatconfig.sectorsize],&flatimg[fatposition],fatconfig.nbofsectorperfat*fatconfig.sectorsize);			


	/*	f=fopen("test.img","wb");
		if(f) 
		{
			fwrite(flatimg,dksize,1,f);
			fclose(f);
		}
*/


	}
	else
	{
		return LOADER_INTERNALERROR;
	}
	
	sectorsize=fatconfig.sectorsize;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);			
				
	for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
	{
				
		floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
		currentcylinder=floppydisk->tracks[j];

		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{
			file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
						(sectorsize*(floppydisk->floppySectorPerTrack)*i);
				
			currentcylinder->sides[i]=tg_generatetrack(&flatimg[file_offset],sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,0,floppydisk->floppyBitRate,currentcylinder->floppyRPM,tracktype,gap3len,2500,-2500);
		}
	}
			
	free(flatimg);
			
	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
	return LOADER_NOERROR;
}
	

