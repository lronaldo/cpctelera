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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"

#include "mfm_file_writer.h"

int write_MFM_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename)
{	

	FILE * hxcmfmfile;
	
	MFMTRACKIMG mfmtrackdesc;
	MFMIMG mfmheader;
	unsigned char * mfmtrack;
	long * offsettrack;
	int mfmsize;
	unsigned int i,j;
	unsigned int trackpos;

	floppycontext->hxc_printf(MSG_INFO_1,"Write MFM file %s...",filename);

	hxcmfmfile=fopen(filename,"wb");
	if(hxcmfmfile)
	{

		sprintf((char*)&mfmheader.headername,"HXCMFM");
		mfmheader.number_of_track=floppy->floppyNumberOfTrack;
		mfmheader.number_of_side=floppy->floppyNumberOfSide;
		mfmheader.floppyBitRate=floppy->floppyBitRate/1000;

		if(floppy->floppyBitRate!=VARIABLEBITRATE)
		{
			mfmheader.floppyBitRate=floppy->floppyBitRate/1000;
		}
		else
		{
			mfmheader.floppyBitRate=floppy->tracks[0]->sides[0]->bitrate/1000;
		}

		mfmheader.floppyRPM=0;//floppy->floppyRPM;
		mfmheader.floppyiftype=(unsigned char)floppy->floppyiftype;
		mfmheader.mfmtracklistoffset=sizeof(mfmheader);
		fwrite(&mfmheader,sizeof(mfmheader),1,hxcmfmfile);
		
		floppycontext->hxc_printf(MSG_INFO_1,"%d Tracks, %d side(s)",mfmheader.number_of_track,mfmheader.number_of_side);

		offsettrack=(long*) malloc(((mfmheader.number_of_track*mfmheader.number_of_side)+1)*sizeof(long));

		i=0;
		trackpos=sizeof(mfmheader)+(sizeof(mfmtrackdesc)*(mfmheader.number_of_track*mfmheader.number_of_side));
		if(trackpos&0x1FF)
		{
			trackpos=(trackpos&(~0x1FF))+0x200;						
		}
		do
		{
			for(j=0;j<(mfmheader.number_of_side);j++)
			{
				memset(&mfmtrackdesc,0,sizeof(mfmtrackdesc));
 				mfmsize=floppy->tracks[i]->sides[j]->tracklen;
				if(mfmsize&7)
					mfmsize=(mfmsize/8)+1;
				else
					mfmsize=mfmsize/8;

				mfmtrackdesc.mfmtracksize=mfmsize;
				mfmtrackdesc.side_number=j;
				mfmtrackdesc.track_number=i;
				offsettrack[(i*mfmheader.number_of_side)+j]=(long)trackpos;
				mfmtrackdesc.mfmtrackoffset=trackpos;
				
				floppycontext->hxc_printf(MSG_DEBUG,"Write Track %d:%d [%x] %x bytes",i,j,mfmtrackdesc.mfmtrackoffset,mfmsize);
				trackpos=trackpos+mfmsize;
				if(trackpos&0x1FF)
				{
					trackpos=(trackpos&(~0x1FF))+0x200;					
				}

				fwrite(&mfmtrackdesc,sizeof(mfmtrackdesc),1,hxcmfmfile);
				
			}

			i++;
		}while(i<(mfmheader.number_of_track));

		mfmtrack=NULL;
		i=0;
		do
		{
			for(j=0;j<(mfmheader.number_of_side);j++)
			{
				memset(&mfmtrackdesc,0,sizeof(mfmtrackdesc));
				mfmsize=floppy->tracks[i]->sides[j]->tracklen;
				if(mfmsize&7)
					mfmsize=(mfmsize/8)+1;
				else
					mfmsize=mfmsize/8;

				mfmtrack=(unsigned char*) malloc(mfmsize);
				
				if(ftell(hxcmfmfile)<offsettrack[(i*mfmheader.number_of_side)+j])
				{
					memset(mfmtrack,0,offsettrack[(i*mfmheader.number_of_side)+j]-ftell(hxcmfmfile));
					fwrite(mfmtrack,offsettrack[(i*mfmheader.number_of_side)+j]-ftell(hxcmfmfile),1,hxcmfmfile);
				}

				memcpy(mfmtrack,floppy->tracks[i]->sides[j]->databuffer,mfmsize);
					
				fwrite(mfmtrack,mfmsize,1,hxcmfmfile);
						
				free(mfmtrack);

			}

			i++;
		}while(i<(mfmheader.number_of_track));

		free(offsettrack);

		fclose(hxcmfmfile);

		return 0;
	}
	else
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot create %s!",filename);

		return -1;
	}

	
}
