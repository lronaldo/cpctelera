#!/usr/bin/perl -w

=back

  Copyright (C) 2013, Molnar Karoly <molnarkaroly@users.sf.net>

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
    contains MCS51 instructions.

    Proposal for use: ./mcs51-disasm.pl -M 8052.h -fl -rj program.hex > program.dasm

	or	./mcs51-disasm.pl -M 8052.h -fl -rj -as program.hex > program.asm

	or	./mcs51-disasm.pl -M 8052.h -fl -rj -as -hc program.hex > program.asm


    Warning! This program is not able to find all variable. Especially them not, whose
    can be found in the indirect or external RAM.

  $Id: mcs51-disasm.pl 9068 2014-09-04 12:57:30Z molnarkaroly $
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

use constant MCS51_ROM_SIZE		=> 0x10000;

################################################################################

my $PROGRAM = 'mcs51-disasm.pl';

my $border0 = ('-' x 79);
my $border1 = ('#' x 79);
my $border2 = ('.' x 39);

my @default_paths =
  (
  '/usr/share/sdcc/include/mcs51',
  '/usr/local/share/sdcc/include/mcs51'
  );

my $default_include_path = '';
my $include_path	 = '';
my $hex_file		 = '';
my $map_file 		 = '';
my $map_readed		 = FALSE;
my $header_file		 = '';
my $name_list		 = '';

my $verbose		  = 0;
my $hex_constant	  = FALSE;
my $gen_assembly_code	  = FALSE;
my $no_explanations	  = FALSE;
my $recognize_jump_tables = FALSE;
my $find_lost_labels	  = FALSE;

my @rom = ();
my $rom_size = MCS51_ROM_SIZE;
my %const_areas_by_address  = ();	# From the command line parameters.

my %const_blocks_by_address = ();

=back
	The structure of one element of the %sfr_by_address hash:

	{
	NAME	  => '',
	REF_COUNT => 0
	}
=cut

my %sfr_by_address	  = ();
my %sfr_by_names	  = ();

=back
	The structure of one element of the %sfr_bit_by_address hash:

	{
	NAME	  => '',
	REF_COUNT => 0
	}
=cut

my %sfr_bit_by_address	  = ();

my %used_banks		  = ();

=back
	The structure of one element of the %bit_by_address hash:

	{
	NAME	  => '',
	REF_COUNT => 0
	}
=cut

my %bit_by_address	  = ();

=back
	The structure of one element of the %ram_by_address hash:

	{
	TYPE	  => 0,
	NAME	  => '',
	SIZE	  => 0,
	REF_COUNT => 0
	}
=cut

use constant RAM_TYPE_DIR => 0;
use constant RAM_TYPE_IND => 1;

my %ram_by_address	  = ();
my %ram_names_by_address  = ();

=back
	The structure of one element of the %xram_by_address hash:

	{
	NAME	  => '',
	SIZE	  => 0,
	REF_COUNT => 0
	}
=cut

my %xram_by_address	  = ();
my %xram_names_by_address = ();

my $stack_start		  = -1;
my $stack_size		  = 0;

	# Sizes of the instructions.

my @instruction_sizes =
  (
  1, 2, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  3, 2, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  3, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  3, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 1, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 1, 1, 3, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
  1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  );

use constant BANK_LAST_ADDR	=> 0x1F;
use constant BIT_LAST_ADDR	=> 0x7F;
use constant RAM_MAX_ADDR	=> 0xFF;
use constant RAM_LAST_DIR_ADDR	=> 0x7F;

use constant SP  => 0x81;
use constant DPL => 0x82;
use constant DPH => 0x83;
use constant PSW => 0xD0;
use constant ACC => 0xE0;

use constant INST_AJMP			=> 0x01;
use constant INST_LJMP			=> 0x02;
use constant INST_ACALL			=> 0x11;
use constant INST_LCALL			=> 0x12;
use constant INST_SJMP			=> 0x80;
use constant INST_RET			=> 0x22;
use constant INST_RETI			=> 0x32;
use constant INST_ADD_A_DATA		=> 0x24;
use constant INST_JMP_A_DPTR		=> 0x73;
use constant INST_MOV_DIRECT_DATA	=> 0x75;
use constant INST_MOVC_A_APC		=> 0x83;
use constant INST_MOV_DIRECT_DIRECT	=> 0x85;
use constant INST_MOV_DPTR_DATA 	=> 0x90;
use constant INST_MOVC_A_DPTR		=> 0x93;
use constant INST_PUSH_DIRECT		=> 0xC0;
use constant INST_XCH_A_DIRECT		=> 0xC5;
use constant INST_POP_DIRECT		=> 0xD0;
use constant INST_CLR_A			=> 0xE4;
use constant INST_MOV_A_DIRECT		=> 0xE5;
use constant INST_MOV_A_Ri		=> 0xE6;
use constant INST_MOV_A_Rn		=> 0xE8;
use constant INST_MOV_DIRECT_A		=> 0xF5;
use constant INST_CJNE_A_DATA		=> 0xB4;
use constant INST_CJNE_A_DIRECT		=> 0xB5;
use constant INST_CJNE__Ri_DATA		=> 0xB6;
use constant INST_CJNE_Rn_DATA		=> 0xB8;
use constant INST_DJNZ_Rn		=> 0xD8;
use constant INST_DJNZ_DIRECT		=> 0xD5;
use constant INST_JBC_BIT		=> 0x10;
use constant INST_JB_BIT		=> 0x20;
use constant INST_JNB_BIT		=> 0x30;
use constant INST_JC			=> 0x40;
use constant INST_JNC			=> 0x50;
use constant INST_JZ			=> 0x60;
use constant INST_JNZ			=> 0x70;

use constant LJMP_SIZE => 3;

my $DPTR;
my @R_regs;

my $prev_is_jump;

use constant SILENT0 => 0;
use constant SILENT1 => 1;
use constant SILENT2 => 2;

my $decoder_silent_level;

use constant EXPL_ALIGN_SIZE => 5;
use constant STAT_ALIGN_SIZE => 6;
use constant TBL_COLUMNS     => 8;

=back
	The structure of one element of the %blocks_by_address hash:

	{
	TYPE  => 0,
	ADDR  => 0,
	SIZE  => 0,
	LABEL => {
		 TYPE       => 0,
		 NAME       => '',
		 PRINTED    => FALSE,
		 CALL_COUNT => 0,
		 JUMP_COUNT => 0
		 }
	}
=cut

use constant BLOCK_INSTR    => 0;
use constant BLOCK_CONST    => 1;
use constant BLOCK_JTABLE   => 2;
use constant BLOCK_EMPTY    => 3;
use constant BLOCK_DISABLED => 4;

use constant BL_TYPE_NONE   => -1;
use constant BL_TYPE_SUB    =>  0;
use constant BL_TYPE_LABEL  =>  1;
use constant BL_TYPE_JTABLE =>  2;
use constant BL_TYPE_JLABEL =>  3;
use constant BL_TYPE_CONST  =>  4;

my %label_names =
  (
  eval BL_TYPE_SUB    => 'Function_',
  eval BL_TYPE_LABEL  => 'Label_',
  eval BL_TYPE_JTABLE => 'Jumptable_',
  eval BL_TYPE_JLABEL => 'JTlabel_',
  eval BL_TYPE_CONST  => 'Constant_'
  );

my %empty_blocks_by_address = ();
my %blocks_by_address = ();
my %labels_by_address = ();
my $max_label_addr = 0;

my %indirect_addr_instr =
  (
  0x06 => TRUE,
  0x16 => TRUE,
  0x26 => TRUE,
  0x36 => TRUE,
  0x46 => TRUE,
  0x56 => TRUE,
  0x66 => TRUE,
  0x76 => TRUE,
  0x86 => TRUE,
  0x96 => TRUE,
  0xA6 => TRUE,
  0xB6 => TRUE,
  0xC6 => TRUE,
  0xD6 => TRUE,
  0xE6 => TRUE,
  0xF6 => TRUE
  );

my %interrupts_by_address =
  (
  0x0000 => 'System_init',
  0x0003 => 'Int0_interrupt',
  0x000B => 'Timer0_interrupt',
  0x0013 => 'Int1_interrupt',
  0x001B => 'Timer1_interrupt',
  0x0023 => 'Uart_interrupt',
  0x002B => 'Timer2_interrupt',
  0x0033 => 'Int2_interrupt',
  0x003B => 'Int3_interrupt'
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

my $dcd_address    = 0;
my $dcd_instr_size = 0;
my $dcd_instr	   = 0;
my $dcd_parm0	   = 0;
my $dcd_parm1	   = 0;
my $dcd_Ri_regs    = 0;
my $dcd_Ri_name    = '';
my $dcd_Rn_regs    = 0;
my $dcd_Rn_name    = '';

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
  my $Text = $_[0];
  my $al   = $_[1] - int(length($Text) / TAB_LENGTH);

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
				   TYPE  => $Type,
				   ADDR  => $Address,
				   SIZE  => $Size,
				   LABEL => $label
				   };

    if ($Type == BLOCK_INSTR)
      {
      if ($LabelType != BL_TYPE_NONE)
	{
	$labels_by_address{$Address} = $label;
	$max_label_addr = $Address if ($max_label_addr < $Address);
	}
      }
    elsif ($Type == BLOCK_CONST || $Type == BLOCK_JTABLE)
      {
      if ($LabelType != BL_TYPE_NONE)
	{
	$labels_by_address{$Address} = $label;
	$max_label_addr = $Address if ($max_label_addr < $Address);
	}

      $const_blocks_by_address{$Address} = $end if ($Size > 0);
      }
    elsif ($Type == BLOCK_EMPTY)
      {
	# At empty area, can not be label.

      $label->{TYPE} = BL_TYPE_NONE;
      $label->{NAME} = '';
      $empty_blocks_by_address{$Address} = $end if ($Size > 0);
      }
    else
      {
      printf STDERR "add_block(0x%04X): Unknown block type!\n", $Address;
      exit(1);
      }
    } # if (! defined($blocks_by_address{$Address}))
  else
    {
    $block = $blocks_by_address{$Address};
    $label = $block->{LABEL};
    $block->{TYPE} = $Type;
    $block->{SIZE} = $Size 	if ($Size > 0);
    $label->{NAME} = $LabelName	if ($label->{NAME} eq '' && $LabelName ne '');

    if ($Type == BLOCK_INSTR)
      {
      if ($LabelType != BL_TYPE_NONE)
	{
	$label->{TYPE} = $LabelType if ($label->{TYPE} != BL_TYPE_JLABEL);
	$labels_by_address{$Address} = $label;
	$max_label_addr = $Address if ($max_label_addr < $Address);
	}
      }
    elsif ($Type == BLOCK_CONST || $Type == BLOCK_JTABLE)
      {
      if ($LabelType != BL_TYPE_NONE)
	{
	$label->{TYPE} = $LabelType;
	$labels_by_address{$Address} = $label;
	$max_label_addr = $Address if ($max_label_addr < $Address);
	}

      $const_blocks_by_address{$Address} = $end if ($Size > 0);
      }
    elsif ($Type == BLOCK_EMPTY)
      {
	# At empty area, can not be label.

      $label->{TYPE} = BL_TYPE_NONE;
      $label->{NAME} = '';
      $empty_blocks_by_address{$Address} = $end if ($Size > 0);
      }
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
  ++$label->{CALL_COUNT} if (! $Map_mode);
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
  ++$label->{JUMP_COUNT} if (! $Map_mode);
  }

#-------------------------------------------------------------------------------

	#
	# Store a bit or sbit name from bit area.
	#

sub add_bit($$$)
  {
  my ($Address, $Name, $Map_mode) = @_;
  my $bit;

  if ($Address > BIT_LAST_ADDR)
    {
    if (! defined($bit = $sfr_bit_by_address{$Address}))
      {
      $sfr_bit_by_address{$Address} = {
				      NAME	=> $Name,
				      REF_COUNT => ($Map_mode) ? 0 : 1
				      };
      }
    else
      {
      if ($Name ne '' && $bit->{NAME} ne $Name)
	{
	Log(sprintf("Warning, the address: 0x%02X already busy by the $bit->{NAME} bit.", $Address), 2);
	}
      else
	{
	++$bit->{REF_COUNT} if (! $Map_mode);
	}
      }
    }
  else
    {
    if (! defined($bit = $bit_by_address{$Address}))
      {
      $bit_by_address{$Address} = {
				  NAME	    => $Name,
				  REF_COUNT => ($Map_mode) ? 0 : 1
				  };
      }
    else
      {
      ++$bit->{REF_COUNT} if (! $Map_mode);
      }
    }
  }

#-------------------------------------------------------------------------------

	#
	# Store a sfr or variable name.
	#

sub add_ram($$$$)
  {
  my ($Address, $Name, $Type, $Map_mode) = @_;
  my $ram;

  return if ($Address == EMPTY);

  if ($Address > RAM_LAST_DIR_ADDR && $Type == RAM_TYPE_DIR)
    {
    if (! defined($ram = $sfr_by_address{$Address}))
      {
      $sfr_by_address{$Address} = {
				  NAME      => $Name,
				  REF_COUNT => ($Map_mode) ? 0 : 1
				  };
      }
    else
      {
      if ($Name ne '' && $ram->{NAME} ne $Name)
	{
	Log(sprintf("Warning, the address: 0x%02X already busy by the $ram->{NAME} register.", $Address), 2);
	}
      else
	{
	++$ram->{REF_COUNT} if (! $Map_mode);
	}
      }

    $sfr_by_names{$Name} = $Address;
    }
  else
    {
    if (! defined($ram = $ram_by_address{$Address}))
      {
      $ram_by_address{$Address} = {
				  TYPE	    => $Type,
				  NAME	    => $Name,
				  SIZE	    => 1,
				  REF_COUNT => ($Map_mode) ? 0 : 1
				  };
      }
    else
      {
      ++$ram->{REF_COUNT} if (! $Map_mode);
      }
    }
  }

#-------------------------------------------------------------------------------

	#
	# Store a variable name from XRAM.
	#

sub add_xram($$$)
  {
  my ($Address, $Name, $Map_mode) = @_;
  my $xram;

  if (! defined($xram = $xram_by_address{$Address}))
    {
    $xram_by_address{$Address} = {
				 NAME	   => $Name,
				 SIZE	   => 1,
				 REF_COUNT => ($Map_mode) ? 0 : 1
				 };
    }
  else
    {
    ++$xram->{REF_COUNT} if (! $Map_mode);
    }
  }

################################################################################
################################################################################

use constant MAP_NULL   => 0;
use constant MAP_BORDER => 1;
use constant MAP_AREA   => 2;
use constant MAP_ABS    => 3;
use constant MAP_CABS   => 4;
use constant MAP_CODE0  => 5;
use constant MAP_CODE1  => 6;
use constant MAP_DATA   => 7;
use constant MAP_ISEG   => 8;
use constant MAP_CONST  => 9;

        #
        # If exists the map file, then extracts out of it the labels,
        # variables and some segments.
        #

