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
// File : teledisk_loader.c
// Contains: Teledisk (TD0) floppy image loader and plugins interfaces
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

#include "teledisk_loader.h"

#include "teledisk_format.h"
#include "td0_lzss.h"


int TeleDisk_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen,i;
	FILE * f;
	unsigned char crctable[32];
	unsigned char CRC16_High,CRC16_Low;
	TELEDISK_HEADER td_header;
	unsigned char * ptr;

	floppycontext->hxc_printf(MSG_DEBUG,"TeleDisk_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{
		pathlen=strlen(imgfile);
		if(pathlen!=0)
		{			
			f=fopen(imgfile,"rb");
			if(f==NULL) 
			{
				floppycontext->hxc_printf(MSG_ERROR,"Cannot open the file!");
				return LOADER_ACCESSERROR;
			}

			memset(&td_header,0,sizeof(td_header));
			fread( &td_header, sizeof(td_header), 1, f );  

			if(ftell(f)==sizeof(td_header))
			{

				if ( ((td_header.TXT[0]!='t') || (td_header.TXT[1]!='d')) && ((td_header.TXT[0]!='T') || (td_header.TXT[1]!='D'))) 
				{
					floppycontext->hxc_printf(MSG_DEBUG,"bad header tag !");
					fclose(f);
					return LOADER_BADFILE;
				}

				CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
				ptr=(unsigned char*)&td_header;
				for(i=0;i<0xA;i++)
				{
					CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
				}
				
				if(td_header.CRC!=((CRC16_High<<8)|CRC16_Low))
				{
					floppycontext->hxc_printf(MSG_DEBUG,"bad header crc !");
					fclose(f);
     				return LOADER_BADFILE;
				}

				floppycontext->hxc_printf(MSG_DEBUG,"it's a Tele disk file!");
				fclose(f);
				return LOADER_ISVALID;
			}
	
			floppycontext->hxc_printf(MSG_DEBUG,"bad header tag !");
			fclose(f);
     		return LOADER_BADFILE;
		}
	}

	return LOADER_BADPARAMETER;
}


int RLEExpander(unsigned char *src,unsigned char *dst,int blocklen)
{
  unsigned char *s1,*s2,d1,d2;
  unsigned char type;
  unsigned short len,len2,rlen;

  unsigned int uBlock;
  unsigned int uCount;
  
  s2=dst;
  
  len=(src[1]<<8)|src[0];
  type=src[2];
  len--;
  src=src+3;

  switch ( type )
  {
    case 0:
    {
	  memcpy(dst,src,len);
      rlen=len;
      break;
    }
    
	case 1:
    {
      len=(src[1]<<8) | src[0];
      rlen=len<<1;
      d1=src[2];
	  d2=src[3];
      while (len--) 
	  {
		  *dst++=d1;
		  *dst++=d2;
	  }
	  src=src+4;
      break;
    }

	case 2:
	{
		rlen=0;
		len2=len;
		s1=&src[0];
		s2=dst;
		do
		{
			if( !s1[0])
			{
				len2--;

				len=s1[1];
				s1=s1+2;
				len2--;

				memcpy(s2,s1,len);
				rlen=rlen+len;
				s2=s2+len;
				s1=s1+len;
				
				len2=len2-len;
			}
			else
			{
				uBlock = 1<<*s1;
				s1++;
				len2--;

				uCount = *s1;
				s1++;
				len2--;
				
				while(uCount)
				{
                        memcpy(s2, s1, uBlock);
						rlen=rlen+uBlock;
						s2=s2+uBlock;
						uCount--;
				}

                s1=s1 + uBlock;
				len2=len2-uBlock;
			}

		}while(len2);
    }

    default:
    {
      rlen=-1;
    }
  }

  return rlen;
}


