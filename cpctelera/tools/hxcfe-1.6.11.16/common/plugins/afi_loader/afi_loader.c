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
// File : MFM_DiskFile.c
// Contains: MFM floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"

#include "afi_loader.h"
#include "afi_file_writer.h"

#include "../common/crc.h"
#include "./libs/zlib/zlib.h"

#include "../common/os_api.h"

unsigned short filecheckcrc(FILE * f,unsigned long fileoffset,unsigned long size)
{
	unsigned char crc16l,crc16h;
	unsigned long temp_fileptr;
	unsigned char buffer[512];
	unsigned char crctable[32];
	int i,s;

	CRC16_Init(&crc16h,&crc16l,(unsigned char*)crctable,0x1021,0xFFFF);	

	temp_fileptr=ftell(f);
	fseek(f,fileoffset,SEEK_SET);
	s=size;
	while(s)
	{

		if(s>512)
		{
			fread(&buffer,512,1,f);
			for(i=0;i<512;i++)
			{
				CRC16_Update(&crc16h,&crc16l,buffer[i],(unsigned char*)crctable);	
			}
			s=s-512;
		}
		else
		{
			fread(&buffer,s,1,f);
			for(i=0;i<s;i++)
			{
				CRC16_Update(&crc16h,&crc16l,buffer[i],(unsigned char*)crctable);	
			}
			s=0;
		}

	}
	fseek(f,temp_fileptr,SEEK_SET);

	return (crc16l<<8) | crc16h;
}


int AFI_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	FILE *f;
	AFIIMG header;

	floppycontext->hxc_printf(MSG_DEBUG,"AFI_libIsValidDiskFile %s",imgfile);

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

				if(strstr( filepath,".afi" )!=NULL)
				{

					f=fopen(imgfile,"rb");
					if(f==NULL) 
					{
						return LOADER_ACCESSERROR;
					}
					fread(&header,sizeof(header),1,f);
					fclose(f);

					if( !strcmp(header.afi_img_tag,AFI_IMG_TAG))
					{
						floppycontext->hxc_printf(MSG_DEBUG,"AFI file !");
						free(filepath);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non AFI file !");
						free(filepath);
						return LOADER_BADFILE;
					}
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non AFI file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}

