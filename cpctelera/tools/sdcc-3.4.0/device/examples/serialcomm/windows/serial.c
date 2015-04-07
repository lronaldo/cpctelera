/*-------------------------------------------------------------------------
   Serial library functions for the Windows OS (95-XP)
   Tested with different versions of MS Visual Studio (C ad C++)

   Written by -  Bela Torok / www.torok.info & www.belatorok.com (February 2006)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License (http://www.gnu.org/licenses/gpl.txt).

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

#include <Windows.h>
#include <stdio.h>

#include "serial.h"
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

HANDLE SerialInit(char *ComPortName, DWORD BaudRate, int ByteSize, int StopBit, char ParityChar, char Protocol, int RxTimeOut, int TxTimeOut, int RxBufSize, int TxBufSize) 
{
	HANDLE			hCom;
	BOOL			bPortReady;
	DCB				dcb;
	COMMTIMEOUTS	CommTimeouts;
	int				Parity;

	switch(ParityChar) {
		case 'N':
		case 'n':
			Parity = NOPARITY;
			break;
		case 'E':
		case 'e':
			Parity = EVENPARITY;
			break;
		case 'O':
		case 'o':
			Parity = ODDPARITY;
			break;
		default:
			return NULL;	// illegal parameter !
	}
	
	switch(StopBit)
	{
		case 1:
			StopBit = ONESTOPBIT;
			break;
		case 2:
			StopBit = TWOSTOPBITS;
			break;
		default:
			return NULL;	// illegal parameter !
	}

	hCom = CreateFile(	ComPortName, 
						GENERIC_READ | GENERIC_WRITE,
						0, // exclusive access
						NULL, // no security
						OPEN_EXISTING,
						0, // no overlapped I/O
						NULL); // null template 

	if(hCom == INVALID_HANDLE_VALUE) return NULL;

	bPortReady = SetupComm(hCom, RxBufSize, TxBufSize); // set Rx and Tx buffer sizes

	// Port settings are specified in a Data Communication Block (DCB). 

	bPortReady = GetCommState(hCom, &dcb);

	dcb.BaudRate	= BaudRate;
	dcb.ByteSize	= ByteSize;
	dcb.Parity		= Parity;
	dcb.StopBits	= StopBit;
	dcb.fAbortOnError = TRUE;

	switch(Protocol) {
	case 'D':	// DTR/DSR
	case 'd':
		// set XON/XOFF
		dcb.fOutX	= FALSE;
		dcb.fInX	= FALSE;
		// set RTSCTS
		dcb.fOutxCtsFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_DISABLE; 
		// set DSRDTR
		dcb.fOutxDsrFlow = TRUE;
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE; 
		break;
	case 'R':	// RTS/CTS
	case 'r':
		// set XON/XOFF
		dcb.fOutX	= FALSE;
		dcb.fInX	= FALSE;
		// set RTSCTS
		dcb.fOutxCtsFlow = TRUE;
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; 
		// set DSRDTR
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE; 
		break;
	case 'X':	// XON/XOFF
	case 'x':
		// set XON/XOFF
		dcb.fOutX	= TRUE;
		dcb.fInX	= TRUE;
		dcb.fTXContinueOnXoff = TRUE;
		dcb.XoffChar = ASCII_XOFF;
		dcb.XoffLim = RxBufSize - (RxBufSize / 4);
		dcb.XonChar = ASCII_XON;
		dcb.XonLim = RxBufSize - (RxBufSize / 2);
		// set RTSCTS
		dcb.fOutxCtsFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_DISABLE; 
		// set DSRDTR
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE;
		break;
	case 'N':	// NOPROTOCOL
	case 'n':
	default:
		// set XON/XOFF
		dcb.fOutX	= FALSE;
		dcb.fInX	= FALSE;
		// set RTSCTS
		dcb.fOutxCtsFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_DISABLE; 
		// set DSRDTR
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE; 
		break;
	}

	bPortReady = SetCommState(hCom, &dcb);

	// Set timeouts
	CommTimeouts.ReadIntervalTimeout = RxTimeOut;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = RxTimeOut;

	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant = TxTimeOut;

	bPortReady = SetCommTimeouts(hCom, &CommTimeouts);

	return hCom;
}

void SerialClose(HANDLE hSerial) {
	CloseHandle(hSerial);
}

int SerialGetc(HANDLE hSerial)
{
	unsigned char rxchar;
	BOOL	bReadRC;
	static	DWORD iBytesRead, dwError;

	if(hSerial == NULL) return 0;

	bReadRC = ReadFile(hSerial, &rxchar, 1, &iBytesRead, NULL);

	if(bReadRC == FALSE) { // error

		ClearCommError(hSerial, &dwError, NULL);

//		PurgeComm(hSerial,  PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
//		PurgeComm(hSerial,  PURGE_RXCLEAR | PURGE_TXCLEAR);

		if(dwError == 0) return TIMEOUT;		// no error, iBytesRead is probably == 0
		if(dwError & CE_BREAK) return BREAK;	// break detected
		if(dwError & CE_FRAME) return BREAK;	// framing error

		/*	One, or a combination of the following conditions:
			CE_IOE		-> 0x0400 I/O error during communication with the device
			CE_OVERRUN	-> 0x0002 character-buffer overrun, the next character is lost
			CE_RXOVER	-> 0x0001 input buffer overflow, no room in the input buffer,
			                      or a character was received after the EOF character
			CE_RXPARITY	-> 0x0004 parity error
			CE_TXFULL	-> 0x0100 transmit buffer is full
		*/
		if(dwError & CE_IOE) printf("SerialGetc() I/O error during communication with the device!\n");
		if(dwError & CE_OVERRUN) printf("SerialGetc() Character-buffer overrun, the next character is lost!\n");
		if(dwError & CE_RXOVER) printf("SerialGetc() Input buffer overflow!\n");
		if(dwError & CE_RXPARITY) printf("SerialGetc() Parity error!\n");
		if(dwError & CE_TXFULL) printf("SerialGetc() Transmit buffer is full!\n");
		return SERIAL_ERROR;
	}

	if(iBytesRead == 0) return TIMEOUT; // Timeout occured

	return (int) rxchar;
}


