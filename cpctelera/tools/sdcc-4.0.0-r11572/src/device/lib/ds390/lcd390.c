/*-------------------------------------------------------------------------
   lcd.c - lcd routines for the DS80C390 (tested on TINI)

   Copyright (C) 2001, Johan Knol <johan.knol AT iduna.nl>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License 
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

#include <tinibios.h>
#include <stdio.h>
#include <stdarg.h>

#define LCD_COLLUMNS 20
#define LCD_ROWS 4

/* set the dd ram addresses for the rows
   this one is for a 20x4 LCD
*/
static unsigned char lcdLinesStart[LCD_ROWS]={0, 0x40, 0x14, 0x54};

/* Commercial TINI stick connector boards don't support rw
   control for the lcd-display.
   My own board does, and it makes it much faster.
*/

//#define LCD_RW

__xdata __at (0x380002) static unsigned char lcdIwr;
__xdata __at (0x38000a) static unsigned char lcdDwr;

#ifdef LCD_RW

__xdata __at (0x380003) static unsigned char lcdIrd;
__xdata __at (0x38000b) static unsigned char lcdDrd;

#define LcdWait { while (lcdIrd&0x80) ; }

#else // ifdef LCD_RW

// wait for 100us
#define LcdWait { ClockMicroSecondsDelay(100) ; }

#endif // ifdef LCD_RW

void LcdInit() {
  
  ClockMilliSecondsDelay(16); // >15 ms
  
  lcdIwr=0x38 ;
  ClockMilliSecondsDelay(5); // >4.1 ms
  
  lcdIwr=0x38;
  ClockMicroSecondsDelay(101); // >100 us
  
  lcdIwr=0x38;
  ClockMicroSecondsDelay(101); // >100 us
  
  lcdIwr=0x38; // interface 8 bit
  ClockMicroSecondsDelay(41); // >40 us
  
  lcdIwr=0x0c; // display on
  ClockMicroSecondsDelay(41); // >40 us

  LcdClear();
}

void LcdOn() {
  lcdIwr=0x0c; // display on
  LcdWait;
}

void LcdOff() {
  lcdIwr=0x08; // display off
  LcdWait;
}

void LcdCursorOn() {
  // TODO
}

void LcdCursorOff() {
  // TODO
}

void LcdScrollOn() {
  // TODO
}

void LcdScrollOff() {
  // TODO
}

void LcdCharDefine() {
  // TODO
}

void LcdClear() {
  lcdIwr=0x01; // display clear
  ClockMilliSecondsDelay(6); // > 5ms
}

void LcdHome() {
  lcdIwr=0x80; // set dd ram address 0
  LcdWait;
}

void LcdGoto(unsigned int collumnRow) { // msb=collumn, lsb=row
  lcdIwr=0x80 + \
    lcdLinesStart[collumnRow&0xff] + (collumnRow>>8);
  LcdWait;
}

void LcdPutChar(char c) {
  lcdDwr=c;
  LcdWait;
}

void LcdPutString (char *string) {
  char c;
  while (c=*string++) {
    LcdPutChar (c);
  }
}

void LcdLPutString (unsigned int collumnRow, char *string) {
  LcdGoto(collumnRow);
  LcdPutString(string);
}

// let's hope that no one ever printf's more than the display width,
// however they will :), so to be sure
static char lcdPrintfBuffer[LCD_COLLUMNS*4];

void LcdPrintf (const char *format, ...) __reentrant {
  va_list arg;

  va_start (arg, format);
  vsprintf (lcdPrintfBuffer, format, arg);
  puts (lcdPrintfBuffer);
  LcdPutString(lcdPrintfBuffer);

  va_end (arg);
}

void LcdLPrintf (unsigned int collumnRow, const char *format, ...) __reentrant {
  va_list arg;

  LcdGoto(collumnRow);

  // we can not just call LcdPrintf since we have no idea what is on the stack,
  // so we have to do it all over again
  va_start (arg, format);
  vsprintf (lcdPrintfBuffer, format, arg);

  LcdPutString(lcdPrintfBuffer);

  va_end (arg);
}