sub read_map_file()
  {
  my ($addr, $name, $state, $label);

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
      if (/^\.\s+\.ABS\.\s+/o)
	{
	$state = MAP_ABS;
	}
      elsif (/^CABS\s+/o)
	{
	$state = MAP_CABS;
	}
      elsif (/^(HOME|CSEG)\s+/o)
	{
	$state = MAP_CODE0;
	}
      elsif (/^GSINIT\d+\s+/o)
	{
	$state = MAP_CODE1;
	}
      elsif (/^(D|O)SEG\s+/o)
	{
	$state = MAP_DATA;
	}
      elsif (/^ISEG\s+/o)
	{
	$state = MAP_ISEG;
	}
      elsif (/^CONST\s+([[:xdigit:]]+)\s+([[:xdigit:]]+)\s+/o)
	{
	my ($start, $size) = (hex($1), hex($2));

        add_const_area($start, $start + $size - 1);
	$state = MAP_CONST;
	}
      elsif (/^SSEG\s+([[:xdigit:]]+)\s+([[:xdigit:]]+)/o)
	{
	($stack_start, $stack_size) = (hex($1), hex($2));
	}
      else
	{
	$state = MAP_NULL;
	}
      }
    elsif ($state == MAP_ABS)
      {
      if (/^.ASxxxx Linker\s+/io)
        {
        $state = MAP_NULL;
        }
      elsif (/\s*([[:xdigit:]]+)\s+(\S+)/o)
	{
	#  000002C0  _Dword               xdata

	($addr, $name) = (hex($1), $2);

	add_xram(hex($1), $2, TRUE) if ($addr > RAM_MAX_ADDR);
	}
      }
    elsif ($state == MAP_CABS)
      {
      if (/^.ASxxxx Linker\s+/io)
        {
        $state = MAP_NULL;
        }
      elsif (/^C:\s+([[:xdigit:]]+)\s+(\S+)/o)
	{
	($addr, $name) = (hex($1), $2);

	if ($name eq 's_CONST' || $name eq 's_XINIT')
	  {
	# C:   000001E8  s_CONST
	# C:   00000215  s_XINIT

	  add_block($addr, BLOCK_CONST, 0, BL_TYPE_CONST, $name);
	  }
	}
      } # elsif ($state == MAP_CABS)
    elsif ($state == MAP_CODE0 || $state == MAP_CODE1)
      {
      if (/^.ASxxxx Linker\s+/io)
        {
        $state = MAP_NULL;
        }
      elsif (/^C:\s+([[:xdigit:]]+)\s+(\S+)/o)
	{
	# C:   00000040  __mcs51_genXINIT
	# C:   00000061  __mcs51_genRAMCLEAR
	# C:   00000067  __mcs51_genXRAMCLEAR
	# C:   00000086  _Uart_int                 main
	# C:   000000CE  _toHexChar                main
	# C:   000001E4  __sdcc_external_startup   _startup

	($addr, $name) = (hex($1), $2);

	if ($state == MAP_CODE0)
	  {
	  add_func_label($addr, $name, TRUE);
	  }
	else
	  {
	  add_jump_label($addr, $name, BL_TYPE_LABEL, EMPTY, TRUE);
	  }
	}
      } # elsif ($state == MAP_CODE0 || $state == MAP_CODE1)
    elsif ($state == MAP_DATA)
      {
      if (/^.ASxxxx Linker\s+/io)
        {
        $state = MAP_NULL;
        }
      elsif (/^\s*([[:xdigit:]]+)\s+(\S+)/o)
	{
	# 00000039  _counter              data
	# 0000004C  _flash_read_PARM_2    flash

	add_ram(hex($1), $2, RAM_TYPE_DIR, TRUE);
	}
      } # elsif ($state == MAP_DATA)
    elsif ($state == MAP_ISEG)
      {
      if (/^.ASxxxx Linker\s+/io)
        {
        $state = MAP_NULL;
        }
      elsif (/^\s*([[:xdigit:]]+)\s+(\S+)/o)
	{
	# 00000082  _length               data

	add_ram(hex($1), $2, RAM_TYPE_IND, TRUE);
	}
      } # elsif ($state == MAP_DATA)
    elsif ($state == MAP_CONST)
      {
      $state = MAP_NULL if (/^.ASxxxx Linker\s+/io);
      }
    } # while (<MAP>)

  $map_readed = TRUE;
  close(MAP);
  }

#-------------------------------------------------------------------------------

use constant NAMES_NULL => 0;
use constant NAMES_BIT  => 1;
use constant NAMES_RAM  => 2;
use constant NAMES_IRAM => 3;
use constant NAMES_XRAM => 4;
use constant NAMES_ROM  => 5;

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

    if (/^\[BIT\]$/io)
      {
      $state = NAMES_BIT;
      next;
      }
    elsif (/^\[RAM\]$/io)
      {
      $state = NAMES_RAM;
      next;
      }
    elsif (/^\[XRAM\]$/io)
      {
      $state = NAMES_XRAM;
      next;
      }
    elsif (/^\[IRAM\]$/io)
      {
      $state = NAMES_IRAM;
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
      when (NAMES_BIT)
	{
	if ($line =~ /^0x([[:xdigit:]]+)\s*:\s*(\S+)$/io)
	  {
	  add_bit(hex($1), $2, TRUE);
	  }
	}

      when (NAMES_RAM)
	{
	if ($line =~ /^0x([[:xdigit:]]+)\s*:\s*(\S+)$/io)
	  {
	  add_ram(hex($1), $2, RAM_TYPE_DIR, TRUE);
	  }
	}

      when (NAMES_IRAM)
	{
	if ($line =~ /^0x([[:xdigit:]]+)\s*:\s*(\S+)$/io)
	  {
	  add_ram(hex($1), $2, RAM_TYPE_IND, TRUE);
	  }
	}

      when (NAMES_XRAM)
	{
	if ($line =~ /^0x([[:xdigit:]]+)\s*:\s*(\S+)$/io)
	  {
	  add_xram(hex($1), $2, TRUE);
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
  my ($prev_addr, $prev_name, $name, $i, $var_size);

  $prev_addr = EMPTY;
  $prev_name = '';
  foreach (sort {$a <=> $b} keys(%ram_by_address))
    {
    $name = $ram_by_address{$_}->{NAME};
    $ram_names_by_address{$_} = $name;

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

	$ram_by_address{$prev_addr}->{SIZE} = $var_size;
	}
      }

    $prev_addr = $_;
    $prev_name = $name;
    }

  $prev_addr = EMPTY;
  $prev_name = '';
  foreach (sort {$a <=> $b} keys(%xram_by_address))
    {
    $name = $xram_by_address{$_}->{NAME};
    $xram_names_by_address{$_} = $name;

    if ($prev_addr != EMPTY)
      {
      $var_size = $_ - $prev_addr;

      if ($var_size > 1)
	{
	# This is a multi-byte variable. Make the aliases.

	for ($i = 1; $i < $var_size; ++$i)
	  {
	  $xram_names_by_address{$prev_addr + $i} = "($prev_name + $i)";
	  }

	$xram_by_address{$prev_addr}->{SIZE} = $var_size;
	}
      }

    $prev_addr = $_;
    $prev_name = $name;
    }
  }

#-------------------------------------------------------------------------------

        #
        # If there is left in yet so label that has no name, this here get one.
        # 

sub add_names_labels()
  {
  my ($addr, $label, $fidx, $lidx, $jidx, $jtidx, $cidx, $type);

  $fidx = 0;
  $lidx = 0;
  $jidx = 0;
  $jtidx = 0;
  $cidx = 0;

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
      $label->{NAME} = sprintf("$label_names{$type}%03u", $jidx++);
      }
    elsif ($type == BL_TYPE_JLABEL)
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

=back
        Instruction set of the 8051 family:

	NOP				00000000
	AJMP	addr11			aaa00001 aaaaaaaa		a10 a9 a8 1 0 0 0 1	a7-a0
	LJMP	addr16			00000010 aaaaaaaa aaaaaaaa	a15-a8 a7-a0	absolute address
	RR	A			00000011
	INC	A			00000100
	INC	direct			00000101 aaaaaaaa		register address
	INC	@Ri			0000011i			R0 .. R1
	INC	Rn			00001rrr			R0 .. R7
	JBC	bit, rel		00010000 bbbbbbbb rrrrrrrr	bit address	relative address
	ACALL	addr11			aaa10001 aaaaaaaa		a10 a9 a8 1 0 0 0 1	a7-a0
	LCALL	addr16			00010010 aaaaaaaa aaaaaaaa	a15-a8 a7-a0	absolute address
	RRC	A			00010011
	DEC	A			00010100
	DEC	direct			00010101 aaaaaaaa		register address
	DEC	@Ri			0001011i			R0 .. R1
	DEC	Rn			00011rrr			R0 .. R7
	JB	bit, rel		00100000 bbbbbbbb rrrrrrrr	bit address	relative address
	RET				00100010
	RL	A			00100011
	ADD	A, #data		00100100 dddddddd		adat
	ADD	A, direct		00100101 aaaaaaaa		register address
	ADD	A, @Ri			0010011i			R0 .. R1
	ADD	A, Rn			00101rrr			R0 .. R7
	JNB	bit, rel		00110000 bbbbbbbb rrrrrrrr	bit address	relative address
	RETI				00110010
	RLC	A			00110011
	ADDC	A, #data		00110100 dddddddd		adat
	ADDC	A, direct		00110101 aaaaaaaa		register address
	ADDC	A, @Ri			0011011i			R0 .. R1
	ADDC	A, Rn			00111rrr			R0 .. R7
	JC	rel			01000000 rrrrrrrr 		relative address
	ORL	direct, A		01000010 aaaaaaaa		register address
	ORL	direct, #data		01000011 aaaaaaaa dddddddd	register address	adat
	ORL	A, #data		01000100 dddddddd		adat
	ORL	A, direct		01000101 aaaaaaaa		register address
	ORL	A, @Ri			0100011i			R0 .. R1
	ORL	A, Rn			01001rrr		        R0 .. R7
	JNC	rel			01010000 rrrrrrrr 		relative address
	ANL	direct, A		01010010 aaaaaaaa		register address
	ANL	direct, #data		01010011 aaaaaaaa dddddddd	register address	adat
	ANL	A, #data		01010100 dddddddd		adat
	ANL	A, direct		01010101 aaaaaaaa		register address
	ANL	A, @Ri			0101011i			R0 .. R1
	ANL	A, Rn			01011rrr			R0 .. R7
	JZ	rel			01100000 rrrrrrrr 		relative address
	XRL	direct, A		01100010 aaaaaaaa		register address
	XRL	direct, #data		01100011 aaaaaaaa dddddddd	register address	adat
	XRL	A, #data		01100100 dddddddd		adat
	XRL	A, direct		01100101 aaaaaaaa		register address
	XRL	A, @Ri			0110011i			R0 .. R1
	XRL	A, Rn			01101rrr			R0 .. R7
	JNZ	rel			01110000 rrrrrrrr 		relative address
	ORL	C, bit			01110010 bbbbbbbb		bit address
	JMP	@A+DPTR			01110011
	MOV	A, #data		01110100 dddddddd		adat
	MOV	direct, #data		01110101 aaaaaaaa dddddddd	register address	adat
	MOV	@Ri, #data		0111011i dddddddd		adat
	MOV	Rn, #data		01111rrr dddddddd		R0 .. R7	adat
	SJMP	rel			10000000 rrrrrrrr		relative address
	ANL	C, bit			10000010 bbbbbbbb		bit address
	MOVC	A, @A+PC		10000011
	DIV	AB			10000100
	MOV	direct, direct		10000101 aaaaaaaa aaaaaaaa	forrás reg.	cél reg.
	MOV	direct, @Ri		1000011i aaaaaaaa		R0 .. R1	register address
	MOV	direct, Rn		10001rrr aaaaaaaa		R0 .. R7	register address
	MOV	DPTR, #data16		10010000 dddddddd dddddddd	d15-d8 d7-d0
	MOV	bit, C			10010010 bbbbbbbb		bit address
	MOVC	A, @A+DPTR		10010011
	SUBB	A, #data		10010100 dddddddd		adat
	SUBB	A, direct		10010101 aaaaaaaa		register address
	SUBB	A, @Ri			1001011i			R0 .. R1
	SUBB	A, Rn			10011rrr			R0 .. R7
	ORL	C, /bit			10100000 bbbbbbbb		bit address
	MOV	C, bit			10100010 bbbbbbbb		bit address
	INC	DPTR			10100011
	MUL	AB			10100100
	MOV	@Ri, direct		1010011i aaaaaaaa		register address
	MOV	Rn, direct		10101rrr aaaaaaaa		R0 .. R7	register address
	ANL	C, /bit			10110000 bbbbbbbb		bit address
	CPL	bit			10110010 bbbbbbbb		bit address
	CPL	C			10110011
	CJNE	A, #data, rel		10110100 dddddddd rrrrrrrr	adat		relative address
	CJNE	A, direct, rel		10110101 aaaaaaaa rrrrrrrr	register address	relative address
	CJNE	@Ri, #data, rel		1011011i dddddddd rrrrrrrr	R0 .. R1 	data	relative address
	CJNE	Rn, #data, rel		10111rrr dddddddd rrrrrrrr	R0 .. R7 	data	relative address
	PUSH	direct			11000000 aaaaaaaa		register address
	CLR	bit			11000010 bbbbbbbb		bit address
	CLR	C			11000011
	SWAP	A			11000100
	XCH	A, direct		11000101 aaaaaaaa		register address
	XCH	A, @Ri			1100011i			R0 .. R1
	XCH	A, Rn			11001rrr			R0 .. R7
	POP	direct			11010000 aaaaaaaa		register address
	SETB	bit			11010010 bbbbbbbb		bit address
	SETB	C			11010011
	DA	A			11010100
	DJNZ	direct, rel		11010101 aaaaaaaa rrrrrrrr	register address	relative address
	XCHD	A, @Ri			1101011i			R0 .. R1
	DJNZ	Rn, rel			11011rrr rrrrrrrr		R0 .. R7	relative address
	MOVX	A, @DPTR		11100000
	MOVX	A, @Ri			1110001i			R0 .. R1
	CLR	A			11100100
	MOV	A, direct		11100101 aaaaaaaa		register address	The "MOV A, ACC" invalid instruction.
	MOV	A, @Ri			1110011i			R0 .. R1
	MOV	A, Rn			11101rrr			R0 .. R7
	MOVX	@DPTR, A		11110000
	MOVX	@Ri, A			1111001i			R0 .. R1
	CPL	A			11110100
	MOV	direct, A		11110101 aaaaaaaa		register address
	MOV	@Ri, A			1111011i			R0 .. R1
	MOV	Rn, A			11111rrr			R0 .. R7
=cut

