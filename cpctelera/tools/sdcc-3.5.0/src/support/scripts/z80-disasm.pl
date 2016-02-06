#!/usr/bin/perl -w

=back

  Copyright (C) 2013-2016, Molnar Karoly <molnarkaroly@users.sf.net>

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.

================================================================================

    This program disassembles the hex files. It assumes that the hex file
    contains Z80 instructions.

    Proposal for use: ./z80-disasm.pl program.hex > program.dasm

  $Id: z80-disasm.pl 9450 2016-01-09 16:47:43Z molnarkaroly $
=cut

use strict;
use warnings;
no if $] >= 5.018, warnings => "experimental::smartmatch";        # perl 5.16
use 5.12.0;                     # when (regex)

use constant FALSE	=> 0;
use constant TRUE	=> 1;

use constant TAB_LENGTH	=> 8;

################################################################################

use constant INHX8M			=> 0;
use constant INHX32			=> 2;

use constant INHX_DATA_REC		=> 0;
use constant INHX_EOF_REC		=> 1;
use constant INHX_EXT_LIN_ADDR_REC	=> 4;

use constant EMPTY			=> -1;

use constant COUNT_SIZE			=> 2;
use constant ADDR_SIZE			=> 4;
use constant TYPE_SIZE			=> 2;
use constant BYTE_SIZE			=> 2;
use constant CRC_SIZE			=> 2;
use constant HEADER_SIZE		=> (COUNT_SIZE + ADDR_SIZE + TYPE_SIZE);
use constant MIN_LINE_LENGTH		=> (HEADER_SIZE + CRC_SIZE);

use constant Z80_ROM_SIZE		=> 0x10000;

################################################################################

my $PROGRAM = 'z80-disasm.pl';

my $border0 = ('-' x 99);
my $border1 = ('#' x 99);
my $border2 = ('.' x 39);

my @default_paths =
  (
  '/usr/share/sdcc/include/z180',
  '/usr/local/share/sdcc/include/z180'
  );

my $default_include_path = '';
my $include_path	 = '';
my $hex_file		 = '';
my $map_file 		 = '';
my $map_readed		 = FALSE;
my $header_file		 = '';
my $name_list		 = '';

my $verbose		 = 0;
my $gen_assembly_code	 = FALSE;
my $no_explanations	 = FALSE;
my $find_lost_labels	 = FALSE;

my @rom = ();
my $rom_size = Z80_ROM_SIZE;
my %const_areas_by_address  = ();	# From the command line parameters.

my %const_blocks_by_address = ();

my %ram_blocks_by_address   = ();
my %ram_names_by_address    = ();

=back
	The structure of one element of the %io_by_address hash:

	{
	NAME	  => '',
	REF_COUNT => 0
	}
=cut

my %io_by_address	 = ();

	# Sizes of the instructions.

use constant IPREFIX_DD => -1;
use constant IPREFIX_ED => -2;
use constant IPREFIX_FD => -3;

my @instruction_sizes_ =
  (
# 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F

  1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,	# 00
  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,	# 10
  2, 3, 3, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,	# 20
  2, 3, 3, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,	# 30
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# 40
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# 50
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# 60
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# 70
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# 80
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# 90
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# A0
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	# B0
  1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,	# C0
  1, 1, 3, 2, 3, 1, 2, 1, 1, 1, 3, 2, 3,IPREFIX_DD, 2, 1,	# D0	-1: DD
  1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3,IPREFIX_ED, 2, 1,	# E0	-2: ED
  1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3,IPREFIX_FD, 2, 1	# F0	-3: FD
  );

my @instruction_sizes_DDFD =
  (
# 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F

  0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0,	# 00
  0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0,	# 10
  0, 4, 4, 2, 2, 2, 3, 0, 0, 2, 4, 2, 2, 2, 3, 0,	# 20
  0, 0, 0, 0, 3, 3, 4, 0, 0, 2, 0, 0, 0, 0, 0, 0,	# 30
  0, 0, 0, 0, 2, 2, 3, 0, 0, 0, 0, 0, 2, 2, 3, 0,	# 40
  0, 0, 0, 0, 2, 2, 3, 0, 0, 0, 0, 0, 2, 2, 3, 0,	# 50
  2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,	# 60
  3, 3, 3, 3, 3, 3, 0, 3, 0, 0, 0, 0, 2, 2, 3, 0,	# 70
  0, 0, 0, 0, 2, 2, 3, 0, 0, 0, 0, 0, 2, 2, 3, 0,	# 80
  0, 0, 0, 0, 2, 2, 3, 0, 0, 0, 0, 0, 2, 2, 3, 0,	# 90
  0, 0, 0, 0, 2, 2, 3, 0, 0, 0, 0, 0, 2, 2, 3, 0,	# A0
  0, 0, 0, 0, 2, 2, 3, 0, 0, 0, 0, 0, 2, 2, 3, 0,	# B0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,	# C0	4: CB
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# D0
  0, 2, 0, 2, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0,	# E0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0	# F0
  );

my @instruction_sizes_ED =
  (
# 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# 00
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# 10
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# 20
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# 30
  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2,	# 40
  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2,	# 50
  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2,	# 60
  2, 2, 2, 4, 2, 2, 2, 0, 2, 2, 2, 4, 2, 2, 2, 0,	# 70
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# 80
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# 90
  2, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0,	# A0
  2, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0,	# B0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# C0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# D0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	# E0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0	# F0
  );

my $prev_is_jump;

use constant SILENT0 => 0;
use constant SILENT1 => 1;

my $decoder_silent_level;

use constant RAM_ALIGN_SIZE  => 3;
use constant EXPL_ALIGN_SIZE => 5;
use constant STAT_ALIGN_SIZE => 6;
use constant TBL_COLUMNS     => 8;

=back
	The structure of one element of the %blocks_by_address hash:

	{
	TYPE      => 0,
	ADDR      => 0,
	SIZE      => 0,
	LABEL     => {
		     TYPE       => 0,
		     NAME       => '',
		     PRINTED    => FALSE,
		     CALL_COUNT => 0,
		     JUMP_COUNT => 0
		     },
	REF_COUNT => 0
	}
=cut

use constant BLOCK_INSTR      => 0;
use constant BLOCK_RAM        => 1;
use constant BLOCK_CONST      => 2;
use constant BLOCK_EMPTY      => 3;

use constant BL_TYPE_NONE     => -1;
use constant BL_TYPE_SUB      =>  0;
use constant BL_TYPE_LABEL    =>  1;
use constant BL_TYPE_JTABLE   =>  2;
use constant BL_TYPE_VARIABLE =>  3;
use constant BL_TYPE_CONST    =>  4;

my %label_names =
  (
  eval BL_TYPE_SUB      => 'Function_',
  eval BL_TYPE_LABEL    => 'Label_',
  eval BL_TYPE_JTABLE   => 'Jumptable_',
  eval BL_TYPE_VARIABLE => 'Variable_',
  eval BL_TYPE_CONST    => 'Constant_'
  );

my %empty_blocks_by_address = ();
my %blocks_by_address = ();
my %labels_by_address = ();
my $max_label_addr = 0;

my %interrupts_by_address =
  (
  0x0000 => 'System_init',
  0x0008 => 'Interrupt_08',
  0x0010 => 'Interrupt_10',
  0x0018 => 'Interrupt_18',
  0x0020 => 'Interrupt_20',
  0x0028 => 'Interrupt_28',
  0x0030 => 'Interrupt_30',
  0x0038 => 'Interrupt_38'
  );

my %control_characters =
  (
  0x00 => '\0',
  0x07 => '\a',
  0x08 => '\b',
  0x09 => '\t',
  0x0A => '\n',
  0x0C => '\f',
  0x0D => '\r',
  0x1B => '\e',
  0x7F => '^?'
  );

use constant INST_LD_HL		=> 0x21;
use constant INST_ADD_HL_DE	=> 0x19;
use constant INST_JP		=> 0xC3;
use constant INST_JP_HL		=> 0xE9;
use constant INST_JP_CC		=> 0xC2;	# mask: 0xC7
use constant INST_JR		=> 0x18;
use constant INST_JR_CC		=> 0x20;	# mask: 0xE7
use constant INST_DJNZ		=> 0x10;
use constant INST_CALL		=> 0xCD;
use constant INST_CALL_CC	=> 0xC4;	# mask: 0xC7
use constant INST_RET		=> 0xC9;
use constant INST_RETI		=> 0x4D;	# with 0xED prefix
use constant INST_RETN		=> 0x45;	# with 0xED prefix

my $dcd_address    = 0;
my $dcd_instr_size = 0;
my $dcd_instr	   = 0;
my $dcd_instr_x    = 0;
my $dcd_instr_y    = 0;
my $dcd_instr_z    = 0;
my $dcd_instr_p    = 0;
my $dcd_instr_q    = 0;
my $dcd_parm0	   = 0;
my $dcd_parm1	   = 0;
my $dcd_parm2	   = 0;

my $table_header = '';
my $table_border = '';

################################################################################
################################################################################

my %pp_defines = ();            # Value of definitions.

my @pp_conditions = ();
my @pp_else_conditions = ();
my $pp_level = 0;               # Shows the lowest level.
my $embed_level;

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@  This a simple preprocessor.  @@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	# Examines that the parameter is defined or not defined.

sub _defined($)
  {
  return defined($pp_defines{$_[0]});
  }

#-------------------------------------------------------------------------------

	# Records a definition.

sub define($)
  {
  my ($Name) = ($_[0] =~ /^(\S+)/op);
  my $Body = ${^POSTMATCH};

  $Body =~ s/^\s+//o;

  die "define(): This definition already exists: \"$Name\"\n" if (_defined($Name));

        # The definition is in fact unnecessary.
  $pp_defines{$Name} = $Body;
  }

#-------------------------------------------------------------------------------

	# Delete a definition.

sub undefine($)
  {
  delete($pp_defines{$_[0]});
  }

#-------------------------------------------------------------------------------

	# Evaluation of the #if give a boolean value. This procedure preserves it.

sub if_condition($)
  {
  my $Val = $_[0];

  push(@pp_conditions, $Val);
  push(@pp_else_conditions, $Val);
  ++$pp_level;
  }

#-------------------------------------------------------------------------------

	# Evaluation of the #else give a boolean value. This procedure preserves it.

