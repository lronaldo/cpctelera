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
#include "plugins/cpcdsk_loader/cpcdsk_format.h"

#include "sector_extractor.h"



unsigned char  size_to_code(unsigned long size)
{

	switch(size)
	{
		case 128:
			return 0;
		break;
		case 256:
			return 1;
		break;
		case 512:
			return 2;
		break;
		case 1024:
			return 3;
		break;
		case 2048:
			return 4;
		break;
		case 4096:
			return 5;
		break;
		case 8192:
			return 6;
		break;
		case 16384:
			return 7;
		break;
		default:
			return 0;
		break;
	}
}

int write_CPCDSK_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename)
{	
	int i,j,k,nbsector;
	FILE * cpcdskfile;
	char * log_str;
	char   tmp_str[256];
	char   disk_info_block[256];
	char rec_mode;
	int sectorsize;
	int track_cnt;
	int sectorlistoffset,trackinfooffset;
	sect_track track;
	cpcdsk_fileheader * cpcdsk_fh;
	cpcdsk_trackheader cpcdsk_th;
	cpcdsk_sector cpcdsk_s;


	floppycontext->hxc_printf(MSG_INFO_1,"Write CPCDSK file %s...",filename);

	log_str=0;
	cpcdskfile=fopen(filename,"wb");
	if(cpcdskfile)
	{
		memset(disk_info_block,0,0x100);
		cpcdsk_fh=(cpcdsk_fileheader *)&disk_info_block;
		sprintf((char*)&cpcdsk_fh->headertag,"EXTENDED CPC DSK File\r\nDisk-Info\r\n");
		sprintf((char*)&cpcdsk_fh->creatorname,"HxCFloppyEmu\r\n");
		fwrite(&disk_info_block,0x100,1,cpcdskfile);
		track_cnt=0;

		for(j=0;j<(int)floppy->floppyNumberOfTrack;j++)
		{
			for(i=0;i<(int)floppy->floppyNumberOfSide;i++)
			{
				sprintf(tmp_str,"track:%.2d:%d file offset:0x%.6x, sectors: ",j,i,ftell(cpcdskfile));

				log_str=0;
				log_str=realloc(log_str,strlen(tmp_str)+1);
				memset(log_str,0,strlen(tmp_str)+1);
				strcat(log_str,tmp_str);

				rec_mode=0;
				memset(&track,0,sizeof(sect_track));
				track.side=i;
				track.track=j;
				rec_mode=2;
				nbsector=analysis_and_extract_sector_MFM(floppycontext,floppy->tracks[j]->sides[i],&track);
				if(!nbsector)
				{
					nbsector=analysis_and_extract_sector_FM(floppycontext,floppy->tracks[j]->sides[i],&track);
					rec_mode=1;
					if(!nbsector)
					{
						rec_mode=0;
						nbsector=analysis_and_extract_sector_AMIGAMFM(floppycontext,floppy->tracks[j]->sides[i],&track);
					}
				}

				memset(&cpcdsk_th,0,sizeof(cpcdsk_trackheader));
				sprintf(cpcdsk_th.headertag,"Track-Info\r\n");
				cpcdsk_th.side_number=i;
				cpcdsk_th.track_number=j;
				cpcdsk_th.gap3_length=78;
				cpcdsk_th.filler_byte=0xE5;
				cpcdsk_th.number_of_sector=track.number_of_sector;
				cpcdsk_th.rec_mode=rec_mode;

				switch(floppy->tracks[j]->sides[i]->bitrate)
				{
					case 250000:
						cpcdsk_th.datarate=1;
						break;
					case 500000:
						cpcdsk_th.datarate=2;
						break;
					case 1000000:
						cpcdsk_th.datarate=3;
						break;
					default:
						cpcdsk_th.datarate=0;
						break;

				}


				if(track.number_of_sector)
				{
					cpcdsk_th.sector_size_code=size_to_code(track.sectorlist[0]->sectorsize);
				}

				trackinfooffset=ftell(cpcdskfile);
				fwrite(&cpcdsk_th,sizeof(cpcdsk_trackheader),1,cpcdskfile);
				sectorlistoffset=ftell(cpcdskfile);

				if(track.number_of_sector)
				{
					if(cpcdsk_fh->number_of_sides<(i+1))cpcdsk_fh->number_of_sides=i+1;
					if(cpcdsk_fh->number_of_tracks<(j+1))cpcdsk_fh->number_of_tracks=j+1;


					memset(&cpcdsk_s,0,sizeof(cpcdsk_sector));
					for(k=0;k<track.number_of_sector;k++) 
					{
						fwrite(&cpcdsk_s,sizeof(cpcdsk_sector),1,cpcdskfile);
					}
					memset(tmp_str,0,0x100);
					fwrite(&tmp_str,0x100-(((sizeof(cpcdsk_sector)*track.number_of_sector)+sizeof(cpcdsk_trackheader))%0x100),1,cpcdskfile);

					sectorsize=track.sectorlist[0]->sectorsize;
			
					k=0;
					do
					{
						if(track.sectorlist[k]->sectorsize!=sectorsize)
						{
							sectorsize=-1;	
						}


						cpcdsk_s.sector_id=track.sectorlist[k]->sector_id;
						cpcdsk_s.side=track.sectorlist[k]->side_id;
						cpcdsk_s.track=track.sectorlist[k]->track_id;
						cpcdsk_s.sector_size_code=size_to_code(track.sectorlist[k]->sectorsize);
						cpcdsk_s.data_lenght=track.sectorlist[k]->sectorsize;								
						fseek(cpcdskfile,sectorlistoffset+(k*sizeof(cpcdsk_sector)),SEEK_SET);
						fwrite(&cpcdsk_s,sizeof(cpcdsk_sector),1,cpcdskfile);

						fseek(cpcdskfile,0,SEEK_END);
						fwrite(track.sectorlist[k]->buffer,track.sectorlist[k]->sectorsize,1,cpcdskfile);

						sprintf(tmp_str,"%d ",track.sectorlist[k]->sector_id);
						log_str=realloc(log_str,strlen(log_str)+strlen(tmp_str)+1);
						strcat(log_str,tmp_str);
						k++;
			
					}while(k<track.number_of_sector);
					
					
				


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
			
					log_str=realloc(log_str,strlen(log_str)+strlen(tmp_str)+1);
					strcat(log_str,tmp_str);

				}

				disk_info_block[sizeof(cpcdsk_fileheader)+track_cnt]=(ftell(cpcdskfile)-trackinfooffset)/256;
				track_cnt++;

				floppycontext->hxc_printf(MSG_INFO_1,log_str);
				free(log_str);
			
			}
		}	

		fseek(cpcdskfile,0,SEEK_SET);
		fwrite(&disk_info_block,0x100,1,cpcdskfile);

		fclose(cpcdskfile);
	}
	
	return 0;
}
