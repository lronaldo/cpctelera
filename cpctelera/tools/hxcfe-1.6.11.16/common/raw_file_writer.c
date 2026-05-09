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
#include "raw_file_writer.h"
#include "plugins/common/crc.h"

#include "sector_extractor.h"



int write_RAW_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename)
{	
	int i,j,k,l,nbsector;
	FILE * rawfile;
	char * log_str;
	char   tmp_str[256];
	int sectorsize,track_type_id;
	sect_track track;

	floppycontext->hxc_printf(MSG_INFO_1,"Write RAW file %s...",filename);

	track_type_id=0;
	log_str=0;
	rawfile=fopen(filename,"wb");
	if(rawfile)
	{
		for(j=0;j<(int)floppy->floppyNumberOfTrack;j++)
		{
			for(i=0;i<(int)floppy->floppyNumberOfSide;i++)
			{
				sprintf(tmp_str,"track:%.2d:%d file offset:0x%.6x, sectors: ",j,i,ftell(rawfile));

				log_str=0;
				log_str=realloc(log_str,strlen(tmp_str)+1);
				memset(log_str,0,strlen(tmp_str)+1);
				strcat(log_str,tmp_str);

				memset(&track,0,sizeof(sect_track));
				track.side=i;
				track.track=j;


				k=0;
				do
				{
					switch(track_type_id)
					{
						case 0:
							nbsector=analysis_and_extract_sector_MFM(floppycontext,floppy->tracks[j]->sides[i],&track);
						break;
						case 1:
							nbsector=analysis_and_extract_sector_FM(floppycontext,floppy->tracks[j]->sides[i],&track);
						break;
						case 2:
							nbsector=analysis_and_extract_sector_AMIGAMFM(floppycontext,floppy->tracks[j]->sides[i],&track);
						break;
						case 3:
							nbsector=analysis_and_extract_sector_EMUIIFM(floppycontext,floppy->tracks[j]->sides[i],&track);
						break;
					}
					
					if(!nbsector)
						track_type_id=(track_type_id+1)%4;

					k++;
						
				}while(!nbsector && k<4);

				if(track.number_of_sector)
				{
					sectorsize=track.sectorlist[0]->sectorsize;
					for(l=0;l<256;l++)
					{

						k=0;
						do
						{
							if(track.sectorlist[k]->sector_id==l)
							{
								if(track.sectorlist[k]->sectorsize!=sectorsize)
								{
									sectorsize=-1;	
								}
								if(track.sectorlist[k]->buffer)
									fwrite(track.sectorlist[k]->buffer,track.sectorlist[k]->sectorsize,1,rawfile);

								sprintf(tmp_str,"%d ",track.sectorlist[k]->sector_id);
								log_str=realloc(log_str,strlen(log_str)+strlen(tmp_str)+1);
								strcat(log_str,tmp_str);
								break;
							}

							k++;
						}while(k<track.number_of_sector);
					
					}
				


					k=0;
					do
					{
						free(track.sectorlist[k]->buffer);
						free(track.sectorlist[k]);
						k++;
					}while(k<track.number_of_sector);


					if(sectorsize!=-1)
					{
						sprintf(tmp_str,",%dB/s",sectorsize);
					}
					else
					{
						sprintf(tmp_str,"");
					}
			
					log_str=realloc(log_str,strlen(log_str)+strlen(tmp_str)+1);
					strcat(log_str,tmp_str);

				}

				floppycontext->hxc_printf(MSG_INFO_1,log_str);
				free(log_str);
			
			}
		}	

		fclose(rawfile);
	}
	
	return 0;
}
