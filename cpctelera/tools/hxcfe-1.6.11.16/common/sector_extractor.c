
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "sector_extractor.h"
#include "plugins/common/crc.h"

extern unsigned char bit_inverter_emuii[];
unsigned short biteven[]=
{
	0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 
	0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055, 
	0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 
	0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155, 
	0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 
	0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455, 
	0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 
	0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555, 
	0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 
	0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055, 
	0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 
	0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155, 
	0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 
	0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455, 
	0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 
	0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555, 
	0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 
	0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055, 
	0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 
	0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155, 
	0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 
	0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455, 
	0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 
	0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555, 
	0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 
	0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055, 
	0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 
	0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155, 
	0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 
	0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455, 
	0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 
	0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};


int getbit(unsigned char * input_data,int bit_offset)
{
		return ((input_data[bit_offset>>3]>>(0x7-(bit_offset&0x7))))&0x01;	
}


int mfmtobin(unsigned char * input_data,int intput_data_size,unsigned char * decod_data,int decod_data_size,int bit_offset,int lastbit)
{
	int i;
	int bitshift;
	
	i=0;
	bitshift=0;
	do
	{

		switch( getbit(input_data,bit_offset) | getbit(input_data,bit_offset+1)<<1 )
		{
			case 0x0:
				decod_data[i]=decod_data[i]&(~(0x01<<(0x7-bitshift)));
				break;
			case 0x1:
				decod_data[i]=decod_data[i]&(~(0x01<<(0x7-bitshift)));
				break;
			case 0x2:
				decod_data[i]=decod_data[i]|(0x01<<(0x7-bitshift));
				break;
			case 0x3:
				decod_data[i]=decod_data[i]&(~(0x01<<(0x7-bitshift)));
				break;
		}

		bitshift++;
		bit_offset=bit_offset+2;
		if(bitshift==8)
		{
			bitshift=0;
			// exchange quartet
			//decod_data[i]= ((decod_data[i]&0xF)<<4) | ((decod_data[i]&0xF0)>>4);
			i++;
		}

	}while(i<decod_data_size);
	
	return 0;
}



int fmtobin(unsigned char * input_data,int intput_data_size,unsigned char * decod_data,int decod_data_size,int bit_offset,int lastbit)
{
	int i;
	int bitshift;
	unsigned char binbyte;

	i=0;
	bitshift=0;
	binbyte=0;
	do
	{
		//0C0D0C0D

		binbyte=binbyte | (getbit(input_data,bit_offset+3)<<1) | (getbit(input_data,bit_offset+7)<<0);

		bitshift=bitshift+2;
	
		if(bitshift==8)
		{
			decod_data[i]=binbyte;
			bitshift=0;
			binbyte=0;
			i++;
		}
		else
		{
			binbyte=binbyte<<2;
		}

		bit_offset=bit_offset+8;


	}while(i<decod_data_size);
	
	return 0;
}

int bitslookingfor(unsigned char * input_data,unsigned long intput_data_size,unsigned char * chr_data,unsigned long chr_data_size,unsigned long bit_offset)
{
	unsigned long i,c,chr_data_offset;
	unsigned char current_data_bit;
	unsigned char stringtosearch[128];

	for(i=0;i<chr_data_size;i++)
	{
		stringtosearch[i]=chr_data[i>>3]>>(0x7-(i&0x7));
	}

	intput_data_size=intput_data_size-chr_data_size;
	do
	{
		chr_data_offset=0;
		i=bit_offset;
		c=chr_data_size;
		do
		{
			current_data_bit=((input_data[i>>3]>>(0x7-(i&0x7)) ) ^ stringtosearch[chr_data_offset]);
			chr_data_offset++;
			i++;		
			c--;
		}while(c && (!(current_data_bit&0x1)));
		
		bit_offset++;

	}while(c && (bit_offset<intput_data_size));

	bit_offset--;
	if(c)
	{
		bit_offset=-1;
	}



	return bit_offset;
}

#define LOOKFOR_GAP1 0x01
#define LOOKFOR_ADDM 0x02
#define ENDOFTRACK 0x03
#define EXTRACTSECTORINFO 0x04

unsigned short sectorsize[]={128,256,512,1024,2048,4096,8192,16384};

