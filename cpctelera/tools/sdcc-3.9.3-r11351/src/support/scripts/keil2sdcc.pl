#!/usr/bin/perl -w

# keil2sdcc.pl
# Scott Bronson
# 22 June 2003


# usage (UNIX):
#    perl keil2sdcc.pl < keil_header.h > sdcc_header.h
# or
#    perl keil2sdcc.pl keil_header.h > sdcc_header.h
#
# usage (Windows):
#    perl keil2sdcc.pl keil_header.h > sdcc_header.h
#
#
# keil_header.h and sdcc_header.h must not be the same file since
# most shells overwrite the output file before opening the input file.


# This script converts Keil-style header files to SDCC.  It tries to
# be pedantic so don't be surprised if you need to munge it a bit to
# get it to work.  On the other hand, it doesn't fully parse the C
# file (for obvious reasons).

# It takes the Keil header file either as an argument or on
# stdin and it produces the output on stdout.

# This script is inspired by keil2sdcc.pl by Bela Torok but a lot
# more pedantic.

use strict;

while(<>) 
{
	s/\r//g;  	# remove DOS line endings if necessary

	# external register (kind of a weird format)
	#
	# in:  EXTERN xdata volatile BYTE GPIF_WAVE_DATA _AT_ 0xE400;
	# out: EXTERN xdata at 0xE400 volatile BYTE GPIF_WAVE_DATA;
	# $1: leading whitespace
	# $2: variable name
	# $3: variable location
	# $4: trailing comments, etc.

	if(/^(\s*)EXTERN\s*xdata\s*volatile\s*BYTE\s*(\w+(?:\s*\[\s*\d+\s*\])?)\s+_AT_\s*([^;]+);(.*)$/) {
		print "$1EXTERN xdata at $3 volatile BYTE $2;$4\n";
		next;
	}

	# sfr statement
	#
	# in:  sfr IOA = 0x80;
	# out: sfr at 0x80 IOA;
	# $1: leading whitespace
	# $2: variable name
	# $3: variable location
	# $4: trailing comments, etc.

	if(/^(\s*)sfr\s*(\w+)\s*=\s*([^;]+);(.*)$/) {
		print "$1sfr at $3 $2;$4\n";
		next;
	}

	# sbit statement
	#
	# in:  sbit SEL = 0x86+0;
	# out: sbit at 0x86+0 SEL;
	# $1: leading whitespace
	# $2: variable name
	# $3: variable location
	# $4: trailing comments, etc.

	if(/^(\s*)sbit\s*(\w+)\s*=\s*([^;]+);(.*)$/) {
		print "$1sbit at $3 $2;$4\n";
		next;
	}



	# entire line is a C++ comment, output it unchanged.
	if(/^(\s*)\/\/(.*)$/) {
		print "$1//$2\n";
		next;
	}

	# C comment, slurp lines until the close comment and output it unchanged.
	if(/^(\s*)\/\*(.*)$/) {
		my($ws,$cmt) = ($1,"$2\n");
		$cmt .= <> while $cmt !~ /\*\/\s*$/;
		$cmt =~ s/\r//g;
		print "$ws/*$cmt";
		next;
	}

	# preprocessor statement (whitespace followed by '#'), don't change
	if(/^(\s*)\#(.*)$/) {
		print "$1#$2\n";
		next;
	}

	# blank line, don't change
	if(/^(\s*)$/) {
		print "\n";
		next;
	}

	chomp;
	die "Unconvertable line: \"$_\"\n";
}