int SerialPutc(HANDLE hCom, char txchar)
{
	BOOL	bWriteRC;
	static	DWORD	iBytesWritten;

	if(hCom == NULL) return -255;
	
	bWriteRC = WriteFile(hCom, &txchar, 1, &iBytesWritten,NULL);

	if(iBytesWritten = 1) return 0;

	return TIMEOUT;
}

char SerialGets(char *rxstring, int MaxNumberOfCharsToRead, HANDLE hCom)
{
	int c;
	int pos = 0;
	unsigned char success = 0; // set to error

	if(hCom == NULL) return success; // Error!

	while(pos <= MaxNumberOfCharsToRead) {
		c = SerialGetc(hCom);

		if(c == TIMEOUT) return success;  // Error
		if(c == '\n') break;
		if(c != '\r') rxstring[pos++] = (char) c;	// discard carriage return
	}
	rxstring[pos] = 0;

	success = 1;

	return success; // No errors
}

void SerialPuts(HANDLE hCom, char *txstring)
{
	BOOL	bWriteRC;
	static	DWORD	iBytesWritten;

	if(hCom == NULL) return;

	bWriteRC = WriteFile(hCom, txstring, (DWORD) strlen(txstring), &iBytesWritten,NULL);
}

int SerialGetModemStatus(HANDLE hCom, int Mask)
{
// The value for Mask must be one of the following definitions:
// MS_CTS_ON
// MS_DSR_ON
// MS_RING_ON
	int ModemStat;

	GetCommModemStatus(hCom, &ModemStat);

	switch( Mask ) {
	case MS_CTS_ON:
	case MS_DSR_ON:
	case MS_RING_ON:
		if((ModemStat & Mask) != 0) {
			return 1;
		} else {
			return 0;
		}
	default:
		return -1;
	}
}

void SerialClearRxBuffer(HANDLE hCom)
{
	PurgeComm(hCom, PURGE_RXCLEAR);
}
