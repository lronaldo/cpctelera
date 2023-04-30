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
// File : snes_smc_DiskFile.c
// Contains: SMC rom floppy image loader and plugins interfaces
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

#include "snes_smc_loader.h"
#include "../fat12floppy_loader/fat12floppy_loader.h"


#include "os_api.h"

#include "../fat12floppy_loader/fat12.h"

extern unsigned char msdos_bootsector;

int snes_smc_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	FILE * f;
	unsigned char * fileheader;
	char * filepath;
	int fileok;
	
	fileok=1;
	floppycontext->hxc_printf(MSG_DEBUG,"snes_smc_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{
		
		f=fopen(imgfile,"r+b");
		if(f)
		{
			fileheader=(unsigned char*)malloc(512);
			if(fileheader)
			{
				fread(fileheader,1,512,f);
				if( fileheader[8]==0xaa && fileheader[9]==0xbb )
				{
					
					switch(fileheader[10])
					{
					case 1:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : Super Magic Card saver data");
						break;
					case 2:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : Magic Griffin program (PC-Engine)");
						break;
					case 3:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : Magic Griffin SRAM data");
						break;
					case 4:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : SNES program");
						break;
					case 5:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : SWC & SMC password, SRAM data");
						break;
					case 6:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : Mega Drive program");
						break;
					case 7:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : SMD SRAM data");
						break;
					case 8:
						floppycontext->hxc_printf(MSG_INFO_1,"File type : SWC & SMC saver data");
						break;
					default:
						fileok=0;					
						break;
					}
					
					if(fileok)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"SMC/SMD file !");
						free(fileheader);
						fclose(f);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non SMC/SMD file !");
						free(fileheader);
						fclose(f);
						return LOADER_BADFILE;
					}
					
				}
				else
				{
						if(!strncmp(&fileheader[8],"SUPERUFO",8))
						{
							floppycontext->hxc_printf(MSG_INFO_1,"File type : SUPERUFO SMC");
						}
						else
						{
							filepath=malloc(strlen(imgfile)+1);
							if(filepath!=0)
							{
								sprintf(filepath,"%s",imgfile);
								strlwr(filepath);
								if(!strstr( filepath,".smc" ))
								{
									fileok=0;
								}
								else
								{
									floppycontext->hxc_printf(MSG_INFO_1,"File type : Super Pro Fighter SMC?");
								}

								free(filepath);
							}

							if(!fileok) floppycontext->hxc_printf(MSG_ERROR,"unknow file type !");
				
						}

					if(fileok)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"SMC/SMD file !");
						free(fileheader);
						fclose(f);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non SMC/SMD file !");
						free(fileheader);
						fclose(f);
						return LOADER_BADFILE;
					}
				}
				
			}
			fclose(f);
			return LOADER_INTERNALERROR;
		}
		return LOADER_ACCESSERROR;
		
	}
	
	return LOADER_BADPARAMETER;
}



int snes_smc_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	floppycontext->hxc_printf(MSG_DEBUG,"snes_smc_libLoad_DiskFile %s",imgfile);
	
	return FAT12FLOPPY_libLoad_DiskFile(floppycontext,floppydisk,imgfile,".fat4572");
}
	

