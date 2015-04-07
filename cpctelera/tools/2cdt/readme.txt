2CDT
====

(c) Kevin Thacker

(Original code written in May 2000, fixed and released in May 2001)

2CDT is a utility to transfer files into a ".CDT" Tape-Image.

A ".CDT" is a tape-image file which describes the data stored on a cassette tape.
 
This file format is very powerful and can describe fast and custom loaders as well
as standard operating system formats.

The ".CDT" file format is identical to the ".TZX" format. The extension is used
to differentiate between Spectrum and Amstrad Tape-Images.

The ".TZX" file format was originally designed to store Spectrum tape programs,
it's format can be found from various sources, one of these is "World Of Spectrum":
http://www.void.jump.org/

There are a number of tools which already create .TZX files, Taper, Voc2TZX and MakeTZX.
However, these are designed to recognise Spectrum tape loaders, and so do not do well
at creating a tape-image for Amstrad formats.

This tool has been designed as a starting point for furthur Amstrad CDT tools,
and as a program to generate reference tape-images which can be used by emulator
authors to support this tape-image format in their programs.

This tool is designed to "inject" one or more file into a ".CDT" in the format written 
by the Amstrad operating system. The tool allows the user to define the ".CDT" "recording" 
method and baud rate.

2CDT	[parameters]	<source file> <output CDT filename>

parameters:
	-n		
			- Blank CDT file before use

			Use this to create a new CDT, otherwise file
			will be added to the end of an existing CDT.

	-s <speed write>
			- Write data with 'speed write' 0 or 1.

			This is the same as typing "SPEED WRITE 0" or "SPEED WRITE 1"
			in BASIC. Speed Write '0' is 1000 baud. Speed write '1' is
			2000 baud.

	-b <baud rate>  
			- Specify Baud Rate.
			
			Allows you to specify faster or slower loading.
			
	-t <method>	
			- Method to write data into TZX (for Amstrad blocks)
			0 = Pure Data Block
			1 = Turbo Loading Data Block (default)

	-m <method>
			- Method to write CPC data 
			0 = blocks (default)
			1 = headerless (Firmware function CAS READ for reading, CAS WRITE for writing )
				To be loaded with firmware function CAS READ.
				Not readable from BASIC. Allows entire program
				to be stored as a single continuous block.
			2 = Spectrum ROM loader

	-r <tape filename>
			- Give the tape file a name. Up to 16 characters.

			 This allows you to give the file on tape a different
			name to the name given on your local filesystem.

			If no name is defined, the file will be unnamed.
			This option applies to CPC 'blocks' methods only.

	-X <address>
			Define execution address (where file doesn't have header), or 
			override execution address (where file has a header)
			
	-L <address>
			Define load address (where file doesn't have header), or 
			override load address (where file has a header)

	-T <address>
			Define type (where file doesn't have header), or 
			override type address (where file has a header)
			0 = BASIC, 2 = BINARY
			
	-p <number>
			Set initial pause (default 3000ms)
	
	-P
			Add a small initial pause block for buggy emulators (e.g. old versions of Arnold)
			Not recommended. Please use newer version of this emulator.
								
Examples:

The following in a bacth file will create the master tape for 'Stranded':

REM create new CDT and put binary loader
2cdt -n -r stranded strandlod.bin stranded.cdt
REM add screen to existing CDT data
2cdt -r screen loading.bin stranded.cdt
REM add code to existing CDT data
2cdt -r code stranded.bin stranded.cdt

If you wanted to create a master tape for a game called 'Columns':

REM create new CDT and put binary loader
2cdt -n -r loader colload.bin columns.cdt
REM put binary file as headerless
2cdt -m 1 colcode.bin columns.cdt



This archive contains a Windows command-line executable that will run under Win95, Win98,
Win2000 and WinNT.

License:

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.