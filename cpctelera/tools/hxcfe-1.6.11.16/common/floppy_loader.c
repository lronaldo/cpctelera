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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "loaders_list.h"

int initHxCFloppyEmulator(HXCFLOPPYEMULATOR* floppycontext)
{
	floppycontext->hxc_printf(MSG_INFO_0,"Starting HxCFloppyEmulator...");
	return 1;
}

int floppy_load(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * newfloppy,char* imgname)
{
	int i;
	int ret;
	floppycontext->hxc_printf(MSG_INFO_0,"Loading %s",imgname);

	memset(newfloppy,0,sizeof(FLOPPY));

	i=0;
	do
	{
		ret=staticplugins[i].libIsValidDiskFile(floppycontext,imgname);
		if(ret==LOADER_ISVALID)
		{
			floppycontext->hxc_printf(MSG_INFO_0,"file loader found!");
			ret=staticplugins[i].libLoad_DiskFile(floppycontext,newfloppy,imgname,0);
			return ret;
		}
		else
		{
			floppycontext->hxc_printf(MSG_DEBUG,"libIsValidDiskFile n%d return %d",i,ret);
		}

		i++;
	}while(staticplugins[i].libIsValidDiskFile!=(ISVALIDDISKFILE)-1);
	
	floppycontext->hxc_printf(MSG_ERROR,"no loader support the file %s !",imgname);
	return LOADER_UNSUPPORTEDFILE;
}


int floppy_unload(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk)
{
	unsigned int i,j;
	for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
	{
		for(i=0;i<floppydisk->floppyNumberOfSide;i++)
		{
			if(floppydisk->tracks[j]->sides[i]->databuffer)
			{
				free(floppydisk->tracks[j]->sides[i]->databuffer);
				floppydisk->tracks[j]->sides[i]->databuffer=0;
			}
			if(floppydisk->tracks[j]->sides[i]->flakybitsbuffer)
			{
				free(floppydisk->tracks[j]->sides[i]->flakybitsbuffer);
				floppydisk->tracks[j]->sides[i]->flakybitsbuffer=0;
			}
			if(floppydisk->tracks[j]->sides[i]->indexbuffer)
			{
				free(floppydisk->tracks[j]->sides[i]->indexbuffer);
				floppydisk->tracks[j]->sides[i]->indexbuffer=0;
			}
			if(floppydisk->tracks[j]->sides[i]->timingbuffer)
			{
				free(floppydisk->tracks[j]->sides[i]->timingbuffer);
				floppydisk->tracks[j]->sides[i]->timingbuffer=0;
			}

			if(floppydisk->tracks[j]->sides[i]->track_encoding_buffer)
			{
				free(floppydisk->tracks[j]->sides[i]->track_encoding_buffer);
				floppydisk->tracks[j]->sides[i]->track_encoding_buffer=0;
			}			
			
			free(floppydisk->tracks[j]->sides[i]);
		}

		free(floppydisk->tracks[j]->sides);
		free(floppydisk->tracks[j]);

	}
	free(floppydisk->tracks);
	return 0;
}

