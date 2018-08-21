/*********************************

Record a raw or AMSDOS binary file
into a TZX/CDT file using a signal
that encodes single or double bits
into either half or full pulses.

*********************************/

#pragma once

// PUBLIC FUNCTION DECLARATIONS
void tiny_tape_usage();
void tiny_tape_setBitGaps(int bg);
void tiny_tape_setSkipHeader(int sk);
void tiny_tape_gen(	const char* srcfile, const char* tzxfile, int _bittype
					, int _bitsize, int _bitbyte, int _bithold);