#-------------------------------------------------------------------------------

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
  my ($instr_size, $instr);
  my ($addr, $instr_mask0, $instr_mask1, $instr_mask2);

  $instr_size  = $BlockRef->{SIZE};
  $instr       = $rom[$Address];

  $instr_mask0 = $instr & 0x1F;
  $instr_mask1 = $instr & 0xFE;
  $instr_mask2 = $instr & 0xF8;

  if ($instr_mask0 == INST_AJMP)
    {
        # AJMP	addr11			aaa00001 aaaaaaaa		a10 a9 a8 0 0 0 0 1	a7-a0

    $addr = (($Address + $instr_size) & 0xF800) | (($instr & 0xE0) << 3) | $rom[$Address + 1];
    add_jump_label($addr, '', BL_TYPE_LABEL, $Address, FALSE);
    }
  elsif ($instr_mask0 == INST_ACALL)
    {
	# ACALL	addr11			aaa10001 aaaaaaaa		a10 a9 a8 1 0 0 0 1	a7-a0

    $addr = (($Address + $instr_size) & 0xF800) | (($instr & 0xE0) << 3) | $rom[$Address + 1];
    add_func_label($addr, '', FALSE);
    }
  elsif ($instr_mask1 == INST_CJNE__Ri_DATA ||
	 $instr_mask2 == INST_CJNE_Rn_DATA)
    {
	# CJNE	@Ri, #data, rel		1011011i dddddddd rrrrrrrr	R0 .. R1 	data		relative address
	# CJNE	Rn, #data, rel		10111rrr dddddddd rrrrrrrr	R0 .. R7 	data		relative address

    $addr = $Address + $instr_size + expand_offset($rom[$Address + 2]);
    add_jump_label($addr, '', BL_TYPE_LABEL, EMPTY, FALSE);
    }
  elsif ($instr_mask2 == INST_DJNZ_Rn)
    {
	# DJNZ	Rn, rel			11011rrr rrrrrrrr		R0 .. R7	relative address

    $addr = $Address + $instr_size + expand_offset($rom[$Address + 1]);
    add_jump_label($addr, '', BL_TYPE_LABEL, EMPTY, FALSE);
    }
  elsif ($instr == INST_LJMP)
    {
	# LJMP	addr16			00000010 aaaaaaaa aaaaaaaa	a15-a8 a7-a0	absolute address

    $addr = ($rom[$Address + 1] << 8) | $rom[$Address + 2];
    add_jump_label($addr, '', BL_TYPE_LABEL, $Address, FALSE);
    }
  elsif ($instr == INST_LCALL)
    {
	# LCALL	addr16			00010010 aaaaaaaa aaaaaaaa	a15-a8 a7-a0	absolute address

    $addr = ($rom[$Address + 1] << 8) | $rom[$Address + 2];
    add_func_label($addr, '', FALSE);
    }
  elsif ($instr == INST_JBC_BIT ||
	 $instr == INST_JB_BIT ||
	 $instr == INST_JNB_BIT ||
	 $instr == INST_CJNE_A_DATA ||
	 $instr == INST_CJNE_A_DIRECT ||
	 $instr == INST_DJNZ_DIRECT)
    {
	# JBC	bit, rel		00010000 bbbbbbbb rrrrrrrr	bit address		relative address
	# JB	bit, rel		00100000 bbbbbbbb rrrrrrrr	bit address		relative address
	# JNB	bit, rel		00110000 bbbbbbbb rrrrrrrr	bit address		relative address
	# CJNE	A, #data, rel		10110100 dddddddd rrrrrrrr	data			relative address
	# CJNE	A, direct, rel		10110101 aaaaaaaa rrrrrrrr	register address	relative address
	# DJNZ	direct, rel		11010101 aaaaaaaa rrrrrrrr	register address	relative address

    $addr = $Address + $instr_size + expand_offset($rom[$Address + 2]);
    add_jump_label($addr, '', BL_TYPE_LABEL, EMPTY, FALSE);
    }
  elsif ($instr == INST_JC ||
	 $instr == INST_JNC ||
	 $instr == INST_JZ ||
	 $instr == INST_JNZ ||
	 $instr == INST_SJMP)
    {
	# JC	rel			01000000 rrrrrrrr 		relative address
	# JNC	rel			01010000 rrrrrrrr 		relative address
	# JZ	rel			01100000 rrrrrrrr 		relative address
	# JNZ	rel			01110000 rrrrrrrr 		relative address
	# SJMP	rel			10000000 rrrrrrrr		relative address

    $addr = $Address + $instr_size + expand_offset($rom[$Address + 1]);
    add_jump_label($addr, '', BL_TYPE_LABEL, EMPTY, FALSE);
    }
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

  if ($Address <= BANK_LAST_ADDR)
    {
	# This register belongs to one of the register bank.

    my $bank = ($Address >> 3) & 3;
    my $reg  = $Address & 7;

    $str = ($gen_assembly_code) ? sprintf("0x%02X", $Address) : "R${reg}<#$bank>";
    ${$StrRef} = $str;

    if (defined($ram_names_by_address{$Address}))
      {
      my $var = $ram_names_by_address{$Address};

      printf STDERR ("This address (0x%02X) belongs to two names: \"$str\" and \"$var\"\n", $Address);
      }
    }
  elsif (defined($ram = $sfr_by_address{$Address}) && $ram->{NAME} ne '')
    {
    $str = $ram->{NAME};
    ${$StrRef} = $str;
    }
  elsif (defined($ram = $ram_names_by_address{$Address}))
    {
    $str = sprintf "0x%02X", $Address;
    ${$StrRef} = "[$str]";
    $str = $ram;
    }
  else
    {
    $str = sprintf "0x%02X", $Address;
    ${$StrRef} = "[$str]";
    }

  return $str;
  }

#-------------------------------------------------------------------------------

	#
	# If exists a XRAM name wich belong to the $Address, then returns it.
	# Otherwise, returns the address.
	#

sub xram_name($$)
  {
  my ($Address, $StrRef) = @_;
  my ($xram, $str);

  $str = sprintf "0x%04X", $Address;
  ${$StrRef} = $str;

  $str = $xram if (defined($xram = $xram_names_by_address{$Address}));

  return $str;
  }

#-------------------------------------------------------------------------------

	#
	# If exists a iRAM name wich belong to the $Address, then returns it.
	# Otherwise, returns the address.
	#

sub iram_name($$)
  {
  my ($Address, $StrRef) = @_;
  my ($ram, $str);

  $str = sprintf "0x%02X", $Address;
  ${$StrRef} = $str;
  $ram = $ram_names_by_address{$Address};

  $str = $ram if (defined($ram));

  return $str;
  }

#-------------------------------------------------------------------------------

	#
	# If exists a bit name wich belong to the $Address, then returns it.
	# Otherwise, returns the address.
	#

sub bit_name($$)
  {
  my ($Address, $StrRef) = @_;
  my ($bit, $str);

  if (defined($bit = $sfr_bit_by_address{$Address}) && $bit->{NAME} ne '')
    {
    $str = $bit->{NAME};
    ${$StrRef} = $str;
    }
  elsif (defined($bit = $bit_by_address{$Address}) && $bit->{NAME} ne '')
    {
    $str = sprintf "0x%02X", $Address;
    ${$StrRef} = "[$str]";
    $str = $bit->{NAME};
    }
  else
    {
    $str = sprintf "0x%02X", $Address;
    ${$StrRef} = "[$str]";
    }

  return $str;
  }

#-------------------------------------------------------------------------------

	#
	# If exists a label name wich belong to the $Address, then returns it.
	# Otherwise, returns the address.
	#

sub labelname($)
  {
  my $Address = $_[0];
  my $label = $labels_by_address{$Address};

  return ((defined($label)) ? $label->{NAME} : (sprintf "0x%04X", $Address));
  }

#-------------------------------------------------------------------------------

	#
	# Auxiliary procedure of prints.
	#

sub print_3($$$)
  {
  return if ($decoder_silent_level > SILENT0);

  if ($no_explanations)
    {
    print(($_[1] ne '') ? "$_[0]\t$_[1]\n" : "$_[0]\n");
    }
  else
    {
    print "$_[0]\t" . align($_[1], EXPL_ALIGN_SIZE) . "; $_[2]\n";
    }
  }

#-------------------------------------------------------------------------------

	#
	# Invalidates the DPTR and the Rx registers.
	#

sub invalidate_DPTR_Rx()
  {
  $DPTR = EMPTY;
  @R_regs[0 .. 7] = ((EMPTY) x 8);
  }

#-------------------------------------------------------------------------------

	#
	# Invalidates the DPTR or the Rx registers.
	#

sub invalidate_reg($)
  {
  my $Address = $_[0];

  return if ($Address == EMPTY);

  if ($Address == DPL || $Address == DPH)
    {
    $DPTR = EMPTY;
    }
  elsif ($Address <= BANK_LAST_ADDR)
    {
    $R_regs[$Address & 7] = EMPTY;
    }
  }

#-------------------------------------------------------------------------------

	#
	# Carries out the operations with the R registers.
	#

use constant Rx_INV => 0;
use constant Rx_INC => 1;
use constant Rx_DEC => 2;
use constant Rx_MOV => 3;

