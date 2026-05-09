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
// File : CPCFSDK_DiskFile.c
// Contains: CPCFSDK floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"


#include "cpcfs_loader.h"

#include "../os_api.h"


#include "./libs/adflib/Lib/adflib.h"


HXCFLOPPYEMULATOR* global_floppycontext;
extern int ScanFile(HXCFLOPPYEMULATOR* floppycontext,struct Volume * adfvolume,char * folder,char * file);

int CPCFSDK_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	
	int pathlen;
	char * filepath;
    struct stat staterep;

	floppycontext->hxc_printf(MSG_DEBUG,"CPCFSDK_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{
		pathlen=strlen(imgfile);
		if(pathlen!=0)
		{		
			stat(imgfile,&staterep);

			if(staterep.st_mode&S_IFDIR)
			{
				filepath=malloc(pathlen+1);
				if(filepath!=0)
				{
					sprintf(filepath,"%s",imgfile);
					strlower(filepath);

					if(strstr( filepath,".cpcfs" )!=NULL)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"CPCFSDK file !");
						free(filepath);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non CPCFSDK file ! (.cpcfs missing)");
						free(filepath);
						return LOADER_BADFILE;
					}
				}
			}
			else
			{
				floppycontext->hxc_printf(MSG_DEBUG,"non CPCFSDK file ! (it's not a directory)");
				return LOADER_BADFILE;
			}
		}
		floppycontext->hxc_printf(MSG_DEBUG,"0 byte string ?");
	}
	return LOADER_BADPARAMETER;


}


int ScanCpcFile(HXCFLOPPYEMULATOR* floppycontext,struct Volume * adfvolume,char * folder,char * file)
{

	return 0;
}

int CPCFSDK_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
		return LOADER_BADFILE;
}


