/*
   bug-3495411.c
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#pragma disable_warning 88
#endif

// Type defines
#define UCHAR	unsigned char
#define USHORT	unsigned short
#define ULONG	unsigned long
//#define BOOLEAN	unsigned char
#define BOOLEAN	_Bool

#define PCHAR	unsigned char *
#define PSHORT	unsigned short *
#define PLONG	unsigned long *

#ifndef NULL
#define NULL	(void *)0
#endif

#define TRUE	1
#define FALSE	0

#define IP_ALEN					4
#define RTP_SSRC_LEN	4
#define MAX_USER_NAME_LEN		32
#define MAX_USER_NUMBER_LEN		32
#define MAX_TAG_LEN			16

typedef struct _CALL_TIME
{
	UCHAR iSec;
	UCHAR iMin;
	UCHAR iHour;
} CALL_TIME;

struct ucb
{
	UCHAR iState;
	USHORT sSrcPort;
	UCHAR iBankOffset;				/* Bank offset of call back function	*/
	USHORT sCallBack;				/* Address of call back function		*/
	USHORT sDstPort;
	UCHAR pDstIP[IP_ALEN];
	USHORT sLen;
};

typedef struct ucb * UDP_SOCKET;

typedef struct sip_lcb * SIP_LCB_HANDLE;
typedef struct sip_tcb * SIP_TCB_HANDLE;

struct sip_tcb
{
	UCHAR iState;
	UCHAR iMethod;
	UCHAR pDstIP[IP_ALEN];
	USHORT sDstPort;
	UCHAR iTimer0;
	UCHAR iTimer1;
	UCHAR iTimer2;
	BOOLEAN b100rel;
	PCHAR pBranch;
	PCHAR pData;
	SIP_LCB_HANDLE pLcb;
};

struct sip_lcb
{
	UCHAR iState;			// State of this line
	BOOLEAN bCallee;		// Caller or callee
	UCHAR iTimer;			// No anser timer
	UCHAR iRemoteRb;		// ringback tone type
	BOOLEAN bHolding;
	BOOLEAN bEarlyDlg;		// already received a 1xx response
 
	UCHAR iVoiceCoder;		// voice coder used by this line
	BOOLEAN bVad;
	UCHAR iRemoteMode;		// remote voice transmission mode
	UCHAR pRtpDstIP[IP_ALEN];	// RTP dst IP
	USHORT sRtpDstPort;		// RTP dst port
	UCHAR iCoderPayload;	// Peer's coder payload
	UCHAR iDtmfPayload;		// Peer's DTMF payload

	UCHAR iLocalMode;		// local voice transmission mode
	UCHAR iSdpType;
	UCHAR pSdpSessionId[9];
	ULONG lSdpVersion;
	UDP_SOCKET pRtpSocket;
	USHORT sRtpPort;		// local RTP port
	USHORT sRtpSeq;			// RTP sequence
	UCHAR pRtpSSRC[RTP_SSRC_LEN];
	UCHAR pRtpDstSSRC[RTP_SSRC_LEN];
	ULONG lRtpTimeStamp;	// RTP timestamp
	ULONG lRtpOffset;
	BOOLEAN bVoiceStart;	
	BOOLEAN bSendingKey;	// RFC2833 send key 
	UCHAR iSendKey;			// Key being sent
	USHORT sSendKeyLen;		// Duration in timestamp
	BOOLEAN bKeyRecved;
	UCHAR pKeyTimeStamp[4];

	UCHAR pDstIP[IP_ALEN];	// Dst IP of this call
	USHORT sDstPort;		// Dst port of this call
	ULONG lRseq;			// RSEQ for PRACK
	ULONG lCurSeq;			// Current sequence of this call
	ULONG lInvSeq;			// Sequence of INVITE request
	PCHAR pInvUri;			// Original request Uri of this call
	PCHAR pReqUri;			// Request URI of this call
	PCHAR pBranch;			// Invite branch
	PCHAR pCallId;			// Call-ID header of this call
	PCHAR pFrom;			// From header of this call
	PCHAR pTo;				// To header
	PCHAR pFromTag;			// Local tag	
	PCHAR pToTag;			// Remote tag
	PCHAR pProxyAuth;		// Proxy-Authenticate information
	PCHAR pWWWAuth;			// WWW-Authenticate information
	PCHAR pRoute;			// Route header to be included in request
	PCHAR pInvHeader;		// Invite header to be included in response
	SIP_TCB_HANDLE pInvTcb;	// Transaction control block of INVITE request

	UCHAR pRespDstIP[IP_ALEN];
	USHORT sRespDstPort;
	PCHAR pReferredBy;
	PCHAR pReplaces;
	UCHAR iXferPart;		// which role do we play? transferor, transferee or transfer-to?

	UCHAR iRefresher;
	ULONG lSessionExpires;
	ULONG lSessionTimer;
	ULONG lMinSE;

	UCHAR pPeerName[MAX_USER_NAME_LEN];
	UCHAR pPeerNumber[MAX_USER_NUMBER_LEN];
	CALL_TIME ct;
	UCHAR pKeyOut[MAX_USER_NUMBER_LEN];
	UCHAR iKeyIndex;
	BOOLEAN bMemCall;
};

__xdata struct sip_lcb l;
extern SIP_LCB_HANDLE Sip_pCurLcb = &l;

void line_start(PCHAR pDst) {}
void sip_add_local_uri(BOOLEAN bIP, BOOLEAN bPort) {}
void sip_new_token(PCHAR pDst, UCHAR iLen) {}
void sip_add_token(PCHAR pToken, PCHAR pValue) {}
void dummy_free(void *p) {}
PCHAR heap_save_str(PCHAR pStr) { return ((PCHAR) 42);}
BOOLEAN Sys_bRegister;
const UCHAR _cTokenTag[] = "";

void sip_new_from()
{
#ifdef __SDCC_mcs51
	UCHAR pBuf[64];
#else
	UCHAR pBuf[128];
#endif
	UCHAR pTag[MAX_TAG_LEN+1];

	line_start(pBuf);
	sip_add_local_uri(!Sys_bRegister, FALSE);
	sip_new_token(pTag, MAX_TAG_LEN);
	sip_add_token(_cTokenTag, pTag);
	dummy_free(Sip_pCurLcb->pFrom);
	Sip_pCurLcb->pFrom = heap_save_str(pBuf);
	dummy_free(Sip_pCurLcb->pFromTag);
	Sip_pCurLcb->pFromTag = heap_save_str(pTag);
}

void
testBug (void)
{
	l.pFrom = (PCHAR) 23;
	sip_new_from();
	ASSERT (l.pFrom == (PCHAR) 42);
}