int analysis_and_extract_sector_MFM(HXCFLOPPYEMULATOR* floppycontext,SIDE * track,sect_track * sectors)
{
	int bit_offset_bak,bit_offset,old_bit_offset;
	int sector_size;
	unsigned char mfm_buffer[32];
	unsigned char tmp_buffer[32];
	unsigned char * tmp_sector;
	unsigned char CRC16_High;
	unsigned char CRC16_Low;
	int sector_extractor_sm;
	int number_of_sector;
	int k;
	unsigned char crctable[32];
	bit_offset=0;
	number_of_sector=0;
	
	sector_extractor_sm=LOOKFOR_GAP1;

	do
	{
		switch(sector_extractor_sm)
		{
			case LOOKFOR_GAP1:
				
				mfm_buffer[0]=0x44;
				mfm_buffer[1]=0x89;
				mfm_buffer[2]=0x44;
				mfm_buffer[3]=0x89;
				mfm_buffer[4]=0x44;
				mfm_buffer[5]=0x89;

				bit_offset=bitslookingfor(track->databuffer,track->tracklen,mfm_buffer,6*8,bit_offset);
						
				if(bit_offset!=-1)
				{		
					sector_extractor_sm=LOOKFOR_ADDM;
				}
				else
				{
					sector_extractor_sm=ENDOFTRACK;
				}
			break;

			case LOOKFOR_ADDM:
				mfmtobin(track->databuffer,track->tracklen,tmp_buffer,3+7,bit_offset,0);
				if(tmp_buffer[3]==0xFE)
				{
					CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
					for(k=0;k<3+7;k++)  
					{
						CRC16_Update(&CRC16_High,&CRC16_Low, tmp_buffer[k],(unsigned char*)crctable );
					}
				
					if(!CRC16_High && !CRC16_Low)
					{ // crc ok !!! 
						number_of_sector++;
						floppycontext->hxc_printf(MSG_DEBUG,"Valid MFM sector header found - Cyl:%d Side:%d Sect:%d Size:%d",tmp_buffer[4],tmp_buffer[5],tmp_buffer[6],sectorsize[tmp_buffer[7]&0x7]);
						old_bit_offset=bit_offset;
						mfm_buffer[0]=0x44;
						mfm_buffer[1]=0x89;
						mfm_buffer[2]=0x44;
						mfm_buffer[3]=0x89;
						mfm_buffer[4]=0x44;
						mfm_buffer[5]=0x89;
						
						bit_offset++;
						sector_size = sectorsize[tmp_buffer[7]&0x7];
						bit_offset_bak=bit_offset;
						bit_offset=bitslookingfor(track->databuffer,track->tracklen,mfm_buffer,6*8,bit_offset);
						
						sectors->number_of_sector++;
						sectors->sectorlist=(sect_sector **)realloc(sectors->sectorlist,sizeof(sect_sector *)*sectors->number_of_sector);
						sectors->sectorlist[sectors->number_of_sector-1]=(sect_sector*)malloc(sizeof(sect_sector));
						memset(sectors->sectorlist[sectors->number_of_sector-1],0,sizeof(sect_sector));

						sectors->sectorlist[sectors->number_of_sector-1]->track_id=tmp_buffer[4];
						sectors->sectorlist[sectors->number_of_sector-1]->side_id=tmp_buffer[5];
						sectors->sectorlist[sectors->number_of_sector-1]->sector_id=tmp_buffer[6];
						sectors->sectorlist[sectors->number_of_sector-1]->sectorsize=sectorsize[tmp_buffer[7]&0x7];
						//sectors->sectorlist[sectors->number_of_sector-1]->type

						if(bit_offset==-1)
						{
							bit_offset=bit_offset_bak;
						}

						if((bit_offset!=-1) && (bit_offset-old_bit_offset<(88+10)*8))
						{

							tmp_sector=(unsigned char*)malloc(3+1+sector_size+2);
							memset(tmp_sector,0,3+1+sector_size+2);
							mfmtobin(track->databuffer,track->tracklen,tmp_sector,3+1+sector_size+2,bit_offset,0);

							CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
							for(k=0;k<3+1+sector_size+2;k++)  
							{
								CRC16_Update(&CRC16_High,&CRC16_Low, tmp_sector[k],(unsigned char*)crctable );
							}
				
							if(!CRC16_High && !CRC16_Low)
							{ // crc ok !!! 
								floppycontext->hxc_printf(MSG_DEBUG,"crc data ok.");
							}
							else
							{
								floppycontext->hxc_printf(MSG_DEBUG,"crc data error!");
							}

							sectors->sectorlist[sectors->number_of_sector-1]->buffer=(unsigned char*)malloc(sector_size);
							memcpy(sectors->sectorlist[sectors->number_of_sector-1]->buffer,&tmp_sector[4],sector_size);
							free(tmp_sector);

							bit_offset=bit_offset+(sector_size*2);
						}
											
					}
					else
					{
						bit_offset++;
						sector_extractor_sm=LOOKFOR_GAP1;
					}
				}
				else
				{
					bit_offset++;
					sector_extractor_sm=LOOKFOR_GAP1;
				}
						
					
				sector_extractor_sm=LOOKFOR_GAP1;
			break;

			case ENDOFTRACK:
			
			break;

			default:
				sector_extractor_sm=ENDOFTRACK;
			break;

		}
	}while(	sector_extractor_sm!=ENDOFTRACK);

	return number_of_sector;
}