int TeleDisk_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	FILE * f;
	unsigned int i;
	unsigned int file_offset;
	unsigned long tracklen;
	unsigned char interleave,skew,trackformat;
	unsigned short rpm,sectorsize;
	int Compress,numberoftrack,sidenumber;
	unsigned short * datalen;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	TELEDISK_HEADER        *td_header;	
	TELEDISK_TRACK_HEADER  *td_track_header;
	TELEDISK_SECTOR_HEADER *td_sector_header;
	TELEDISK_COMMENT * td_comment;
	unsigned char tempdata[8*1024];
	unsigned char crctable[32];
	unsigned char CRC16_High,CRC16_Low;
	unsigned char * ptr;
	unsigned long filesize;
	SECTORCONFIG  * sectorconfig;
	unsigned char * fileimage;
	unsigned long fileimage_buffer_offset;
	int rlen;
	floppycontext->hxc_printf(MSG_DEBUG,"TeleDisk_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}

	fseek(f,0,SEEK_END);
	filesize=ftell(f);
	if(!filesize)
	{
		floppycontext->hxc_printf(MSG_ERROR,"0 byte file !");
		fclose(f);
		return LOADER_BADFILE;
	}
	fseek(f,0,SEEK_SET);

	fileimage_buffer_offset=0;
	fileimage=(unsigned char*)malloc(filesize+512);
	if(!fileimage)
	{
		floppycontext->hxc_printf(MSG_ERROR,"Malloc error !");
		fclose(f);
		return LOADER_INTERNALERROR;
	}
	memset(fileimage,0,filesize+512);
	fread(fileimage,filesize,1,f);
	fclose(f);


	td_header=(TELEDISK_HEADER*)&fileimage[fileimage_buffer_offset];
	fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_HEADER);

	if ( ((td_header->TXT[0]!='t') || (td_header->TXT[1]!='d')) && ((td_header->TXT[0]!='T') || (td_header->TXT[1]!='D'))) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"bad header tag !");
		free(fileimage);
		return LOADER_BADFILE;
	}

	CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
	ptr=(unsigned char*)td_header;
	for(i=0;i<0xA;i++)
	{
		CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
	}
				
	if(td_header->CRC!=((CRC16_High<<8)|CRC16_Low))
	{
		floppycontext->hxc_printf(MSG_ERROR,"bad header crc !");
		free(fileimage);
		return LOADER_BADFILE;
	}

	floppycontext->hxc_printf(MSG_INFO_1,"Teledisk version : %d",td_header->TDVer);
	if((td_header->TDVer>21) || (td_header->TDVer<10))
	{
		floppycontext->hxc_printf(MSG_ERROR,"Unsupported version !");
		free(fileimage);
		return LOADER_BADFILE;
	}

	Compress=0;
	if(((td_header->TXT[0]=='T') && (td_header->TXT[1]=='D')))
	{
		floppycontext->hxc_printf(MSG_INFO_1,"Normal compression");
		Compress=0;
	}

	if(((td_header->TXT[0]=='t') && (td_header->TXT[1]=='d')))
	{
		floppycontext->hxc_printf(MSG_INFO_1,"Advanced compression");
		fileimage=unpack(fileimage,filesize);
		Compress=1;
	}

	td_header=(TELEDISK_HEADER*)&fileimage[0];

	if(td_header->TrkDens&0x80)
	{

		CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);

		td_comment=(TELEDISK_COMMENT *)&fileimage[fileimage_buffer_offset];
		fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_COMMENT);

		//fread( &td_comment, sizeof(td_comment), 1, f );
		ptr=(unsigned char*)td_comment;
		ptr=ptr+2;
		for(i=0;i<sizeof(TELEDISK_COMMENT)-2;i++)
		{
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
		}

		memcpy(&tempdata,&fileimage[fileimage_buffer_offset],td_comment->Len);
		fileimage_buffer_offset=fileimage_buffer_offset+td_comment->Len;

		ptr=(unsigned char*)&tempdata;
		for(i=0;i<td_comment->Len;i++)
		{
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
		}


		floppycontext->hxc_printf(MSG_INFO_1,"Creation date: %.2d/%.2d/%.4d %.2d:%.2d:%.2d",td_comment->bDay,\
																							td_comment->bMon+1,\
																							td_comment->bYear+1900,\
																							td_comment->bHour,\
																							td_comment->bMin,\
																							td_comment->bSec);
		floppycontext->hxc_printf(MSG_INFO_1,"Comment: %s",tempdata);

	}

	interleave=1;
	numberoftrack=0;
	sectorsize=512;
	
	file_offset=fileimage_buffer_offset;

	floppydisk->floppyNumberOfSide=td_header->Surface;

	td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
	fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);

	while(td_track_header->SecPerTrk!=0xFF)
	{
		if(td_track_header->PhysCyl>numberoftrack)
		{
			numberoftrack=td_track_header->PhysCyl;
		}
		CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
		ptr=(unsigned char*)td_track_header;
		for(i=0;i<0xA;i++)
		{
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
		}


		for ( i=0;i < td_track_header->SecPerTrk;i++ )
		{
			td_sector_header=(TELEDISK_SECTOR_HEADER  *)&fileimage[fileimage_buffer_offset];
			fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_SECTOR_HEADER);

			if  ( (td_sector_header->Syndrome & 0x30) == 0 && (td_sector_header->SLen & 0xf8) == 0 )
			{
				//fileimage_buffer_offset=fileimage_buffer_offset+sizeof(unsigned short);

				datalen=(unsigned short*)&fileimage[fileimage_buffer_offset];
				fileimage_buffer_offset=fileimage_buffer_offset+(*datalen)+2;
			}
		}
		td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
		fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);
	}

	floppydisk->floppyNumberOfTrack=numberoftrack+1;
	floppydisk->floppySectorPerTrack=-1;
	floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
	memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

	//Source disk density (0 = 250K bps,  1 = 300K bps,  2 = 500K bps ; +128 = single-density FM)
	switch(td_header->Dens)
	{
		case 0:
			floppydisk->floppyBitRate=250000;
			break;
		case 1:
			floppydisk->floppyBitRate=300000;
			break;
		case 2:
			floppydisk->floppyBitRate=500000;
			break;
		default:
			floppydisk->floppyBitRate=250000;
			break;
	}

	floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;

	skew=1;
	rpm=300; // normal rpm			
	floppycontext->hxc_printf(MSG_INFO_1,"%d tracks, %d side(s), gap3:%d,rpm:%d bitrate:%d",floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,rpm,floppydisk->floppyBitRate);
	
	tracklen=(floppydisk->floppyBitRate/(rpm/60))/4;

	//////////////////////////////////
	fileimage_buffer_offset=file_offset;

	td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
	fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);
	
	while(td_track_header->SecPerTrk!=0xFF)
	{
		if(td_track_header->PhysSide&0x7F)
		{
			sidenumber=1;
		}
		else
		{
			sidenumber=0;
		}

		if(td_track_header->PhysSide&0x80)
		{
			trackformat=ISOFORMAT_SD;
		}
		else
		{
			trackformat=ISOFORMAT_DD;
		}
		
		floppycontext->hxc_printf(MSG_DEBUG,"------------- Track:%d, Side:%d, Number of Sector:%d -------------",td_track_header->PhysCyl,sidenumber,td_track_header->SecPerTrk);
		

		if(!floppydisk->tracks[td_track_header->PhysCyl])
		{
			floppydisk->tracks[td_track_header->PhysCyl]=(CYLINDER*)malloc(sizeof(CYLINDER));
			memset(floppydisk->tracks[td_track_header->PhysCyl],0,sizeof(CYLINDER));
		}

		currentcylinder=floppydisk->tracks[td_track_header->PhysCyl];
		currentcylinder->number_of_side=floppydisk->floppyNumberOfSide;
		if(!currentcylinder->sides)
		{
			currentcylinder->sides=(SIDE**)malloc(sizeof(SIDE*)*currentcylinder->number_of_side);
			memset(currentcylinder->sides,0,sizeof(SIDE*)*currentcylinder->number_of_side);
		}

		currentcylinder->floppyRPM=rpm;

		////////////////////crc track header///////////////////
		CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0xA097,0x0000);
		ptr=(unsigned char*)td_track_header;
		for(i=0;i<0x3;i++)
		{
			CRC16_Update(&CRC16_High,&CRC16_Low, ptr[i],(unsigned char*)crctable );
		}
		if(CRC16_Low!=td_track_header->CRC)
		{
			floppycontext->hxc_printf(MSG_ERROR,"!!!! Track header CRC Error !!!!");			
		}
		////////////////////////////////////////////////////////

		sectorconfig=(SECTORCONFIG  *)malloc(sizeof(SECTORCONFIG)*td_track_header->SecPerTrk);
		memset(sectorconfig,0,sizeof(SECTORCONFIG)*td_track_header->SecPerTrk);
		for ( i=0;i < td_track_header->SecPerTrk;i++ )
		{
			td_sector_header=(TELEDISK_SECTOR_HEADER  *)&fileimage[fileimage_buffer_offset];
			fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_SECTOR_HEADER);

			sectorconfig[i].cylinder=td_sector_header->Cyl;
			sectorconfig[i].head=td_sector_header->Side;
			sectorconfig[i].sector=td_sector_header->SNum;
			sectorconfig[i].sectorsize=128<<td_sector_header->SLen;
			sectorconfig[i].bitrate=floppydisk->floppyBitRate;
			sectorconfig[i].gap3=255;
			sectorconfig[i].trackencoding=trackformat;

			
			if(td_sector_header->Syndrome & 0x04)
			{
				sectorconfig[i].use_alternate_datamark=1;
				sectorconfig[i].alternate_datamark=0xF8;
			}

			if(td_sector_header->Syndrome & 0x02)
			{
				sectorconfig[i].use_alternate_data_crc=2;
			}

			if(td_sector_header->Syndrome & 0x20)
			{
				sectorconfig[i].missingdataaddressmark=1;
			}
			
			sectorconfig[i].input_data=malloc(sectorconfig[i].sectorsize);
			if  ( (td_sector_header->Syndrome & 0x30) == 0 && (td_sector_header->SLen & 0xf8) == 0 )
			{			
				//fileimage_buffer_offset=fileimage_buffer_offset+sizeof(unsigned short);

				datalen=(unsigned short*)&fileimage[fileimage_buffer_offset];
				memcpy(&tempdata,&fileimage[fileimage_buffer_offset],(*datalen)+2);
				fileimage_buffer_offset=fileimage_buffer_offset+(*datalen)+2;
				
				rlen=RLEExpander(tempdata,sectorconfig[i].input_data,(int)datalen);
			}
			else
			{
				memset(sectorconfig[i].input_data,0,sectorconfig[i].sectorsize);
			}

			floppycontext->hxc_printf(MSG_DEBUG,"track:%d, side:%d, sector:%d, sectorsize:%d, flag:%.2x",sectorconfig[i].cylinder,sectorconfig[i].head,sectorconfig[i].sector,sectorconfig[i].sectorsize,td_sector_header->Syndrome);
		}

		currentside=tg_generatetrackEx((unsigned short)td_track_header->SecPerTrk,sectorconfig,interleave,0,floppydisk->floppyBitRate,rpm,trackformat,2500 | NO_SECTOR_UNDER_INDEX,-2500);
		currentcylinder->sides[sidenumber]=currentside;

		for ( i=0;i < td_track_header->SecPerTrk;i++ )
		{
			if(sectorconfig[i].input_data)
				free(sectorconfig[i].input_data);
		}
		free(sectorconfig);

		td_track_header=(TELEDISK_TRACK_HEADER  *)&fileimage[fileimage_buffer_offset];
		fileimage_buffer_offset=fileimage_buffer_offset+sizeof(TELEDISK_TRACK_HEADER);

	}
		
	floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");

	free(fileimage);
		
	return LOADER_NOERROR;
}