int AFI_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	AFIIMG header;
	AFIIMGINFO  afiinfo;
	AFITRACKLIST trackliststruct;
	AFITRACK track;
	AFIDATA datablock;
	unsigned int i,j,k;
	unsigned long destLen;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	unsigned long * tracklistoffset;
	unsigned long * datalistoffset;

	unsigned char * temp_uncompressbuffer;
	
	floppycontext->hxc_printf(MSG_DEBUG,"AFI_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	

	fread(&header,sizeof(header),1,f);
	
	if(!strcmp(header.afi_img_tag,AFI_IMG_TAG))
	{

		
		if(filecheckcrc(f,0,sizeof(AFIIMG)))
		{
				floppycontext->hxc_printf(MSG_ERROR,"bad header CRC !");
				fclose(f);
				return LOADER_BADFILE;
		}


		fseek(f,header.floppyinfo_offset,SEEK_SET);
		fread(&afiinfo,sizeof(afiinfo),1,f);

		floppydisk->floppyNumberOfTrack=afiinfo.end_track+1;
		floppydisk->floppyNumberOfSide=afiinfo.end_side+1;
		floppydisk->floppySectorPerTrack=-1;
		floppydisk->floppyiftype=ATARIST_DD_FLOPPYMODE;


		switch(afiinfo.platformtype_code)
		{
		case AFI_PLATFORM_ATARI_ST:
			switch(afiinfo.mediatype_code)
			{
			case AFI_MEDIA_3P50_DD:
				floppydisk->floppyiftype=ATARIST_DD_FLOPPYMODE;
				break;
			case AFI_MEDIA_3P50_HD:
				floppydisk->floppyiftype=ATARIST_HD_FLOPPYMODE;
				break;

			}
			break;
		case AFI_PLATFORM_AMIGA:
			switch(afiinfo.mediatype_code)
			{
			case AFI_MEDIA_3P50_DD:
				floppydisk->floppyiftype=AMIGA_DD_FLOPPYMODE;
				break;
			case AFI_MEDIA_3P50_HD:
				floppydisk->floppyiftype=AMIGA_HD_FLOPPYMODE;
				break;

			}
			break;

		case AFI_PLATFORM_PC:
			switch(afiinfo.mediatype_code)
			{
			case AFI_MEDIA_3P50_DD:
				floppydisk->floppyiftype=IBMPC_DD_FLOPPYMODE;
				break;
			case AFI_MEDIA_3P50_HD:
				floppydisk->floppyiftype=IBMPC_HD_FLOPPYMODE;
				break;
			case AFI_MEDIA_3P50_ED:
				floppydisk->floppyiftype=IBMPC_ED_FLOPPYMODE;
				break;
			}
			break;
		case AFI_PLATFORM_CPC:
			floppydisk->floppyiftype=CPC_DD_FLOPPYMODE;
			break;
		case AFI_PLATFORM_MSX2:
			floppydisk->floppyiftype=MSX2_DD_FLOPPYMODE;
			break;
		}

		//floppydisk->floppyBitRate=header.floppyBitRate*1000;
		floppycontext->hxc_printf(MSG_DEBUG,"AFI File : %d track, %d side, %d bit/s, %d sectors, mode %d",
			floppydisk->floppyNumberOfTrack,
			floppydisk->floppyNumberOfSide,
			floppydisk->floppyBitRate,
			floppydisk->floppySectorPerTrack,
			floppydisk->floppyiftype);


		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
		memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
		
		fseek(f,header.track_list_offset,SEEK_SET);
		fread(&trackliststruct,sizeof(trackliststruct),1,f);
		if(strcmp(trackliststruct.afi_img_track_list_tag,AFI_TRACKLIST_TAG))
		{
				floppycontext->hxc_printf(MSG_ERROR,"bad AFI_TRACKLIST_TAG");
				return LOADER_BADFILE;
		}

		tracklistoffset=(unsigned long*)malloc(trackliststruct.number_of_track*4);
		fread(tracklistoffset,trackliststruct.number_of_track*4,1,f);

		for(i=0;i<trackliststruct.number_of_track;i++)
		{			

			fseek(f,header.track_list_offset+tracklistoffset[i],SEEK_SET); 
			fread(&track,sizeof(track),1,f);
			if(strcmp(track.afi_track_tag,AFI_TRACK_TAG))
			{	
				floppycontext->hxc_printf(MSG_ERROR,"bad AFI_TRACK_TAG");
				return LOADER_BADFILE;
			}

			if(filecheckcrc(f,header.track_list_offset+tracklistoffset[i],(track.number_of_data_chunk*sizeof(unsigned long))+sizeof(AFITRACK)+sizeof(unsigned short)))
			{
				floppycontext->hxc_printf(MSG_ERROR,"bad track CRC !");
				fclose(f);
				return LOADER_BADFILE;
			}

			datalistoffset=(unsigned long *)malloc(track.number_of_data_chunk*sizeof(unsigned long));
			fread(datalistoffset,track.number_of_data_chunk*sizeof(unsigned long),1,f);

			if(!floppydisk->tracks[track.track_number])
			{
				floppydisk->tracks[track.track_number]=(CYLINDER*)malloc(sizeof(CYLINDER));
				currentcylinder=floppydisk->tracks[track.track_number];
				currentcylinder->number_of_side=floppydisk->floppyNumberOfSide;
				currentcylinder->sides=(SIDE**)malloc(sizeof(SIDE*)*currentcylinder->number_of_side);
				memset(currentcylinder->sides,0,sizeof(SIDE*)*currentcylinder->number_of_side);								
				currentcylinder->floppyRPM=0;//header.floppyRPM;
			}
			

			floppycontext->hxc_printf(MSG_DEBUG,"read track %d side %d at offset 0x%x (0x%x bytes)",
			track.track_number,
			track.side_number,
			track.number_of_data_chunk,
			track.nb_of_element);

			currentcylinder->sides[track.side_number]=malloc(sizeof(SIDE));
			memset(currentcylinder->sides[track.side_number],0,sizeof(SIDE));
			currentside=currentcylinder->sides[track.side_number];

			currentside->number_of_sector=floppydisk->floppySectorPerTrack;
			currentside->tracklen=track.nb_of_element;
			
			currentside->track_encoding=UNKNOWN_ENCODING;
			currentside->bitrate=250000;
			currentside->flakybitsbuffer=0;
			for(j=0;j<track.number_of_data_chunk;j++)
			{


				fseek(f,header.track_list_offset+tracklistoffset[i]+datalistoffset[j],SEEK_SET); 
				fread(&datablock,sizeof(datablock),1,f);

				if(strcmp(datablock.afi_data_tag,AFI_DATA_TAG))
				{	
					floppycontext->hxc_printf(MSG_ERROR,"bad AFI_DATA_TAG");
					return LOADER_BADFILE;
				}

				if(filecheckcrc(f,header.track_list_offset+tracklistoffset[i]+datalistoffset[j],sizeof(AFIDATA)+datablock.packed_size+sizeof(unsigned short)))
				{
					floppycontext->hxc_printf(MSG_ERROR,"bad data CRC !");
					fclose(f);
					return LOADER_BADFILE;
				}

				switch(datablock.TYPEIDCODE)
				{
				case AFI_DATA_MFM:
					switch(datablock.packer_id)
					{
						case AFI_COMPRESS_NONE:
							currentside->databuffer=malloc(datablock.unpacked_size);
							fread(currentside->databuffer,datablock.unpacked_size,1,f);
							break;
						case AFI_COMPRESS_GZIP:
							currentside->databuffer=malloc(datablock.unpacked_size);
							temp_uncompressbuffer=malloc(datablock.packed_size);
							fread(temp_uncompressbuffer,datablock.packed_size,1,f);
							destLen=datablock.unpacked_size;
							uncompress(currentside->databuffer, &destLen,temp_uncompressbuffer, datablock.packed_size);
							free(temp_uncompressbuffer);
							break;
						default:
						break;
					}
					break;

				case AFI_DATA_INDEX:
					switch(datablock.packer_id)
					{
						case AFI_COMPRESS_NONE:
							currentside->indexbuffer=malloc(datablock.packed_size);
							fread(currentside->indexbuffer,datablock.packed_size,1,f);

							break;
						case AFI_COMPRESS_GZIP:
							currentside->indexbuffer=malloc(datablock.unpacked_size);
							temp_uncompressbuffer=malloc(datablock.packed_size);
							fread(temp_uncompressbuffer,datablock.packed_size,1,f);
							destLen=datablock.unpacked_size;
							uncompress(currentside->indexbuffer, &destLen,temp_uncompressbuffer, datablock.packed_size);
							free(temp_uncompressbuffer);
							break;
						default:
						break;
					}
					break;

				case AFI_DATA_BITRATE:
					currentside->bitrate=VARIABLEBITRATE;
					switch(datablock.packer_id)
					{
						case AFI_COMPRESS_NONE:
							
							currentside->timingbuffer=malloc(datablock.packed_size);
							fread(currentside->timingbuffer,datablock.packed_size,1,f);
							
							k=0;
							do
							{
								k++;
							}while((currentside->timingbuffer[k-1]==currentside->timingbuffer[k]) && k<currentside->tracklen);

							if(k==currentside->tracklen)
							{
								currentside->bitrate=currentside->timingbuffer[0];
								free(currentside->timingbuffer);
								currentside->timingbuffer=0;
							}


							break;

						case AFI_COMPRESS_GZIP:
							currentside->timingbuffer=malloc(datablock.unpacked_size);
							temp_uncompressbuffer=malloc(datablock.packed_size);
							fread(temp_uncompressbuffer,datablock.packed_size,1,f);
							destLen=datablock.unpacked_size;
							uncompress((unsigned char*)currentside->timingbuffer, &destLen,temp_uncompressbuffer, datablock.packed_size);
							
							k=0;
							do
							{
								k++;
							}while((currentside->timingbuffer[k-1]==currentside->timingbuffer[k]) && k<currentside->tracklen);

							if(k==currentside->tracklen)
							{
								currentside->bitrate=currentside->timingbuffer[0];
								free(currentside->timingbuffer);
								currentside->timingbuffer=0;
							}
							
							free(temp_uncompressbuffer);
							break;
						default:
						break;
					}
					break;

				case AFI_DATA_PDC:
					break;

				case AFI_DATA_WEAKBITS:
					switch(datablock.packer_id)
					{
						case AFI_COMPRESS_NONE:
							currentside->flakybitsbuffer=malloc(datablock.packed_size);
							fread(currentside->flakybitsbuffer,datablock.packed_size,1,f);
							k=0;
							do
							{
								k++;
							}while((currentside->flakybitsbuffer[k-1]==currentside->flakybitsbuffer[k]) && k<currentside->tracklen);

							if(k==currentside->tracklen)
							{
								free(currentside->flakybitsbuffer);
								currentside->flakybitsbuffer=0;
							}
							break;
						case AFI_COMPRESS_GZIP:
							currentside->flakybitsbuffer=malloc(datablock.unpacked_size);
							temp_uncompressbuffer=malloc(datablock.packed_size);
							fread(temp_uncompressbuffer,datablock.packed_size,1,f);
							destLen=datablock.unpacked_size;
							uncompress(currentside->flakybitsbuffer, &destLen,temp_uncompressbuffer, datablock.packed_size);
							
							k=0;
							do
							{
								k++;
							}while((currentside->flakybitsbuffer[k-1]==currentside->flakybitsbuffer[k]) && !currentside->flakybitsbuffer[k] && k<currentside->tracklen);

							if(k==currentside->tracklen)
							{
								free(currentside->flakybitsbuffer);
								currentside->flakybitsbuffer=0;
							}
							
							free(temp_uncompressbuffer);
							break;
						default:
						break;
					}
					break;

				default:
					break;

				}
			}

			//currentside->bitrate=VARIABLEBITRATE;//floppydisk->floppyBitRate;
					
		}			
	

		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{		
				if(floppydisk->tracks[j])
				{
					if(floppydisk->tracks[j]->sides[i])
					{
						currentside=floppydisk->tracks[j]->sides[i];
						currentside->tracklen=currentside->tracklen*8;
					}
				}					
			}
		}

		fclose(f);
		return LOADER_NOERROR;
	}	
	
	fclose(f);	
	floppycontext->hxc_printf(MSG_ERROR,"bad header");
	return LOADER_BADFILE;
}