void sortbuffer(unsigned char * buffer,unsigned char * outbuffer,int size)
{
	int i;
	unsigned short * word_outbuffer,w;

	word_outbuffer=(unsigned short *)outbuffer;
	for(i=0;i<(size/2);i++)
	{
		w=(biteven[buffer[i]]<<1)| (biteven[buffer[i+(size/2)]]);
		word_outbuffer[i]=(w>>8) | (w<<8);
	}

}

int analysis_and_extract_sector_AMIGAMFM(HXCFLOPPYEMULATOR* floppycontext,SIDE * track,sect_track * sectors)
{
	int bit_offset,old_bit_offset;
	int sector_size;
	unsigned char mfm_buffer[32];
	unsigned char tmp_buffer[32];
	unsigned char sector[544];
	unsigned char temp_sector[512];
	unsigned char * tmp_sector;
	int sector_extractor_sm;
	int number_of_sector;

	bit_offset=0;
	number_of_sector=0;
	
	sector_extractor_sm=LOOKFOR_GAP1;

	do
	{
		switch(sector_extractor_sm)
		{
			case LOOKFOR_GAP1:

				mfm_buffer[0]=0xAA;
				mfm_buffer[1]=0xAA;
				mfm_buffer[2]=0x44;
				mfm_buffer[3]=0x89;
				mfm_buffer[4]=0x44;
				mfm_buffer[5]=0x89;

				bit_offset=bitslookingfor(track->databuffer,track->tracklen,mfm_buffer,6*8,bit_offset);
						
				if(bit_offset!=-1)
				{		
					sector_extractor_sm=LOOKFOR_ADDM;
				}
				else
				{
					sector_extractor_sm=ENDOFTRACK;
				}
			break;

			case LOOKFOR_ADDM:
				bit_offset=bit_offset-(8*2);
				mfmtobin(track->databuffer,track->tracklen,sector,544,bit_offset,0);
				sortbuffer(&sector[4],tmp_buffer,4);
			    memcpy(&sector[4],tmp_buffer,4);

				if(tmp_buffer[0]==0xFF)
				{

					sortbuffer(&sector[24],tmp_buffer,4);
					memcpy(&sector[24],tmp_buffer,4);
					
					sortbuffer(&sector[32],temp_sector,512);		
					memcpy(&sector[32],temp_sector,512);
					sector_size=512;
					number_of_sector++;
					floppycontext->hxc_printf(MSG_DEBUG,"Valid Amiga MFM sector header found - Cyl:%d Side:%d Sect:%d Size:%d",sector[5]>>1,sector[5]&1,sector[6],sector_size);
					old_bit_offset=bit_offset;
						
											
					sectors->number_of_sector++;
					sectors->sectorlist=(sect_sector **)realloc(sectors->sectorlist,sizeof(sect_sector *)*sectors->number_of_sector);
					sectors->sectorlist[sectors->number_of_sector-1]=(sect_sector*)malloc(sizeof(sect_sector));
					memset(sectors->sectorlist[sectors->number_of_sector-1],0,sizeof(sect_sector));

					sectors->sectorlist[sectors->number_of_sector-1]->track_id=sector[5]>>1;
					sectors->sectorlist[sectors->number_of_sector-1]->side_id=sector[5]&1;
					sectors->sectorlist[sectors->number_of_sector-1]->sector_id=sector[6];
					sectors->sectorlist[sectors->number_of_sector-1]->sectorsize=sector_size;
					//sectors->sectorlist[sectors->number_of_sector-1]->type


					tmp_sector=(unsigned char*)malloc(sector_size);
					memset(tmp_sector,0,sector_size);
				
					if(1)
					{ // crc ok !!! 
					//	floppycontext->hxc_printf(MSG_DEBUG,"crc data ok.");
					}

					sectors->sectorlist[sectors->number_of_sector-1]->buffer=(unsigned char*)malloc(sector_size);
					memcpy(sectors->sectorlist[sectors->number_of_sector-1]->buffer,&sector[32],sector_size);
					free(tmp_sector);

					bit_offset=bit_offset+(sector_size*2);
											
				}
				else
				{
					bit_offset=bit_offset+(8*2)+1;
					sector_extractor_sm=LOOKFOR_GAP1;
				}
	
			
				sector_extractor_sm=LOOKFOR_GAP1;
			break;

			case ENDOFTRACK:
			
			break;

			default:
				sector_extractor_sm=ENDOFTRACK;
			break;

		}
	}while(	sector_extractor_sm!=ENDOFTRACK);

	return number_of_sector;
}


