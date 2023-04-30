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

#include "vtrucco_file_writer.h"

extern unsigned char bit_inverter[];

int write_vtrucco_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename,int forceifmode)
{	

	vtrucco_pictrack * track;

	FILE * hxcpicfile;

	vtrucco_picfileformatheader * FILEHEADER;
	unsigned char * mfmtracks0,*mfmtracks1,*mfmtrackfinal;
	unsigned char * offsettrack;
	int mfmsize,mfmsize2;
	unsigned int i,j,k;
	unsigned int trackpos;
	unsigned int tracklistlen;
	unsigned int tracksize;

	floppycontext->hxc_printf(MSG_INFO_1,"Write vtrucco-HFE file %s for the standalone emulator.",filename);

	hxcpicfile=fopen(filename,"wb");

	if(hxcpicfile)
	{
		FILEHEADER=(vtrucco_picfileformatheader *) malloc(512);
		memset(FILEHEADER,0xFF,512);
		sprintf((char*)&FILEHEADER->HEADERSIGNATURE,"VTrucco");

		FILEHEADER->number_of_track=(unsigned char)floppy->floppyNumberOfTrack;
		FILEHEADER->number_of_side=(unsigned char)floppy->floppyNumberOfSide;
		FILEHEADER->bitRate=floppy->floppyBitRate/1000;
		FILEHEADER->floppyRPM=0;//floppy->floppyRPM;
		if(forceifmode==-1)
		{
			FILEHEADER->floppyinterfacemode=(unsigned char)floppy->floppyiftype;
		}
		else
		{
			FILEHEADER->floppyinterfacemode=forceifmode;
		}
		FILEHEADER->track_encoding=0;
		FILEHEADER->formatrevision=0;
		FILEHEADER->track_list_offset=1;
		FILEHEADER->write_protected=1;

		sprintf((char*)&FILEHEADER->CREDITS,"based on original project from Jean François Del Nero HxC Floppy Emulator");
        
        fwrite(FILEHEADER,512,1,hxcpicfile);

		tracklistlen=(((((FILEHEADER->number_of_track+1)*sizeof(vtrucco_pictrack))/512)+1));
		offsettrack=(unsigned char*) malloc(tracklistlen*512);
		memset(offsettrack,0xFF,tracklistlen*512);
		
		i=0;
		trackpos=FILEHEADER->track_list_offset+tracklistlen;

		do
		{
				mfmsize=0;
				mfmsize2=0;


				mfmsize=floppy->tracks[i]->sides[0]->tracklen;
				if(mfmsize&7)
					mfmsize=(mfmsize/8)+1;
				else
					mfmsize=mfmsize/8;


				if(floppy->tracks[i]->number_of_side==2)
				{
					mfmsize2=floppy->tracks[i]->sides[1]->tracklen;
					if(mfmsize2&7)
						mfmsize2=(mfmsize2/8)+1;
					else
						mfmsize2=mfmsize2/8;
				}

				if(mfmsize2>mfmsize) mfmsize=mfmsize2;
			
				track=(vtrucco_pictrack *)(offsettrack+(i*sizeof(vtrucco_pictrack)));
				if(mfmsize*2>0xFFFF)
				{
					floppycontext->hxc_printf(MSG_ERROR,"Argg!! track %d too long (%x) and shorten to 0xFFFF !",i,mfmsize*2);
					mfmsize=0x7FFF;
				}
				track->track_len=mfmsize*2;
				track->offset=trackpos;


				if((mfmsize*2)%512)
					trackpos=trackpos+(((mfmsize*2)/512)+1);
				else
					trackpos=trackpos+((mfmsize*2)/512);

				//trackpos=trackpos+(((mfmsize*2)/512)+1);
			i++;
		}while(i<(FILEHEADER->number_of_track));

        fwrite(offsettrack,512*tracklistlen,1,hxcpicfile);

		i=0;
		do
		{

				mfmsize=floppy->tracks[i]->sides[0]->tracklen;
				if(mfmsize&7)
					mfmsize=(mfmsize/8)+1;
				else
					mfmsize=mfmsize/8;

				mfmsize2=0;
				if(floppy->tracks[i]->number_of_side==2)
				{
					mfmsize2=floppy->tracks[i]->sides[1]->tracklen;
					if(mfmsize2&7)
						mfmsize2=(mfmsize2/8)+1;
					else
						mfmsize2=mfmsize2/8;
				}
				if(mfmsize>0x7FFF)
				{
					mfmsize=0x7FFF;
				}
				if(mfmsize2>0x7FFF)
				{
					mfmsize2=0x7FFF;
				}
				track=(vtrucco_pictrack *)(offsettrack+(i*sizeof(vtrucco_pictrack)));

				if(track->track_len%512)
					tracksize=((track->track_len&(~0x1FF))+0x200)/2;
				else
					tracksize=track->track_len/2;

				mfmtracks0=(unsigned char*) malloc(tracksize);
				mfmtracks1=(unsigned char*) malloc(tracksize);
				mfmtrackfinal=(unsigned char*) malloc(tracksize*2);

				memset(mfmtracks0,0x00,tracksize);
				memset(mfmtracks1,0x00,tracksize);
				memset(mfmtrackfinal,0x55,tracksize*2);

				memcpy(mfmtracks0,floppy->tracks[i]->sides[0]->databuffer,mfmsize);
				
				if(floppy->tracks[i]->number_of_side==2)
					memcpy(mfmtracks1,floppy->tracks[i]->sides[1]->databuffer,mfmsize2);
				
				for(k=0;k<tracksize/256;k++)
				{

					for(j=0;j<256;j++)
					{
						// inversion des bits pour le EUSART du PIC.

						// head 0
						mfmtrackfinal[(k*512)+j]=     bit_inverter[mfmtracks0[(k*256)+j]];
						// head 1
						mfmtrackfinal[(k*512)+j+256]= bit_inverter[mfmtracks1[(k*256)+j]];

					}
				}
				
				
				fwrite(mfmtrackfinal,tracksize*2,1,hxcpicfile);
					
				free(mfmtracks0);
				free(mfmtracks1);
				free(mfmtrackfinal);

		
			i++;
		}while(i<(FILEHEADER->number_of_track));

		free(offsettrack);

        fclose(hxcpicfile);
		
		floppycontext->hxc_printf(MSG_INFO_1,"%d tracks written to the file",FILEHEADER->number_of_track);

		free(FILEHEADER);

		return 0;
	}
	else
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot create %s!",filename);

		return -1;
	}

}