sub operation_R_reg
  {
  my $Rx   = shift(@_);
  my $Oper = shift(@_);
  my $r;

  if ($Oper == Rx_INV)
    {
    $R_regs[$Rx] = EMPTY;
    }
  elsif ($Oper == Rx_INC)
    {
    $r = $R_regs[$Rx];

    if ($r != EMPTY)
      {
      ++$r;
      $R_regs[$Rx] = $r & 0xFF;
      return TRUE;
      }
    }
  elsif ($Oper == Rx_DEC)
    {
    $r = $R_regs[$Rx];

    if ($r != EMPTY)
      {
      --$r;
      $R_regs[$Rx] = $r & 0xFF;
      return TRUE;
      }
    }
  elsif ($Oper == Rx_MOV)
    {
    $R_regs[$Rx] = shift(@_) & 0xFF;
    return TRUE;
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

	#
	# If possible, returns the character.
	#

sub present_char($)
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
	# Decodes value of the $Ch.
	#

sub decode_char($)
  {
  my $Ch = $_[0];

  if ($Ch >= ord(' ') && $Ch < 0x7F)
    {
    return sprintf "'%c'", $Ch;
    }
  elsif (defined($control_characters{$Ch}))
    {
    return "'$control_characters{$Ch}'";
    }

  return sprintf "0x%02X", $Ch;
  }

#-------------------------------------------------------------------------------

	#
	# Determines direction of jump.
	#

sub jump_direction($)
  {
  my $TargetAddr = $_[0];

  if ($dcd_address < $TargetAddr)
    {
    return ' (forward)';
    }
  elsif ($dcd_address == $TargetAddr)
    {
    return '';
    }
  else
    {
    return ' (backward)';
    }
  }

#-------------------------------------------------------------------------------

	#
	# Returns with TRUE if the instruction use indirect RAM addressing
	# with @Ri.
	#

sub is_Ri_instr($)
  {
  my $Address = $_[0];
  my $block;

  $block = \%{$blocks_by_address{$Address}};
  return FALSE if (! defined($block) || $block->{TYPE} != BLOCK_INSTR);

	# These instructions use indirect RAM addressing with @Ri?
  return defined($indirect_addr_instr{$rom[$Address] & 0xFE});
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@                                   @@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@   These the instruction decoders.   @@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@                                   @@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 

sub ajmp()
  {
  my ($addr, $a11, $rb0, $rb1, $str0, $str1, $str2);

        # AJMP	addr11			aaa00001 aaaaaaaa		a10 a9 a8 0 0 0 0 1	a7-a0

  if ($decoder_silent_level == SILENT0)
    {
    $rb1  = (($dcd_instr & 0xE0) << 3) | $dcd_parm0;
    $addr = (($dcd_address + $dcd_instr_size) & 0xF800) | $rb1;
    $rb0  = labelname($addr);
    $a11  = sprintf "0x%04X", $rb1;
    $str0 = sprintf "0x%04X", $addr;

    if ($dcd_address < $addr)
      {
      $str1 = '';
      $str2 = ' (forward)';
      }
    elsif ($dcd_address == $addr)
      {
      $str1 = ' (endless loop)';
      $str2 = '';
      }
    else
      {
      $str1 = '';
      $str2 = ' (backward)';
      }

    print_3('ajmp', $rb0, "Jumps$str2 hither: $str0 (PC += $dcd_instr_size, PC(10-0) = $a11)$str1");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub acall()
  {
  my ($addr, $rb0, $rb1, $str0, $str1, $str2);

	# ACALL	addr11			aaa10001 aaaaaaaa		a10 a9 a8 1 0 0 0 1	a7-a0

  if ($decoder_silent_level == SILENT0)
    {
    $rb1  = (($dcd_instr & 0xE0) << 3) | $dcd_parm0;
    $addr = (($dcd_address + $dcd_instr_size) & 0xF800) | $rb1;
    $rb0  = labelname($addr);
    $str0 = sprintf "0x%04X", $rb1;
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    print_3('acall', $rb0, "Calls$str2 this: $str1 (PC += $dcd_instr_size, [++SP] = PCL, [++SP] = PCH, PC(10-0) = $str0)");
    }

  invalidate_DPTR_Rx();
  }

#-------------------------------------------------------------------------------

sub inc_ind_Ri()
  {
	# INC	@Ri			0000011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('inc', "\@$dcd_Ri_name", "++[$dcd_Ri_name]");
  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub dec_ind_Ri()
  {
	# DEC	@Ri			0001011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('dec', "\@$dcd_Ri_name", "--[$dcd_Ri_name]");
  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub add_A_ind_Ri()
  {
	# ADD	A, @Ri			0010011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('add', "A, \@$dcd_Ri_name", "ACC += [$dcd_Ri_name]");
  }

#-------------------------------------------------------------------------------

sub addc_A_ind_Ri()
  {
	# ADDC	A, @Ri			0011011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('addc', "A, \@$dcd_Ri_name", "ACC += [$dcd_Ri_name] + CY");
  }

#-------------------------------------------------------------------------------

sub orl_A_ind_Ri()
  {
	# ORL	A, @Ri			0100011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('orl', "A, \@$dcd_Ri_name", "ACC |= [$dcd_Ri_name]");
  }

#-------------------------------------------------------------------------------

sub anl_A_ind_Ri()
  {
	# ANL	A, @Ri			0101011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('anl', "A, \@$dcd_Ri_name", "ACC &= [$dcd_Ri_name]");
  }

#-------------------------------------------------------------------------------

sub xrl_A_ind_Ri()
  {
	# XRL	A, @Ri			0110011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('xrl', "A, \@$dcd_Ri_name", "ACC ^= [$dcd_Ri_name]");
  }

#-------------------------------------------------------------------------------

sub mov_ind_Ri_data()
  {
  my ($rb, $str);

	# MOV	@Ri, #data		0111011i dddddddd		data

  my $i = $R_regs[$dcd_Ri_regs];

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('mov', "\@$dcd_Ri_name, #$rb", "[$dcd_Ri_name] = $rb$str");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($i, '', RAM_TYPE_IND, FALSE);
    }

  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub mov_direct_ind_Ri()
  {
  my ($rb, $name);

	# MOV	direct, @Ri		1000011i aaaaaaaa		R0 .. R1	register address

  my $i = $R_regs[$dcd_Ri_regs];

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('mov', "$rb, \@$dcd_Ri_name", "$name = [$dcd_Ri_name]");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    add_ram($i, '', RAM_TYPE_IND, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub subb_A_ind_Ri()
  {
	# SUBB	A, @Ri			1001011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('subb', "A, \@$dcd_Ri_name", "ACC -= [$dcd_Ri_name] + CY");
  }

#-------------------------------------------------------------------------------

sub mov_ind_Ri_direct()
  {
  my ($rb, $name);

	# MOV	@Ri, direct		1010011i aaaaaaaa		register address

  my $i = $R_regs[$dcd_Ri_regs];

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('mov', "\@$dcd_Ri_name, $rb", "[$dcd_Ri_name] = $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    add_ram($i, '', RAM_TYPE_IND, FALSE);
    }

  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub cjne_ind_Ri_data()
  {
  my ($addr, $rb, $str0, $str1, $str2, $str3);

	# CJNE	@Ri, #data, rel		1011011i dddddddd rrrrrrrr	R0 .. R1 	data		relative address

  my $i = $R_regs[$dcd_Ri_regs];

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb   = labelname($addr);
    $str0 = sprintf "0x%02X", $dcd_parm0;
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    $str3 = present_char($dcd_parm0);
    print_3('cjne', "\@$dcd_Ri_name, #$str0, $rb", "If ([$dcd_Ri_name] != $str0$str3) then jumps$str2 hither: $str1");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($i, '', RAM_TYPE_IND, FALSE);
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub xch_A_ind_Ri()
  {
	# XCH	A, @Ri			1100011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('xch', "A, \@$dcd_Ri_name", "ACC <-> [$dcd_Ri_name]");
  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub xchd_A_ind_Ri()
  {
	# XCHD	A, @Ri			1101011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('xchd', "A, \@$dcd_Ri_name", "ACC(3-0) <-> [$dcd_Ri_name](3-0)");
  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub movx_A_ind_Ri()
  {
	# MOVX	A, @Ri			1110001i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_xram($i, '', FALSE) if ($i != EMPTY && $decoder_silent_level == SILENT1);

  print_3('movx', "A, \@$dcd_Ri_name", "ACC = XRAM[$dcd_Ri_name]");
  }

#-------------------------------------------------------------------------------

sub mov_A_ind_Ri()
  {
	# MOV	A, @Ri			1110011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('mov', "A, \@$dcd_Ri_name", "ACC = [$dcd_Ri_name]");
  }

#-------------------------------------------------------------------------------

sub movx_ind_Ri_A()
  {
	# MOVX	@Ri, A			1111001i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_xram($i, '', FALSE) if ($i != EMPTY && $decoder_silent_level == SILENT1);

  print_3('movx', "\@$dcd_Ri_name, A", "XRAM[$dcd_Ri_name] = ACC");
  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub mov_ind_Ri_A()
  {
	# MOV	@Ri, A			1111011i			R0 .. R1

  my $i = $R_regs[$dcd_Ri_regs];

  add_ram($i, '', RAM_TYPE_IND, FALSE) if ($decoder_silent_level == SILENT1);

  print_3('mov', "\@$dcd_Ri_name, A", "[$dcd_Ri_name] = ACC");
  invalidate_reg($i);
  }

#-------------------------------------------------------------------------------

sub inc_Rn()
  {
  my $str = '';

	# INC	Rn			00001rrr			R0 .. R7

  if (operation_R_reg($dcd_Rn_regs, Rx_INC))
    {
    $str = sprintf " (0x%02X)", $R_regs[$dcd_Rn_regs];
    }

  print_3('inc', $dcd_Rn_name, "++$dcd_Rn_name$str");
  }

#-------------------------------------------------------------------------------

sub dec_Rn()
  {
  my $str = '';

	# DEC	Rn			00011rrr			R0 .. R7

  if (operation_R_reg($dcd_Rn_regs, Rx_DEC))
    {
    $str = sprintf " (0x%02X)", $R_regs[$dcd_Rn_regs];
    }

  print_3('dec', $dcd_Rn_name, "--$dcd_Rn_name$str");
  }

#-------------------------------------------------------------------------------

sub add_A_Rn()
  {
	# ADD	A, Rn			00101rrr			R0 .. R7

  print_3('add', "A, $dcd_Rn_name", "ACC += $dcd_Rn_name");
  }

#-------------------------------------------------------------------------------

sub addc_A_Rn()
  {
	# ADDC	A, Rn			00111rrr			R0 .. R7

  print_3('addc', "A, $dcd_Rn_name", "ACC += $dcd_Rn_name + CY");
  }

#-------------------------------------------------------------------------------

sub orl_A_Rn()
  {
	# ORL	A, Rn			01001rrr		        R0 .. R7

  print_3('orl', "A, $dcd_Rn_name", "ACC |= $dcd_Rn_name");
  }

#-------------------------------------------------------------------------------

sub anl_A_Rn()
  {
	# ANL	A, Rn			01011rrr			R0 .. R7

  print_3('anl', "A, $dcd_Rn_name", "ACC &= $dcd_Rn_name");
  }

#-------------------------------------------------------------------------------

sub xrl_A_Rn()
  {
	# XRL	A, Rn			01101rrr			R0 .. R7

  print_3('xrl', "A, $dcd_Rn_name", "ACC ^= $dcd_Rn_name");
  }

#-------------------------------------------------------------------------------

sub mov_Rn_data()
  {
  my ($rb, $name, $str);

	# MOV	Rn, #data		01111rrr dddddddd		R0 .. R7	data

  if ($decoder_silent_level == SILENT0)
    {
    if (($dcd_Rn_regs == 0 || $dcd_Rn_regs == 1) && is_Ri_instr($dcd_address + $dcd_instr_size))
      {
      $rb = iram_name($dcd_parm0, \$name);
      print_3('mov', "$dcd_Rn_name, #$rb", "$dcd_Rn_name = $name");
      }
    else
      {
      $rb  = sprintf "0x%02X", $dcd_parm0;
      $str = present_char($dcd_parm0);
      print_3('mov', "$dcd_Rn_name, #$rb", "$dcd_Rn_name = $rb$str");
      }
    }

  operation_R_reg($dcd_Rn_regs, Rx_MOV, $dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub mov_direct_Rn()
  {
  my ($rb, $name);

	# MOV	direct, Rn		10001rrr aaaaaaaa		R0 .. R7	register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('mov', "$rb, $dcd_Rn_name", "$name = $dcd_Rn_name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub subb_A_Rn()
  {
	# SUBB	A, Rn			10011rrr			R0 .. R7

  print_3('subb', "A, $dcd_Rn_name", "ACC -= $dcd_Rn_name + CY");
  }

#-------------------------------------------------------------------------------

sub mov_Rn_direct()
  {
  my ($rb, $name);

	# MOV	Rn, direct		10101rrr aaaaaaaa		R0 .. R7	register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('mov', "$dcd_Rn_name, $rb", "$dcd_Rn_name = $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  operation_R_reg($dcd_Rn_regs, Rx_INV);
  }

#-------------------------------------------------------------------------------

sub cjne_Rn_data()
  {
  my ($addr, $rb, $str0, $str1, $str2, $str3);

	# CJNE	Rn, #data, rel		10111rrr dddddddd rrrrrrrr	R0 .. R7 	data		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb   = labelname($addr);
    $str0 = sprintf "0x%02X", $dcd_parm0;
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    $str3 = present_char($dcd_parm0);
    print_3('cjne', "$dcd_Rn_name, #$str0, $rb", "If ($dcd_Rn_name != $str0$str3) then jumps$str2 hither: $str1");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub xch_A_Rn()
  {
	# XCH	A, Rn			11001rrr			R0 .. R7

  print_3('xch', "A, $dcd_Rn_name", "ACC <-> $dcd_Rn_name");
  operation_R_reg($dcd_Rn_regs, Rx_INV);
  }

#-------------------------------------------------------------------------------

sub djnz_Rn()
  {
  my ($addr, $rb, $str0, $str1, $str2);

	# DJNZ	Rn, rel			11011rrr rrrrrrrr		R0 .. R7	relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm0);
    $rb   = labelname($addr);
    $str0 = ($dcd_address == $addr) ? ' (waiting loop)' : '';
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    print_3('djnz', "$dcd_Rn_name, $rb", "If (--$dcd_Rn_name != 0) then jumps$str2 hither: $str1$str0");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub mov_A_Rn()
  {
	# MOV	A, Rn			11101rrr			R0 .. R7

  print_3('mov', "A, $dcd_Rn_name", "ACC = $dcd_Rn_name");
  }

#-------------------------------------------------------------------------------

sub mov_Rn_A()
  {
	# MOV	Rn, A			11111rrr			R0 .. R7

  print_3('mov', "$dcd_Rn_name, A", "$dcd_Rn_name = ACC");
  operation_R_reg($dcd_Rn_regs, Rx_INV);
  }

#-------------------------------------------------------------------------------

sub nop()
  {
	# NOP				00000000
  print "nop\n" if ($decoder_silent_level == SILENT0);
  }

#-------------------------------------------------------------------------------

sub ljmp()
  {
  my ($addr, $rb, $str0, $str1, $str2);

	# LJMP	addr16			00000010 aaaaaaaa aaaaaaaa	a15-a8 a7-a0	absolute address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = ($dcd_parm0 << 8) | $dcd_parm1;
    $rb   = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;

    if ($dcd_address < $addr)
      {
      $str1 = '';
      $str2 = ' (forward)';
      }
    elsif ($dcd_address == $addr)
      {
      $str1 = ' (endless loop)';
      $str2 = '';
      }
    else
      {
      $str1 = '';
      $str2 = ' (backward)';
      }

    print_3('ljmp', $rb, "Jumps$str2 hither: $str0$str1");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub rr_A()
  {
	# RR	A			00000011

  print_3('rr', 'A', 'ACC[76543210] = ACC[07654321]');
  }

#-------------------------------------------------------------------------------

sub inc_A()
  {
	# INC	A			00000100

  print_3('inc', 'A', '++ACC');
  }

#-------------------------------------------------------------------------------

sub inc_direct()
  {
  my ($rb, $name);

	# INC	direct			00000101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('inc', $rb, "++$name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub jbc_bit()
  {
  my ($addr, $rb0, $rb1, $name0, $str0, $str1);

	# JBC	bit, rel		00100000 bbbbbbbb rrrrrrrr	bit address		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb0  = bit_name($dcd_parm0, \$name0);
    $rb1  = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;
    $str1 = jump_direction($addr);
    print_3('jbc', "$rb0, $rb1", "If ($name0 == H) then $name0 = L and jumps$str1 hither: $str0");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub lcall()
  {
  my ($addr, $rb, $str0, $str1);

	# LCALL	addr16			00010010 aaaaaaaa aaaaaaaa	a15-a8 a7-a0	absolute address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = ($dcd_parm0 << 8) | $dcd_parm1;
    $rb   = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;
    $str1 = jump_direction($addr);
    print_3('lcall', $rb, "Calls$str1 this: $str0 (PC += $dcd_instr_size, [++SP] = PCL, [++SP] = PCH, PC = $str0)");
    }

  invalidate_DPTR_Rx();
  }

#-------------------------------------------------------------------------------

sub rrc_A()
  {
	# RRC	A			00010011

  print_3('rrc', 'A', 'ACC[76543210] = ACC[C7654321], CY = ACC[0]');
  }

#-------------------------------------------------------------------------------

sub dec_A()
  {
	# DEC	A			00010100

  print_3('dec', 'A', '--ACC');
  }

#-------------------------------------------------------------------------------

sub dec_direct()
  {
  my ($rb, $name);

	# DEC	direct			00010101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('dec', $rb, "--$name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub jb_bit()
  {
  my ($addr, $rb0, $rb1, $name0, $str0, $str1, $str2);

	# JB	bit, rel		00100000 bbbbbbbb rrrrrrrr	bit address		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb0  = bit_name($dcd_parm0, \$name0);
    $rb1  = labelname($addr);
    $str0 = ($dcd_address == $addr) ? ' (waiting loop)' : '';
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    print_3('jb', "$rb0, $rb1", "If ($name0 == H) then jumps$str2 hither: $str1$str0");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub ret()
  {
	# RET				00100010

  print_3('ret', '', 'PCH = [SP--], PCL = [SP--]');
  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub rl_A()
  {
	# RL	A			00100011

  print_3('rl', 'A', 'ACC[76543210] = ACC[65432107]');
  }

#-------------------------------------------------------------------------------

sub add_A_data()
  {
  my ($rb, $str);

	# ADD	A, #data		00100100 dddddddd		data

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('add', "A, #$rb", "ACC += $rb$str");
    }
  }

#-------------------------------------------------------------------------------

sub add_A_direct()
  {
  my ($rb, $name);

	# ADD	A, direct		00100101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('add', "A, $rb", "ACC += $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub jnb_bit()
  {
  my ($addr, $rb0, $rb1, $name0, $str0, $str1, $str2);

	# JNB	bit, rel		00110000 bbbbbbbb rrrrrrrr	bit address		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb0  = bit_name($dcd_parm0, \$name0);
    $rb1  = labelname($addr);
    $str0 = ($dcd_address == $addr) ? ' (waiting loop)' : '';
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    print_3('jnb', "$rb0, $rb1", "If ($name0 == L) then jumps$str2 hither: $str1$str0");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub reti()
  {
	# RETI				00110010

  print_3('reti', '', 'PCH = [SP--], PCL = [SP--]');
  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub rcl_A()
  {
	# RLC	A			00110011

  print_3('rlc', 'A', 'ACC[76543210] = ACC[6543210C], CY = ACC[7]');
  }

#-------------------------------------------------------------------------------

sub addc_A_data()
  {
  my ($rb, $str);

	# ADDC	A, #data		00110100 dddddddd		data

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('addc', "A, #$rb", "ACC += $rb + CY$str");
    }
  }

#-------------------------------------------------------------------------------

sub addc_A_direct()
  {
  my ($rb, $name);

	# ADDC	A, direct		00110101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('addc', "A, $rb", "ACC += $name + CY");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub jc()
  {
  my ($addr, $rb, $str0, $str1);

	# JC	rel			01000000 rrrrrrrr 		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm0);
    $rb   = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;
    $str1 = jump_direction($addr);
    print_3('jc', $rb, "If (CY == H) then jumps$str1 hither: $str0");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub orl_direct_A()
  {
  my ($rb, $name);

	# ORL	direct, A		01000010 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('orl', "$rb, A", "$name |= ACC");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub orl_direct_data()
  {
  my ($rb0, $rb1, $name, $str);

	# ORL	direct, #data		01000011 aaaaaaaa dddddddd	register address	data

  if ($decoder_silent_level == SILENT0)
    {
    $rb0 = reg_name($dcd_parm0, \$name);
    $rb1 = sprintf "0x%02X", $dcd_parm1;
    $str = present_char($dcd_parm1);
    }

  if ($dcd_parm0 == PSW)
    {
    my $bank = ($dcd_parm1 >> 3) & 0x03;

    $used_banks{$bank} = TRUE;

    if ($decoder_silent_level == SILENT0)
      {
      $str = " (select bank #$bank)" if (($dcd_parm1 & ~0x18) == 0x00);
      }
    }

  if ($decoder_silent_level == SILENT0)
    {
    print_3('orl', "$rb0, #$rb1", "$name |= $rb1$str");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub orl_A_data()
  {
  my ($rb, $str);

	# ORL	A, #data		01000100 dddddddd		data

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('orl', "A, #$rb", "ACC |= $rb$str");
    }
  }

#-------------------------------------------------------------------------------

sub orl_A_direct()
  {
  my ($rb, $name);

	# ORL	A, direct		01000101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('orl', "A, $rb", "ACC |= $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub jnc()
  {
  my ($addr, $rb, $str0, $str1);

	# JNC	rel			01010000 rrrrrrrr 		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm0);
    $rb   = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;
    $str1 = jump_direction($addr);
    print_3('jnc', $rb, "If (CY == L) then jumps$str1 hither: $str0");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub anl_direct_A()
  {
  my ($rb, $name);

	# ANL	direct, A		01010010 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('anl', "$rb, A", "$name &= ACC");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub anl_direct_data()
  {
  my ($rb0, $rb1, $name, $str);

	# ANL	direct, #data		01010011 aaaaaaaa dddddddd	register address	data

  if ($decoder_silent_level == SILENT0)
    {
    $rb0 = reg_name($dcd_parm0, \$name);
    $rb1 = sprintf "0x%02X", $dcd_parm1;
    $str = present_char($dcd_parm1);
    print_3('anl', "$rb0, #$rb1", "$name &= $rb1$str");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub anl_A_data()
  {
  my ($rb, $str);

	# ANL	A, #data		01010100 dddddddd		data

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('anl', "A, #$rb", "ACC &= $rb$str");
    }
  }

#-------------------------------------------------------------------------------

sub anl_A_direct()
  {
  my ($rb, $name);

	# ANL	A, direct		01010101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('anl', "A, $rb", "ACC &= $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub jz()
  {
  my ($addr, $rb, $str0, $str1);

	# JZ	rel			01100000 rrrrrrrr 		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm0);
    $rb   = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;
    $str1 = jump_direction($addr);
    print_3('jz', $rb, "If (ACC == 0) then jumps$str1 hither: $str0");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub xrl_direct_A()
  {
  my ($rb, $name);

	# XRL	direct, A		01100010 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('xrl', "$rb, A", "$name ^= ACC");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub xrl_direct_data()
  {
  my ($rb0, $rb1, $name, $str);

	# XRL	direct, #data		01100011 aaaaaaaa dddddddd	register address	data

  if ($decoder_silent_level == SILENT0)
    {
    $rb0 = reg_name($dcd_parm0, \$name);
    $rb1 = sprintf "0x%02X", $dcd_parm1;
    $str = present_char($dcd_parm1);
    print_3('xrl', "$rb0, #$rb1", "$name |= $rb1$str");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub xrl_A_data()
  {
  my ($rb, $str);

	# XRL	A, #data		01100100 dddddddd		data

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('xrl', "A, #$rb", "ACC ^= $rb$str");
    }
  }

#-------------------------------------------------------------------------------

sub xrl_A_direct()
  {
  my ($rb, $name);

	# XRL	A, direct		01100101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('xrl', "A, $rb", "ACC |= $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub jnz()
  {
  my ($addr, $rb, $str0, $str1);

	# JNZ	rel			01110000 rrrrrrrr 		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm0);
    $rb   = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;
    $str1 = jump_direction($addr);
    print_3('jnz', $rb, "If (ACC != 0) then jumps$str1 hither: $str0");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub orl_C_bit()
  {
  my ($rb, $name);

	# ORL	C, bit			01110010 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('orl', "C, $rb", "CY |= $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub jmp_A_DPTR()
  {
  my $str;

	# JMP	@A+DPTR			01110011

  if ($DPTR != EMPTY)
    {
    add_jump_label($DPTR, '', BL_TYPE_JTABLE, EMPTY, FALSE);
    $str = jump_direction($DPTR);
    }
  else
    {
    $str = '';
    }

  print_3('jmp', '@A+DPTR', "Jumps$str hither: [DPTR + ACC]");
  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub mov_A_data()
  {
  my ($rb, $str);

	# MOV	A, #data		01110100 dddddddd		data

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('mov', "A, #$rb", "ACC = $rb$str");
    }
  }

#-------------------------------------------------------------------------------

sub mov_direct_data()
  {
  my ($rb0, $rb1, $name, $str);

	# MOV	direct, #data		01110101 aaaaaaaa dddddddd	register address	data

  if ($decoder_silent_level == SILENT0)
    {
    $rb0 = reg_name($dcd_parm0, \$name);
    $rb1 = sprintf "0x%02X", $dcd_parm1;
    $str = '';
    }

  if ($dcd_parm0 == PSW)
    {
    my $bank = ($dcd_parm1 >> 3) & 0x03;

    $used_banks{$bank} = TRUE;

    if ($decoder_silent_level == SILENT0)
      {
      $str = " (select bank #$bank)" if (($dcd_parm1 & ~0x18) == 0x00);
      }
    }
#  elsif ($dcd_parm0 == SP)
#    {
#    $stack_start = ($dcd_parm1 + 1) if ($stack_start < 0);
#    }
  else
    {
    $str = present_char($dcd_parm1);
    }

  if ($decoder_silent_level == SILENT0)
    {
    print_3('mov', "$rb0, #$rb1", "$name = $rb1$str");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub sjmp()
  {
  my ($addr, $rb0, $rb1, $name0, $name1, $str0, $str1, $str2);

	# SJMP	rel			10000000 rrrrrrrr		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm0);
    $rb0  = labelname($addr);
    $str0 = ($dcd_address == $addr) ? ' (endless loop)' : '';
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    print_3('sjmp', $rb0, "Jumps$str2 hither: $str1$str0");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub anl_C_bit()
  {
  my ($rb, $name);

	# ANL	C, bit			10000010 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('anl', "C, $rb", "CY &= $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub movc_A_A_PC()
  {
	# MOVC	A, @A+PC		10000011

  print_3('movc', 'A, @A+PC', "ACC = ROM[PC + $dcd_instr_size + ACC]");
  }

#-------------------------------------------------------------------------------

sub div_AB()
  {
	# DIV	AB			10000100

  print_3('div', 'AB', 'ACC = ACC / B, B = ACC % B');
  }

#-------------------------------------------------------------------------------

sub mov_direct_direct()
  {
  my ($rb0, $rb1, $name0, $name1);

	# MOV	direct, direct		10000101 aaaaaaaa aaaaaaaa	forrás reg.	cél reg.

  if ($decoder_silent_level == SILENT0)
    {
    $rb0 = reg_name($dcd_parm0, \$name0);
    $rb1 = reg_name($dcd_parm1, \$name1);
    print_3('mov', "$rb1, $rb0", "$name1 = $name0");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    add_ram($dcd_parm1, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub mov_DPTR_data()
  {
  my ($addr, $str, $name);

	# MOV	DPTR, #data16		10010000 dddddddd dddddddd	d15-d8 d7-d0

  $addr = ($dcd_parm0 << 8) | $dcd_parm1;

  if ($decoder_silent_level == SILENT0)
    {
    $str = xram_name($addr, \$name);
    print_3('mov', "DPTR, #$str", "DPTR = $name");
    }

  $DPTR = $addr;
  }

#-------------------------------------------------------------------------------

sub mov_bit_C()
  {
  my ($rb, $name);

	# MOV	bit, C			10010010 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('mov', "$rb, C", "$name = CY");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub movc_A_A_DPTR()
  {
	# MOVC	A, @A+DPTR		10010011

  add_jump_label($DPTR, '', BL_TYPE_LABEL, EMPTY, FALSE) if ($DPTR != EMPTY);

  print_3('movc', 'A, @A+DPTR', 'ACC = ROM[DPTR + ACC]');
  }

#-------------------------------------------------------------------------------

sub subb_A_data()
  {
  my ($rb, $str);

	# SUBB	A, #data		10010100 dddddddd		data

  if ($decoder_silent_level == SILENT0)
    {
    $rb  = sprintf "0x%02X", $dcd_parm0;
    $str = present_char($dcd_parm0);
    print_3('subb', "A, #$rb", "ACC -= $rb + CY$str");
    }
  }

#-------------------------------------------------------------------------------

sub subb_A_direct()
  {
  my ($rb, $name);

	# SUBB	A, direct		10010101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('subb', "A, $rb", "ACC -= $name + CY");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub orl_C__bit()
  {
  my ($rb, $name);

	# ORL	C, /bit			10100000 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('orl', "C, /$rb", "CY = ~$name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub mov_C_bit()
  {
  my ($rb, $name);

	# MOV	C, bit			10100010 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('mov', "C, $rb", "CY = $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub inc_DPTR()
  {
  my $str;

	# INC	DPTR			10100011

  if ($DPTR != EMPTY)
    {
    ++$DPTR;
    $str = sprintf " (0x%04X)", $DPTR;
    }
  else
    {
    $str = '';
    }

  print_3('inc', 'DPTR', "++DPTR$str");
  }

#-------------------------------------------------------------------------------

sub mul_AB()
  {
	# MUL	AB			10100100

  print_3('mul', 'AB', 'B:ACC = ACC * B');
  }

#-------------------------------------------------------------------------------

sub unknown()
  {
  my ($rb, $str0, $str1);

	# The unknown instruction is actually simple embedded data in the code.

  if ($decoder_silent_level == SILENT0)
    {
    $rb   = sprintf "0x%02X", $dcd_instr & 0xFF;
    $str0 = present_char($dcd_instr);
    $str1 = sprintf "0x%04X", $dcd_address;
    print_3('.db', $rb, "$str1: $rb$str0");

    if ($dcd_instr_size == 2)
      {
      $rb   = sprintf "0x%02X", $dcd_parm0 & 0xFF;
      $str0 = present_char($dcd_parm0);
      $str1 = sprintf "0x%04X", $dcd_address;
      print_3('.db', $rb, "$str1: $rb$str0");
      }
    elsif ($dcd_instr_size == 3)
      {
      $rb   = sprintf "0x%02X", $dcd_parm1 & 0xFF;
      $str0 = present_char($dcd_parm1);
      $str1 = sprintf "0x%04X", $dcd_address;
      print_3('.db', $rb, "$str1: $rb$str0");
      }
    }
  }

#-------------------------------------------------------------------------------

sub anl_C__bit()
  {
  my ($rb, $name);

	# ANL	C, /bit			10110000 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('anl', "C, /$rb", "CY &= ~$name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub cpl_bit()
  {
  my ($rb, $name);

	# CPL	bit			10110010 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('cpl', $rb, "$name = ~$name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub cpl_C()
  {
	# CPL	C			10110011

  print_3('cpl', 'C', 'CY = ~CY');
  }

#-------------------------------------------------------------------------------

sub cjne_A_data()
  {
  my ($addr, $rb0, $rb1, $str0, $str1, $str2);

	# CJNE	A, #data, rel		10110100 dddddddd rrrrrrrr	data		relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb0  = labelname($addr);
    $rb1  = sprintf "0x%02X", $dcd_parm0;
    $str0 = sprintf "0x%04X", $addr;
    $str1 = jump_direction($addr);
    $str2 = present_char($dcd_parm0);
    print_3('cjne', "A, #$rb1, $rb0", "If (ACC != $rb1$str2) then jumps$str1 hither: $str0");
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub cjne_A_direct()
  {
  my ($addr, $rb0, $rb1, $name, $str0, $str1, $str2);

	# CJNE	A, direct, rel		10110101 aaaaaaaa rrrrrrrr	register address	relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb0  = reg_name($dcd_parm0, \$name);
    $rb1  = labelname($addr);
    $str0 = sprintf "0x%04X", $addr;
    $str1 = ($dcd_address == $addr) ? ' (waiting loop)' : '';
    $str2 = jump_direction($addr);
    print_3('cjne', "A, $rb0, $rb1", "If (ACC != $name) then jumps$str2 hither: $str0$str1");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub push_direct()
  {
  my ($rb, $name);

	# PUSH	direct			11000000 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('push', $rb, "++SP, [SP] = $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub clr_bit()
  {
  my ($rb, $name);

	# CLR	bit			11000010 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('clr', $rb, "$name = L");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub clr_C()
  {
	# CLR	C			11000011

  print_3('clr', 'C', 'CY = L');
  }

#-------------------------------------------------------------------------------

sub swap_A()
  {
	# SWAP	A			11000100

  print_3('swap', 'A', 'ACC[76543210] = ACC[32107654]');
  }

#-------------------------------------------------------------------------------

sub xch_A_direct()
  {
  my ($rb, $name);

	# XCH	A, direct		11000101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('xch', "A, $rb", "ACC <-> $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub pop_direct()
  {
  my ($rb, $name);

	# POP	direct			11010000 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('pop', $rb, "$name = [SP], --SP");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

sub setb_bit()
  {
  my ($rb, $name);

	# SETB	bit			11010010 bbbbbbbb		bit address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = bit_name($dcd_parm0, \$name);
    print_3('setb', $rb, "$name = H");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_bit($dcd_parm0, '', FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub setb_C()
  {
	# SETB	C			11010011

  print_3('setb', 'C', 'CY = H');
  }

#-------------------------------------------------------------------------------

sub da_A()
  {
	# DA	A			11010100

  print_3('da', 'A', 'Decimal adjust the ACC.');
  }

#-------------------------------------------------------------------------------

sub djnz_direct()
  {
  my ($addr, $rb0, $rb1, $name, $str0, $str1, $str2);

	# DJNZ	direct, rel		11010101 aaaaaaaa rrrrrrrr	register address	relative address

  if ($decoder_silent_level == SILENT0)
    {
    $addr = $dcd_address + $dcd_instr_size + expand_offset($dcd_parm1);
    $rb0  = reg_name($dcd_parm0, \$name);
    $rb1  = labelname($addr);
    $str0 = ($dcd_address == $addr) ? ' (waiting loop)' : '';
    $str1 = sprintf "0x%04X", $addr;
    $str2 = jump_direction($addr);
    print_3('djnz', "$rb0, $rb1", "If (--$name != 0) then jumps$str2 hither: $str1$str0");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_DPTR_Rx();
  $prev_is_jump = TRUE;
  }

#-------------------------------------------------------------------------------

sub movx_A_DPTR()
  {
	# MOVX	A, @DPTR		11100000

  add_xram($DPTR, '', FALSE) if ($DPTR != EMPTY &&  $decoder_silent_level == SILENT1);

  print_3('movx', 'A, @DPTR', 'ACC = XRAM[DPTR]');
  }

#-------------------------------------------------------------------------------

sub clr_A()
  {
	# CLR	A			11100100

  print_3('clr', 'A', 'ACC = 0');
  }

#-------------------------------------------------------------------------------

sub mov_A_direct()
  {
  my ($rb, $name);

	# MOV	A, direct		11100101 aaaaaaaa		register address	The "MOV A, ACC" invalid instruction.

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('mov', "A, $rb", "ACC = $name");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }
  }

#-------------------------------------------------------------------------------

sub movx_DPTR_A()
  {
	# MOVX	@DPTR, A		11110000

  add_xram($DPTR, '', FALSE) if ($DPTR != EMPTY && $decoder_silent_level == SILENT1);

  print_3('movx', '@DPTR, A', 'XRAM[DPTR] = ACC');
  }

#-------------------------------------------------------------------------------

sub cpl_A()
  {
	# CPL	A			11110100

  print_3('cpl', 'A', 'ACC = ~ACC');
  }

#-------------------------------------------------------------------------------

sub mov_direct_A()
  {
  my ($rb, $name);

	# MOV	direct, A		11110101 aaaaaaaa		register address

  if ($decoder_silent_level == SILENT0)
    {
    $rb = reg_name($dcd_parm0, \$name);
    print_3('mov', "$rb, A", "$name = ACC");
    }
  elsif ($decoder_silent_level == SILENT1)
    {
    add_ram($dcd_parm0, '', RAM_TYPE_DIR, FALSE);
    }

  invalidate_reg($dcd_parm0);
  }

#-------------------------------------------------------------------------------

my @instr_decoders =
  (
	# 0x00 - 0x0F
  \&nop,
  \&ajmp,
  \&ljmp,
  \&rr_A,
  \&inc_A,
  \&inc_direct,
  \&inc_ind_Ri,
  \&inc_ind_Ri,
  \&inc_Rn,
  \&inc_Rn,
  \&inc_Rn,
  \&inc_Rn,
  \&inc_Rn,
  \&inc_Rn,
  \&inc_Rn,
  \&inc_Rn,

	# 0x10 - 0x1F
  \&jbc_bit,
  \&acall,
  \&lcall,
  \&rrc_A,
  \&dec_A,
  \&dec_direct,
  \&dec_ind_Ri,
  \&dec_ind_Ri,
  \&dec_Rn,
  \&dec_Rn,
  \&dec_Rn,
  \&dec_Rn,
  \&dec_Rn,
  \&dec_Rn,
  \&dec_Rn,
  \&dec_Rn,

	# 0x20 - 0x2F
  \&jb_bit,
  \&ajmp,
  \&ret,
  \&rl_A,
  \&add_A_data,
  \&add_A_direct,
  \&add_A_ind_Ri,
  \&add_A_ind_Ri,
  \&add_A_Rn,
  \&add_A_Rn,
  \&add_A_Rn,
  \&add_A_Rn,
  \&add_A_Rn,
  \&add_A_Rn,
  \&add_A_Rn,
  \&add_A_Rn,

	# 0x30 - 0x3F
  \&jnb_bit,
  \&acall,
  \&reti,
  \&rcl_A,
  \&addc_A_data,
  \&addc_A_direct,
  \&addc_A_ind_Ri,
  \&addc_A_ind_Ri,
  \&addc_A_Rn,
  \&addc_A_Rn,
  \&addc_A_Rn,
  \&addc_A_Rn,
  \&addc_A_Rn,
  \&addc_A_Rn,
  \&addc_A_Rn,
  \&addc_A_Rn,

	# 0x40 - 0x4F
  \&jc,
  \&ajmp,
  \&orl_direct_A,
  \&orl_direct_data,
  \&orl_A_data,
  \&orl_A_direct,
  \&orl_A_ind_Ri,
  \&orl_A_ind_Ri,
  \&orl_A_Rn,
  \&orl_A_Rn,
  \&orl_A_Rn,
  \&orl_A_Rn,
  \&orl_A_Rn,
  \&orl_A_Rn,
  \&orl_A_Rn,
  \&orl_A_Rn,

	# 0x50 - 0x5F
  \&jnc,
  \&acall,
  \&anl_direct_A,
  \&anl_direct_data,
  \&anl_A_data,
  \&anl_A_direct,
  \&anl_A_ind_Ri,
  \&anl_A_ind_Ri,
  \&anl_A_Rn,
  \&anl_A_Rn,
  \&anl_A_Rn,
  \&anl_A_Rn,
  \&anl_A_Rn,
  \&anl_A_Rn,
  \&anl_A_Rn,
  \&anl_A_Rn,

	# 0x60 - 0x6F
  \&jz,
  \&ajmp,
  \&xrl_direct_A,
  \&xrl_direct_data,
  \&xrl_A_data,
  \&xrl_A_direct,
  \&xrl_A_ind_Ri,
  \&xrl_A_ind_Ri,
  \&xrl_A_Rn,
  \&xrl_A_Rn,
  \&xrl_A_Rn,
  \&xrl_A_Rn,
  \&xrl_A_Rn,
  \&xrl_A_Rn,
  \&xrl_A_Rn,
  \&xrl_A_Rn,

	# 0x70 - 0x7F
  \&jnz,
  \&acall,
  \&orl_C_bit,
  \&jmp_A_DPTR,
  \&mov_A_data,
  \&mov_direct_data,
  \&mov_ind_Ri_data,
  \&mov_ind_Ri_data,
  \&mov_Rn_data,
  \&mov_Rn_data,
  \&mov_Rn_data,
  \&mov_Rn_data,
  \&mov_Rn_data,
  \&mov_Rn_data,
  \&mov_Rn_data,
  \&mov_Rn_data,

	# 0x80 - 0x8F
  \&sjmp,
  \&ajmp,
  \&anl_C_bit,
  \&movc_A_A_PC,
  \&div_AB,
  \&mov_direct_direct,
  \&mov_direct_ind_Ri,
  \&mov_direct_ind_Ri,
  \&mov_direct_Rn,
  \&mov_direct_Rn,
  \&mov_direct_Rn,
  \&mov_direct_Rn,
  \&mov_direct_Rn,
  \&mov_direct_Rn,
  \&mov_direct_Rn,
  \&mov_direct_Rn,

	# 0x90 - 0x9F
  \&mov_DPTR_data,
  \&acall,
  \&mov_bit_C,
  \&movc_A_A_DPTR,
  \&subb_A_data,
  \&subb_A_direct,
  \&subb_A_ind_Ri,
  \&subb_A_ind_Ri,
  \&subb_A_Rn,
  \&subb_A_Rn,
  \&subb_A_Rn,
  \&subb_A_Rn,
  \&subb_A_Rn,
  \&subb_A_Rn,
  \&subb_A_Rn,
  \&subb_A_Rn,

	# 0xA0 - 0xAF
  \&orl_C__bit,
  \&ajmp,
  \&mov_C_bit,
  \&inc_DPTR,
  \&mul_AB,
  \&unknown,
  \&mov_ind_Ri_direct,
  \&mov_ind_Ri_direct,
  \&mov_Rn_direct,
  \&mov_Rn_direct,
  \&mov_Rn_direct,
  \&mov_Rn_direct,
  \&mov_Rn_direct,
  \&mov_Rn_direct,
  \&mov_Rn_direct,
  \&mov_Rn_direct,

	# 0xB0 - 0xBF
  \&anl_C__bit,
  \&acall,
  \&cpl_bit,
  \&cpl_C,
  \&cjne_A_data,
  \&cjne_A_direct,
  \&cjne_ind_Ri_data,
  \&cjne_ind_Ri_data,
  \&cjne_Rn_data,
  \&cjne_Rn_data,
  \&cjne_Rn_data,
  \&cjne_Rn_data,
  \&cjne_Rn_data,
  \&cjne_Rn_data,
  \&cjne_Rn_data,
  \&cjne_Rn_data,

	# 0xC0 - 0xCF
  \&push_direct,
  \&ajmp,
  \&clr_bit,
  \&clr_C,
  \&swap_A,
  \&xch_A_direct,
  \&xch_A_ind_Ri,
  \&xch_A_ind_Ri,
  \&xch_A_Rn,
  \&xch_A_Rn,
  \&xch_A_Rn,
  \&xch_A_Rn,
  \&xch_A_Rn,
  \&xch_A_Rn,
  \&xch_A_Rn,
  \&xch_A_Rn,

	# 0xD0 - 0xDF
  \&pop_direct,
  \&acall,
  \&setb_bit,
  \&setb_C,
  \&da_A,
  \&djnz_direct,
  \&xchd_A_ind_Ri,
  \&xchd_A_ind_Ri,
  \&djnz_Rn,
  \&djnz_Rn,
  \&djnz_Rn,
  \&djnz_Rn,
  \&djnz_Rn,
  \&djnz_Rn,
  \&djnz_Rn,
  \&djnz_Rn,

	# 0xE0 - 0xEF
  \&movx_A_DPTR,
  \&ajmp,
  \&movx_A_ind_Ri,
  \&movx_A_ind_Ri,
  \&clr_A,
  \&mov_A_direct,
  \&mov_A_ind_Ri,
  \&mov_A_ind_Ri,
  \&mov_A_Rn,
  \&mov_A_Rn,
  \&mov_A_Rn,
  \&mov_A_Rn,
  \&mov_A_Rn,
  \&mov_A_Rn,
  \&mov_A_Rn,
  \&mov_A_Rn,

	# 0xF0 - 0xFF
  \&movx_DPTR_A,
  \&acall,
  \&movx_ind_Ri_A,
  \&movx_ind_Ri_A,
  \&cpl_A,
  \&mov_direct_A,
  \&mov_ind_Ri_A,
  \&mov_ind_Ri_A,
  \&mov_Rn_A,
  \&mov_Rn_A,
  \&mov_Rn_A,
  \&mov_Rn_A,
  \&mov_Rn_A,
  \&mov_Rn_A,
  \&mov_Rn_A,
  \&mov_Rn_A
  );

#-------------------------------------------------------------------------------

        #
        # Decodes the $BlockRef.
        #

sub instruction_decoder($$)
  {
  my ($Address, $BlockRef) = @_;
  my ($label, $invalid);

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
  else
    {
    printf STDERR "An instruction and a block overlapped with each other at this address: 0x%04X!\n", $dcd_address;
    }

  $dcd_Ri_regs = $dcd_instr & 0x01;
  $dcd_Ri_name = "R$dcd_Ri_regs";

  $dcd_Rn_regs = $dcd_instr & 0x07;
  $dcd_Rn_name = "R$dcd_Rn_regs";

  $prev_is_jump = FALSE;

  invalidate_DPTR_Rx() if ($label->{TYPE} != BL_TYPE_NONE);

  if ($dcd_instr != EMPTY && ! $invalid)
    {
    $instr_decoders[$dcd_instr]();
    }
  else
    {
    unknown();
    }
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

    add_ram(hex($1), $2, RAM_TYPE_DIR, TRUE);
    }
  elsif ($Line =~ /^SFR\s*\(\s*([\w_]+)\s*,\s*0x([[:xdigit:]]+)\s*\)/io)
    {
	# SFR(P0, 0x80); // Port 0

    add_ram(hex($2), $1, RAM_TYPE_DIR, TRUE);
    }
  elsif ($Line =~ /^sfr\s+([\w_]+)\s*=\s*0x([[:xdigit:]]+)/io)
    {
	# sfr P1  = 0x90;

    add_ram(hex($2), $1, RAM_TYPE_DIR, TRUE);
    }
  elsif ($Line =~ /^__sbit\s+__at\s*(?:\(\s*)?0x([[:xdigit:]]+)(?:\s*\))?\s+([\w_]+)/io)
    {
	# __sbit __at (0x86) P0_6  ;

    add_bit(hex($1), $2, TRUE);
    }
  elsif ($Line =~ /^SBIT\s*\(\s*([\w_]+)\s*,\s*0x([[:xdigit:]]+)\s*,\s*(\d)\s*\)/io)
    {
	# SBIT(P0_0, 0x80, 0); // Port 0 bit 0

    add_bit(hex($2) + int($3), $1, TRUE);
    }
  elsif ($Line =~ /^sbit\s+([\w_]+)\s*=\s*0x([[:xdigit:]]+)/io)
    {
	# sbit P3_1 = 0xB1;

    add_bit(hex($2), $1, TRUE);
    }
  elsif ($Line =~ /^sbit\s+([\w_]+)\s*=\s*([\w_]+)\s*\^\s*(\d)/io)
    {
	# sbit SM0  = SCON^7;

    my ($name, $reg, $bit) = ($1, $2, $3);

    add_bit($sfr_by_names{$reg} + $bit, $name, TRUE) if (defined($sfr_by_names{$reg}));
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
	# Among the blocks stows description of an instruction.
	#

sub add_instr_block($$)
  {
  my ($Address, $Instruction) = @_;
  my $instr_size = $instruction_sizes[$Instruction];
  my $invalid;

  $dcd_address = $Address;
  $dcd_instr   = $rom[$dcd_address];
  $invalid     = FALSE;

  if (! $instr_size)
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
      $dcd_parm0 = $rom[$dcd_address + 1];
      $invalid = TRUE if ($dcd_parm0 == EMPTY);
      }

    if ($instr_size == 3)
      {
      $dcd_parm1 = $rom[$dcd_address + 2];
      $invalid = TRUE if ($dcd_parm1 == EMPTY);
      }

    if ($invalid)
      {
      add_block($Address, BLOCK_CONST, $instr_size, BL_TYPE_NONE, '') if ($instr_size);
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
  $empty_begin = 0;
  $is_const = FALSE;
  $const_begin = 0;

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

      $i += add_instr_block($i, $instr);
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
	# If the $BlockRef a call, then follows.
        #

sub follow_call($)
  {
  my $BlockRef = $_[0];
  my ($addr, $size, $instr);

  $addr  = $BlockRef->{ADDR};
  $size  = $BlockRef->{SIZE};
  $instr = $rom[$addr];

  if ($instr == INST_LCALL)
    {
    return (($rom[$addr + 1] << 8) | $rom[$addr + 2]);
    }
  elsif (($instr & 0x1F) == INST_ACALL)
    {
    return ((($addr + $size) & 0xF800) | (($instr & 0xE0) << 3) | $rom[$addr + 1]);
    }

  return EMPTY;
  }

#-------------------------------------------------------------------------------

	#
	# If the $BlockRef a unconditional jump, then follows.
	#

sub follow_unconditional_jump($)
  {
  my $BlockRef = $_[0];
  my ($addr, $size, $instr);

  $addr  = $BlockRef->{ADDR};
  $size  = $BlockRef->{SIZE};
  $instr = $rom[$addr];

  if ($size == 3 && $instr == INST_LJMP)
    {
    return (($rom[$addr + 1] << 8) | $rom[$addr + 2]);
    }
  elsif ($size == 2)
    {
    if (($instr & 0x1F) == INST_AJMP)
      {
      return ((($addr + $size) & 0xF800) | ((($instr & 0xE0) << 3) | $rom[$addr + 1]));
      }
    elsif ($instr == INST_SJMP)
      {
      return ($addr + $size + expand_offset($rom[$addr + 1]));
      }
    }

  return EMPTY;
  }

#-------------------------------------------------------------------------------

        #
	# If the $BlockRef a conditional jump, then follows.
        #

sub follow_conditional_jump($)
  {
  my $BlockRef = $_[0];
  my ($addr, $size, $instr, $instr_mask1, $instr_mask2);

  return EMPTY if ($BlockRef->{TYPE} != BLOCK_INSTR);

  $addr  = $BlockRef->{ADDR};
  $size  = $BlockRef->{SIZE};
  $instr = $rom[$addr];

  $instr_mask1 = $instr & 0xFE;
  $instr_mask2 = $instr & 0xF8;

  if ($instr_mask1 == INST_CJNE__Ri_DATA ||
      $instr_mask2 == INST_CJNE_Rn_DATA)
    {
    return ($addr + $size + expand_offset($rom[$addr + 2]));
    }
  elsif ($instr_mask2 == INST_DJNZ_Rn)
    {
    return ($addr + $size + expand_offset($rom[$addr + 1]));
    }
  elsif ($instr == INST_JBC_BIT ||
	 $instr == INST_JB_BIT ||
	 $instr == INST_JNB_BIT ||
	 $instr == INST_CJNE_A_DATA ||
	 $instr == INST_CJNE_A_DIRECT ||
	 $instr == INST_DJNZ_DIRECT)
    {
    return ($addr + $size + expand_offset($rom[$addr + 2]));
    }
  elsif ($instr == INST_JC ||
	 $instr == INST_JNC ||
	 $instr == INST_JZ ||
	 $instr == INST_JNZ ||
	 $instr == INST_SJMP)
    {
    return ($addr + $size + expand_offset($rom[$addr + 1]));
    }

  return EMPTY;
  }

#-------------------------------------------------------------------------------

	#
	# Determines the start of stack.
	#

sub determine_stack()
  {
  my ($block, $instr, $addr, $count, $size, $t);

  return if (! defined($blocks_by_address{0}));

  $addr = follow_unconditional_jump(\%{$blocks_by_address{0}});
  return if ($addr == EMPTY);

  return if (! defined($blocks_by_address{$addr}));

	# At most so much block looks through.
  $count = 30;
  do
    {
    $block = \%{$blocks_by_address{$addr}};

    return if ($block->{TYPE} != BLOCK_INSTR);
    return if (follow_unconditional_jump($block) != EMPTY);
    return if (follow_call($block) != EMPTY);

    $addr  = $block->{ADDR};
    $size  = $block->{SIZE};
    $instr = $rom[$addr];

    return if ($size == 2 && ($instr == INST_PUSH_DIRECT || $instr == INST_POP_DIRECT));

    $t = follow_conditional_jump($block);

    return if ($t != EMPTY && $t > $addr);

    if ($size == 3 && $instr == INST_MOV_DIRECT_DATA && $rom[$addr + 1] == SP)
      {
      $stack_start = $rom[$addr + 2] + 1;
      $stack_size  = RAM_MAX_ADDR - $stack_start + 1;
      return;
      }

    $addr += $size;	# Compute the address of next block.
    }
  while (--$count);
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

    next if ($block->{TYPE} != BLOCK_INSTR);

    if ($prev_addr != EMPTY)
      {
      $instr = $rom[$prev_addr];
      $label = $block->{LABEL};

      if (defined($label) && $label->{TYPE} == BL_TYPE_NONE)
	{
	if ($instr == INST_RET || $instr == INST_RETI)
	  {
	  Log(sprintf("Lost function label at the 0x%04X address.", $_), 5);
	  add_func_label($_, '', TRUE);
	  }
	elsif ($instr == INST_LJMP || $instr == INST_SJMP || ($instr & 0x1F) == INST_AJMP)
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
	# Adds the jump addresses from a jump table.
	#

sub add_jump_address_in_table($$$)
  {
  my ($AddrLow, $AddrHigh, $Size) = @_;
  my ($end, $addr);

  $end = $AddrLow + $Size;
  while ($AddrLow < $end)
    {
    $addr = $rom[$AddrLow] | ($rom[$AddrHigh] << 8);
    add_jump_label($addr, '', BL_TYPE_JLABEL, EMPTY, FALSE);
    ++$AddrLow;
    ++$AddrHigh;
    }
  }

#-------------------------------------------------------------------------------

	#
	# Jump tables looking for in the code.
	#

sub recognize_jump_tables_in_code($)
  {
  my $Survey = $_[0];
  my @blocks = ((undef) x 9);
  my @instrs = ((EMPTY) x 9);
  my ($parm, $data1, $data2, $addrL, $addrH, $end, $size);

  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    shift(@instrs);
    push(@instrs, $rom[$_]);

    shift(@blocks);
    push(@blocks, \%{$blocks_by_address{$_}});

    next if (! defined($blocks[0]) || ! defined($blocks[8]));
    next if ($blocks[0]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[1]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[2]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[3]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[4]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[5]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[6]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[7]->{TYPE} != BLOCK_INSTR);
    next if ($blocks[8]->{TYPE} != BLOCK_INSTR);

    if ($blocks[0]->{SIZE} == 2 && $instrs[0] == INST_ADD_A_DATA &&		# add	A, #0xZZ

	$blocks[1]->{SIZE} == 1 && $instrs[1] == INST_MOVC_A_APC &&		# movc	A, @A+PC

	$blocks[2]->{SIZE} == 2 && $rom[$blocks[2]->{ADDR} + 1] == DPL &&
	($instrs[2] == INST_XCH_A_DIRECT ||					# xch	A, DPL
	 $instrs[2] == INST_MOV_DIRECT_A) &&					# mov	DPL, A

	(($blocks[3]->{SIZE} == 2 && $instrs[3] == INST_MOV_A_DIRECT) ||	# mov	A, direct
	 ($blocks[3]->{SIZE} == 1 && (($instrs[3] & 0xF8) == INST_MOV_A_Rn || 	# mov	A, Rn
				      ($instrs[3] & 0xFE) == INST_MOV_A_Ri))) && # mov	A, @Ri

	$blocks[4]->{SIZE} == 2 && $instrs[4] == INST_ADD_A_DATA &&		# add	A, #0xZZ

	$blocks[5]->{SIZE} == 1 && $instrs[5] == INST_MOVC_A_APC &&		# movc	A, @A+PC

	$blocks[6]->{SIZE} == 2 && $instrs[6] == INST_MOV_DIRECT_A &&		# mov	DPH, A
	$rom[$blocks[6]->{ADDR} + 1] == DPH &&

	$blocks[7]->{SIZE} == 1 && $instrs[7] == INST_CLR_A &&			# clr	A

	$blocks[8]->{SIZE} == 1 && $instrs[8] == INST_JMP_A_DPTR)		# jmp	@A+DPTR
      {
=back
	24 0B		add	A, #0x0B
	83		movc	A, @A+PC
	F5 82		mov	DPL, A

	E5 F0		mov	A, B
	    or
	ED		mov	A, R5

	24 25		add	A, #0x25
	83		movc	A, @A+PC
	F5 83		mov	DPH, A
	E4		clr	A
	73		jmp	@A+DPTR
=cut

      $addrL = $blocks[2]->{ADDR} + $rom[$blocks[0]->{ADDR} + 1];
      $addrH = $blocks[6]->{ADDR} + $rom[$blocks[4]->{ADDR} + 1];
      $size  = $addrH - $addrL;
      $end   = $addrH + $size - 1;

      if ($Survey)
	{
	add_const_area($addrL, $end);
	}
      else
	{
	add_block($addrL, BLOCK_JTABLE, $size, BL_TYPE_JTABLE, '');
	add_block($addrH, BLOCK_JTABLE, $size, BL_TYPE_JTABLE, '');
	add_jump_address_in_table($addrL, $addrH, $size);
	}
      }
    elsif ($blocks[0]->{SIZE} == 3 && $instrs[0] == INST_MOV_DPTR_DATA &&	# mov	DPTR, #tLow

	   $blocks[1]->{SIZE} == 1 && $instrs[1] == INST_MOVC_A_DPTR &&		# movc	A, @A+DPTR

	   (($blocks[2]->{SIZE} == 2 && $instrs[2] == INST_XCH_A_DIRECT) ||	# xch	A, XX
	    ($blocks[2]->{SIZE} == 2 && $instrs[2] == INST_PUSH_DIRECT &&	# push	ACC
	     $rom[$blocks[2]->{ADDR} + 1] == ACC)) &&

	   $blocks[3]->{SIZE} == 3 && $instrs[3] == INST_MOV_DPTR_DATA &&	# mov	DPTR, #tHigh

	   $blocks[4]->{SIZE} == 1 && $instrs[4] == INST_MOVC_A_DPTR &&		# movc	A, @A+DPTR

	   $blocks[5]->{SIZE} == 2 && $instrs[5] == INST_MOV_DIRECT_A &&	# mov	DPH, A
	   $rom[$blocks[5]->{ADDR} + 1] == DPH &&

	   (($blocks[6]->{SIZE} == 3 && $instrs[6] == INST_MOV_DIRECT_DIRECT &&	# mov	DPL, XX
	     $rom[$blocks[6]->{ADDR} + 2] == DPL) ||
	    ($blocks[6]->{SIZE} == 2 && $instrs[6] == INST_POP_DIRECT &&	# pop	DPL
	     $rom[$blocks[6]->{ADDR} + 1] == DPL)) &&

	   $blocks[7]->{SIZE} == 1 && $instrs[7] == INST_CLR_A &&		# clr	A

	   $blocks[8]->{SIZE} == 1 && $instrs[8] == INST_JMP_A_DPTR)		# jmp	@A+DPTR
      {
=back
	90 00 79	mov	DPTR, #0x0079
	93		movc	A, @A+DPTR

	C5 F0		xch	A, B
	    or
	C0 E0		push	ACC

	90 01 79	mov	DPTR, #0x0179
	93		movc	A, @A+DPTR
	F5 83		mov	DPH, A

	85 F0 82	mov	DPL, B
	    or
	D0 82		pop	DPL

	E4		clr	A
	73		jmp	@A+DPTR
=cut

      $addrL = ($rom[$blocks[0]->{ADDR} + 1] << 8) | $rom[$blocks[0]->{ADDR} + 2];
      $addrH = ($rom[$blocks[3]->{ADDR} + 1] << 8) | $rom[$blocks[3]->{ADDR} + 2];
      $size  = $addrH - $addrL;
      $end   = $addrH + $size - 1;

      if ($Survey)
	{
	add_const_area($addrL, $end);
	}
      else
	{
	add_block($addrL, BLOCK_JTABLE, $size, BL_TYPE_JTABLE, '');
	add_block($addrH, BLOCK_JTABLE, $size, BL_TYPE_JTABLE, '');
	add_jump_address_in_table($addrL, $addrH, $size);
	}
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
	# Prints the SFRs.
	#

sub emit_sfrs($)
  {
  my $Assembly_mode = $_[0];
  my ($sfr, $cnt, $str0, $str1);

  return if (! scalar(keys(%sfr_by_address)));

  if ($Assembly_mode)
    {
    print ";$border0\n;\tSpecial function registers\n;$border0\n\n" .
	  "\t.area\tRSEG\t(ABS,DATA)\n\t.org\t0x0000\n\n";

    foreach (sort {$a <=> $b} keys(%sfr_by_address))
      {
      printf "$sfr_by_address{$_}->{NAME}\t=\t0x%02X\n", $_;
      }
    }
  else
    {
    print ";$border0\n;\tSFR registers\n;$border0\n\n";

    foreach (sort {$a <=> $b} keys(%sfr_by_address))
      {
      $sfr  = $sfr_by_address{$_};
      $str0 = sprintf "0x%02X", $_;
      $cnt  = sprintf "%3u", $sfr->{REF_COUNT};
      $str1 = ($sfr->{REF_COUNT}) ? "used $cnt times" : 'not used';
      print "${str0}:\t" . align($sfr->{NAME}, STAT_ALIGN_SIZE) . "($str1)\n";
      }
    }

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints the SFR bits.
	#

sub emit_sfr_bits($)
  {
  my $Assembly_mode = $_[0];
  my ($sbit, $cnt, $str0, $str1);

  return if (! scalar(keys(%sfr_bit_by_address)));

  if ($Assembly_mode)
    {
    print ";$border0\n;\tSpecial function bits\n;$border0\n\n" .
	  "\t.area\tRSEG\t(ABS,DATA)\n\t.org\t0x0000\n\n";

    foreach (sort {$a <=> $b} keys(%sfr_bit_by_address))
      {
      printf "$sfr_bit_by_address{$_}->{NAME}\t=\t0x%02X\n", $_;
      }
    }
  else
    {
    print ";$border0\n;\tSpecial function bits\n;$border0\n\n";

    foreach (sort {$a <=> $b} keys(%sfr_bit_by_address))
      {
      $sbit = $sfr_bit_by_address{$_};
      $str0 = sprintf "0x%02X", $_;
      $cnt  = sprintf "%3u", $sbit->{REF_COUNT};
      $str1 = ($sbit->{REF_COUNT}) ? "used $cnt times" : 'not used';
      print "${str0}:\t" . align($sbit->{NAME}, STAT_ALIGN_SIZE) . "($str1)\n";
      }
    }

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints the banks.
	#

sub emit_banks($)
  {
  my $Assembly_mode = $_[0];

  $used_banks{0} = TRUE;

  print ";$border0\n;\tOverlayable register banks\n;$border0\n\n";

  if ($Assembly_mode)
    {
    foreach (sort {$a <=> $b} keys(%used_banks))
      {
      print "\t.area\tREG_BANK_$_\t(REL,OVR,DATA)\n\t.ds 8\n";
      }
    }
  else
    {
    foreach (sort {$a <=> $b} keys(%used_banks))
      {
      printf "0x%02X:\tREG_BANK_$_\n", $_ * 8;
      }
    }

  print "\n";
  }


#-------------------------------------------------------------------------------

	#
	# Prints the registers (variables).
	#

sub emit_ram_data($)
  {
  my $Assembly_mode = $_[0];
  my ($ram, $first, $next_addr, $size);
  my ($cnt, $str0, $str1);

  return if (! scalar(keys(%ram_by_address)));

  if ($Assembly_mode)
    {
    print ";$border0\n;\tAbsolute internal RAM data\n;$border0\n\n" .
	  "\t.area\tIABS\t(ABS,DATA)\n";

    $first = TRUE;
    $next_addr = EMPTY;
    foreach (sort {$a <=> $b} keys(%ram_by_address))
      {
      last if ($_ > RAM_LAST_DIR_ADDR);

      $ram = $ram_by_address{$_};

      next if ($ram->{TYPE} != RAM_TYPE_DIR);

      if ($first)
	{
	printf "\t.org\t0x%02X\n\n", $_;
	$first = FALSE;
	}

      if ($next_addr != EMPTY)
	{
	next if ($_ < $next_addr);

	printf("\t.ds 0x%02X\n", $_ - $next_addr) if ($_ > $next_addr);
	}

      if ($ram->{NAME} ne '')
	{
	print "$ram->{NAME}::\n\t.ds $ram->{SIZE}\n";
	$next_addr = $_ + $size;
	}
      else
	{
	printf "r0x%02X::\n\t.ds 1\n", $_;
	$next_addr = $_ + 1;
	}
      } # foreach (sort {$a <=> $b} keys(%ram_by_address))
    }
  else
    {
    print ";$border0\n;\tInternal RAM data\n;$border0\n\n";

    $next_addr = EMPTY;
    foreach (sort {$a <=> $b} keys(%ram_by_address))
      {
      last if ($_ > RAM_LAST_DIR_ADDR);

      $ram = $ram_by_address{$_};

      next if ($ram->{TYPE} != RAM_TYPE_DIR);

      next if ($next_addr != EMPTY && $_ < $next_addr);

      $str0 = sprintf "0x%02X", $_;
      $cnt  = sprintf "%3u", $ram->{REF_COUNT};
      $str1 = ($ram->{REF_COUNT}) ? "used $cnt times" : 'not used';

      if ($ram->{NAME} ne '')
	{
        $cnt = sprintf "%3u", $ram->{SIZE};
	print "${str0}:\t" . align($ram->{NAME}, STAT_ALIGN_SIZE) . "($cnt bytes) ($str1)\n";
	$next_addr = $_ + $ram->{SIZE};
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
      } # foreach (sort {$a <=> $b} keys(%ram_by_address))
    }

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints the list of registers in indirect RAM.
	#

sub emit_indirect_ram($)
  {
  my $Assembly_mode = $_[0];
  my ($ram, $name, $cnt, $str0, $str1, $str2);

  return if (! scalar(keys(%ram_by_address)));

  if ($Assembly_mode)
    {
    print ";$border0\n;\tIndirectly addressable internal RAM data\n;$border0\n\n" .
	  "\t.area\tISEG\t(DATA)\n\n";

    foreach (sort {$a <=> $b} keys(%ram_by_address))
      {
      $ram = $ram_by_address{$_};

      next if ($ram->{TYPE} != RAM_TYPE_IND);

      $name = $ram->{NAME};
      $str0 = sprintf "0x%02X", $_;
      $str1 = ($name ne '') ? $name : "iram_$str0";
      printf "${str1}::\n\t.ds 1\n", $_;
      }
    }
  else
    {
    print ";$border0\n;\tIndirectly addressable internal RAM data\n;$border0\n\n";

    foreach (sort {$a <=> $b} keys(%ram_by_address))
      {
      $ram = $ram_by_address{$_};

      next if ($ram->{TYPE} != RAM_TYPE_IND);

      $name = $ram->{NAME};
      $str0 = sprintf "0x%02X", $_;
      $str1 = ($name ne '') ? $name : "iram_$str0";
      $cnt  = sprintf "%3u", $ram->{REF_COUNT};
      $str2 = ($ram->{REF_COUNT}) ? "used $cnt times" : 'not used';
      print "${str0}:\t" . align($str1, STAT_ALIGN_SIZE) . "($str2)\n";
      }
    }

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints the list of registers in external RAM.
	#

sub emit_external_ram($)
  {
  my $Assembly_mode = $_[0];
  my ($xram, $name, $next_addr, $cnt, $str0, $str1, $str2);

  return if (! scalar(keys(%xram_by_address)));

  if ($Assembly_mode)
    {
    print ";$border0\n;\tExternal RAM data\n;$border0\n\n\t.area\tXSEG\t(XDATA)\n\n";

    foreach (sort {$a <=> $b} keys(%xram_by_address))
      {
      $xram = $xram_by_address{$_};
      $name = $xram->{NAME};
      $str0 = sprintf "0x%04X", $_;
      $str1 = ($name ne '') ? $name : "xram_$str0";
      print "$str1\t=\t$str0\n";
      }
    }
  else
    {
    print ";$border0\n;\tExternal RAM data\n;$border0\n\n";

    $next_addr = EMPTY;
    foreach (sort {$a <=> $b} keys(%xram_by_address))
      {
      next if ($next_addr != EMPTY && $_ < $next_addr);

      $xram = $xram_by_address{$_};
      $str0 = sprintf "0x%04X", $_;
      $cnt  = sprintf "%3u", $xram->{REF_COUNT};
      $str1 = ($xram->{REF_COUNT}) ? "used $cnt times" : 'not used';

      if ($xram->{NAME} ne '')
	{
        $cnt = sprintf "%3u", $xram->{SIZE};
	print "${str0}:\t" . align($xram->{NAME}, STAT_ALIGN_SIZE) . "($cnt bytes) ($str1)\n";
	$next_addr = $_ + $xram->{SIZE};
	}
      else
	{
	$str2 = "xram_$str0";
	print "${str0}:\t" . align($str2, STAT_ALIGN_SIZE) . "($str1)\n";
	$next_addr = $_ + 1;
	}
      }
    }

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints the bits.
	#

sub emit_bits($)
  {
  my $Assembly_mode = $_[0];
  my ($bit, $name, $cnt, $str0, $str1, $str2);

  return if (! scalar(keys(%bit_by_address)));

  if ($Assembly_mode)
    {
    print ";$border0\n;\tbit data\n;$border0\n\n" .
	  "\t.area\tBSEG\t(BIT)\n\n";

    foreach (sort {$a <=> $b} keys(%bit_by_address))
      {
      $bit  = $bit_by_address{$_};
      $name = $bit->{NAME};
      $str0 = ($name ne '') ? $name : (sprintf "bit_0x%02X", $_);
      print "${str0}::\n\t.ds 1\n";
      }
    }
  else
    {
    print ";$border0\n;\tbit data\n;$border0\n\n";

    foreach (sort {$a <=> $b} keys(%bit_by_address))
      {
      $bit  = $bit_by_address{$_};
      $name = $bit->{NAME};
      $str0 = sprintf "0x%02X", $_;
      $str1 = ($name ne '') ? $name : "bit_$str0";
      $cnt  = sprintf "%3u", $bit->{REF_COUNT};
      $str2 = ($bit->{REF_COUNT}) ? "used $cnt times" : 'not used';
      print "${str0}:\t" . align($str1, STAT_ALIGN_SIZE) . "($str2)\n";
      }
    }

  print "\n";
  }

#-------------------------------------------------------------------------------

	#
	# Prints the map from the RAM.
	#

sub emit_ram_map()
  {
  my @map = ();
  my ($ram, $i, $v);

  for ($i = 0; $i <= RAM_MAX_ADDR; ++$i)
    {
    $ram = $ram_by_address{$i};

    if (defined($ram))
      {
      $map[$i] = ($ram->{TYPE} == RAM_TYPE_DIR) ? 'd' : 'I';
      }
    else
      {
      $map[$i] = ' ';
      }
    }

  $used_banks{0} = TRUE;
  foreach (keys(%used_banks))
    {
    $i = $_ * 8;
    $v = $i + 7;
    do
      {
      $map[$i] = $_;
      ++$i;
      }
    while ($i <= $v);
    }

  foreach (keys(%bit_by_address))
    {
    $map[0x20 + int($_ / 8)] = 'B';
    }

  if ($stack_start > 0)
    {
    for ($i = $stack_start; $i <= RAM_MAX_ADDR; ++$i)
      {
      $map[$i] = 'S';
      }
    }

  print ";$border0\n;\tInternal RAM layout\n;$border0\n\n";

  for ($i = 0; $i <= RAM_MAX_ADDR; $i += 16)
    {
    printf "0x%02X: |", $i & 0xF0;
    print join('|', @map[$i .. ($i + 15)]) . "|\n";
    }

  print "\n0-3:Register Banks, B:Bits, d:Data, I:iRAM, S:Stack\n\n";
  }

#-------------------------------------------------------------------------------

	#
	# Find the last address of HOME segment.
	#

sub find_HOME_last_address()
  {
  my $last_addr = 0;

  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    my $block = \%{$blocks_by_address{$_}};

    last if ($block->{TYPE} == BLOCK_CONST);

    if ($block->{TYPE} == BLOCK_INSTR)
      {
      last if (! defined($interrupts_by_address{$_}));

      last if ($block->{SIZE} > LJMP_SIZE);

      $last_addr = $_;
      }
    }

  return $last_addr;
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

  if ($type == BL_TYPE_SUB)
    {
    print "\n;$border0\n";
    }
  elsif ($type == BL_TYPE_JLABEL)
    {
    print "\n\t;$border2\n";
    }

  printf "\n$label->{NAME}:\n\n";
  $label->{PRINTED} = TRUE;
  $prev_is_jump = FALSE;
  return TRUE;
  }

#-------------------------------------------------------------------------------

	#
	# Prints a table of constants.
	#

sub print_constants($$)
  {
  my ($Address, $BlockRef) = @_;
  my ($size, $i, $len, $frag, $byte, $spc, $col);
  my ($left_align, $right_align, $hc);
  my @constants;
  my @line;

  $size = $BlockRef->{SIZE};

  return if (! $size);

  $hc = ($BlockRef->{TYPE} == BLOCK_JTABLE) || $hex_constant;

  $prev_is_jump = FALSE;

  $col = ($gen_assembly_code) ? '      ' : '    ';

  if ($gen_assembly_code)
    {
    print ";$table_border\n;\t\t $table_header  |\n;$table_border\n";
    }
  else
    {
    print "$table_border\n|          |  $table_header  |  $table_header  |\n$table_border\n";
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
      printf "|  0x%04X  | ", $Address;
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

    if ($gen_assembly_code)
      {
      print $left_align .
	    join(', ', map {
			   sprintf((($hc || $_ < ord(' ') || $_ >= 0x7F) ? "0x%02X" : "'%c' "), $_ & 0xFF);
			   } @line) . "$right_align ;\n";
      }
    else
      {
      print " $left_align" . join(' ', map { sprintf("%02X ", $_ & 0xFF); } @line);

      print "$right_align | $left_align " .
	    join(' ', map {
			  sprintf((($_ < ord(' ') || $_ >= 0x7F) ? "%02X " : "'%c'"), $_ & 0xFF);
			  } @line) . "$right_align |\n";
      }
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
  my ($ref, $home_last_addr, $sname);

  $home_last_addr = 0;
  $prev_is_jump = FALSE;
  $decoder_silent_level = SILENT0;

  if ($gen_assembly_code)
    {
    $table_header = join('    ', map { sprintf '%02X', $_ } (0 .. (TBL_COLUMNS - 1)));
    $table_border = ('-' x (TBL_COLUMNS * 6 + 14)) . '+';
    }
  else
    {
    $table_header = join('  ', map { sprintf '%02X', $_ } (0 .. (TBL_COLUMNS - 1)));
    $table_border = '+' . ('-' x 10) . '+' . ('-' x (TBL_COLUMNS * 4 + 2)) . '+' . ('-' x (TBL_COLUMNS * 4 + 2)) . '+';
    }

  invalidate_DPTR_Rx();
  print "\n";

  if ($gen_assembly_code)
    {
    emit_globals(TRUE);
    emit_sfrs(TRUE);
    emit_sfr_bits(TRUE);
    emit_banks(TRUE);
    emit_ram_data(TRUE);
    emit_bits(TRUE);

    if ($stack_start > 0)
      {
      print ";$border0\n;\tStack segment\n;$border0\n\n\t.area\tSSEG\t(DATA)\n" .
	    "__start__stack:\n\t.ds 1\n\n";
      }

    emit_indirect_ram(TRUE);
    emit_external_ram(TRUE);

    $home_last_addr = find_HOME_last_address();
    $sname = ($home_last_addr > 0) ? 'HOME' : 'CSEG';
    print ";$border0\n;\tCode\n;$border0\n\n\t.area\t$sname\t(CODE)\n\n";
    }
  else
    {
    emit_ram_map();
    emit_globals(FALSE);
    emit_sfrs(FALSE);
    emit_sfr_bits(FALSE);
    emit_banks(FALSE);
    emit_ram_data(FALSE);
    emit_bits(FALSE);

    if ($stack_start > 0)
      {
      printf ";$border0\n;\tStack: start=0x%02X, size=%u bytes\n;$border0\n\n", $stack_start, $stack_size;
      }

    emit_indirect_ram(FALSE);
    emit_external_ram(FALSE);
    print ";$border0\n\n";
    }

  foreach (sort {$a <=> $b} keys(%blocks_by_address))
    {
    $ref = \%{$blocks_by_address{$_}};

    if ($ref->{TYPE} == BLOCK_INSTR)
      {
      if ($home_last_addr > 0 && $_ > $home_last_addr)
	{
	print "\n;$border0\n;\tCode\n;$border0\n\n\t.area\tCSEG\t(CODE)\n";
	$home_last_addr = 0;
	}

      print_label($_);
      print "\n" if ($prev_is_jump);

      instruction_decoder($_, $ref);
      }
    elsif ($ref->{TYPE} == BLOCK_CONST || $ref->{TYPE} == BLOCK_JTABLE)
      {
      print_label($_);
      print "\n" if ($prev_is_jump);

      print_constants($_, $ref);
      }
    elsif ($ref->{TYPE} == BLOCK_EMPTY)
      {
      my $next_block = $_ + $ref->{SIZE};

      if (! $gen_assembly_code)
	{
	printf("\n0x%04X: -- -- --\n  ....  -- -- --\n0x%04X: -- -- --\n", $_, $next_block - 1);
	}
      elsif ($next_block < $rom_size)
	{
	# Skip the empty code space.

	printf "\n\t.ds\t%u\n", $ref->{SIZE};
	}
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

	    Path of the header files of MCS51 MCUs. (Default: $default_include_path)

	--map-file <file.map>

	    The map file belonging to the input hex file. (optional)

	-r|--rom-size <size of program memory>

EOT
;
  printf "\t    Defines size of the program memory. (Default %u bytes.)\n", MCS51_ROM_SIZE;
  print <<EOT;

	--const-area <start address> <end address>

	    Designates a constant area (jumptables, texts, etc.), where data is
	    stored happen. The option may be given more times, that to select
	    more areas at the same time. (optional)

	-hc|--hex-constant

	    The constants only shows in hexadecimal form. Otherwise if possible,
	    then shows all in character form.

	-as|--assembly-source

	    Generates the assembly source file. (Eliminates before the instructions
	    visible address, hex codes and besides replaces the pseudo Rn<#x>
	    register names.) Emits global symbol table, SFR table, Bits table, etc.

	-rj|--recognize-jump-tables

	    Recognizes the jump tables.

	-fl|--find-lost-labels

	    Finds the "lost" labels. These may be found such in program parts,
	    which are directly not get call.

	--name-list <list_file>

	    The file contains list of names. They may be: Names of variables and
	    names of labels. For example:

		[BIT]
		0x07:bit_variable
		..
		..
		..
		[RAM]
		0x31:direct_ram_variable
		..
		..
		..
		[IRAM]
		0x91:indirect_ram_variable
		..
		..
		..
		[XRAM]
		0x208:external_ram_variable
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
	printf STDERR "$PROGRAM: Code size of the MCS51 family greater than 1024 bytes!\n";
	exit(1);
	}
      elsif ($rom_size > MCS51_ROM_SIZE)
	{
	printf STDERR "$PROGRAM: Code size of the MCS51 family not greater %u bytes!\n", MCS51_ROM_SIZE;
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
	$start = MCS51_ROM_SIZE - 1;
	$end   = MCS51_ROM_SIZE - 1;
	}

      add_const_area($start, $end) if ($start < $end);
      } # when (/^--const-area$/o)

    when (/^-(hc|-hex-constant)$/o)
      {
      $hex_constant = TRUE;
      }

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

    when (/^-(rj|-recognize-jump-tables)$/o)
      {
      $recognize_jump_tables = TRUE;
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
else
  {
  $embed_level = 0;
  foreach (grep(! /^\s*$|^\s*#/o, <DATA>))
    {
    process_header_line($_);
    }
  }

if ($map_file eq '')
  {
  ($map_file) = ($hex_file =~ /^(.+)\.hex$/io);
  $map_file .= '.map';
  }

$map_file = '' if (! -e $map_file);

is_file_ok($name_list) if ($name_list ne '');

if ($recognize_jump_tables)
  {
  split_code_to_blocks();
  recognize_jump_tables_in_code(TRUE);
  %blocks_by_address = ();
  read_map_file();
  read_name_list();
  split_code_to_blocks();
  recognize_jump_tables_in_code(FALSE);
  }
else
  {
  read_map_file();
  read_name_list();
  split_code_to_blocks();
  }

fix_multi_byte_variables();
determine_stack();
preliminary_survey(SILENT2);
preliminary_survey(SILENT1);
find_labels_in_code();
find_lost_labels_in_code() if ($find_lost_labels);
add_names_labels();
disassembler();
print_hidden_labels() if ($verbose > 2);

__END__
################################################################################
#
# These the SFR-s and SBIT-s of the i8052 MCU.
#

__sfr __at (0x80) P0;
__sfr __at (0x81) SP;
__sfr __at (0x82) DPL;
__sfr __at (0x83) DPH;
__sfr __at (0x87) PCON;
__sfr __at (0x88) TCON;
__sfr __at (0x89) TMOD;
__sfr __at (0x8A) TL0;
__sfr __at (0x8B) TL1;
__sfr __at (0x8C) TH0;
__sfr __at (0x8D) TH1;
__sfr __at (0x90) P1;
__sfr __at (0x98) SCON;
__sfr __at (0x99) SBUF;
__sfr __at (0xA0) P2;
__sfr __at (0xA8) IE;
__sfr __at (0xB0) P3;
__sfr __at (0xB8) IP;
__sfr __at (0xC8) T2CON;
__sfr __at (0xCA) RCAP2L;
__sfr __at (0xCB) RCAP2H;
__sfr __at (0xCC) TL2;
__sfr __at (0xCD) TH2;
__sfr __at (0xD0) PSW;
__sfr __at (0xE0) ACC;
__sfr __at (0xF0) B;

__sbit __at (0x80) P0_0;
__sbit __at (0x81) P0_1;
__sbit __at (0x82) P0_2;
__sbit __at (0x83) P0_3;
__sbit __at (0x84) P0_4;
__sbit __at (0x85) P0_5;
__sbit __at (0x86) P0_6;
__sbit __at (0x87) P0_7;

__sbit __at (0x88) IT0;
__sbit __at (0x89) IE0;
__sbit __at (0x8A) IT1;
__sbit __at (0x8B) IE1;
__sbit __at (0x8C) TR0;
__sbit __at (0x8D) TF0;
__sbit __at (0x8E) TR1;
__sbit __at (0x8F) TF1;

__sbit __at (0x90) P1_0;
__sbit __at (0x91) P1_1;
__sbit __at (0x92) P1_2;
__sbit __at (0x93) P1_3;
__sbit __at (0x94) P1_4;
__sbit __at (0x95) P1_5;
__sbit __at (0x96) P1_6;
__sbit __at (0x97) P1_7;

__sbit __at (0x98) RI;
__sbit __at (0x99) TI;
__sbit __at (0x9A) RB8;
__sbit __at (0x9B) TB8;
__sbit __at (0x9C) REN;
__sbit __at (0x9D) SM2;
__sbit __at (0x9E) SM1;
__sbit __at (0x9F) SM0;

__sbit __at (0xA0) P2_0;
__sbit __at (0xA1) P2_1;
__sbit __at (0xA2) P2_2;
__sbit __at (0xA3) P2_3;
__sbit __at (0xA4) P2_4;
__sbit __at (0xA5) P2_5;
__sbit __at (0xA6) P2_6;
__sbit __at (0xA7) P2_7;

__sbit __at (0xA8) EX0;
__sbit __at (0xA9) ET0;
__sbit __at (0xAA) EX1;
__sbit __at (0xAB) ET1;
__sbit __at (0xAC) ES;
__sbit __at (0xAD) ET2;
__sbit __at (0xAF) EA;

__sbit __at (0xB0) RXD;
__sbit __at (0xB1) TXD;
__sbit __at (0xB2) INT0;
__sbit __at (0xB3) INT1;
__sbit __at (0xB4) T0;
__sbit __at (0xB5) T1;
__sbit __at (0xB6) WR;
__sbit __at (0xB7) RD;

__sbit __at (0xB8) PX0;
__sbit __at (0xB9) PT0;
__sbit __at (0xBA) PX1;
__sbit __at (0xBB) PT1;
__sbit __at (0xBC) PS;
__sbit __at (0xBD) PT2;

__sbit __at (0xC8) CP_RL2;
__sbit __at (0xC9) C_T2;
__sbit __at (0xCA) TR2;
__sbit __at (0xCB) EXEN2;
__sbit __at (0xCC) TCLK;
__sbit __at (0xCD) RCLK;
__sbit __at (0xCE) EXF2;
__sbit __at (0xCF) TF2;

__sbit __at (0xD0) P;
__sbit __at (0xD1) F1;
__sbit __at (0xD2) OV;
__sbit __at (0xD3) RS0;
__sbit __at (0xD4) RS1;
__sbit __at (0xD5) F0;
__sbit __at (0xD6) AC;
__sbit __at (0xD7) CY;