int analysis_and_extract_sector_FM(HXCFLOPPYEMULATOR* floppycontext,SIDE * track,sect_track * sectors)
{
	int bit_offset,old_bit_offset;
	int sector_size;
	unsigned char fm_buffer[32];
	unsigned char tmp_buffer[32];
	unsigned char * tmp_sector;
	unsigned char CRC16_High;
	unsigned char CRC16_Low;
	int sector_extractor_sm;
	int number_of_sector;
	int k;
	unsigned char crctable[32];
	bit_offset=0;
	number_of_sector=0;
	
	sector_extractor_sm=LOOKFOR_GAP1;

	do
	{
		switch(sector_extractor_sm)
		{
			case LOOKFOR_GAP1:
				fm_buffer[0]=0x55;
				fm_buffer[1]=0x11;
				fm_buffer[2]=0x15;
				fm_buffer[3]=0x54;
				
				bit_offset=bitslookingfor(track->databuffer,track->tracklen,fm_buffer,4*8,bit_offset);
						
				if(bit_offset!=-1)
				{		
					sector_extractor_sm=LOOKFOR_ADDM;
				}
				else
				{
					sector_extractor_sm=ENDOFTRACK;
				}
			break;

			case LOOKFOR_ADDM:
				fmtobin(track->databuffer,track->tracklen,tmp_buffer,7,bit_offset,0);
				if(tmp_buffer[0]==0xFE)
				{
					CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
					for(k=0;k<7;k++)  
					{
						CRC16_Update(&CRC16_High,&CRC16_Low, tmp_buffer[k],(unsigned char*)crctable );
					}
				
					if(!CRC16_High && !CRC16_Low)
					{ // crc ok !!! 
						bit_offset=bit_offset+7*8;
						number_of_sector++;
						floppycontext->hxc_printf(MSG_DEBUG,"Valid FM sector header found - Cyl:%d Side:%d Sect:%d Size:%d",tmp_buffer[1],tmp_buffer[2],tmp_buffer[3],sectorsize[tmp_buffer[4]&0x7]);
						old_bit_offset=bit_offset;


						sector_size = sectorsize[tmp_buffer[4]&0x7];
						//11111011
						fm_buffer[0]=0x55;
						fm_buffer[1]=0x11;
						fm_buffer[2]=0x14;
						fm_buffer[3]=0x55;
						if((unsigned int)(bit_offset+100*8)<track->tracklen)
							bit_offset=bitslookingfor(track->databuffer,bit_offset+100*8 ,fm_buffer,4*8,bit_offset);
						else
							bit_offset=bitslookingfor(track->databuffer,track->tracklen,fm_buffer,4*8,bit_offset);

						if((bit_offset-old_bit_offset>((88+10)*8*2)) || bit_offset==-1)
						{
							bit_offset=old_bit_offset;
							fm_buffer[0]=0x55; //11111000
							fm_buffer[1]=0x11;
							fm_buffer[2]=0x14;
							fm_buffer[3]=0x44;
							bit_offset=bitslookingfor(track->databuffer,track->tracklen,fm_buffer,4*8,bit_offset);
						}

						if((bit_offset-old_bit_offset<((88+10)*8*2)) && bit_offset!=-1)
						{
							sectors->number_of_sector++;
							sectors->sectorlist=(sect_sector **)realloc(sectors->sectorlist,sizeof(sect_sector *)*sectors->number_of_sector);
							sectors->sectorlist[sectors->number_of_sector-1]=(sect_sector*)malloc(sizeof(sect_sector));
							memset(sectors->sectorlist[sectors->number_of_sector-1],0,sizeof(sect_sector));

							sectors->sectorlist[sectors->number_of_sector-1]->track_id=tmp_buffer[1];
							sectors->sectorlist[sectors->number_of_sector-1]->side_id=tmp_buffer[2];
							sectors->sectorlist[sectors->number_of_sector-1]->sector_id=tmp_buffer[3];
							sectors->sectorlist[sectors->number_of_sector-1]->sectorsize=sectorsize[tmp_buffer[4]&0x7];
							//sectors->sectorlist[sectors->number_of_sector-1]->type


								tmp_sector=(unsigned char*)malloc(1+sector_size+2);
								memset(tmp_sector,0,1+sector_size+2);
								fmtobin(track->databuffer,track->tracklen,tmp_sector,1+sector_size+2,bit_offset+(0*8),0);

								CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x1021,0xFFFF);
								for(k=0;k<1+sector_size+2;k++)  
								{
									CRC16_Update(&CRC16_High,&CRC16_Low, tmp_sector[k],(unsigned char*)crctable );
								}
					
								if(!CRC16_High && !CRC16_Low)
								{ // crc ok !!! 
									floppycontext->hxc_printf(MSG_DEBUG,"crc data ok.");
								}
								else
								{
									floppycontext->hxc_printf(MSG_DEBUG,"crc data error!");
								}

								sectors->sectorlist[sectors->number_of_sector-1]->buffer=(unsigned char*)malloc(sector_size);
								memcpy(sectors->sectorlist[sectors->number_of_sector-1]->buffer,&tmp_sector[1],sector_size);
								free(tmp_sector);

								bit_offset=bit_offset+(((sector_size+2)*4)*8);
						
						}
						else
						{
							bit_offset=old_bit_offset+1;
							floppycontext->hxc_printf(MSG_DEBUG,"No data!");

						}
					}
					else
					{
						sector_extractor_sm=LOOKFOR_GAP1;
						bit_offset++;
					}
				}
				else
				{
					sector_extractor_sm=LOOKFOR_GAP1;
					bit_offset++;
				}
						
				sector_extractor_sm=LOOKFOR_GAP1;
			break;

			case ENDOFTRACK:
			
			break;

			default:
				sector_extractor_sm=ENDOFTRACK;
			break;

		}
	}while(	sector_extractor_sm!=ENDOFTRACK);

	return number_of_sector;
}


