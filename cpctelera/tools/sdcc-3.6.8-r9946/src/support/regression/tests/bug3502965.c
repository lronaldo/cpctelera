/*
   bug3502965.c
*/

#include <testfwk.h>

#ifndef __SDCC_mcs51
#pragma disable_warning 85

#define UCHAR	unsigned char
#define USHORT	unsigned short
#define PCHAR	unsigned char *
#define HW_ALEN					6
#define IP_ALEN					4

typedef struct _PACKET_LIST PACKET_LIST;
struct _PACKET_LIST
{
	PACKET_LIST *	next;
	USHORT			sLen;
	USHORT			sAddr;
	UCHAR			pRecvIP[IP_ALEN];
};

void memcpy4(PCHAR dst) {}

#define EP_TYPE (HW_ALEN + HW_ALEN)
#define EP_DATA (EP_TYPE + 2)    
#define IP_DATA     20
#define ETHERNET_MAX_SIZE				1514
#define IP_DATA_MAX_SIZE			(ETHERNET_MAX_SIZE - EP_DATA - IP_DATA)
UCHAR Adapter_pPacketBuf[ETHERNET_MAX_SIZE + 1 + IP_DATA_MAX_SIZE + 1];
#define Adapter_pReceivePacket	(PCHAR)(Adapter_pPacketBuf + ETHERNET_MAX_SIZE + 1)

#define TCP_OFFSET	12	
PCHAR _pReceive;
UCHAR _pReceiveIP[IP_ALEN];
USHORT _sReceiveDataLen;

void TcpRun(PACKET_LIST * p)
{
	UCHAR iHeadLen;
	USHORT sLen;
	PCHAR pRecvIP;

	sLen = p->sLen;

	_pReceive = Adapter_pReceivePacket;
	
	memcpy4(_pReceiveIP);

	// check if packet length is valid
	iHeadLen = (_pReceive[TCP_OFFSET] & 0xf0) >> 2;
	if (sLen < iHeadLen)
	{
		return;
	}
	_sReceiveDataLen = sLen - iHeadLen;
}
#endif

void testBug(void)
{
#ifndef __SDCC_mcs51
  PACKET_LIST pl;
  pl.sLen = 0xff;
  (Adapter_pReceivePacket)[TCP_OFFSET] = 0x00;
  _sReceiveDataLen = 0xaa55;
  TcpRun(&pl);
  ASSERT(_sReceiveDataLen == 0xff);
#endif
}