sub else_condition($$)
  {
  my ($File, $Line_number) = @_;

  die "else_condition(): The ${Line_number}th line of $File there is a #else, but does not belong him #if.\n" if ($pp_level <= 0);

  my $last = $#pp_conditions;

  if ($last > 0 && $pp_conditions[$last - 1])
    {
    $pp_conditions[$last] = ($pp_else_conditions[$#pp_else_conditions]) ? FALSE : TRUE;
    }
  else
    {
    $pp_conditions[$last] = FALSE;
    }
  }

#-------------------------------------------------------------------------------

	# Closes a logical unit which starts with a #if.

sub endif_condition($$)
  {
  my ($File, $Line_number) = @_;

  die "endif_condition(): The ${Line_number}th line of $File there is a #endif, but does not belong him #if.\n" if ($pp_level <= 0);

  pop(@pp_conditions);
  pop(@pp_else_conditions);
  --$pp_level;
  }

#-------------------------------------------------------------------------------

sub reset_preprocessor()
  {
  %pp_defines = ();
  @pp_conditions = ();
  push(@pp_conditions, TRUE);
  @pp_else_conditions = ();
  push(@pp_else_conditions, FALSE);
  $pp_level = 0;
  }

#-------------------------------------------------------------------------------

        # This the preprocessor.

sub run_preprocessor($$$$)
  {
  my ($Fname, $Function, $Line, $Line_number) = @_;

  if ($Line =~ /^#\s*ifdef\s+(\S+)$/o)
    {
    if ($pp_conditions[$#pp_conditions])
      {
        # The ancestor is valid, therefore it should be determined that
        # the descendants what kind.

      if_condition(_defined($1));
      }
    else
      {
        # The ancestor is invalid, so the descendants will invalid also.

      if_condition(FALSE);
      }
    }
  elsif ($Line =~ /^#\s*ifndef\s+(\S+)$/o)
    {
    if ($pp_conditions[$#pp_conditions])
      {
        # The ancestor is valid, therefore it should be determined that
        # the descendants what kind.

      if_condition(! _defined($1));
      }
    else
      {
        # The ancestor is invalid, so the descendants will invalid also.

      if_condition(FALSE);
      }
    }
  elsif ($Line =~ /^#\s*else/o)
    {
    else_condition($Fname, $Line_number);
    }
  elsif ($Line =~ /^#\s*endif/o)
    {
    endif_condition($Fname, $Line_number);
    }
  elsif ($Line =~ /^#\s*define\s+(.+)$/o)
    {
        # This level is valid, so it should be recorded in the definition.

    define($1) if ($pp_conditions[$#pp_conditions]);
    }
  elsif ($Line =~ /^#\s*undef\s+(.+)$/o)
    {
        # This level is valid, so it should be deleted in the definition.

    undefine($1) if ($pp_conditions[$#pp_conditions]);
    }
  elsif ($pp_conditions[$#pp_conditions])
    {
        # This is a valid line. (The whole magic is in fact therefore there is.)

    $Function->($Line);
    }
  }

################################################################################
################################################################################
################################################################################

sub basename($)
  {
  return ($_[0] =~ /([^\/]+)$/) ? $1 : '';
  }

#-------------------------------------------------------------------------------

sub param_exist($$)
  {
  die "This option \"$_[0]\" requires a parameter.\n" if ($_[1] > $#ARGV);
  }

#-------------------------------------------------------------------------------

sub Log
  {
  return if (pop(@_) > $verbose);
  foreach (@_) { print STDERR $_; }
  print STDERR "\n";
  }

#-------------------------------------------------------------------------------

sub str2int($)
  {
  my $Str = $_[0];

  return hex($1)   if ($Str =~ /^0x([[:xdigit:]]+)$/io);
  return int($Str) if ($Str =~ /^-?\d+$/o);

  die "str2int(): This string not integer: \"$Str\"";
  }

#-------------------------------------------------------------------------------

	#
	# Before print, formats the $Text.
	#

sub align($$)
  {
  my ($Text, $Tab_count) = @_;
  my ($al, $ct);

  $ct = $Text;
  1 while $ct =~ s/\t/' ' x (TAB_LENGTH - ($-[0] % TAB_LENGTH))/e;
  $al = $Tab_count - (int(length($ct) / TAB_LENGTH));

	# One space will surely becomes behind it.
  if ($al < 1)
    {
    return "$Text ";
    }
  else
    {
    return ($Text . ("\t" x $al));
    }
  }

#-------------------------------------------------------------------------------

	#
	# Multiple file test.
	#

sub is_file_ok($)
  {
  my $File = $_[0];

  if (! -e $File)
    {
    print STDERR "$PROGRAM: Not exists -> \"$File\"\n";
    exit(1);
    }

  if (! -f $File)
    {
    print STDERR "$PROGRAM: Not file -> \"$File\"\n";
    exit(1);
    }

  if (! -r $File)
    {
    print STDERR "$PROGRAM: Can not read -> \"$File\"\n";
    exit(1);
    }

  if (! -s $File)
    {
    print STDERR "$PROGRAM: Empty file -> \"$File\"\n";
    exit(1);
    }
  }

#-------------------------------------------------------------------------------

	#
	# Initializes the @rom array.
	#

sub init_mem($$)
  {
  my ($Start, $End) = @_;

  @rom[$Start .. $End] = ((EMPTY) x ($End - $Start + 1));
  }

#-------------------------------------------------------------------------------

	#
	# Store values of the $Code to $AddrRef address.
	#

sub store_code($$)
  {
  my ($Code, $AddrRef) = @_;

  if ($$AddrRef >= $rom_size)
    {
    printf STDERR "Warning, this address (0x%04X) outside the code area (0x%04X)!\n", $$AddrRef, $rom_size - 1;
    }

  Log(sprintf("rom[0x%08X] = 0x%02X", $$AddrRef, $Code), 9);
  $rom[$$AddrRef++] = $Code;
  }

#-------------------------------------------------------------------------------

        #
        # Reads contents of the $Hex.
        #

sub read_hex($)
  {
  my $Hex    = $_[0];
  my $addr_H = 0;
  my $format = INHX32;
  my $line_num = 0;

  if (! open(IN, '<', $Hex))
    {
    print STDERR "$PROGRAM : Could not open. -> \"$Hex\"\n";
    exit(1);
    }

  while (<IN>)
    {
    chomp;
    s/\r$//o;
    ++$line_num;

    my $len = length() - 1;

    if ($len < MIN_LINE_LENGTH)
      {
      close(IN);
      print STDERR "$PROGRAM: ${line_num}th line <- Shorter than %u character.\n", MIN_LINE_LENGTH;
      exit(1);
      }

    Log("$..(1) (\"$_\") length() = " . length(), 7);

    my $bytecount = int(($len - MIN_LINE_LENGTH) / BYTE_SIZE);

    my $binrec = pack('H*', substr($_, 1));

    if (unpack('%8C*', $binrec) != 0)
      {
      close(IN);
      print STDERR "$PROGRAM: $Hex <- Crc error. (${line_num}th line \"$_\").\n";
      exit(1);
      }

    my ($count, $addr, $type, $bytes) = unpack('CnCX4Cx3/a', $binrec);

    my @codes = unpack('C*', $bytes);

    Log(sprintf("$..(2) (\"$_\") count = $count, bytecount = $bytecount, addr = 0x%04X, type = $type", $addr), 7);

    if ($type == INHX_EOF_REC)
      {
      last;
      }
    elsif ($type == INHX_EXT_LIN_ADDR_REC)
      {
      $addr_H = unpack('n', $bytes);	# big-endian

      Log(sprintf("$..(3) (\"$_\") addr_H = 0x%04X\n", $addr_H), 7);

      $format = INHX32;
      Log('format = INHX32', 7);
      next;
      }
    elsif ($type != INHX_DATA_REC)
      {
      close(IN);
      printf STDERR "$PROGRAM: $Hex <- Unknown type of record: 0x%02X (${line_num}th line \"$_\").\n", $type;
      exit(1);
      }

    if ($bytecount == $count)			# INHX32
      {
      if ($format == INHX8M)
	{
	close(IN);
	print STDERR "$PROGRAM: $Hex <- Mixed format of file (${line_num}th line \"$_\").\n";
	exit(1);
	}

      my $addr32 = ($addr_H << 16) | $addr;

      map { store_code($_, \$addr32) } @codes;
      }
    elsif ($bytecount == ($count * BYTE_SIZE))	# INHX8M
      {
      if ($format == INHX32)
	{
	close(IN);
	print STDERR "$PROGRAM: $Hex <- Mixed format of file (${line_num}th line \"$_\").\n";
	exit(1);
	}

      map { store_code($_, \$addr) } @codes;
      }
    else
      {
      close(IN);
      print STDERR "$PROGRAM: $Hex <- Wrong format of file (${line_num}th line \"$_\").\n";
      exit(1);
      }
    } # while (<IN>)

  close(IN);
  }

#-------------------------------------------------------------------------------

	#
	# Determines that the $Address belongs to a constant.
	#

sub is_constant($)
  {
  my $Address = $_[0];

  foreach (sort {$a <=> $b} keys(%const_areas_by_address))
    {
    return TRUE if ($_ <= $Address && $Address <= $const_areas_by_address{$_});
    last if ($_ > $Address);
    }

  foreach (sort {$a <=> $b} keys(%const_blocks_by_address))
    {
    return TRUE if ($_ <= $Address && $Address <= $const_blocks_by_address{$_});
    last if ($_ > $Address);
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

	#
	# Determines that the $Address belongs to a empty area.
	#

sub is_empty($)
  {
  my $Address = $_[0];

  foreach (sort {$a <=> $b} keys(%empty_blocks_by_address))
    {
    return TRUE if ($_ <= $Address && $Address <= $empty_blocks_by_address{$_});
    last if ($_ > $Address);
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

	#
	# Creates a const block.
	#

sub add_const_area($$)
  {
  $const_areas_by_address{$_[0]} = $_[1];
  }

#-------------------------------------------------------------------------------

	#
	# Creates a new block, or modifies one.
	#

sub add_block($$$$$)
  {
  my ($Address, $Type, $Size, $LabelType, $LabelName) = @_;
  my ($block, $label, $end);

  $end = $Address + $Size - 1;

  if (! defined($blocks_by_address{$Address}))
    {
    $label = {
	     TYPE       => $LabelType,
	     NAME       => $LabelName,
	     PRINTED    => FALSE,
	     CALL_COUNT => 0,
	     JUMP_COUNT => 0
	     };

    $blocks_by_address{$Address} = {
				   TYPE      => $Type,
				   ADDR      => $Address,
				   SIZE      => $Size,
				   LABEL     => $label,
				   REF_COUNT => 0
				   };

    given ($Type)
      {
      when (BLOCK_INSTR)
	{
	if ($LabelType != BL_TYPE_NONE)
	  {
	  $labels_by_address{$Address} = $label;
	  $max_label_addr = $Address if ($max_label_addr < $Address);
	  }
	}

      when (BLOCK_RAM)
	{
	if ($LabelType != BL_TYPE_NONE)
	  {
	  $labels_by_address{$Address} = $label;
	  $max_label_addr = $Address if ($max_label_addr < $Address);
	  }

	$ram_blocks_by_address{$Address} = $end if ($Size > 0);
	}

      when (BLOCK_CONST)
	{
	if ($LabelType != BL_TYPE_NONE)
	  {
	  $labels_by_address{$Address} = $label;
	  $max_label_addr = $Address if ($max_label_addr < $Address);
	  }

	$const_blocks_by_address{$Address} = $end if ($Size > 0);
	}

      when (BLOCK_EMPTY)
	{
	# At empty area, can not be label.

	$label->{TYPE} = BL_TYPE_NONE;
	$label->{NAME} = '';
	$empty_blocks_by_address{$Address} = $end if ($Size > 0);
	}

      default
	{
	printf STDERR "add_block(0x%04X): Unknown block type!\n", $Address;
	exit(1);
	}
      } # given ($Type)
    } # if (! defined($blocks_by_address{$Address}))
  else
    {
    $block = $blocks_by_address{$Address};
    $label = $block->{LABEL};
    $block->{TYPE} = $Type;
    $block->{SIZE} = $Size 	if ($Size > 0);
    $label->{NAME} = $LabelName	if ($label->{NAME} eq '' && $LabelName ne '');

    given ($Type)
      {
      when (BLOCK_INSTR)
	{
	if ($LabelType != BL_TYPE_NONE)
	  {
	  $label->{TYPE} = $LabelType;
	  $labels_by_address{$Address} = $label;
	  $max_label_addr = $Address if ($max_label_addr < $Address);
	  }
	}

      when (BLOCK_RAM)
	{
	if ($LabelType != BL_TYPE_NONE)
	  {
	  $label->{TYPE} = $LabelType;
	  $labels_by_address{$Address} = $label;
	  $max_label_addr = $Address if ($max_label_addr < $Address);
	  }

	$ram_blocks_by_address{$Address} = $end if ($Size > 0);
	}

      when (BLOCK_CONST)
	{
	if ($LabelType != BL_TYPE_NONE)
	  {
	  $label->{TYPE} = $LabelType;
	  $labels_by_address{$Address} = $label;
	  $max_label_addr = $Address if ($max_label_addr < $Address);
	  }

	$const_blocks_by_address{$Address} = $end if ($Size > 0);
	}

      when (BLOCK_EMPTY)
	{
	# At empty area, can not be label.

	$label->{TYPE} = BL_TYPE_NONE;
	$label->{NAME} = '';
	$empty_blocks_by_address{$Address} = $end if ($Size > 0);
	}
      } # given ($Type)
    }

  return $label;
  }

#-------------------------------------------------------------------------------

	#
	# Store address entry of a procedure.
	#

sub add_func_label($$$)
  {
  my ($Address, $Name, $Map_mode) = @_;
  my $label;

  if ($Address < 0)
    {
    Log(sprintf("add_func_label(): This address (0x%04X) negative!", $Address), 2);
    return;
    }

  if (! $Map_mode)
    {
    if (! defined($blocks_by_address{$Address}))
      {
      Log(sprintf("add_func_label(): This address (0x%04X) does not shows an instruction!", $Address), 2);
      return;
      }
    }

  if (is_constant($Address) || is_empty($Address))
    {
    Log(sprintf("add_func_label(): This address (0x%04X) outside the code area!", $Address), 2);
    return;
    }

  $label = add_block($Address, BLOCK_INSTR, 0, BL_TYPE_SUB, $Name);

  if (! $Map_mode)
    {
    ++$label->{CALL_COUNT};
    ++$blocks_by_address{$Address}->{REF_COUNT};
    }
  }

#-------------------------------------------------------------------------------

	#
	# Store a label.
	#

sub add_jump_label($$$$$)
  {
  my ($TargetAddr, $Name, $Type, $SourceAddr, $Map_mode) = @_;
  my ($label, $type);

  if ($TargetAddr < 0)
    {
    Log(sprintf("add_jump_label(): This address (0x%04X) negative!", $TargetAddr), 2);
    return;
    }

  if (! $Map_mode)
    {
    if (! defined($blocks_by_address{$TargetAddr}))
      {
      Log(sprintf("add_jump_label(): This address (0x%04X) does not shows an instruction!", $TargetAddr), 2);
      return;
      }
    }

  if (is_constant($TargetAddr) || is_empty($TargetAddr))
    {
    Log(sprintf("add_jump_label(): This address (0x%04X) outside the code area!", $TargetAddr), 2);
    return;
    }

  if (defined($interrupts_by_address{$SourceAddr}))
    {
    $Type = BL_TYPE_SUB;
    $Name = $interrupts_by_address{$SourceAddr} if ($Name eq '');
    }

  $label = add_block($TargetAddr, BLOCK_INSTR, 0, $Type, $Name);

  if (! $Map_mode)
    {
    ++$label->{JUMP_COUNT};
    ++$blocks_by_address{$TargetAddr}->{REF_COUNT};
    }
  }

#-------------------------------------------------------------------------------

	#
	# Store a variable name.
	#

sub add_ram($$$)
  {
  my ($Address, $Name, $Map_mode) = @_;

  return if ($Address == EMPTY);

  if ($Address < 0)
    {
    Log(sprintf("add_ram(): This address (0x%04X) negative!", $Address), 2);
    return;
    }

  add_block($Address, BLOCK_RAM, 1, BL_TYPE_VARIABLE, $Name);

  ++$blocks_by_address{$Address}->{REF_COUNT} if (! $Map_mode);
  }

#-------------------------------------------------------------------------------

	#
	# Store a I/O port name.
	#

sub add_io($$$)
  {
  my ($Address, $Name, $Map_mode) = @_;
  my $io;

  return if ($Address == EMPTY);

  if (! defined($io = $io_by_address{$Address}))
    {
    $io_by_address{$Address} =  {
				NAME	  => $Name,
				REF_COUNT => ($Map_mode) ? 0 : 1
				};
    }
  else
    {
    ++$io->{REF_COUNT} if (! $Map_mode);
    }
  }

################################################################################
################################################################################

use constant MAP_NULL   => 0;
use constant MAP_BORDER => 1;
use constant MAP_AREA   => 2;
use constant MAP_CODE   => 3;
use constant MAP_DATA   => 4;

        #
        # If exists the map file, then extracts out of it the labels,
        # variables and some segments.
        #

sub read_map_file()
  {
  my $state;

  return if ($map_file eq '');

  $state = MAP_NULL;

  if (! open(MAP, '<', $map_file))
    {
    print STDERR "$PROGRAM : Could not open. -> \"$map_file\"\n";
    exit(1);
    }

  while (<MAP>)
    {
    chomp;
    s/\r$//o;

    if ($state == MAP_NULL)
      {
      $state = MAP_BORDER if (/^Area\s+/io);
      }
    elsif ($state == MAP_BORDER)
      {
      $state = MAP_AREA if (/^-+\s+/o);
      }
    elsif ($state == MAP_AREA)
      {
      if (/^_CODE\s+/o)
	{
	$state = MAP_CODE;
	}
      elsif (/^_(DATA|INITIALIZED)\s+/o)
	{
	$state = MAP_DATA;
	}
      else
	{
	$state = MAP_NULL;
	}
      }
    elsif ($state == MAP_CODE)
      {
      if (/^.ASxxxx Linker\s+/io)
        {
        $state = MAP_NULL;
        }
      elsif (/^\s+([[:xdigit:]]+)\s+(\S+)/o)
	{
	#     00000180  _main                              main
	#     00000190  _main_end                          main
	#     000001A2  _puts                              conio
	#     000001B5  _puthex                            conio
	#     00000201  _puthex8                           conio

	add_func_label(hex($1), $2, TRUE);
	}
      } # elsif ($state == MAP_CODE)
    elsif ($state == MAP_DATA)
      {
      if (/^.ASxxxx Linker\s+/io)
        {
        $state = MAP_NULL;
        }
      elsif (/^\s*([[:xdigit:]]+)\s+(\S+)/o)
	{
	#     000006C6  _heap_top
	#     000006C8  _last_error
	#     000006C9  _old_isr

	add_ram(hex($1), $2, TRUE);
	}
      } # elsif ($state == MAP_DATA)
    } # while (<MAP>)

  $map_readed = TRUE;
  close(MAP);
  }

#-------------------------------------------------------------------------------

use constant NAMES_NULL => 0;
use constant NAMES_RAM  => 1;
use constant NAMES_IO   => 2;
use constant NAMES_ROM  => 3;

sub read_name_list()
  {
  my ($line, $addr, $name, $state);

  return if ($name_list eq '');

  if (! open(NAMES, '<', $name_list))
    {
    print STDERR "$PROGRAM : Could not open. -> \"$name_list\"\n";
    exit(1);
    }

  $state = NAMES_NULL;

  foreach (grep(! /^\s*$/o, <NAMES>))
    {
    chomp;
    s/\r$//o;
    s/^\s*|\s*$//go;

    if (/^\[RAM\]$/io)
      {
      $state = NAMES_RAM;
      next;
      }
    elsif (/^\[IO\]$/io)
      {
      $state = NAMES_IO;
      next;
      }
    elsif (/^\[ROM\]$/io)
      {
      $state = NAMES_ROM;
      next;
      }

    $line = $_;

    given ($state)
      {
      when (NAMES_RAM)
	{
	if ($line =~ /^0x([[:xdigit:]]+)\s*:\s*(\S+)$/io)
	  {
	  add_ram(hex($1), $2, TRUE);
	  }
	}

      when (NAMES_IO)
	{
	if ($line =~ /^0x([[:xdigit:]]+)\s*:\s*(\S+)$/io)
	  {
	  add_io(hex($1), $2, TRUE);
	  }
	}

      when (NAMES_ROM)
	{
	if ($line =~ /^0x([[:xdigit:]]+)\s*:\s*(\S+)$/io)
	  {
	  add_jump_label(hex($1), $2, BL_TYPE_LABEL, EMPTY, TRUE);
	  }
	}
      } # given ($state)
    } # foreach (grep(! /^\s*$/o, <NAMES>))

  close(NAMES);
  }

#-------------------------------------------------------------------------------

	#
	# There are some variables that are multi-byte. However, only
	# the LSB byte of having a name. This procedure gives a name
	# for the higher-significant bytes.
	#

sub fix_multi_byte_variables()
  {
  my ($block, $prev_addr, $prev_name, $name, $i, $var_size);

  $prev_addr = EMPTY;
  $prev_name = '';
  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    $block = $blocks_by_address{$_};
    $name = $block->{LABEL}->{NAME};

    if ($block->{TYPE} != BLOCK_RAM)
      {
      $prev_addr = EMPTY;
      $prev_name = '';
      next;
      }

    $ram_names_by_address{$_} = $name;

    if ($name eq '')
      {
      $prev_addr = EMPTY;
      $prev_name = '';
      next;
      }

    if ($prev_addr != EMPTY)
      {
      $var_size = $_ - $prev_addr;

      if ($var_size > 1)
	{
	# This is a multi-byte variable. Make the aliases.

	for ($i = 1; $i < $var_size; ++$i)
	  {
	  $ram_names_by_address{$prev_addr + $i} = "($prev_name + $i)";
	  }

	$blocks_by_address{$prev_addr}->{SIZE} = $var_size;
	}
      }

    $prev_addr = $_;
    $prev_name = $name;
    } # foreach (sort {$a <=> $b} keys(%blocks_by_address))
  }

#-------------------------------------------------------------------------------

sub fix_io_names()
  {
  my $i = 0;

  foreach (sort {$a <=> $b} keys(%io_by_address))
    {
    next if ($io_by_address{$_}->{NAME} ne '');

    $io_by_address{$_}->{NAME} = "io_$i";
    ++$i;
    }
  }

#-------------------------------------------------------------------------------

        #
        # If there is left in yet so label that has no name, this here get one.
        #

sub add_names_labels()
  {
  my ($addr, $label, $fidx, $lidx, $jtidx, $cidx, $type);

  $fidx  = 0;
  $lidx  = 0;
  $jtidx = 0;
  $cidx  = 0;

  for ($addr = 0; $addr <= $max_label_addr; ++$addr)
    {
    $label = $labels_by_address{$addr};

    next if (! defined($label));

    $type = $label->{TYPE};

    next if ($type == BL_TYPE_NONE || (defined($label->{NAME}) && $label->{NAME} ne ''));

    if ($type == BL_TYPE_SUB)
      {
      $label->{NAME} = sprintf("$label_names{$type}%03u", $fidx++);
      }
    elsif ($type == BL_TYPE_LABEL)
      {
      $label->{NAME} = sprintf("$label_names{$type}%03u", $lidx++);
      }
    elsif ($type == BL_TYPE_JTABLE)
      {
      $label->{NAME} = sprintf("$label_names{$type}%03u", $jtidx++);
      }
    elsif ($type == BL_TYPE_CONST)
      {
      $label->{NAME} = sprintf("$label_names{$type}%03u", $cidx++);
      }
    }
  }

################################################################################
################################################################################

	#
	# Expand a relative offset value.
	#

sub expand_offset($)
  {
  my $Offset = $_[0];

  return ($Offset & 0x80) ? -(($Offset ^ 0xFF) + 1) : $Offset;
  }

#-------------------------------------------------------------------------------

        #
	# Finds address of branchs and procedures.
        #

sub label_finder($$)
  {
  my ($Address, $BlockRef) = @_;
  my ($instr_size, $instr, $addr);

  $instr_size = $BlockRef->{SIZE};
  $instr      = $rom[$Address];

  if ($instr == INST_JP)
    {
	# JP	addr16			11000011 aaaaaaaa aaaaaaaa	a7-a0 a15-a8	absolute address

    $addr = ($rom[$Address + 2] << 8) | $rom[$Address + 1];
    add_jump_label($addr, '', BL_TYPE_LABEL, $Address, FALSE);
    }
  elsif (($instr & 0xC7) == INST_JP_CC)
    {
	# JP	cc, addr16		11ccc010 aaaaaaaa aaaaaaaa	a7-a0 a15-a8	absolute address

    $addr = ($rom[$Address + 2] << 8) | $rom[$Address + 1];
    add_jump_label($addr, '', BL_TYPE_LABEL, $Address, FALSE);
    }
  elsif ($instr == INST_JR)
    {
	# JR	rel			00011000 eeeeeee				relative address

    $addr = $Address + $instr_size + expand_offset($rom[$Address + 1]);
    add_jump_label($addr, '', BL_TYPE_LABEL, EMPTY, FALSE);
    }
  elsif (($instr & 0xE7) == INST_JR_CC)
    {
	# JR	cc, rel			00100000 eeeeeee				relative address

    $addr = $Address + $instr_size + expand_offset($rom[$Address + 1]);
    add_jump_label($addr, '', BL_TYPE_LABEL, EMPTY, FALSE);
    }
  elsif ($instr == INST_DJNZ)
    {
	# DJNZ	rel			00010000 eeeeeee				relative address

    $addr = $Address + $instr_size + expand_offset($rom[$Address + 1]);
    add_jump_label($addr, '', BL_TYPE_LABEL, EMPTY, FALSE);
    }
  elsif ($instr == INST_CALL)
    {
	# CALL	addr16			11001101 aaaaaaaa aaaaaaaa	a7-a0 a15-a8	absolute address

    $addr = ($rom[$Address + 2] << 8) | $rom[$Address + 1];
    add_func_label($addr, '', FALSE);
    }
  elsif (($instr & 0xC7) == INST_CALL_CC)
    {
	# CALL	cc, addr16		11ccc100 aaaaaaaa aaaaaaaa	a7-a0 a15-a8	absolute address

    $addr = ($rom[$Address + 2] << 8) | $rom[$Address + 1];
    add_func_label($addr, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

	#
	# If exists a label name wich belong to the $Address, then returns it.
	# Otherwise, returns the address.
	#

sub label_name($)
  {
  my $Address = $_[0];
  my $label = $labels_by_address{$Address};

  return ((defined($label) && $label->{NAME} ne '') ? $label->{NAME} : (sprintf '0x%04X', $Address));
  }

#-------------------------------------------------------------------------------

	#
	# If exists a I/O port name wich belong to the $Address, then returns it.
	# Otherwise, returns the address.
	#

sub io_name($)
  {
  my $Address = $_[0];
  my $io = $io_by_address{$Address};

  return ((defined($io) && $io->{NAME} ne '') ? $io->{NAME} : (sprintf '0x%02X', $Address));
  }

#-------------------------------------------------------------------------------

	#
	# If exists a variable name wich belong to the $Address, then returns it.
	# Otherwise, returns the address.
	#

sub reg_name($$)
  {
  my ($Address, $StrRef) = @_;
  my ($ram, $str);

  if (defined($ram = $ram_names_by_address{$Address}) && $ram ne '')
    {
    $str = sprintf "0x%04X", $Address;
    ${$StrRef} = "[$str]";
    $str = $ram;
    }
  else
    {
    $str = sprintf "0x%04X", $Address;
    ${$StrRef} = "[$str]";
    }

  return $str;
  }

#-------------------------------------------------------------------------------

	#
	# Auxiliary procedure of prints.
	#

sub print_3($$$)
  {
  my ($Instr, $Param, $Expl) = @_;

  return if ($decoder_silent_level > SILENT0);

  if ($no_explanations)
    {
    print(($Param ne '') ? "$Instr\t$Param\n" : "$Instr\n");
    }
  elsif ($Expl ne '')
    {
    print "$Instr\t" . align($Param, EXPL_ALIGN_SIZE) . "; $Expl\n";
    }
  else
    {
    print(($Param ne '') ? "$Instr\t$Param\n" : "$Instr\n");
    }
  }

#-------------------------------------------------------------------------------

	#
	# If possible, returns the character.
	#

sub decode_char($)
  {
  my $Ch = $_[0];

  if ($Ch >= ord(' ') && $Ch < 0x7F)
    {
    return sprintf " {'%c'}", $Ch;
    }
  elsif (defined($control_characters{$Ch}))
    {
    return " {'$control_characters{$Ch}'}";
    }

  return '';
  }

#-------------------------------------------------------------------------------

	#
	# Determines direction of jump.
	#

sub jump_direction($)
  {
  my $TargetAddr = $_[0];
  my ($str0, $str1, $str2);

  $str0 = sprintf "0x%04X", $TargetAddr;

  if ($dcd_address < $TargetAddr)
    {
    $str1 = '';
    $str2 = ' (forward)';
    }
  elsif ($dcd_address == $TargetAddr)
    {
    $str1 = ' (endless loop)';
    $str2 = '';
    }
  else
    {
    $str1 = '';
    $str2 = ' (backward)';
    }

  return "$str2 hither: $str0$str1";
  }

#---------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------

my @core_registers8 =
  (
    {
    NAME => 'B',
    EXPL => 'B'
    },
    {
    NAME => 'C',
    EXPL => 'C'
    },
    {
    NAME => 'D',
    EXPL => 'D'
    },
    {
    NAME => 'E',
    EXPL => 'E'
    },
    {
    NAME => 'H',
    EXPL => 'H'
    },
    {
    NAME => 'L',
    EXPL => 'L'
    },
    {
    NAME => '(HL)',
    EXPL => '[HL]'
    },
    {
    NAME => 'A',
    EXPL => 'A'
    }
  );

my @core_registers16a = ( 'BC', 'DE', 'HL', 'SP' );
my @core_registers16b = ( 'BC', 'DE', 'HL', 'AF' );
my @core_registers16c = ( 'BC', 'DE', 'IX', 'SP' );

my @CB_shift_instr =
  (
    {
    INSTR => 'rlc',
    EXPL  => 'CF <- %s[7..0] <- %s.7'
    },
    {
    INSTR => 'rrc',
    EXPL  => '%s.0 -> %s[7..0] -> CF'
    },
    {
    INSTR => 'rl',
    EXPL  => 'CF <- %s[7..0] <- CF'
    },
    {
    INSTR => 'rr',
    EXPL  => 'CF -> %s[7..0] -> CF'
    },
    {
    INSTR => 'sla',
    EXPL  => 'CF <- %s[7..0] <- 0'
    },
    {
    INSTR => 'sra',
    EXPL  => '%s.7 -> %s[7..0] -> CF'
    },
    {
    INSTR => 'sll',
    EXPL  => 'CF <- %s[7..0] <- 1'
    },
    {
    INSTR => 'srl',
    EXPL  => '0 -> %s[7..0] -> CF'
    }
  );

sub CB_prefix_decoder()
  {
  my ($str, $i_reg, $reg);

  given ($dcd_instr_x)
    {
    when (0)
      {
#	RLC	r		CB xx		11001011  00000rrr
#	RRC	r		CB xx		11001011  00001rrr
#	RL	r		CB xx		11001011  00010rrr
#	RR	r		CB xx		11001011  00011rrr
#	SLA	r		CB xx		11001011  00100rrr
#	SRA	r		CB xx		11001011  00101rrr
#	SLL	r		CB xx		11001011  00110rrr
#	SRL	r		CB xx		11001011  00111rrr
#							  xxyyyzzz
      if ($decoder_silent_level == SILENT0)
	{
	my $i_shift = $CB_shift_instr[$dcd_instr_y];

	$i_reg = $core_registers8[$dcd_instr_z];
	$reg = $i_reg->{EXPL};
	if ($dcd_instr_y == 0)
	  {
	  $str = sprintf $i_shift->{EXPL}, $reg, $reg;
	  }
	else
	  {
	  $str = sprintf $i_shift->{EXPL}, $reg;
	  }
	print_3($i_shift->{INSTR}, $i_reg->{NAME}, $str);
	}
      }

    when (1)
      {
#	BIT	b, r		CB		11001011  01bbbrrr
#							  xxyyyzzz

      if ($decoder_silent_level == SILENT0)
	{
	$i_reg = $core_registers8[$dcd_instr_z];
	print_3('bit', "$dcd_instr_y, $i_reg->{NAME}", "ZF = !$i_reg->{EXPL}.$dcd_instr_y");
	}
      }

    when (2)
      {
#	RES	b, r		CB		11001011  10bbbrrr
#							  xxyyyzzz

      if ($decoder_silent_level == SILENT0)
	{
	$i_reg = $core_registers8[$dcd_instr_z];
	print_3('res', "$dcd_instr_y, $i_reg->{NAME}", "$i_reg->{EXPL}.$dcd_instr_y = 0");
	}
      }

    default
      {
#	SET	b, r		CB		11001011  11bbbrrr
#							  xxyyyzzz

      if ($decoder_silent_level == SILENT0)
	{
	$i_reg = $core_registers8[$dcd_instr_z];
	print_3('set', "$dcd_instr_y, $i_reg->{NAME}", "$i_reg->{EXPL}.$dcd_instr_y = 1");
	}
      }
    } # given ($dcd_instr_x)
  }

#-------------------------------------------------------------------------------

	#
	# $IndexReg: IX or IY
	#

sub DDFD_CB_prefix_decoder($)
  {
  my $IndexReg = $_[0];
  my $offset = expand_offset($dcd_parm1);
  my ($offs_str, $offs_expl, $i_reg);

  if ($offset < 0)
    {
    $offs_str  = "$offset($IndexReg)";
    $offs_expl = "[$IndexReg$offset]";
    }
  else
    {
    $offs_str  = "$offset($IndexReg)";
    $offs_expl = "[${IndexReg}+$offset]";
    }

  given ($dcd_instr_x)
    {
=back
	r:	000	B
		001	C
		010	D
		011	E
		100	H
		101	L
		110	(HL)
		111	A
=cut
    when (0)
      {
      if ($dcd_instr_z == 6)
	{
#	RLC	(IX+d)		DD CB dd 06	11011101  11001011  dddddddd  00000110	d: two's complement number
#	RLC	(IY+d)		FD CB dd 06	11111101  11001011  dddddddd  00000110	d: two's complement number
#	RRC	(IX+d)		DD CB dd 0E	11011101  11001011  dddddddd  00001110	d: two's complement number
#	RRC	(IY+d)		FD CB dd 0E	11111101  11001011  dddddddd  00001110	d: two's complement number
#	RL	(IX+d)		DD CB dd 16	11011101  11001011  dddddddd  00010110	d: two's complement number
#	RL	(IY+d)		FD CB dd 16	11111101  11001011  dddddddd  00010110	d: two's complement number
#	RR	(IX+d)		DD CB dd 1E	11011101  11001011  dddddddd  00011110	d: two's complement number
#	RR	(IY+d)		FD CB dd 1E	11111101  11001011  dddddddd  00011110	d: two's complement number
#	SLA	(IX+d)		DD CB dd 26	11011101  11001011  dddddddd  00100110	d: two's complement number
#	SLA	(IY+d)		FD CB dd 26	11111101  11001011  dddddddd  00100110	d: two's complement number
#	SRA	(IX+d)		DD CB dd 2E	11011101  11001011  dddddddd  00101110	d: two's complement number
#	SRA	(IY+d)		FD CB dd 2E	11111101  11001011  dddddddd  00101110	d: two's complement number
#	SLL	(IX+d)		DD CB dd 36	11011101  11001011  dddddddd  00110110	d: two's complement number
#	SLL	(IY+d)		FD CB dd 36	11111101  11001011  dddddddd  00110110	d: two's complement number
#	SRL	(IX+d)		DD CB dd 3E	11011101  11001011  dddddddd  00111110	d: two's complement number
#	SRL	(IY+d)		FD CB dd 3E	11111101  11001011  dddddddd  00111110	d: two's complement number
#									      xxyyyzzz

	if ($decoder_silent_level == SILENT0)
          {
	  my $shift = $CB_shift_instr[$dcd_instr_y];
	  my $str = sprintf $shift->{EXPL}, $offs_expl, $offs_expl;

	  print_3($shift->{INSTR}, $offs_str, $str);
	  }
	}
      else
	{
#	LD	r, RLC(IX+d)	DD CB dd 0x	11011101  11001011  dddddddd  00000rrr	d: two's complement number
#	LD	r, RLC(IY+d)	FD CB dd 0x	11111101  11001011  dddddddd  00000rrr	d: two's complement number
#	LD	r, RRC(IX+d)	DD CB dd 0x	11011101  11001011  dddddddd  00001rrr	d: two's complement number
#	LD	r, RRC(IY+d)	FD CB dd 0x	11111101  11001011  dddddddd  00001rrr	d: two's complement number
#	LD	r, RL(IX+d)	DD CB dd 1x	11011101  11001011  dddddddd  00010rrr	d: two's complement number
#	LD	r, RL(IY+d)	FD CB dd 1x	11111101  11001011  dddddddd  00010rrr	d: two's complement number
#	LD	r, RR(IX+d)	DD CB dd 1x	11011101  11001011  dddddddd  00011rrr	d: two's complement number
#	LD	r, RR(IY+d)	FD CB dd 1x	11111101  11001011  dddddddd  00011rrr	d: two's complement number
#	LD	r, SLA(IX+d)	DD CB dd 2x	11011101  11001011  dddddddd  00100rrr	d: two's complement number
#	LD	r, SLA(IY+d)	FD CB dd 2x	11111101  11001011  dddddddd  00100rrr	d: two's complement number
#	LD	r, SRA(IX+d)	DD CB dd 2x	11011101  11001011  dddddddd  00101rrr	d: two's complement number
#	LD	r, SRA(IY+d)	FD CB dd 2x	11111101  11001011  dddddddd  00101rrr	d: two's complement number
#	LD	r, SLL(IX+d)	DD CB dd 3x	11011101  11001011  dddddddd  00110rrr	d: two's complement number
#	LD	r, SLL(IY+d)	FD CB dd 3x	11111101  11001011  dddddddd  00110rrr	d: two's complement number
#	LD	r, SRL(IX+d)	DD CB dd 3x	11011101  11001011  dddddddd  00111rrr	d: two's complement number
#	LD	r, SRL(IY+d)	FD CB dd 3x	11111101  11001011  dddddddd  00111rrr	d: two's complement number
#									      xxyyyzzz

	if ($decoder_silent_level == SILENT0)
          {
	  my $shift = $CB_shift_instr[$dcd_instr_y];
	  my $str = sprintf $shift->{EXPL}, $offs_expl, $offs_expl;

	  $i_reg = $core_registers8[$dcd_instr_z];
	  print_3('ld', "$i_reg->{NAME}, $shift->{INSTR} $offs_str", "$i_reg->{EXPL} = $str");
	  }
	}
      } # $dcd_instr_x == 0

    when (1)
      {
#	BIT	b, (IX+d)	DD CB dd 4x	11011101  11001011  dddddddd  01bbbxxx	d: two's complement number
#	BIT	b, (IY+d)	FD CB dd 4x	11111101  11001011  dddddddd  01bbbxxx	d: two's complement number
#									      xxyyyzzz

      print_3('bit', "$dcd_instr_y, $offs_str", "ZF = !${offs_expl}.$dcd_instr_y");
      } # $dcd_instr_x == 1

    when (2)
      {
      if ($dcd_instr_z == 6)
	{
#	RES	b, (IX+d)	DD CB dd xx	11011101  11001011  dddddddd  10bbb110	d: two's complement number
#	RES	b, (IY+d)	FD CB dd xx	11111101  11001011  dddddddd  10bbb110	d: two's complement number
#									      xxyyyzzz

	print_3('res', "$dcd_instr_y, $offs_str", "${offs_expl}.$dcd_instr_y = 0");
	}
      else
	{
#	LD	r, RES b, (IX+d) DD CB dd xx	11011101  11001011  dddddddd  10bbbrrr	d: two's complement number
#	LD	r, RES b, (IY+d) FD CB dd xx	11111101  11001011  dddddddd  10bbbrrr	d: two's complement number
#									      xxyyyzzz

	$i_reg = $core_registers8[$dcd_instr_z];
	print_3('ld', "$i_reg->{NAME}, res $dcd_instr_y, $offs_str", "${offs_expl}.$dcd_instr_y = 0; $i_reg->{NAME} = $offs_expl");
	}
      } # $dcd_instr_x == 2

    default
      {
      if ($dcd_instr_z == 6)
	{
#	SET	b, (IX+d)	DD CB dd xx	11011101  11001011  dddddddd  11bbb110	d: two's complement number
#	SET	b, (IY+d)	FD CB dd xx	11111101  11001011  dddddddd  11bbb110	d: two's complement number
#									      xxyyyzzz

	print_3('set', "$dcd_instr_y, $offs_str", "${offs_expl}.$dcd_instr_y = 1");
	}
      else
	{
#	LD	r, SET b, (IX+d) DD CB dd xx	11011101  11001011  dddddddd  11bbbrrr	d: two's complement number
#	LD	r, SET b, (IY+d) FD CB dd xx	11111101  11001011  dddddddd  11bbbrrr	d: two's complement number
#									      xxyyyzzz

	$i_reg = $core_registers8[$dcd_instr_z];
	print_3('ld', "$i_reg->{NAME}, set $dcd_instr_y, $offs_str", "${offs_expl}.$dcd_instr_y = 1; $i_reg->{NAME} = $offs_expl");
	}
      }
    } # given ($dcd_instr_x)
  }

#-------------------------------------------------------------------------------

	#
	# $IndexReg: IX or IY
	#

my @DDFD_instr =
  (
    {
    INSTR => 'add',
    EXPL  => 'A +='
    },
    {
    INSTR => 'adc',
    EXPL  => 'A += CF +'
    },
    {
    INSTR => 'sub',
    EXPL  => 'A -='
    },
    {
    INSTR => 'sbc',
    EXPL  => 'A -= CF +'
    },
    {
    INSTR => 'and',
    EXPL  => 'A &='
    },
    {
    INSTR => 'xor',
    EXPL  => 'A ^='
    },
    {
    INSTR => 'or',
    EXPL  => 'A |='
    },
    {
    INSTR => 'cp',
    EXPL  => 'A ?='
    }
  );

sub DDFD_prefix_decoder($)
  {
  my $IndexReg = $_[0];
  my ($addr, $offset, $offs_str, $offs_expl, $str);

  if ($dcd_parm0 == 0xCB)
    {
    instruction_take_to_pieces($dcd_parm2);
    DDFD_CB_prefix_decoder($IndexReg);
    }
  else
    {
    instruction_take_to_pieces($dcd_parm0);

    $offset = expand_offset($dcd_parm1);

    if ($offset < 0)
      {
      $offs_str  = "$offset($IndexReg)";
      $offs_expl = "[$IndexReg$offset]";
      }
    else
      {
      $offs_str  = "$offset($IndexReg)";
      $offs_expl = "[${IndexReg}+$offset]";
      }

    given ($dcd_instr_x)
      {
      when (0)
	{
	if ($dcd_instr_q == 1 && $dcd_instr_z == 1)
	  {
#	ADD	IX, rp		DD 09		11011101  00rr1001
#	ADD	IY, rp		FD 09		11111101  00rr1001
#							  xxppqzzz
#		rp: BC, DE, IX, SP

	  $str = $core_registers16c[$dcd_instr_p];
	  print_3('add', "$IndexReg, $str", "$IndexReg += $str");
	  }
	elsif ($dcd_instr_y == 4)
	  {
	  given ($dcd_instr_z)
	    {
	    when (1)
	      {
#	LD	IX, #nn		DD 21 nn nn	11011101  00100001  a7-0  a15-8
#	LD	IY, #nn		FD 21 nn nn	11111101  00100001  a7-0  a15-8
#							  xxyyyzzz

	      $str = sprintf '0x%04X', ($dcd_parm2 << 8) | $dcd_parm1;
	      print_3('ld', "$IndexReg, #$str", "$IndexReg = $str");
	      }

	    when (2)
	      {
#	LD	(nn), IX	DD 22 nn nn	11011101  00100010  a7-0  a15-8
#	LD	(nn), IY	FD 22 nn nn	11111101  00100010  a7-0  a15-8
#							  xxyyyzzz

	      $addr = ($dcd_parm2 << 8) | $dcd_parm1;

	      if ($decoder_silent_level == SILENT0)
	        {
	        my $name;

		$str = reg_name($addr, \$name);
		print_3('ld', "($str), $IndexReg", "$name = $IndexReg");
		}
	      elsif ($decoder_silent_level == SILENT1)
		{
		add_ram($addr, '', FALSE);
		}
	      }

	    when (3)
	      {
#	INC	IX		DD 23		11011101  00100011
#	INC	IY		FD 23		11111101  00100011
#							  xxyyyzzz

	      print_3('inc', $IndexReg, "++$IndexReg");
	      }

	    when (4)
	      {
#	INC	IXh		DD 24		11011101  00100100
#	INC	IYh		FD 24		11111101  00100100
#							  xxyyyzzz

	      print_3('inc', "${IndexReg}h", "++${IndexReg}.h");
	      }

	    when (5)
	      {
#	DEC	IXh		DD 25		11011101  00100101
#	DEC	IYh		FD 25		11111101  00100101
#							  xxyyyzzz

	      print_3('dec', "${IndexReg}h", "--${IndexReg}.h");
	      }

	    when (6)
	      {
#	LD	IXh, #n		DD 26 nn	11011101  00100110  nnnnnnnn
#	LD	IYh, #n		FD 26 nn	11111101  00100110  nnnnnnnn
#							  xxyyyzzz

	      my $char = decode_char($dcd_parm1);

	      $str = sprintf '0x%02X', $dcd_parm1;
	      print_3('ld', "${IndexReg}h, #$str", "${IndexReg}.h = $str$char");
	      }
	    } # given ($dcd_instr_z)
	  }
	elsif ($dcd_instr_y == 5)
	  {
	  given ($dcd_instr_z)
	    {
	    when (2)
	      {
#	LD	IX, (nn)	DD 2A nn nn	11011101  00101010  a7-0  a15-8
#	LD	IY, (nn)	FD 2A nn nn	11111101  00101010  a7-0  a15-8
#							  xxyyyzzz

	      $addr = ($dcd_parm2 << 8) | $dcd_parm1;

	      if ($decoder_silent_level == SILENT0)
	        {
	        my $name;

		$str = reg_name($addr, \$name);
		print_3('ld', "$IndexReg, ($str)", "$IndexReg = $name");
		}
	      elsif ($decoder_silent_level == SILENT1)
		{
		add_ram($addr, '', FALSE);
		}
	      }

	    when (3)
	      {
#	DEC	IX		DD 2B		11011101  00101011
#	DEC	IY		FD 2B		11111101  00101011
#							  xxyyyzzz

	      print_3('dec', $IndexReg, "--$IndexReg");
	      }

	    when (4)
	      {
#	INC	IXl		DD 2C		11011101  00101100
#	INC	IYl		FD 2C		11111101  00101100
#							  xxyyyzzz

	      print_3('inc', "${IndexReg}l", "++${IndexReg}.l");
	      }

	    when (5)
	      {
#	DEC	IXl		DD 2D		11011101  00101101
#	DEC	IYl		FD 2D		11111101  00101101
#							  xxyyyzzz

	      print_3('dec', "${IndexReg}l", "--${IndexReg}.l");
	      }

	    when (6)
	      {
#	LD	IXl, #n		DD 2E nn	11011101  00101110  nnnnnnnn
#	LD	IYl, #n		FD 2E nn	11111101  00101110  nnnnnnnn
#							  xxyyyzzz

	      my $char = decode_char($dcd_parm1);

	      $str = sprintf '0x%02X', $dcd_parm1;
	      print_3('ld', "${IndexReg}l, #$str", "${IndexReg}.l = $str$char");
	      }
	    } # given ($dcd_instr_z)
	  }
	elsif ($dcd_instr_y == 6)
	  {
	  given ($dcd_instr_z)
	    {
	    when (4)
	      {
#	INC	(IX+d)		DD 34 dd	11011101  00110100  dddddddd  		d: two's complement number
#	INC	(IY+d)		FD 34 dd	11111101  00110100  dddddddd  		d: two's complement number
#							  xxyyyzzz

	      print_3('inc', $offs_str, "++$offs_expl");
	      }

	    when (5)
	      {
#	DEC	(IX+d)		DD 35 dd	11011101  00110101  dddddddd  		d: two's complement number
#	DEC	(IY+d)		FD 35 dd	11111101  00110101  dddddddd  		d: two's complement number
#							  xxyyyzzz

	      print_3('dec', $offs_str, "--$offs_expl");
	      }

	    when (6)
	      {
#	LD	(IX+d), #n	DD 36 dd nn	11011101  00110110  dddddddd  nnnnnnnn	d: two's complement number
#	LD	(IY+d), #n	FD 36 dd nn	11111101  00110110  dddddddd  nnnnnnnn	d: two's complement number
#							  xxyyyzzz

	      my $char = decode_char($dcd_parm2);

	      $str = sprintf '0x%02X', $dcd_parm2;
	      print_3('ld', "$offs_str, #$str", "$offs_expl = $str$char");
	      }
	    } # given ($dcd_instr_z)
	  }
	} # $dcd_instr_x == 0

      when (1)
        {
	given ($dcd_instr_y)
	  {
	  when ([0 .. 3])
	    {
	    given ($dcd_instr_z)
	      {
	      when (4)
		{
#	LD	r, IXh		DD 44		11011101  010rr100
#	LD	r, IYh		FD 44		11111101  010rr100
#							  xxyyyzzz
#		r: B, C, D, E

		$str = $core_registers8[$dcd_instr_y]->{NAME};
		print_3('ld', "$str, ${IndexReg}h", "$str = ${IndexReg}.h");
		}

	      when (5)
		{

#	LD	r, IXl		DD 45		11011101  010rr101
#	LD	r, IYl		FD 45		11111101  010rr101
#							  xxyyyzzz
#		r: B, C, D, E

		$str = $core_registers8[$dcd_instr_y]->{NAME};
		print_3('ld', "$str, ${IndexReg}l", "$str = ${IndexReg}.l");
		}

	      when (6)
		{

#	LD	r, (IX+d)	DD 46 dd	11011101  010rr110  dddddddd  		d: two's complement number
#	LD	r, (IY+d)	FD 46 dd	11111101  010rr110  dddddddd  		d: two's complement number
#							  xxyyyzzz
#		r: B, C, D, E

		$str = $core_registers8[$dcd_instr_y]->{NAME};
		print_3('ld', "$str, $offs_str", "$str = $offs_expl");
		}
	      } # given ($dcd_instr_z)
	    } # when ([0 .. 3])

	  when ([4, 5])
	    {
	    my $r = ($dcd_instr_y == 4) ? 'h' : 'l';

	    given ($dcd_instr_z)
	      {
	      when ([0 .. 3])
		{
#	LD	IXh, B		DD 60		11011101  011000rr
#	LD	IYh, B		FD 60		11111101  011000rr
#	LD	IXh, C		DD 61		11011101  01100001
#	LD	IYh, C		FD 61		11111101  01100001
#	LD	IXh, D		DD 62		11011101  01100010
#	LD	IYh, D		FD 62		11111101  01100010
#	LD	IXh, E		DD 63		11011101  01100011
#	LD	IYh, E		FD 63		11111101  01100011
#							  xxyyyzzz
#		r: B, C, D, E

		$str = $core_registers8[$dcd_instr_z]->{NAME};
		print_3('ld', "$IndexReg$r, $str", "$IndexReg$r = $str");
		}

	      when (4)
		{
#	LD	IXh, IXh	DD 64		11011101  01100100
#	LD	IYh, IYh	FD 64		11111101  01100100
#							  xxyyyzzz

		print_3('ld', "$IndexReg$r, {IndexReg}h", "$IndexReg$r = {IndexReg}.h");
		}

	      when (5)
		{
#	LD	IXh, IXl	DD 65		11011101  01100101
#	LD	IYh, IYl	FD 65		11111101  01100101
#							  xxyyyzzz

		print_3('ld', "$IndexReg$r, {IndexReg}l", "$IndexReg$r = {IndexReg}.l");
		}

	      when (6)
		{
#	LD	H, (IX+d)	DD 66 dd	11011101  01100110  dddddddd  		d: two's complement number
#	LD	H, (IY+d)	FD 66 dd	11111101  01100110  dddddddd  		d: two's complement number
#							  xxyyyzzz

		$str = uc($r);
		print_3('ld', "$str, $offs_str", "$str = $offs_expl");
		}

	      when (7)
		{
#	LD	IXh, A		DD 67		11011101  01100111
#	LD	IYh, A		FD 67		11111101  01100111
#							  xxyyyzzz

		print_3('ld', "$IndexReg$r, A", "$IndexReg$r = A");
		}
	      } # given ($dcd_instr_z)
	    } # when ([4, 5])

	  when (6)
	    {
#	LD	(IX+d), r	DD 70 dd	11011101  01110rrr  dddddddd  		d: two's complement number
#	LD	(IY+d), r	FD 70 dd	11111101  01110rrr  dddddddd  		d: two's complement number
#							  xxyyyzzz
#		r: B, C, D, E, H, L, -, A

	    $str = $core_registers8[$dcd_instr_z]->{NAME};
	    print_3('ld', "$offs_str, $str", "$offs_expl = $str");
	    }

	  default
	    {
	    given ($dcd_instr_z)
	      {
	      when (4)
		{
#	LD	A, IXh		DD 7C		11011101  01111100
#	LD	A, IYh		FD 7C		11111101  01111100
#							  xxyyyzzz

		print_3('ld', "A, ${IndexReg}h", "A = ${IndexReg}.h");
		}

	      when (5)
		{
#	LD	A, IXl		DD 7D		11011101  01111101
#	LD	A, IYl		FD 7D		11111101  01111101
#							  xxyyyzzz

		print_3('ld', "A, ${IndexReg}l", "A = ${IndexReg}.l");
		}

	      when (6)
		{
#	LD	A, (IX+d)	DD 7E dd	11011101  01111110  dddddddd  		d: two's complement number
#	LD	A, (IY+d)	FD 7E dd	11111101  01111110  dddddddd  		d: two's complement number
#							  xxyyyzzz

		print_3('ld', "A, $offs_str", "A = $offs_expl");
		}
	      } # given ($dcd_instr_z)
	    }
	  } # given ($dcd_instr_y)
	} # $dcd_instr_x == 1

      when (2)
        {
        given ($dcd_instr_z)
          {
          when (4)
            {
#	ADD	A, IXh		DD 84		11011101  10000100
#	ADD	A, IYh		FD 84		11111101  10000100
#	ADC	A, IXh		DD 8C		11011101  10001100
#	ADC	A, IYh		FD 8C		11111101  10001100
#	SUB	A, IXh		DD 94		11011101  10010100
#	SUB	A, IYh		FD 94		11111101  10010100
#	SBC	A, IXh		DD 9C		11011101  10011100
#	SBC	A, IYh		FD 9C		11111101  10011100
#	AND	A, IXh		DD A4		11011101  10100100
#	AND	A, IYh		FD A4		11111101  10100100
#	XOR	A, IXh		DD AC		11011101  10101100
#	XOR	A, IYh		FD AC		11111101  10101100
#	OR	A, IXh		DD B4		11011101  10110100
#	OR	A, IYh		FD B4		11111101  10110100
#	CP	A, IXh		DD BC		11011101  10111100
#	CP	A, IYh		FD BC		11111101  10111100
#							  xxyyyzzz

	    my $i_arith = $DDFD_instr[$dcd_instr_y];

	    print_3($i_arith->{INSTR}, "A, ${IndexReg}h", "$i_arith->{EXPL} ${IndexReg}.h");
	    }

          when (5)
            {
#	ADD	A, IXl		DD 85		11011101  10000101
#	ADD	A, IYl		FD 85		11111101  10000101
#	ADC	A, IXl		DD 8D		11011101  10001101
#	ADC	A, IYl		FD 8D		11111101  10001101
#	SUB	A, IXl		DD 95		11011101  10010101
#	SUB	A, IYl		FD 95		11111101  10010101
#	SBC	A, IXl		DD 9D		11011101  10011101
#	SBC	A, IYl		FD 9D		11111101  10011101
#	AND	A, IXl		DD A5		11011101  10100101
#	AND	A, IYl		FD A5		11111101  10100101
#	XOR	A, IXl		DD AD		11011101  10101101
#	XOR	A, IYl		FD AD		11111101  10101101
#	OR	A, IXl		DD B5		11011101  10110101
#	OR	A, IYl		FD B5		11111101  10110101
#	CP	A, IXl		DD BD		11011101  10111101
#	CP	A, IYl		FD BD		11111101  10111101
#							  xxyyyzzz

	    my $i_arith = $DDFD_instr[$dcd_instr_y];

	    print_3($i_arith->{INSTR}, "A, ${IndexReg}l", "$i_arith->{EXPL} ${IndexReg}.l");
	    }

          when (6)
            {
#	ADD	A, (IX+d)	DD 86 dd	11011101  10000110  dddddddd  		d: two's complement number
#	ADD	A, (IY+d)	FD 86 dd	11111101  10000110  dddddddd  		d: two's complement number
#	ADC	A, (IX+d)	DD 8E dd	11011101  10001110  dddddddd  		d: two's complement number
#	ADC	A, (IY+d)	FD 8E dd	11111101  10001110  dddddddd  		d: two's complement number
#	SUB	A, (IX+d)	DD 96 dd	11011101  10010110  dddddddd  		d: two's complement number
#	SUB	A, (IY+d)	FD 96 dd	11111101  10010110  dddddddd  		d: two's complement number
#	SBC	A, (IX+d)	DD 9E dd	11011101  10011110  dddddddd  		d: two's complement number
#	SBC	A, (IY+d)	FD 9E dd	11111101  10011110  dddddddd  		d: two's complement number
#	AND	A, (IX+d)	DD A6 dd	11011101  10100110  dddddddd  		d: two's complement number
#	AND	A, (IY+d)	FD A6 dd	11111101  10100110  dddddddd  		d: two's complement number
#	XOR	A, (IX+d)	DD AE dd	11011101  10101110  dddddddd  		d: two's complement number
#	XOR	A, (IY+d)	FD AE dd	11111101  10101110  dddddddd  		d: two's complement number
#	OR	A, (IX+d)	DD B6 dd	11011101  10110110  dddddddd  		d: two's complement number
#	OR	A, (IY+d)	FD B6 dd	11111101  10110110  dddddddd  		d: two's complement number
#	CP	A, (IX+d)	DD BE dd	11011101  10111110  dddddddd  		d: two's complement number
#	CP	A, (IY+d)	FD BE dd	11111101  10111110  dddddddd  		d: two's complement number
#							  xxyyyzzz

	    my $i_arith = $DDFD_instr[$dcd_instr_y];

	    print_3($i_arith->{INSTR}, "A, $offs_str", "$i_arith->{EXPL} $offs_expl");
	    }
          } # given ($dcd_instr_z)
	} # $dcd_instr_x == 2

      default
	{
	given ($dcd_parm0)
	  {
	  when (0xE1)
	    {
#	POP	IX		DD E1		11011101  11100001
#	POP	IY		FD E1		11111101  11100001

	    print_3('pop', $IndexReg, "${IndexReg}.l = [SP++]; ${IndexReg}.h = [SP++]");
	    }

	  when (0xE3)
	    {
#	EX	(SP), IX	DD E3		11011101  11100011
#	EX	(SP), IY	FD E3		11111101  11100011

	    print_3('ex', "(SP), $IndexReg", "[SP] <-> ${IndexReg}.l; [SP+1] <-> ${IndexReg}.h");
	    }

	  when (0xE5)
	    {
#	PUSH	IX		DD E5		11011101  11100101
#	PUSH	IY		FD E5		11111101  11100101

	    print_3('push', $IndexReg, "[--SP] = ${IndexReg}.h; [--SP] = ${IndexReg}.l");
	    }

	  when (0xE9)
	    {
#	JP	(IX)		DD E9		11011101  11101001
#	JP	(IY)		FD E9		11111101  11101001

	    print_3('jp', "($IndexReg)", "Jumps hither: [$IndexReg]");
	    $prev_is_jump = TRUE;
	    }

	  when (0xF9)
	    {
#	LD	SP, IX		DD F9		11011101  11111001
#	LD	SP, IY		FD F9		11111101  11111001

	    print_3('ld', "SP, $IndexReg", "SP = $IndexReg");
	    }
	  } # given ($dcd_parm0)
	} # $dcd_instr_x == 3
      } # given ($dcd_instr_x)
    }
  }

#-------------------------------------------------------------------------------

my @block_instr =
  (
    [
      {
      INSTR => 'ldi',
      EXPL  => '[DE++] = [HL++]; --BC'
      },
      {
      INSTR => 'cpi',
      EXPL  => 'A ?= [HL++]; --BC'
      },
      {
      INSTR => 'ini',
      EXPL  => '[HL++] = In{C}; --B'
      },
      {
      INSTR => 'outi',
      EXPL  => 'Out{C} = [HL++]; --B'
      }
    ],
    [
      {
      INSTR => 'ldd',
      EXPL  => '[DE--] = [HL--]; --BC'
      },
      {
      INSTR => 'cpd',
      EXPL  => 'A ?= [HL--]; --BC'
      },
      {
      INSTR => 'ind',
      EXPL  => '[HL--] = In{C}; --B'
      },
      {
      INSTR => 'outd',
      EXPL  => 'Out{C} = [HL--]; --B'
      }
    ],
    [
      {
      INSTR => 'ldir',
      EXPL  => '[DE++] = [HL++]; --BC; Exit this loop, then BC == 0.'
      },
      {
      INSTR => 'cpir',
      EXPL  => 'A ?= [HL++]; --BC; Exit this loop, then BC == 0 or A == [HL].'
      },
      {
      INSTR => 'inir',
      EXPL  => '[HL++] = In{C}; --B; Exit this loop, then B == 0.'
      },
      {
      INSTR => 'otir',
      EXPL  => 'Out{C} = [HL++]; --B; Exit this loop, then B == 0.'
      }
    ],
    [
      {
      INSTR => 'lddr',
      EXPL  => '[DE--] = [HL--]; --BC; Exit this loop, then BC == 0.'
      },
      {
      INSTR => 'cpdr',
      EXPL  => 'A ?= [HL--]; --BC; Exit this loop, then BC == 0 or A == [HL].'
      },
      {
      INSTR => 'indr',
      EXPL  => '[HL--] = In{C}; --B; Exit this loop, then B == 0.'
      },
      {
      INSTR => 'otdr',
      EXPL  => 'Out{C} = [HL--]; --B; Exit this loop, then B == 0.'
      }
    ]
  );

sub ED_prefix_decoder()
  {
  my ($addr, $str, $i_reg, $reg);

  instruction_take_to_pieces($dcd_parm0);

  if ($dcd_instr_x == 1)
    {
    given ($dcd_instr_z)
      {
      when (0)
	{
	if ($decoder_silent_level == SILENT0)
	  {
	  $i_reg = $core_registers8[$dcd_instr_z];

	  if ($dcd_instr_y == 6)
	    {
#	IN	(C)		ED 70		11101011  01110000
#							  xxyyyzzz

	    print_3('in', '(C)', "$i_reg->{EXPL} = In{[C]}");
	    }
	  else
	    {
#	IN	r, (C)		ED xx		11101011  01rrr000
#							  xxyyyzzz

	    print_3('in', "$i_reg->{NAME}, (C)", "$i_reg->{EXPL} = In{[C]}");
	    }
	  }
	} # $dcd_instr_z == 0

      when (1)
	{
	if ($decoder_silent_level == SILENT0)
	  {
	  $i_reg = $core_registers8[$dcd_instr_z];

	  if ($dcd_instr_y == 6)
	    {
#	OUT	(C)		ED 71		11101101  01110001
#							  xxyyyzzz

	    print_3('out', '(C)', "Out{[C]} = $i_reg->{EXPL}");
	    }
	  else
	    {
#	OUT	(C), r		ED xx		11101101  01rrr001
#							  xxyyyzzz

	    print_3('out', "(C), $i_reg->{NAME}", "Out{[C]} = $i_reg->{EXPL}");
	    }
	  }
	} # $dcd_instr_z == 1

      when (2)
	{
	if ($dcd_instr_q == 0)
	  {
#	SBC	HL, pp		ED x2		11101101  01pp0010
#							  xxppqzzz

	  $str = $core_registers16a[$dcd_instr_p];
	  print_3('sbc', "HL, $str", "HL -= $str + CF");
	  }
	else
	  {
#	ADC	HL, pp		ED xA		11101101  01pp1010
#							  xxppqzzz

	  $str = $core_registers16a[$dcd_instr_p];
	  print_3('adc', "HL, $str", "HL += $str + CF");
	  }
	} # $dcd_instr_z == 2

      when (3)
	{
	$addr = ($dcd_parm2 << 8) | $dcd_parm1;

	if ($dcd_instr_q == 0)
	  {
#	LD	(nn), pp	ED x3 aa aa	11101101  01pp0011  a7-0  a15-8
#							  xxppqzzz

	  if ($decoder_silent_level == SILENT0)
	    {
	    my $name;

	    $reg = $core_registers16a[$dcd_instr_p];
	    $str = reg_name($addr, \$name);
	    print_3('ld', "($str), $reg", "$name = $reg");
	    }
	  elsif ($decoder_silent_level == SILENT1)
	    {
	    add_ram($addr, '', FALSE);
	    }
	  }
	else
	  {
#	LD	pp, (nn)	ED xB aa aa	11101101  01pp1011  a7-0  a15-8
#							  xxppqzzz

	  if ($decoder_silent_level == SILENT0)
	    {
	    my $name;

	    $reg = $core_registers16a[$dcd_instr_p];
	    $str = reg_name($addr, \$name);
	    print_3('ld', "$reg, ($str)", "$reg = $name");
	    }
	  elsif ($decoder_silent_level == SILENT1)
	    {
	    add_ram($addr, '', FALSE);
	    }
	  }
	} # $dcd_instr_z == 3

      when (4)
	{
#	NEG			ED xx		11101101  01xxx100
#							  xxyyyzzz

	print_3('neg', '', 'A = -A');
	} # $dcd_instr_z == 4

      when (5)
	{
	if ($dcd_instr_y == 1)
	  {
#	RETI			ED 4D		11101101  01001101
#							  xxyyyzzz

	  print_3('reti', '', 'PC.l = [SP++]; PC.h = [SP++]; End of maskable interrupt.');
	  $prev_is_jump = TRUE;
	  }
	else
	  {
#	RETN			ED xx		11101101  01xxx101
#							  xxyyyzzz

	  print_3('retn', '', 'PC.l = [SP++]; PC.h = [SP++]; End of non-maskable interrupt.');
	  $prev_is_jump = TRUE;
	  }
	} # $dcd_instr_z == 5

      when (6)
	{
#	IM	n		ED xx		11101101  01xxx110
#							  xxyyyzzz
#	y: 0 - im 0
#	   1 - im 0
#	   2 - im 1
#	   3 - im 2
#	   4 - im 0
#	   5 - im 0
#	   6 - im 1
#	   7 - im 2

	$dcd_instr_y &= 3;
	--$dcd_instr_y if ($dcd_instr_y);

	print_3('im', $dcd_instr_y, "Interrupt mode ${dcd_instr_y}.");
	} # $dcd_instr_z == 6

      when (7)
	{
	given ($dcd_instr_y)
          {
	  when (0)
	    {
#	LD	I, A		ED 47		11101101  01000111
#							  xxyyyzzz

	    print_3('ld', 'I, A', 'I = A');
	    }

	  when (1)
	    {
#	LD	R, A		ED 4F		11101101  01001111
#							  xxyyyzzz

	    print_3('ld', 'R, A', 'R = A');
	    }

	  when (2)
	    {
#	LD	A, I		ED 57		11101101  01010111
#							  xxyyyzzz

	    print_3('ld', 'A, I', 'A = I');
	    }

	  when (3)
	    {
#	LD	A, R		ED 5F		11101101  01011111
#							  xxyyyzzz

	    print_3('ld', 'A, R', 'A = R');
	    }

	  when (4)
	    {
#	RRD			ED 67		11101101  01100111
#							  xxyyyzzz

	    print_3('rrd', '', 'A[3..0] -> [HL][7..4] -> [HL][3..0] -> A[3..0]');
	    }

	  when (5)
	    {
#	RLD			ED 6F		11101101  01101111
#							  xxyyyzzz

	    print_3('rld', '', 'A[3..0] <- [HL][7..4] <- [HL][3..0] <- A[3..0]');
	    }

	  default
	    {
#	NOP			ED 77		11101101  01110111
#	NOP			ED 7F		11101101  01111111
#							  xxyyyzzz

	    print_3('nop', '', 'No operation.');
	    } # $dcd_instr_y == 6 || $dcd_instr_y == 7
	  } # given ($dcd_instr_y)
	} # $dcd_instr_z == 7
      } # given ($dcd_instr_z)
    } # if ($dcd_instr_x == 1)
  elsif ($dcd_instr_x == 2 && $dcd_instr_y >= 4 && $dcd_instr_z <= 3)
    {
#	LDI			ED A0		11101101  10100000
#	CPI			ED A1		11101101  10100001
#	INI			ED A2		11101101  10100010
#	OUTI			ED A3		11101101  10100011
#							  xxyyyzzz

#	LDD			ED A8		11101101  10101000
#	CPD			ED A9		11101101  10101001
#	IND			ED AA		11101101  10101010
#	OUTD			ED AB		11101101  10101011
#							  xxyyyzzz

#	LDIR			ED B0		11101101  10110000
#	CPIR			ED B1		11101101  10110001
#	INIR			ED B2		11101101  10110010
#	OTIR			ED B3		11101101  10110011
#							  xxyyyzzz

#	LDDR			ED B8		11101101  10111000
#	CPDR			ED B9		11101101  10111001
#	INDR			ED BA		11101101  10111010
#	OTDR			ED BB		11101101  10111011
#							  xxyyyzzz

    my $i_block = $block_instr[$dcd_instr_y - 4][$dcd_instr_z];

    print_3($i_block->{INSTR}, '', $i_block->{EXPL});
    }
  else
    {
    print_3('invalid instruction', '', '');
    }
  }

#-------------------------------------------------------------------------------

sub instruction_take_to_pieces($)
  {
  my $Instruction = $_[0];

  $dcd_instr_x = ($Instruction >> 6) & 3;
  $dcd_instr_y = ($Instruction >> 3) & 7;
  $dcd_instr_z = $Instruction & 7;
  $dcd_instr_p = ($Instruction >> 4) & 3;
  $dcd_instr_q = ($Instruction >> 3) & 1;
  }

#-------------------------------------------------------------------------------

        #
        # Decodes the $BlockRef.
        #

my @shift_instr =
  (
    {
    INSTR => 'rlca',
    EXPL  => 'CF <- A[7..0] <- A.7'
    },
    {
    INSTR => 'rrca',
    EXPL  => 'A.0 -> A[7..0] -> CF'
    },
    {
    INSTR => 'rla',
    EXPL  => 'CF <- A[7..0] <- CF'
    },
    {
    INSTR => 'rra',
    EXPL  => 'CF -> A[7..0] -> CF'
    },
    {
    INSTR => 'daa',
    EXPL  => 'Conditionally decimal adjusts the Accumulator.'
    },
    {
    INSTR => 'cpl',
    EXPL  => 'A = ~A'
    },
    {
    INSTR => 'scf',
    EXPL  => 'CF = 1'
    },
    {
    INSTR => 'ccf',
    EXPL  => 'CF = 0'
    }
  );

my @conditions =
  (
    {
    COND => 'NZ',
    EXPL => 'ZF == 0'
    },
    {
    COND => 'Z',
    EXPL => 'ZF == 1'
    },
    {
    COND => 'NC',
    EXPL => 'CF == 0'
    },
    {
    COND => 'C',
    EXPL => 'CF == 1'
    },
    {
    COND => 'PO',
    EXPL => 'PF == 0'
    },
    {
    COND => 'PE',
    EXPL => 'PF == 1'
    },
    {
    COND => 'P',
    EXPL => 'SF == 0'
    },
    {
    COND => 'M',
    EXPL => 'SF == 1'
    }
  );

sub instruction_decoder($$)
  {
  my ($Address, $BlockRef) = @_;
  my ($addr, $label, $invalid, $str);

  $dcd_address	  = $Address;
  $dcd_instr_size = $BlockRef->{SIZE};
  $dcd_instr	  = $rom[$dcd_address];
  $label 	  = $BlockRef->{LABEL};

  if ($decoder_silent_level == SILENT0)
    {
    printf("0x%04X: %02X", $dcd_address, $dcd_instr) if (! $gen_assembly_code);
    }

  $invalid = FALSE;

  if ($dcd_instr_size == 1)
    {
    if ($decoder_silent_level == SILENT0)
      {
      print(($gen_assembly_code) ? "\t" : "\t\t");
      }
    }
  elsif ($dcd_instr_size == 2)
    {
    $dcd_parm0 = $rom[$dcd_address + 1];
    $invalid = TRUE if ($dcd_parm0 == EMPTY);

    if ($decoder_silent_level == SILENT0)
      {
      if ($gen_assembly_code)
	{
	print "\t";
	}
      else
	{
	printf " %02X\t\t", $dcd_parm0;
	}
      }
    }
  elsif ($dcd_instr_size == 3)
    {
    $dcd_parm0 = $rom[$dcd_address + 1];
    $dcd_parm1 = $rom[$dcd_address + 2];
    $invalid = TRUE if ($dcd_parm0 == EMPTY || $dcd_parm1 == EMPTY);

    if ($decoder_silent_level == SILENT0)
      {
      if ($gen_assembly_code)
	{
	print "\t";
	}
      else
	{
	printf " %02X %02X\t", $dcd_parm0, $dcd_parm1;
	}
      }
    }
  elsif ($dcd_instr_size == 4)
    {
    $dcd_parm0 = $rom[$dcd_address + 1];
    $dcd_parm1 = $rom[$dcd_address + 2];
    $dcd_parm2 = $rom[$dcd_address + 3];
    $invalid = TRUE if ($dcd_parm0 == EMPTY || $dcd_parm1 == EMPTY || $dcd_parm2 == EMPTY);

    if ($decoder_silent_level == SILENT0)
      {
      if ($gen_assembly_code)
	{
	print "\t";
	}
      else
	{
	printf " %02X %02X %02X\t", $dcd_parm0, $dcd_parm1, $dcd_parm2;
	}
      }
    }
  else
    {
    printf STDERR "Internal error: The size of instruction (addr:0x%04X) is zero!", $dcd_address;
    exit(1);
    }

	#
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  |         | |               | |               |
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  .  |  .  |  .  |  .  |  .  |  .  |  .  |  .  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              |         | |   |
	#              +---\ /---+ +\ /+
	#                   p        q
	#

  $prev_is_jump = FALSE;

  instruction_take_to_pieces($dcd_instr);

  if ($dcd_instr_x == 0)
    {
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  .  |  .  |  .  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

    given ($dcd_instr_z)
      {
      when (0)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  0  |  0  |  0  |
	# +-----------------------------------------------+

	given ($dcd_instr_y)
	  {
	  when (0)
	    {
#	NOP			00		00000000
#						xxyyyzzz

	    print_3('nop', '', 'No operation.');
	    }

	  when (1)
	    {
#	EX	AF, AF'		08		00001000
#						xxyyyzzz

	    print_3('ex', "AF, AF'", "AF <-> AF'");
	    }

	  when (2)
	    {
#	DJNZ	e		10		00010000  eeeeeeee			e: two's complement number
#						xxyyyzzz

	    if ($decoder_silent_level == SILENT0)
	      {
	      my $addr = $dcd_address + 2 + expand_offset($dcd_parm0);
	      my $target;

	      $str    = label_name($addr);
	      $target = jump_direction($addr);
	      print_3('djnz', $str, "If (--B != 0) jumps$target");
	      $prev_is_jump = TRUE;
	      }
	    }

	  when (3)
	    {
#	JR	e		18		00011000  eeeeeeee			e: two's complement number
#						xxyyyzzz

	    if ($decoder_silent_level == SILENT0)
	      {
	      my $addr = $dcd_address + 2 + expand_offset($dcd_parm0);
	      my $target;

	      $str    = label_name($addr);
	      $target = jump_direction($addr);
	      print_3('jr', $str, "Jumps$target");
	      $prev_is_jump = TRUE;
	      }
	    }

	  default
	    {
		# 4-7
#	JR	cc, e		xx		00ccc000  eeeeeeee			e: two's complement number
#						xxyyyzzz

	    if ($decoder_silent_level == SILENT0)
	      {
	      my $addr = $dcd_address + 2 + expand_offset($dcd_parm0);
	      my $cond = $conditions[$dcd_instr_y - 4];
	      my $target;

	      $str    = label_name($addr);
	      $target = jump_direction($addr);
	      print_3('jr', "$cond->{COND}, $str", "Jumps if ($cond->{EXPL})$target");
	      $prev_is_jump = TRUE;
	      }
	    }
	  } # given ($dcd_instr_y)
	} # $dcd_instr_z == 0

      when (1)
	{
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  0  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	if ($dcd_instr_q == 0)
	  {
#	LD	rp, #nn		x1		00rr0001  nnnnnnnn  nnnnnnnn
#						xxppqzzz
#		rp: BC, DE, HL, SP

	  my $r16 = $core_registers16a[$dcd_instr_p];

	  $str = sprintf '0x%04X', ($dcd_parm1 << 8) | $dcd_parm0;
	  print_3('ld', "$r16, #$str", "$r16 = $str");
	  }
	else
	  {
#	ADD	HL, rp		x9		00rr1001
#						xxppqzzz
#		rp: BC, DE, HL, SP

	  $str = $core_registers16a[$dcd_instr_p];
	  print_3('add', "HL, $str", "HL += $str");
	  }
	} # $dcd_instr_z == 1

      when (2)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  0  |  1  |  0  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	if ($dcd_instr_q == 0)
	  {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  0  |  0  |  1  |  0  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	  given ($dcd_instr_p)
	    {
	    when (0)
	      {
#	LD	(BC), A		02		00000010
#						xxppqzzz

	      print_3('ld', '(BC), A', '[BC] = A');
	      } # $dcd_instr_p == 0

	    when (1)
	      {
#	LD	(DE), A		12		00010010
#						xxppqzzz

	      print_3('ld', '(DE), A', '[DE] = A');
	      } # $dcd_instr_p == 1

	    when (2)
	      {
#	LD	(nn), HL	22		00100010  nnnnnnnn  nnnnnnnn
#						xxppqzzz

	      $addr = ($dcd_parm1 << 8) | $dcd_parm0;

	      if ($decoder_silent_level == SILENT0)
	        {
		my $name;

		$str = reg_name($addr, \$name);
		print_3('ld', "($str), HL", "$name = HL");
		}
	      elsif ($decoder_silent_level == SILENT1)
		{
		add_ram($addr, '', FALSE);
		}
	      } # $dcd_instr_p == 2

	    when (3)
	      {
#	LD	(nn), A		32		00110010  nnnnnnnn  nnnnnnnn
#						xxppqzzz

	      $addr = ($dcd_parm1 << 8) | $dcd_parm0;

	      if ($decoder_silent_level == SILENT0)
	        {
		my $name;

		$str = reg_name($addr, \$name);
		print_3('ld', "($str), A", "$name = A");
		}
	      elsif ($decoder_silent_level == SILENT1)
		{
		add_ram($addr, '', FALSE);
		}
	      } # $dcd_instr_p == 3
	    } # given ($dcd_instr_p)
	  } # if ($dcd_instr_q == 0)
	else
	  {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  1  |  0  |  1  |  0  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	  given ($dcd_instr_p)
	    {
	    when (0)
	      {
#	LD	A, (BC)		0A		00001010
#						xxppqzzz

	      print_3('ld', 'A, (BC)', 'A = [BC]');
	      } # $dcd_instr_p == 0

	    when (1)
	      {
#	LD	A, (DE)		1A		00011010
#						xxppqzzz

	      print_3('ld', 'A, (DE)', 'A = [DE]');
	      } # $dcd_instr_p == 1

	    when (2)
	      {
#	LD	HL, (nn)	2A		00101010  nnnnnnnn  nnnnnnnn
#						xxppqzzz

	      $addr = ($dcd_parm1 << 8) | $dcd_parm0;

	      if ($decoder_silent_level == SILENT0)
	        {
		my $name;

		$str = reg_name($addr, \$name);
		print_3('ld', "HL, ($str)", "HL = $name");
		}
	      elsif ($decoder_silent_level == SILENT1)
		{
		add_ram($addr, '', FALSE);
		}
	      } # $dcd_instr_p == 2

	    when (3)
	      {
#	LD	A, (nn)		3A		00111010  nnnnnnnn  nnnnnnnn
#						xxppqzzz

	      $addr = ($dcd_parm1 << 8) | $dcd_parm0;

	      if ($decoder_silent_level == SILENT0)
	        {
		my $name;

		$str = reg_name($addr, \$name);
		print_3('ld', "A, ($str)", "A = $name");
		}
	      elsif ($decoder_silent_level == SILENT1)
		{
		add_ram($addr, '', FALSE);
		}
	      } # $dcd_instr_p == 3
	    } # given ($dcd_instr_p)
	  }
	} # $dcd_instr_z == 2

      when (3)
	{
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  0  |  1  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	if ($dcd_instr_q == 0)
	  {
#	INC	rp		x3		00rr0011
#						xxppqzzz
#		rp: BC, DE, HL, SP

	  $str = $core_registers16a[$dcd_instr_p];
	  print_3('inc', $str, "++$str");
	  }
	else
	  {
#	DEC	rp		x3		00rr1011
#						xxppqzzz
#		rp: BC, DE, HL, SP

	  $str = $core_registers16a[$dcd_instr_p];
	  print_3('dec', $str, "--$str");
	  }
	} # $dcd_instr_z == 3

      when (4)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  1  |  0  |  0  |
	# +-----------------------------------------------+

#	INC	r		xx		00rrr100
#						xxyyyzzz
#		r: B, C, D, E, H, L, (HL), A

	if ($decoder_silent_level == SILENT0)
	  {
	  my $i_reg = $core_registers8[$dcd_instr_y];

	  print_3('inc', $i_reg->{NAME}, "++$i_reg->{EXPL}");
	  }
	} # $dcd_instr_z == 4

      when (5)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  1  |  0  |  1  |
	# +-----------------------------------------------+

#	DEC	r		xx		00rrr101
#						xxyyyzzz
#		r: B, C, D, E, H, L, (HL), A

	if ($decoder_silent_level == SILENT0)
	  {
	  my $i_reg = $core_registers8[$dcd_instr_y];

	  print_3('dec', $i_reg->{NAME}, "--$i_reg->{EXPL}");
	  }
	} # $dcd_instr_z == 5

      when (6)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  1  |  1  |  0  |
	# +-----------------------------------------------+

#	LD	r, #n				00rrr110  nnnnnnnn
#						xxyyyzzz

	if ($decoder_silent_level == SILENT0)
	  {
	  my $i_reg = $core_registers8[$dcd_instr_y];
	  my $char = decode_char($dcd_parm0);

	  $str = sprintf '0x%02X', $dcd_parm0;
	  print_3('ld', "$i_reg->{NAME}, #$str", "$i_reg->{EXPL} = $str$char");
	  }
	} # $dcd_instr_z == 6

      when (7)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  0  |  .  |  .  |  .  |  1  |  1  |  1  |
	# +-----------------------------------------------+

#	RLCA			07		00000111
#	RRCA			0F		00001111
#	RLA			17		00010111
#	RRA			1F		00011111
#	DAA			27		00100111
#	CPL			2F		00101111
#	SCF			37		00110111
#	CCF			3F		00111111
#						xxyyyzzz

	my $s_instr = $shift_instr[$dcd_instr_y];

	print_3($s_instr->{INSTR}, '', $s_instr->{EXPL});
	} # $dcd_instr_z == 7
      } # given ($dcd_instr_z)
    } # if ($dcd_instr_x == 0)
  elsif ($dcd_instr_x == 1)
    {
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  1  |  .  |  .  |  .  |  .  |  .  |  .  |
	# +-----------------------------------------------+

    if ($dcd_instr_y == 6)
      {
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  1  |  1  |  1  |  0  |  .  |  .  |  .  |
	# +-----------------------------------------------+

#	HALT			76		01110110
#						xxyyyzzz

      print_3('halt', '', 'Suspends CPU.');
      }
    else
      {
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  0  |  1  |  .  |  .  |  .  |  .  |  .  |  .  |
	# +-----------------------------------------------+

#	LD	r, r'		xx		01dddsss
#						xxyyyzzz
#		r: B, C, D, E, H, L, (HL), A

      if ($decoder_silent_level == SILENT0)
	{
	my $i_rega = $core_registers8[$dcd_instr_y];
	my $i_regb = $core_registers8[$dcd_instr_z];

        print_3('ld', "$i_rega->{NAME}, $i_regb->{NAME}", "$i_rega->{EXPL} = $i_regb->{EXPL}");
        }
      }
    } # elsif ($dcd_instr_x == 1)
  elsif ($dcd_instr_x == 2)
    {
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  0  |  .  |  .  |  .  |  .  |  .  |  .  |
	# +-----------------------------------------------+

#	ADD	A, r		8x		10000rrr
#	ADC	A, r		8x		10001rrr
#	SUB	A, r		9x		10010rrr
#	SBC	A, r		9x		10011rrr
#	AND	A, r		Ax		10100rrr
#	XOR	A, r		Ax		10101rrr
#	OR	A, r		Bx		10110rrr
#	CP	A, r		Bx		10111rrr
#						xxyyyzzz
#		r: B, C, D, E, H, L, (HL), A

    if ($decoder_silent_level == SILENT0)
      {
      my $i_arith = $DDFD_instr[$dcd_instr_y];
      my $i_reg = $core_registers8[$dcd_instr_z];
      my $str0 = ($dcd_instr_z == 7) ? ' (A = 0)' : '';

      print_3($i_arith->{INSTR}, "A, $i_reg->{NAME}", "$i_arith->{EXPL} $i_reg->{EXPL}$str0");
      }
    }
  else # $dcd_instr_x == 3
    {
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  .  |  .  |  .  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

    given ($dcd_instr_z)
      {
      when (0)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  0  |  0  |  0  |
	# +-----------------------------------------------+

#	RET	cc	xx			11ccc000
#						xxyyyzzz
#		cc: NZ, Z, NC, C, PO, PE, P, M

	my $cond = $conditions[$dcd_instr_y];

	print_3('ret', $cond->{COND}, "If ($cond->{EXPL}) PC.l = [SP++]; PC.h = [SP++]");
	$prev_is_jump = TRUE;
	}

      when (1)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  0  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	if ($dcd_instr_q == 0)
	  {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  0  |  0  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

#	POP	rp		xx		11rr0001
#						xxppqzzz
#		rp: BC, DE, HL, AF

	  given ($dcd_instr_p)
	    {
	    when (0) { $str = 'C = [SP++]; B = [SP++]'; }
	    when (1) { $str = 'E = [SP++]; D = [SP++]'; }
	    when (2) { $str = 'L = [SP++]; H = [SP++]'; }
	    when (3) { $str = 'F = [SP++]; A = [SP++]'; }
	    }

	  print_3('pop', $core_registers16b[$dcd_instr_p], $str);
	  }
	else
	  {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  1  |  0  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	  given ($dcd_instr_p)
	    {
	    when (0)
	      {
#	RET			C9		11001001
#						xxppqzzz

	      print_3('ret', '', 'PC.l = [SP++]; PC.h = [SP++]');
	      $prev_is_jump = TRUE;
	      }

	    when (1)
	      {
#	EXX			D9		11011001
#						xxppqzzz

	      print_3('exx', '', "BC <-> BC'; DE <-> DE'; HL <-> HL'");
	      }

	    when (2)
	      {
#	JP	(HL)		E9		11101001
#						xxppqzzz

	      print_3('jp', '(HL)', 'Jumps to value of HL.');
	      $prev_is_jump = TRUE;
	      }

	    when (3)
	      {
#	LD	SP, HL		F9		11111001
#						xxppqzzz

	      print_3('ld', 'SP, HL', 'SP = HL');
	      }
	    } # given ($dcd_instr_p)
	  }
	} # $dcd_instr_z == 1

      when (2)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  0  |  1  |  0  |
	# +-----------------------------------------------+

#	JP	cc, nn		xx nn nn	11ccc010  a7-0 a15-8
#						xxyyyzzz
#		cc: NZ, Z, NC, C, PO, PE, P, M

	if ($decoder_silent_level == SILENT0)
	  {
	  my $addr = ($dcd_parm1 << 8) | $dcd_parm0;
	  my $cond = $conditions[$dcd_instr_y];
	  my $target;

	  $str    = label_name($addr);
	  $target = jump_direction($addr);
	  print_3('jp', "$cond->{COND}, $str", "Jumps if ($cond->{EXPL})$target");
	  $prev_is_jump = TRUE;
	  }
	}

      when (3)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  0  |  1  |  1  |
	# +-----------------------------------------------+

	given ($dcd_instr_y)
	  {
	  when (0)
	    {
#	JP	nn				11000011  a7-0 a15-8
#						xxyyyzzz

	    if ($decoder_silent_level == SILENT0)
	      {
	      my $addr = ($dcd_parm1 << 8) | $dcd_parm0;
	      my $target;

	      $str    = label_name($addr);
	      $target = jump_direction($addr);
	      print_3('jp', $str, "Jumps$target");
	      $prev_is_jump = TRUE;
	      }
	    }

	  when (1)
	    {
	    instruction_take_to_pieces($dcd_parm0);
	    CB_prefix_decoder();
	    }

	  when (2)
	    {
#	OUT	(n), A		D3		11010011  nnnnnnnn
#						xxyyyzzz

	    if ($decoder_silent_level == SILENT0)
	      {
	      my $io = sprintf '0x%02X', $dcd_parm0;

	      if ($gen_assembly_code)
		{
		print_3('out', "($io), A", "Out{$io} = A");
		}
	      else
		{
		$str = io_name($dcd_parm0);
		print_3('out', "($str), A", "Out{$io} = A");
		}
	      }
	    elsif ($decoder_silent_level == SILENT1)
	      {
	      add_io($dcd_parm0, '', FALSE);
	      }
	    }

	  when (3)
	    {
#	IN	A, (n)		DB		11011011  nnnnnnnn
#						xxyyyzzz

	    if ($decoder_silent_level == SILENT0)
	      {
	      my $io = sprintf '0x%02X', $dcd_parm0;

	      if ($gen_assembly_code)
		{
		print_3('in', "A, ($io)", "A = In{$io}");
		}
	      else
		{
		$str = io_name($dcd_parm0);
		print_3('in', "A, ($str)", "A = In{$io}");
		}
	      }
	    elsif ($decoder_silent_level == SILENT1)
	      {
	      add_io($dcd_parm0, '', FALSE);
	      }
	    }

	  when (4)
	    {
#	EX	(SP), HL	E3		11100011
#						xxyyyzzz

	    print_3('ex', '(SP), HL', "[SP] <-> L; [SP+1] <-> H");
	    }

	  when (5)
	    {
#	EX	DE, HL		EB		11101011
#						xxyyyzzz

	    print_3('ex', 'DE, HL', "E <-> L; D <-> H");
	    }

	  when (6)
	    {
#	DI			F3		11110011
#						xxyyyzzz

	    print_3('di', '', 'Disable interrupts.');
	    }

	  when (7)
	    {
#	EI			FB		11111011
#						xxyyyzzz

	    print_3('ei', '', 'Enable interrupts.');
	    }
	  } # given ($dcd_instr_y)
	} # $dcd_instr_z == 3

      when (4)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  1  |  0  |  0  |
	# +-----------------------------------------------+

#	CALL	cc, nn		xx nn nn	11ccc100  a7-0 a15-8
#						xxyyyzzz
#		cc: NZ, Z, NC, C, PO, PE, P, M

        if ($decoder_silent_level == SILENT0)
	  {
	  my $addr = ($dcd_parm1 << 8) | $dcd_parm0;
	  my $cond = $conditions[$dcd_instr_y];
	  my $target;

	  $str    = label_name($addr);
	  $target = jump_direction($addr);
	  print_3('call', "$cond->{COND}, $str", "Calls ([--SP] = PC.h; [--SP] = PC.l) if ($cond->{EXPL})$target");
	  }
	} # $dcd_instr_z == 4

      when (5)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  1  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	if ($dcd_instr_q == 0)
	  {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  0  |  1  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

#	PUSH	rp		xx		11rr0101
#						xxppqzzz
#		rp: BC, DE, HL, AF

	  given ($dcd_instr_p)
	    {
	    when (0) { $str = '[--SP] = B; [--SP] = C'; }
	    when (1) { $str = '[--SP] = D; [--SP] = E'; }
	    when (2) { $str = '[--SP] = H; [--SP] = L'; }
	    when (3) { $str = '[--SP] = A; [--SP] = F'; }
	    }

	  print_3('push', $core_registers16b[$dcd_instr_p], $str);
	  }
	else
	  {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  1  |  1  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	  given ($dcd_instr_p)
	    {
	    when (0)
	      {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  0  |  0  |  1  |  1  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

#	CALL	nn		CD nn nn	11001101  a7-0 a15-8
#						xxyyyzzz

	      if ($decoder_silent_level == SILENT0)
		{
		my $addr = ($dcd_parm1 << 8) | $dcd_parm0;
		my $target;

		$str    = label_name($addr);
		$target = jump_direction($addr);
		print_3('call', $str, "Calls ([--SP] = PC.h; [--SP] = PC.l)$target");
		}
	      }

	    when (1)
	      {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  0  |  1  |  1  |  1  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	      DDFD_prefix_decoder('IX');
	      }

	    when (2)
	      {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  1  |  0  |  1  |  1  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	      ED_prefix_decoder();
	      }

	    when (3)
	      {
	#       x                                z
	#  +---/ \---+                   +------/ \------+
	#  | 7     6 |                   | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  1  |  1  |  1  |  1  |  0  |  1  |
	# +-----------------------------------------------+
	#              | 5     4 | | 3 |
	#              +---\ /---+ +\ /+
	#                   p        q

	      DDFD_prefix_decoder('IY');
	      }
	    } # given ($dcd_instr_p)
	  }
	} # $dcd_instr_z == 5

      when (6)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  1  |  1  |  0  |
	# +-----------------------------------------------+

#	ADD	A, #n				11000110  nnnnnnnn
#	ADC	A, #n				11001110  nnnnnnnn
#	SUB	A, #n				11010110  nnnnnnnn
#	SBC	A, #n				11011110  nnnnnnnn
#	AND	A, #n				11100110  nnnnnnnn
#	XOR	A, #n				11101110  nnnnnnnn
#	OR	A, #n				11110110  nnnnnnnn
#	CP	A, #n				11111110  nnnnnnnn
#						xxyyyzzz

	my $i_arith = $DDFD_instr[$dcd_instr_y];
	my $num = sprintf '0x%02X', $dcd_parm0;
	my $char = decode_char($dcd_parm0);
	my $str0 = '';

	$str0 = ' (A = A)'  if ($dcd_instr_y == 0 && $dcd_parm0 == 0);		# ADD	A, 0
	$str0 = ' (A = A)'  if ($dcd_instr_y == 2 && $dcd_parm0 == 0);		# SUB	A, 0
	$str0 = ' (A = A)'  if ($dcd_instr_y == 4 && $dcd_parm0 == 0xFF);	# AND	A, 0xFF
	$str0 = ' (A = ~A)' if ($dcd_instr_y == 5 && $dcd_parm0 == 0xFF);	# XOR	A, 0xFF
	$str0 = ' (A = A)'  if ($dcd_instr_y == 6 && $dcd_parm0 == 0);		# OR	A, 0

	print_3($i_arith->{INSTR}, "A, #$num", "$i_arith->{EXPL} $num$char$str0");
	} # $dcd_instr_z == 6

      when (7)
	{
	#       x              y                 z
	#  +---/ \---+ +------/ \------+ +------/ \------+
	#  | 7     6 | | 5     4     3 | | 2     1     0 |
	# +-----------------------------------------------+
	# |  1  |  1  |  .  |  .  |  .  |  1  |  1  |  1  |
	# +-----------------------------------------------+

#	RST	t		xx		11ttt111
#						xxyyyzzz
#		t: 0 - 7

	$addr = sprintf '0x%04X', $dcd_instr_y * 8;
	$str  = sprintf '0x%02X', $dcd_instr_y * 8;
	print_3('rst', $str, "Calls interrupt: [--SP] = PC.h; [--SP] = PC.l; PC = $addr");
	} # $dcd_instr_z == 7
      } # given ($dcd_instr_z)
    } # $dcd_instr_x == 3
  }

################################################################################
################################################################################

	#
	# Reads the sfrs and bits from the $Line.
	#

sub process_header_line($)
  {
  my $Line = $_[0];

  Log((' ' x $embed_level) . $Line, 5);

  if ($Line =~ /^#\s*include\s+["<]\s*(\S+)\s*[">]$/o)
    {
    $embed_level += 4;
    &read_header("$include_path/$1");
    $embed_level -= 4;
    }
  elsif ($Line =~ /^__sfr\s+__at\s*(?:\(\s*)?0x([[:xdigit:]]+)(?:\s*\))?\s+([\w_]+)/io)
    {
	# __sfr __at (0x80) P0 ;  /* PORT 0 */

    add_ram(hex($1), $2, TRUE);
    }
  elsif ($Line =~ /^SFR\s*\(\s*([\w_]+)\s*,\s*0x([[:xdigit:]]+)\s*\)/io)
    {
	# SFR(P0, 0x80); // Port 0

    add_ram(hex($2), $1, TRUE);
    }
  elsif ($Line =~ /^sfr\s+([\w_]+)\s*=\s*0x([[:xdigit:]]+)/io)
    {
	# sfr P1  = 0x90;

    add_ram(hex($2), $1, TRUE);
    }
  }

#-------------------------------------------------------------------------------

	#
	# Reads in a MCU.h file.
	#

sub read_header($)
  {
  my $Header = $_[0];
  my ($fh, $pre_comment, $comment, $line_number);
  my $head;

  if (! open($fh, '<', $Header))
    {
    print STDERR "$PROGRAM: Could not open. -> \"$Header\"\n";
    exit(1);
    }

  $head = ' ' x $embed_level;

  Log("${head}read_header($Header) >>>>", 5);

  $comment = FALSE;
  $line_number = 1;
  while (<$fh>)
    {
    chomp;
    s/\r$//o;			# '\r'

	# Filters off the C comments.

    s/\/\*.*\*\///o;		# /* ... */
    s/\/\/.*$//o;		# // ...
    s/^\s*|\s*$//go;

    if (/\/\*/o)		# /*
      {
      $pre_comment = TRUE;
      s/\s*\/\*.*$//o;
      }
    elsif (/\*\//o)		# */
      {
      $pre_comment = FALSE;
      $comment = FALSE;
      s/^.*\*\/\s*//o;
      }

    if ($comment)
      {
      ++$line_number;
      next;
      }

    $comment = $pre_comment if ($pre_comment);

    if (/^\s*$/o)
      {
      ++$line_number;
      next;
      }

    run_preprocessor($Header, \&process_header_line, $_, $line_number);
    ++$line_number;
    } # while (<$fh>)

  Log("${head}<<<< read_header($Header)", 5);
  close($fh);
  }

#-------------------------------------------------------------------------------

	#
	# Determines size of the $dcd_instr.
	#

sub determine_instr_size()
  {
  my $instr;
  my $size = $instruction_sizes_[$dcd_instr];

  return $size if ($size >= 0);

  $instr = $rom[$dcd_address + 1];

  if ($size == IPREFIX_DD || $size == IPREFIX_FD)
    {
    return $instruction_sizes_DDFD[$instr];
    }
  elsif ($size == IPREFIX_ED)
    {
    return $instruction_sizes_ED[$instr];
    }
  else
    {
    return 0;
    }
  }

#-------------------------------------------------------------------------------

	#
	# Among the blocks stows description of an instruction.
	#

sub add_instr_block($)
  {
  my $Address = $_[0];
  my ($instr_size, $invalid);

  $dcd_address = $Address;
  $dcd_instr   = $rom[$dcd_address];
  $invalid     = FALSE;

  $instr_size  = determine_instr_size();

  if ($instr_size == 0)
    {
    $instr_size = 1;
    add_block($Address, BLOCK_CONST, $instr_size, BL_TYPE_NONE, '');
    }
  else
    {
    if ($instr_size == 1)
      {
      $invalid = TRUE if ($dcd_instr == EMPTY);
      }

    if ($instr_size == 2)
      {
      $invalid = TRUE if ($rom[$dcd_address + 1] == EMPTY);
      }

    if ($instr_size == 3)
      {
      $invalid = TRUE if ($rom[$dcd_address + 2] == EMPTY);
      }

    if ($instr_size == 4)
      {
      $invalid = TRUE if ($rom[$dcd_address + 3] == EMPTY);
      }

    if ($invalid)
      {
      add_block($Address, BLOCK_CONST, $instr_size, BL_TYPE_NONE, '');
      }
    else
      {
      add_block($Address, BLOCK_INSTR, $instr_size, BL_TYPE_NONE, '');
      }
    }

  return $instr_size;
  }

#-------------------------------------------------------------------------------

	#
	# Splits the program into small blocks.
	#

sub split_code_to_blocks()
  {
  my ($i, $instr);
  my ($is_empty, $empty_begin);
  my ($is_const, $const_begin);

  $is_empty = FALSE;
  $is_const = FALSE;

  for ($i = 0; $i < $rom_size; )
    {
    $instr = $rom[$i];

    if ($instr == EMPTY)
      {
      if (! $is_empty)
        {
	# The begin of the empty section.

	if ($is_const)
	  {
	# The end of the constant section.

	  add_block($const_begin, BLOCK_CONST, $i - $const_begin, BL_TYPE_NONE, '');
	  $is_const = FALSE;
	  }

	$empty_begin = $i;
	$is_empty = TRUE;
	}

      ++$i;
      } # if ($instr == EMPTY)
    elsif (is_constant($i))
      {
      if (! $is_const)
	{
	if ($is_empty)
	  {
	# The end of the empty section.

	  add_block($empty_begin, BLOCK_EMPTY, $i - $empty_begin, BL_TYPE_NONE, '');
	  $is_empty = FALSE;
	  }

	$const_begin = $i;
	$is_const = TRUE;
	}

      ++$i;
      } # elsif (is_constant($i))
    else
      {
      if ($is_const)
	{
	# The end of the constant section.

	add_block($const_begin, BLOCK_CONST, $i - $const_begin, BL_TYPE_NONE, '');
	$is_const = FALSE;
	}

      if ($is_empty)
	{
	# The end of the empty section.

	add_block($empty_begin, BLOCK_EMPTY, $i - $empty_begin, BL_TYPE_NONE, '');
	$is_empty = FALSE;
	}

      $i += add_instr_block($i);
      }
    } # for ($i = 0; $i < $rom_size; )

  if ($is_const)
    {
    add_block($const_begin, BLOCK_CONST, $i - $const_begin, BL_TYPE_NONE, '');
    }

  if ($is_empty)
    {
    add_block($empty_begin, BLOCK_EMPTY, $i - $empty_begin, BL_TYPE_NONE, '');
    }
  }

#-------------------------------------------------------------------------------

	#
	# Previously assess the code.
	#

sub preliminary_survey($)
  {
  $decoder_silent_level = $_[0];
  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    my $block = \%{$blocks_by_address{$_}};

    next if ($block->{TYPE} != BLOCK_INSTR);

    instruction_decoder($_, $block);
    }
  }

#-------------------------------------------------------------------------------

	#
	# Finds address of branchs and procedures.
	#

sub find_labels_in_code()
  {
  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    my $block = \%{$blocks_by_address{$_}};

    next if ($block->{TYPE} != BLOCK_INSTR);

    label_finder($_, $block);
    }
  }

#-------------------------------------------------------------------------------

	#
	# Finds lost address of branchs and procedures.
	#

sub find_lost_labels_in_code()
  {
  my ($block, $prev_block, $prev_addr, $label, $instr);

  $prev_addr  = EMPTY;
  $prev_block = undef;
  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    $block = \%{$blocks_by_address{$_}};

    last if ($block->{TYPE} == BLOCK_RAM);
    next if ($block->{TYPE} != BLOCK_INSTR);

    if ($prev_addr != EMPTY)
      {
      $instr = $rom[$prev_addr];
      $label = $block->{LABEL};

      if (defined($label) && $label->{TYPE} == BL_TYPE_NONE)
	{
#	if ($instr == INST_RET || $instr == INST_RETI)
	if ($instr == INST_RET)
	  {
	  Log(sprintf("Lost function label at the 0x%04X address.", $_), 5);
	  add_func_label($_, '', TRUE);
	  }
	elsif ($instr == INST_JP || $instr == INST_JR || $instr == INST_JP_HL)
	  {
	  Log(sprintf("Lost jump label at the 0x%04X address.", $_), 5);
	  add_jump_label($_, '', BL_TYPE_LABEL, EMPTY, TRUE);
	  }
	}
      }

    $prev_addr  = $_;
    $prev_block = $block;
    }
  }

#-------------------------------------------------------------------------------

	#
	# Jump tables looking for in the code.
	#

sub recognize_jump_tables_in_code()
  {
  my @blocks = ((undef) x 5);
  my @instrs = ((EMPTY) x 5);
  my ($addr);

  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    shift(@instrs);
    push(@instrs, $rom[$_]);

    shift(@blocks);
    push(@blocks, \%{$blocks_by_address{$_}});

    next if (! defined($blocks[0]) || ! defined($blocks[4]));
    next if ($blocks[0]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[1]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[2]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[3]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[4]->{TYPE} != BLOCK_INSTR);

    if ($blocks[0]->{SIZE} == 3 && $instrs[0] == INST_LD_HL &&
	$blocks[1]->{SIZE} == 1 && $instrs[1] == INST_ADD_HL_DE &&
	$blocks[2]->{SIZE} == 1 && $instrs[2] == INST_ADD_HL_DE &&
	$blocks[3]->{SIZE} == 1 &&
				   (($instrs[3] == INST_ADD_HL_DE && $blocks[4]->{SIZE} == 1 && $instrs[4] == INST_JP_HL) ||
				    $instrs[3] == INST_JP_HL))
      {
=back
0x019D: 21 A4 01	ld	HL, #0x01A4				; HL = 0x01A4
0x01A0: 19		add	HL, DE					; HL += DE
0x01A1: 19		add	HL, DE					; HL += DE
0x01A2: 19		add	HL, DE					; HL += DE
0x01A3: E9		jp	(HL)					; Jumps to value of HL.

0x01A4: C3 D4 01	jp	Label_021				; Jumps (forward) hither: 0x01D4

----------------------------------------------------------------------------------------------------

0x019D: 21 A4 01	ld	HL, #0x01A3				; HL = 0x01A4
0x01A0: 19		add	HL, DE					; HL += DE
0x01A1: 19		add	HL, DE					; HL += DE
0x01A2: E9		jp	(HL)					; Jumps to value of HL.

0x01A3: 18 2F		jr	Label_021				; Jumps (forward) hither: 0x01D4
=cut

      $addr = ($rom[$blocks[0]->{ADDR} + 2] << 8) | $rom[$blocks[0]->{ADDR} + 1];
      add_jump_label($addr, '', BL_TYPE_JTABLE, EMPTY, FALSE);
      }
    }
  }

#-------------------------------------------------------------------------------

	#
	# Prints the global symbols.
	#

sub emit_globals($)
  {
  my $Assembly_mode = $_[0];
  my ($label, $cnt0, $cnt1, $str0, $str1);

  return if (! scalar(keys(%labels_by_address)));

  print ";$border0\n;\tPublic labels\n;$border0\n\n";

  if ($Assembly_mode)
    {
    foreach (sort {$a <=> $b} keys(%labels_by_address))
      {
      $label = $labels_by_address{$_};

      next if ($label->{TYPE} != BL_TYPE_SUB);

      print "\t.globl\t$label->{NAME}\n";
      }
    }
  else
    {
    foreach (sort {$a <=> $b} keys(%labels_by_address))
      {
      $label = $labels_by_address{$_};

      next if ($label->{TYPE} != BL_TYPE_SUB);

      $str0 = sprintf "0x%04X", $_;
      $cnt0 = sprintf "%3u", $label->{CALL_COUNT};
      $cnt1 = sprintf "%3u", $label->{JUMP_COUNT};
      $str1 = ($label->{CALL_COUNT} || $label->{JUMP_COUNT}) ? "calls: $cnt0, jumps: $cnt1" : 'not used';
      print "${str0}:\t" . align($label->{NAME}, STAT_ALIGN_SIZE) . "($str1)\n";
      }
    }

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints the registers (variables).
	#

sub emit_ram_data()
  {
  my ($block, $first, $name, $next_addr, $size, $cnt, $str0, $str1);

  return if (! scalar(keys(%ram_blocks_by_address)));

  print ";$border0\n;\tRAM data\n;$border0\n\n";

  $next_addr = EMPTY;
  foreach (sort {$a <=> $b} keys(%ram_blocks_by_address))
    {
    $block = $blocks_by_address{$_};

    if ($block->{TYPE} != BLOCK_RAM)
      {
      $next_addr = EMPTY;
      next;
      }

    next if ($next_addr != EMPTY && $_ < $next_addr);

    $str0 = sprintf "0x%04X", $_;
    $cnt  = sprintf "%3u", $block->{REF_COUNT};
    $str1 = ($block->{REF_COUNT}) ? "used $cnt times" : 'not used';
    $name = $ram_names_by_address{$_};

    if (defined($name) && $name ne '')
      {
      $cnt = sprintf "%5u", $block->{SIZE};
      print "${str0}:\t" . align($name, STAT_ALIGN_SIZE) . "($cnt bytes) ($str1)\n";
      $next_addr = $_ + $block->{SIZE};
      }
    else
      {
      if ($map_readed)
	{
	print "${str0}:\t" . align("variable_$str0", STAT_ALIGN_SIZE) . "(  1 bytes) ($str1)\n";
	}
      else
	{
	print "${str0}:\t" . align("variable_$str0", STAT_ALIGN_SIZE) . "($str1)\n";
	}

      $next_addr = $_ + 1;
      }
    } # foreach (sort {$a <=> $b} keys(%ram_blocks_by_address))

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints I/O ports.
	#

sub emit_io_ports()
  {
  my ($io, $cnt, $str0, $str1);

  return if (! scalar(keys(%io_by_address)));

  print ";$border0\n;\tI/O ports\n;$border0\n\n";

  foreach (sort {$a <=> $b} keys(%io_by_address))
    {
    $io = $io_by_address{$_};

    $str0 = sprintf "0x%02X", $_;
    $cnt  = sprintf "%3u", $io->{REF_COUNT};
    $str1 = ($io->{REF_COUNT}) ? "used $cnt times" : 'not used';

    if ($io->{NAME} ne '')
      {
      print "${str0}:\t" . align($io->{NAME}, STAT_ALIGN_SIZE) . "($str1)\n";
      }
    else
      {
      print "${str0}:\t" . align("port_$str0", STAT_ALIGN_SIZE) . "($str1)\n";
      }
    } # foreach (sort {$a <=> $b} keys(%io_by_address))

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints a label belonging to the $Address.
	#

sub print_label($)
  {
  my $Address = $_[0];
  my ($label, $type);

  $label = $labels_by_address{$Address};

  return FALSE if (! defined($label) || $label->{TYPE} == BL_TYPE_NONE);

  $type = $label->{TYPE};

  print "\n;$border0\n" if ($type == BL_TYPE_SUB);

  printf "\n$label->{NAME}:\n\n";
  $label->{PRINTED} = TRUE;
  $prev_is_jump = FALSE;
  return TRUE;
  }

#-------------------------------------------------------------------------------

	#
	# Prints a variable belonging to the $Address.
	#

sub print_variable($$)
  {
  my ($Address, $BlockRef) = @_;
  my ($name, $size, $str0, $str1);

  $size = $BlockRef->{SIZE};

  return if (! $size);

  $name = $ram_names_by_address{$Address};

  return if (! defined($name) || $name eq '');

  $str0 = sprintf "0x%04X", $Address;

  given ($size)
    {
    when (1)	{ $str1 = '.db'; }
    when (2)	{ $str1 = '.dw'; }
    when (4)	{ $str1 = '.dd'; }
    when (8)	{ $str1 = '.dq'; }
    when (10)	{ $str1 = '.dt'; }
    default	{ $str1 = '.db'; }
    }

  if ($gen_assembly_code)
    {
    print "$name:\n";
    $str0 = "\t$str1\t?";
    }
  else
    {
    $str0 = align("$str0:$name", RAM_ALIGN_SIZE) . "$str1\t?";
    }

  print align($str0, RAM_ALIGN_SIZE + 1 + EXPL_ALIGN_SIZE) . "; $size bytes\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints a table of constants.
	#

sub print_constants($$)
  {
  my ($Address, $BlockRef) = @_;
  my ($size, $i, $len, $frag, $byte, $spc, $col, $brd);
  my ($left_align, $right_align);
  my @constants;
  my @line;

  $size = $BlockRef->{SIZE};

  return if (! $size);

  $prev_is_jump = FALSE;
  $col = '    ';

  if ($gen_assembly_code)
    {
    print ";$table_border\n;\t\t $table_header  |  $table_header  |\n;$table_border\n";
    $brd = ' ';
    }
  else
    {
    print "$table_border\n|          |  $table_header  |  $table_header  |\n$table_border\n";
    $brd = '|';
    }

  @constants = @rom[$Address .. ($Address + $size - 1)];
  $i = 0;
  while (TRUE)
    {
    $len = $size - $i;

    last if (! $len);

    $len = TBL_COLUMNS if ($len > TBL_COLUMNS);

    if ($gen_assembly_code)
      {
      print "\t.db\t";
      }
    else
      {
      printf "$brd  0x%04X  $brd ", $Address;
      }

    if (($spc = $Address % TBL_COLUMNS))
      {
      $frag = TBL_COLUMNS - $spc;
      $len  = $frag if ($len > $frag);
      }

    $left_align  = $col x $spc;
    $right_align = $col x (TBL_COLUMNS - $spc - $len);
    @line = @constants[$i .. ($i + $len - 1)];
    $Address += $len;
    $i       += $len;

    print " $left_align" . join(' ', map { sprintf("%02X ", $_ & 0xFF); } @line);

    print "$right_align $brd $left_align " .
	  join(' ', map {
			sprintf((($_ < ord(' ') || $_ >= 0x7F) ? "%02X " : "'%c'"), $_ & 0xFF);
			} @line) . "$right_align $brd\n";
    } # while (TRUE)

  print (($gen_assembly_code) ? ";$table_border\n" : "$table_border\n");
  $prev_is_jump = FALSE;
  }

#-------------------------------------------------------------------------------

	#
	# Disassembly contents of $blocks_by_address array.
	#

sub disassembler()
  {
  my ($sname, $prev_block_type, $ref);

  $prev_is_jump = FALSE;
  $decoder_silent_level = SILENT0;

  $table_header = join('  ', map { sprintf '%02X', $_ } (0 .. (TBL_COLUMNS - 1)));

  if ($gen_assembly_code)
    {
    $table_border = ('-' x (TBL_COLUMNS * 4 + 16)) . '+' . ('-' x (TBL_COLUMNS * 4 + 2)) . '+';
    }
  else
    {
    $table_border = '+' . ('-' x 10) . '+' . ('-' x (TBL_COLUMNS * 4 + 2)) . '+' . ('-' x (TBL_COLUMNS * 4 + 2)) . '+';
    }

  print "\n";

  if ($gen_assembly_code)
    {
    emit_globals(TRUE);
    print ";$border0\n;\tCode\n;$border0\n\n\t.area\tCODE\t(CODE)\n\n";
    }
  else
    {
    emit_globals(FALSE);
    emit_ram_data();
    emit_io_ports();
    print ";$border0\n";
    }

  $prev_block_type = EMPTY;
  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    $ref = $blocks_by_address{$_};

    if ($ref->{TYPE} == BLOCK_INSTR)
      {
      print_label($_);
      print "\n" if ($prev_is_jump);

      instruction_decoder($_, $ref);
      $prev_block_type = BLOCK_INSTR;
      }
    elsif ($ref->{TYPE} == BLOCK_RAM)
      {
      print "\n;$border0\n\n" if ($prev_block_type != BLOCK_RAM);

      print_variable($_, $ref);
      $prev_block_type = BLOCK_RAM;
      }
    elsif ($ref->{TYPE} == BLOCK_CONST)
      {
      print "\n;$border0\n" if ($prev_block_type != BLOCK_CONST);

      print_label($_);
      print "\n" if ($prev_is_jump);

      print_constants($_, $ref);
      $prev_block_type = BLOCK_CONST;
      }
    elsif ($ref->{TYPE} == BLOCK_EMPTY)
      {
      my $next_block = $_ + $ref->{SIZE};

      print "\n;$border0\n" if ($prev_block_type != BLOCK_EMPTY);

      if (! $gen_assembly_code)
	{
	printf("\n0x%04X: -- -- --\n  ....  -- -- --\n0x%04X: -- -- --\n", $_, $next_block - 1);
	}
      elsif ($next_block <= $rom_size)
	{
	# Skip the empty code space.

	printf "\n\t.ds\t%u\n", $ref->{SIZE};
	}

      $prev_block_type = BLOCK_EMPTY;
      }
    } # foreach (sort {$a <=> $b} keys(%blocks_by_address))
  }

#-------------------------------------------------------------------------------

	#
	# If there are datas in the code, it is possible that some labels will
	# be lost. This procedure prints them.
	#

sub print_hidden_labels()
  {
  foreach (sort {$a <=> $b} keys(%labels_by_address))
    {
    my $label = $labels_by_address{$_};

    print STDERR "The label: $label->{NAME} is hidden!\n" if (! $label->{PRINTED});
    }
  }

################################################################################
################################################################################

sub usage()
  {
  print <<EOT;
Usage: $PROGRAM [options] <hex file>

    Options are:

	-M|--mcu <header.h>

	    Header file of the MCU.

	-I|--include <path to header>

	    Path of the header files of Z80 MCUs. (Default: $default_include_path)

	--map-file <file.map>

	    The map file belonging to the input hex file. (optional)

	-r|--rom-size <size of program memory>

EOT
;
  printf "\t    Defines size of the program memory. (Default %u bytes.)\n", Z80_ROM_SIZE;
  print <<EOT;

	--const-area <start address> <end address>

	    Designates a constant area (jumptables, texts, etc.), where data is
	    stored happen. The option may be given more times, that to select
	    more areas at the same time. (optional)

	-as|--assembly-source

	    Generates the assembly source file. (Eliminates before the instructions
	    visible address and hex codes.) Emits global symbol table, etc.

	-fl|--find-lost-labels

	    Finds the "lost" labels. These may be found such in program parts,
	    which are directly not get call.

	--name-list <list_file>

	    The file contains list of names. They may be: Names of variables and
	    names of labels. For example:

		[IO]
		0x21:keyboard_io
		..
		..
		..
		[RAM]
		0x8021:ram_variable
		..
		..
		..
		[ROM]
		0x05FC:function_or_label
		..
		..
		..

	    The contents of list override the names from map file.

	-ne|--no-explanations

	    Eliminates after the instructions visible explaining texts.

	-v <level> or --verbose <level>

	    It provides information on from the own operation.
	    Possible value of the level between 0 and 10. (default: 0)

	-h|--help

            This text.
EOT
;
  }

################################################################################
################################################################################
################################################################################

foreach (@default_paths)
  {
  if (-d $_)
    {
    $default_include_path = $_;
    last;
    }
  }

if (! @ARGV)
  {
  usage();
  exit(1);
  }

for (my $i = 0; $i < @ARGV; )
  {
  my $opt = $ARGV[$i++];

  given ($opt)
    {
    when (/^-(r|-rom-size)$/o)
      {
      param_exist($opt, $i);
      $rom_size = str2int($ARGV[$i++]);

      if ($rom_size < 1024)
	{
	printf STDERR "$PROGRAM: Code size of the Z80 family greater than 1024 bytes!\n";
	exit(1);
	}
      elsif ($rom_size > Z80_ROM_SIZE)
	{
	printf STDERR "$PROGRAM: Code size of the Z80 family not greater %u bytes!\n", Z80_ROM_SIZE;
	exit(1);
	}
      }

    when (/^--const-area$/o)
      {
      my ($start, $end);

      param_exist($opt, $i);
      $start = str2int($ARGV[$i++]);

      param_exist($opt, $i);
      $end = str2int($ARGV[$i++]);

      if ($start > $end)
	{
	my $t = $start;

	$start = $end;
	$end = $t;
	}
      elsif ($start == $end)
	{
	$start = Z80_ROM_SIZE - 1;
	$end   = Z80_ROM_SIZE - 1;
	}

      add_const_area($start, $end) if ($start < $end);
      } # when (/^--const-area$/o)

    when (/^-(I|-include)$/o)
      {
      param_exist($opt, $i);
      $include_path = $ARGV[$i++];
      }

    when (/^-(M|-mcu)$/o)
      {
      param_exist($opt, $i);
      $header_file = $ARGV[$i++];
      }

    when (/^--map-file$/o)
      {
      param_exist($opt, $i);
      $map_file = $ARGV[$i++];
      }

    when (/^-(as|-assembly-source)$/o)
      {
      $gen_assembly_code = TRUE;
      }

    when (/^-(fl|-find-lost-labels)$/o)
      {
      $find_lost_labels = TRUE;
      }

    when (/^--name-list$/o)
      {
      param_exist($opt, $i);
      $name_list = $ARGV[$i++];
      }

    when (/^-(ne|-no-explanations)$/o)
      {
      $no_explanations = TRUE;
      }

    when (/^-(v|-verbose)$/o)
      {
      param_exist($opt, $i);
      $verbose = int($ARGV[$i++]);
      $verbose = 0 if (! defined($verbose) || $verbose < 0);
      $verbose = 10 if ($verbose > 10);
      }

    when (/^-(h|-help)$/o)
      {
      usage();
      exit(0);
      }

    default
      {
      if ($hex_file eq '')
	{
	$hex_file = $opt;
	}
      else
	{
	print STDERR "$PROGRAM: We already have the source file name: $hex_file.\n";
	exit(1);
	}
      }
    } # given ($opt)
  } # for (my $i = 0; $i < @ARGV; )

$include_path = $default_include_path if ($include_path eq '');

if ($hex_file eq '')
  {
  print STDERR "$PROGRAM: What do you have to disassembled?\n";
  exit(1);
  }

is_file_ok($hex_file);

init_mem(0, $rom_size - 1);
read_hex($hex_file);

if ($header_file ne '')
  {
  is_file_ok("$include_path/$header_file");
  reset_preprocessor();
  $embed_level = 0;
  read_header("$include_path/$header_file");
  }

if ($map_file eq '')
  {
  ($map_file) = ($hex_file =~ /^(.+)\.hex$/io);
  $map_file .= '.map';
  }

$map_file = '' if (! -e $map_file);

is_file_ok($name_list) if ($name_list ne '');

###################################

read_map_file();
read_name_list();
split_code_to_blocks();
recognize_jump_tables_in_code();
preliminary_survey(SILENT1);
find_labels_in_code();
find_lost_labels_in_code() if ($find_lost_labels);
add_names_labels();
fix_multi_byte_variables();
fix_io_names();
disassembler();
print_hidden_labels() if ($verbose > 2);