int analysis_and_extract_sector_EMUIIFM(HXCFLOPPYEMULATOR* floppycontext,SIDE * track,sect_track * sectors)
{
	int bit_offset,old_bit_offset;
	int sector_size;
	unsigned char fm_buffer[32];
	unsigned char tmp_buffer[32];
	unsigned char * tmp_sector;
	unsigned char CRC16_High;
	unsigned char CRC16_Low;
	int sector_extractor_sm;
	int number_of_sector;
	int k;
	unsigned char crctable[32];
	bit_offset=0;
	number_of_sector=0;
	
	sector_extractor_sm=LOOKFOR_GAP1;

	do
	{
		switch(sector_extractor_sm)
		{
			case LOOKFOR_GAP1:
/*				fm_buffer[0]=0x55;
				fm_buffer[1]=0x55;
				fm_buffer[2]=0x54;
				fm_buffer[3]=0x54;
				fm_buffer[4]=0x54;
				fm_buffer[5]=0x45;
				fm_buffer[6]=0x45;
				fm_buffer[7]=0x54;*/

				fm_buffer[0]=0x45;
				fm_buffer[1]=0x45;
				fm_buffer[2]=0x55;
				fm_buffer[3]=0x55;
				fm_buffer[4]=0x45;
				fm_buffer[5]=0x54;
				fm_buffer[6]=0x54;
				fm_buffer[7]=0x45;

				
				bit_offset=bitslookingfor(track->databuffer,track->tracklen,fm_buffer,8*8,bit_offset);
						
				if(bit_offset!=-1)
				{		
					sector_extractor_sm=LOOKFOR_ADDM;
				}
				else
				{
					sector_extractor_sm=ENDOFTRACK;
				}
			break;

			case LOOKFOR_ADDM:
				fmtobin(track->databuffer,track->tracklen,tmp_buffer,5,bit_offset,0);
				if((bit_inverter_emuii[tmp_buffer[0]]==0xFA) && (bit_inverter_emuii[tmp_buffer[1]]==0x96))
				{
					CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x8005,0x0000);
					for(k=0;k<3;k++)
					CRC16_Update(&CRC16_High,&CRC16_Low, tmp_buffer[2+k],(unsigned char*)crctable );

				
					if(!CRC16_High && !CRC16_Low)
					{ // crc ok !!! 
						number_of_sector++;
						floppycontext->hxc_printf(MSG_DEBUG,"Valid EmuII FM sector header found - Sect:%d",bit_inverter_emuii[tmp_buffer[2]]);
						old_bit_offset=bit_offset;

						sector_size = 0xE00;

						//11111011
						fm_buffer[0]=0x45;
						fm_buffer[1]=0x45;
						fm_buffer[2]=0x55;
						fm_buffer[3]=0x55;
						fm_buffer[4]=0x45;
						fm_buffer[5]=0x54;
						fm_buffer[6]=0x54;
						fm_buffer[7]=0x45;
						
						bit_offset=bitslookingfor(track->databuffer,track->tracklen,fm_buffer,8*8,bit_offset+(4*8*4));

						if((bit_offset-old_bit_offset<((88+10)*8*2)) && bit_offset!=-1)
						{
							sectors->number_of_sector++;
							sectors->sectorlist=(sect_sector **)realloc(sectors->sectorlist,sizeof(sect_sector *)*sectors->number_of_sector);
							sectors->sectorlist[sectors->number_of_sector-1]=(sect_sector*)malloc(sizeof(sect_sector));
							memset(sectors->sectorlist[sectors->number_of_sector-1],0,sizeof(sect_sector));

							sectors->sectorlist[sectors->number_of_sector-1]->track_id=bit_inverter_emuii[tmp_buffer[2]]>>1;
							sectors->sectorlist[sectors->number_of_sector-1]->side_id=bit_inverter_emuii[tmp_buffer[2]]&1;
							sectors->sectorlist[sectors->number_of_sector-1]->sector_id=1;
							sectors->sectorlist[sectors->number_of_sector-1]->sectorsize=0xE00;

							tmp_sector=(unsigned char*)malloc(sector_size+2);
							memset(tmp_sector,0,sector_size+2);
							
							fmtobin(track->databuffer,track->tracklen,tmp_sector,sector_size+2,bit_offset+(8 *8),0);

							CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)crctable,0x8005,0x0000);
							for(k=0;k<sector_size+2;k++)
							{
								CRC16_Update(&CRC16_High,&CRC16_Low, tmp_sector[k],(unsigned char*)crctable );
							}
					
							if(!CRC16_High && !CRC16_Low)
							{ // crc ok !!! 
								floppycontext->hxc_printf(MSG_DEBUG,"crc data ok.");
							}
							else
							{
								floppycontext->hxc_printf(MSG_DEBUG,"crc data error!");
							}

							for(k=0;k<sector_size;k++)  
							{
								tmp_sector[k]=bit_inverter_emuii[tmp_sector[k]];
							}

							sectors->sectorlist[sectors->number_of_sector-1]->buffer=(unsigned char*)malloc(sector_size);
							memcpy(sectors->sectorlist[sectors->number_of_sector-1]->buffer,tmp_sector,sector_size);
							free(tmp_sector);

							bit_offset=bit_offset+(sector_size*4);
						
						}
						else
						{
							bit_offset=old_bit_offset+1;
							floppycontext->hxc_printf(MSG_DEBUG,"No data!");

						}
					}
					else
					{
						sector_extractor_sm=LOOKFOR_GAP1;
						bit_offset++;
					}
				}
				else
				{
					sector_extractor_sm=LOOKFOR_GAP1;
					bit_offset++;
				}
						
				sector_extractor_sm=LOOKFOR_GAP1;
			break;

			case ENDOFTRACK:
			
			break;

			default:
				sector_extractor_sm=ENDOFTRACK;
			break;

		}
	}while(	sector_extractor_sm!=ENDOFTRACK);

	return number_of_sector;
}
