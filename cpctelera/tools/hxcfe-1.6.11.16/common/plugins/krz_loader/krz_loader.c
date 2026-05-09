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
// File : krz_loader.c
// Contains: Kurzweil KRZ floppy image loader and plugins interfaces
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

#include "krz_loader.h"

#include "os_api.h"

#include "../fat12floppy_loader/fat12.h"

extern unsigned char msdos_bootsector;

int KRZ_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	unsigned int filesize;
	char * filepath;
	FILE *f;
	floppycontext->hxc_printf(MSG_DEBUG,"krz_libIsValidDiskFile %s",imgfile);
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

				if(strstr( filepath,".krz" )!=NULL)
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

					floppycontext->hxc_printf(MSG_DEBUG,"Kurzweil KRZ file !");
					free(filepath);
					return LOADER_ISVALID;
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non Kurzweil KRZ file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}



int KRZ_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	unsigned int i,j;
	unsigned int file_offset;
	unsigned char * flatimg;
	unsigned char gap3len,interleave,skew;
	unsigned char trackformat;
	unsigned short rpm;
	unsigned short sectorsize;
	int numberofcluster;
	unsigned long   fatposition;
	unsigned long   rootposition;
	unsigned long   dataposition;
	int pcbootsector;
	int dksize;
	CYLINDER* currentcylinder;

	FATCONFIG fatconfig;

	floppycontext->hxc_printf(MSG_DEBUG,"krz_libLoad_DiskFile %s",imgfile);

	floppydisk->floppyNumberOfTrack=80;
	floppydisk->floppyNumberOfSide=2;
	floppydisk->floppySectorPerTrack=18;
	floppydisk->floppyBitRate=500000;
	rpm=300;
	pcbootsector=1;
	skew=0;
	gap3len=84;
	interleave=1;
	sectorsize=512;
	trackformat=IBMFORMAT_DD;

	dksize=floppydisk->floppyNumberOfTrack*
			(floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide*512);


	floppycontext->hxc_printf(MSG_INFO_1,"floppy size:%dkB, %d tracks, %d side(s), %d sectors/track, rpm:%d",dksize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,rpm);

	flatimg=(char*)malloc(dksize);
	if(flatimg!=NULL)
	{
		memset(flatimg,0,dksize);
		if(pcbootsector)
			memcpy(flatimg,&msdos_bootsector,512);
	

		fatconfig.sectorsize=512;
		fatconfig.clustersize=2;
		fatconfig.reservedsector=1;
		fatconfig.numberoffat=2;
		fatconfig.numberofrootentries=224;// 7secteurs = 32 *512
		fatconfig.nbofsector=(floppydisk->floppyNumberOfTrack*
								(floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide));
		fatconfig.nbofsectorperfat=((fatconfig.nbofsector-(fatconfig.reservedsector+(fatconfig.numberofrootentries/32)))/((fatconfig.sectorsize*8)/12))+1;
		//sprintf(&flatimg[CHSTOADR(0,0,0)+3],"HXC.EMU");

		*( (unsigned short*) &flatimg[0x0B])=fatconfig.sectorsize; //Nombre d'octets par secteur
		*( (unsigned char*)  &flatimg[0x0D])=fatconfig.clustersize; //Nombre de secteurs par cluster (1, 2, 4, 8, 16, 32, 64 ou 128).
		*( (unsigned short*) &flatimg[0x0E])=fatconfig.reservedsector; //Nombre de secteur réservé en comptant le secteur de boot (32 par défaut pour FAT32, 1 par défaut pour FAT12/16).
		*( (unsigned char*)  &flatimg[0x10])=fatconfig.numberoffat; //Nombre de FATs sur le disque (2 par défaut).
		*( (unsigned short*) &flatimg[0x11])=fatconfig.numberofrootentries; //Taille du répertoire racine (0 par défaut pour FAT32).
		*( (unsigned short*) &flatimg[0x13])=fatconfig.nbofsector; //Nombre total de secteur 16-bit (0 par défaut pour FAT32).

		if(	floppydisk->floppyBitRate==250000)
		{
			*( (unsigned char*)  &flatimg[0x15])=0xF9; //Type de disque (0xF8 pour les disques durs, 0xF0 pour les disquettes). 0xF9 Double sided, 80 tracks per side, 9 sectors per track
		}
		else
		{
			*( (unsigned char*)  &flatimg[0x15])=0xF0; //Type de disque (0xF8 pour les disques durs, 0xF0 pour les disquettes). 0xF9 Double sided, 80 tracks per side, 9 sectors per track
		}

		*( (unsigned short*) &flatimg[0x16])=fatconfig.nbofsectorperfat; //Taille d'une FAT en secteurs (0 par défaut pour FAT32).
		*( (unsigned short*) &flatimg[0x18])=floppydisk->floppySectorPerTrack; //Sectors per track
		*( (unsigned short*) &flatimg[0x1a])=floppydisk->floppyNumberOfSide; //Number of heads.
		*( (unsigned short*) &flatimg[0x1FE])=0xAA55;//End of sector marker (0x55 0xAA)

		fatposition=fatconfig.sectorsize*fatconfig.reservedsector;
		rootposition=((fatconfig.reservedsector)+(fatconfig.numberoffat*fatconfig.nbofsectorperfat))*fatconfig.sectorsize;
		dataposition=(((fatconfig.reservedsector)+(fatconfig.numberoffat*fatconfig.nbofsectorperfat))*fatconfig.sectorsize)+(32*fatconfig.numberofrootentries);

		numberofcluster=(fatconfig.nbofsector-(dataposition/fatconfig.sectorsize))/fatconfig.clustersize;

		if(ScanFileAndAddToFAT(floppycontext,imgfile,0,&flatimg[fatposition],&flatimg[rootposition],&flatimg[dataposition],0,&fatconfig,numberofcluster))
		{
			return LOADER_BADFILE;
		}
		memcpy(&flatimg[((fatconfig.reservedsector)+(fatconfig.nbofsectorperfat))*fatconfig.sectorsize],&flatimg[fatposition],fatconfig.nbofsectorperfat*fatconfig.sectorsize);


	}
	else
	{
		return LOADER_INTERNALERROR;
	}


	floppydisk->floppyiftype=IBMPC_HD_FLOPPYMODE;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
	{
		floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
		currentcylinder=floppydisk->tracks[j];

		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{
			file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
						(sectorsize*(floppydisk->floppySectorPerTrack)*i);

			currentcylinder->sides[i]=tg_generatetrack(&flatimg[file_offset],sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,1,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500|NO_SECTOR_UNDER_INDEX,-2500);
		}
	}

	free(flatimg);

	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
	return LOADER_NOERROR;
}
	

