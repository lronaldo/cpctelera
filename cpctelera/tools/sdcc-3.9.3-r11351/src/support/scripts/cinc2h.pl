#!/usr/bin/perl -w

=back

  Copyright (C) 2012-2015, Molnar Karoly <molnarkaroly@users.sf.net>

    This file is part of SDCC. 

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

  cinc2h.pl (common-inc2h.pl)

  This program parse the gpasm header (p1xxx.inc) files and creates
  from them the SDCC header and device library files. In addition it
  needs the gpprocessor.c file also. These is included in the source
  package of gputils. Mode of download of the latest source:

        http://gputils.sourceforge.net/#Download

  The program is available on request provide assistance: cinc2h.pl -h

  -------------------------------------------------

  Steps to add a new target device to SDCC/PIC16:
  (Following Raphael Neider <rneider AT web.de>)

   1. Create the picDEVICE.c and picDEVICE.h from pDEVICE.inc using
      ./cinc2h.pl -p 18f4520 -cb -cp -gp "path/to/gputils_source" -o "path/to/output"

   2. mv picDEVICE.h $SDCC/device/non-free/include/pic16
   3. mv picDEVICE.c $SDCC/device/non-free/lib/pic16/libdev
   4. either

      (a) adjust $SDCC/device/lib/pic16/libio/*.ignore
          if the device does not support ADC, I2C, or USART
          --- OR ---
      (b) adjust
          * SDCC/scripts/pic18fam-h-gen.pl
          * SDCC/device/include/pic16/adc.h (if required)
          * SDCC/device/include/pic16/usart.h (if required)
          * SDCC/device/lib/pic16/libio/*/* (if required)
          to add the new device to the appropriate I/O style
          and implement new styles (if required).

          Having modified pic18fam-h-gen.pl, you need to run the
          script to generate pic18fam.h.gen, which in turn must
          then replace your .../include/pic16/pic18fam.h to take
          effect; see pic18fam-h-gen.pl for usage information.
   6. edit $SDCC/device/include/pic16/pic18fregs.h
   7. edit $SDCC/device/include/pic16/pic16devices.txt
   8. run cd $SDCC/device/non-free/lib/pic16 && sh update.sh
      to regenerate .../libdev/Makefile.am and processors.ac

   The file format of steps 6 and 7 is self explanatory, in most
   if not all cases you can copy and paste another device's records
   and adjust them to the newly added device.

  -------------------------------------------------

  Steps to add a new target device to SDCC/PIC14:

   1. Create the picDEVICE.c and picDEVICE.h from pDEVICE.inc using
      ./cinc2h.pl -p 16f1503 -cb -cp -gp "path/to/gputils_source" -o "path/to/output"

   2. mv picDEVICE.h $SDCC/device/non-free/include/pic14
   3. mv picDEVICE.c $SDCC/device/non-free/lib/pic14/libdev
   4. add DEVICE to $SDCC/device/non-free/lib/pic14/libdev/devices.txt
      (The names of the enhanced devices the "# enhanced cores" line
       after follow.)

   5. edit $SDCC/device/include/pic14/pic14devices.txt

   The file format of step 5 is self explanatory, in most if not all
   cases you can copy and paste another device's records and adjust
   them to the newly added device.

  $Id: cinc2h.pl 9450 2016-01-09 16:47:43Z molnarkaroly $
=cut

use strict;
use warnings;
no if $] >= 5.018, warnings => "experimental::smartmatch";        # perl 5.16
use 5.12.0;                     # when (regex)
use File::Path 'make_path';
use feature 'switch';           # Starting from 5.10.1.
use POSIX qw(strftime);

use constant FALSE => 0;
use constant TRUE  => 1;

use constant ST_NONE       => 0;
use constant ST_REG_ADDR   => 1;
use constant ST_REG1_DEF   => 2;
use constant ST_REG2_DEF   => 3;
use constant ST_RAM_DEF    => 4;
use constant ST_CONFIG_DEF => 5;
use constant ST_DEVID_DEF  => 6;
use constant ST_IDLOC_DEF  => 7;

use constant DIST_ADDRSIZE => 32;
use constant DIST_BITSIZE  => 32;
use constant DIST_DEFSIZE  => 32;
use constant DIST_COMSIZE  => 32;

my $PROGRAM  = 'cinc2h.pl';
my $time_str = '';
my $year     = '';

my $gputils_path   = "$ENV{HOME}/svn_snapshots/gputils/gputils";
my $gp_header_path = '';
my $gpprocessor_c  = 'gpprocessor.c';
my $gpproc_path;

my $name_filter = qr/10l?f\d+[a-z]*|1[26]((c(e|r)?)|hv)\d+[a-z]*|1[268]l?f\d+([a-z]*|[a-z]+\d+[a-z]*)/;
my $header_name_filter = 'p${name_filter}.inc';

my $p14_out_path = 'pic14';
my $p16_out_path = 'pic16';

my $mcu;
my $short_mcu_name;
my $is_pic16   = FALSE;
my $conf_size  = 4;
my $caddr_size = 4;
my $conf_head  = '_';
my $verbose    = 0;

my $create_bitfields  = FALSE;
my $emit_legacy_names = FALSE;
my $no_timestamp      = FALSE;

my $section = '//' . ('=' x 78);
my $btail = 'bits';
my $btype_t = "${btail}_t";

        # Here those names to be entered that are defective.
        # BAD => 'GOOD'  or
        # 'BAD' => 'GOOD'

my %correction_of_names =
  (
  OPTION => 'OPTION_REG'
  );

        # At some processors there is such register name, that is different
        # from what the other processors in used. This is a replacement table.

my %register_aliases =
  (
  BAUDCTL => 'BAUDCON'
  );

#-----------------------------------------------

=back
        The structure of one element of the @registers array:

        {
        NAME        => '',      The name of register.
        ADDRESS     => 0,       The address of register.
        NEED_PREFIX => 0,       Indicates if in front the name of bits necessary an prefix.
        BITNAMES    => [        The bits of register.
                         [],      The names of 0th bit.
                         [],
                         [],
                         [],
                         [],
                         [],
                         [],
                         []       The names of 7th bit.
                       ],

                       ...

                       []

        BITFIELDS   => {
                       'ADCS' => {           This the descriptor of the ADCS field.
                                 ADDRESSES => [],  Physical start addresses of bits of the field.
                                 INDEXES   => [],  Bit indexes of bits of the field (ADCS2 -> '2').
                                 WIDTH     => 0    So many bit the width of the bit-field.
                                 },

                       'ANS'  => {}

                       ...

                       }
        }
=cut

my @registers = ();

#-----------------------------------------------

        # References of registers according to name of registers.
my %reg_refs_by_names = ();

        # References of registers according to name of bits of registers.
        # With help of recognizable the repetitive bit names.
my %reg_refs_by_bits = ();

#-----------------------------------------------

=back
        The structure of one element of the @configs array:

        {
        NAME    => '',      The name of config.
        ADDRESS => 0,       The address of config.
        OPTIONS => [        The options of config.
                     {
                     NAME        => '',
                     VALUE       => 0,
                     EXPLANATION => ''
                     },

                     ...

                     {
                     }
                   ]
        }
=cut

my @configs = ();

#-----------------------------------------------

my %conf_names = ();

my @devids = ();
my @idlocs = ();

my $header_name;
my $device_name;
my $out_path = './';
my $out_handler;

my $device_registers = '';
my $full_bitdefs     = '';
my $legacy_names     = '';

################################################################################
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

sub str2int($)
  {
  my $Str = $_[0];

  return hex($1)   if ($Str =~ /^H'([[:xdigit:]]+)'$/io);
  return hex($1)   if ($Str =~ /^0x([[:xdigit:]]+)$/io);
  return int($Str) if ($Str =~ /^-?\d+$/o);

  die "str2int(): This string not integer: \"$Str\"";
  }

#-------------------------------------------------------------------------------

sub align($$)
  {
  my $text = $_[0];
  my $al   = $_[1] - length($text);

        # One space will surely becomes behind it.
  $al = 1 if ($al < 1);

  return ($text . ' ' x $al);
  }

#-------------------------------------------------------------------------------

sub Log
  {
  return if (pop(@_) > $verbose);
  foreach (@_) { print $_; }
  print "\n";
  }

#-------------------------------------------------------------------------------

sub Out
  {
  foreach (@_) { print $out_handler $_; }
  }

#-------------------------------------------------------------------------------

sub Outl
  {
  Out(@_);
  print $out_handler "\n";
  }

#-------------------------------------------------------------------------------

sub versionCompare($$)
  {
  my ($Str1, $Str2) = @_;

  if ((${$Str1} =~ /^\d/o) && (${$Str2} =~ /^\d/o))
    {
        # $Str1 number and $Str2 number
    return (int(${$Str1}) <=> int(${$Str2}));
    }

  return (${$Str1} cmp ${$Str2});
  }

#-------------------------------------------------------------------------------

sub versionSort($$)
  {
  my @a_s = ($_[0] =~ /(\d+|\D+)/go);
  my @b_s = ($_[1] =~ /(\d+|\D+)/go);
  my ($i, $k, $end, $ret);

  $i = scalar(@a_s);
  $k = scalar(@b_s);

  if ($i < $k)
    {
    $end = $i;
    $ret = -1;
    }
  elsif ($i == $k)
    {
    $end = $i;
    $ret = 0;
    }
  else
    {
    $end = $k;
    $ret = 1;
    }

  for ($i = 0; $i < $end; ++$i)
    {
    $k = versionCompare(\$a_s[$i], \$b_s[$i]);

    return $k if ($k != 0);
    }

  return $ret;
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@                                                      @@@@@@@@@@@@
#@@@@@@@@@@@@  Load all definitions, which will find in the header.  @@@@@@@@@@@
#@@@@@@@@@@@@@                                                      @@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        # If the $Word included in a list then it will replace.

sub correct_name($)
  {
  my $Word = $_[0];
  my $corr = $correction_of_names{$Word};

  if (defined($corr))
    {
    Log("$Word --> $corr ($mcu)", 7);
    return $corr;
    }

  return $Word;
  }

#-------------------------------------------------------------------------------

        # Adds to the list the $Name register.

sub new_register($$)
  {
  my ($Name, $Address) = @_;

  if (defined($reg_refs_by_names{$Name}))
    {
    die "The \"$Name\" register has been included on the list. ($mcu)\n";
    }

  my $reg = {
            NAME        => correct_name($Name),
            ADDRESS     => $Address,
            NEED_PREFIX => FALSE,
            BITNAMES    => undef,
            BITFIELDS   => undef
            };

  push(@registers, $reg);
  $reg_refs_by_names{$Name} = $reg;
  return $reg;
  }

#-------------------------------------------------------------------------------

        # Cuts the unnecessary prefix or suffix.

sub bit_filtration($$)
  {
  my ($Regname, $Bits) = @_;

  for (my $i = 0; $i < 8; ++$i)
    {
    my $array = $Bits->[$i];

    next if (! defined($array));

    my $changed = 0;
    my $new_bits = [];

    foreach (@{$array})
      {
        # $Regname : 'CMCON'
        # $_       : 'C1OUT_CMCON'
        # Operation: 'C1OUT_CMCON' --> 'C1OUT'
        #

      $changed += ($_ =~ s/^${Regname}_|_${Regname}$//);
      $changed += ($_ =~ s/^(\d+)$/bit_$1/o);
      push(@{$new_bits}, $_);
      }

    $Bits->[$i] = $new_bits if ($changed);
    }
  }

#-------------------------------------------------------------------------------

        # Tries the $Bit insert into a bitfield of the $Register.

sub bitfield_preparation($$$)
  {
  my ($Register, $Bit, $Address) = @_;
  my $fields = $Register->{BITFIELDS};

  $fields = $Register->{BITFIELDS} = {} if (! defined($fields));

  Log("BIT: $Bit", 7);

  if ($Bit =~ /(\d+)$/op)
    {
    my $flname = ${^PREMATCH};
    my $flidx  = $1;

    if (! defined($fields->{$flname}))
      {
        # Creates a new field.

      Log("BIT first    : $flname\[$Address\] = '$flidx'", 7);
      $fields->{$flname} = {
                           ADDRESSES => [ $Address ],
                           INDEXES   => [],
                           WIDTH     => 0
                           };
      }
    else
      {
        # The bit inserts into an existing field.

      Log("BIT remaining: $flname\[$Address\] = '$flidx'", 7);
      push(@{$fields->{$flname}->{ADDRESSES}}, $Address);
      }

    $fields->{$flname}->{INDEXES}->[$Address] = $flidx;
    }
  }

#-------------------------------------------------------------------------------

        # If can, classifies the $Bits into a field.

sub bitfield_registration($$)
  {
  my ($Register, $Bits) = @_;

  for (my $i = 0; $i < 8; ++$i)
    {
    my $array = $Bits->[$i];

    next if (! defined($array));

    Log("bitfield_registration() -- $i", 8);
    foreach (@{$array})
      {
      bitfield_preparation($Register, $_, $i);
      }
    }
  }

#-------------------------------------------------------------------------------

        # In the $Queue are in register's names.
        # Assigns to these the contents of $Bits.

sub add_reg_bits($$)
  {
  my ($Queue, $Bits) = @_;

  return if ((scalar(@{$Queue}) == 0) || (scalar(@{$Bits}) == 0));

  foreach (@{$Queue})
    {
    next if ($_ eq 'and');      # This here easiest to filter out.

    my $name = correct_name($_);
    my $reg = $reg_refs_by_names{$name};

    if (! defined($reg))
      {
      Log("The $name register is not directly be reached or does not exist. ($mcu)", 2);
      $reg = new_register($name, -1);
      }

    bit_filtration($name, $Bits);
    bitfield_registration($reg, $Bits);
    $reg->{BITNAMES} = [ @{$Bits} ];
    }

  @{$Queue} = ();
  @{$Bits}  = ();
  }

#-------------------------------------------------------------------------------

        # Finds the $Name in the $Bits.

sub find_bit($$)
  {
  my ($Bits, $Name) = @_;

  return FALSE if (! defined($Bits));

  for (my $i = 0; $i < 8; ++$i)
    {
    my $array = $Bits->[$i];

    next if (! defined($array));

    foreach (@{$array})
      {
      return TRUE if ($_ eq $Name);
      }
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

        # Adds to the list the $Name config byte/word.

sub add_conf_word($$)
  {
  my ($Name, $Address) = @_;

  my $conf = $conf_names{$Name};

  if (defined($conf))
    {
        # The $Name config byte/word still unknown, but there are related words.

        # If the $Name config byte/word are defined later than
        # the associated bits, then this section is executed.
        #

    $conf->{ADDRESS} = $Address if ($conf->{ADDRESS} < 0 && $Address >= 0);
    }
  else
    {
        # The $Name config byte/word still unknown and there are no related words.

    $conf = {
            NAME    => $Name,
            ADDRESS => $Address,
            OPTIONS => []
            };

    push(@configs, $conf);
    $conf_names{$Name} = $conf;
    }
  }

#-------------------------------------------------------------------------------

sub add_conf_options($$)
  {
  my ($Queue, $Options) = @_;

  return if ((scalar(@{$Queue}) == 0) || (scalar(@{$Options}) == 0));

  foreach (@{$Queue})
    {
    my $conf = $conf_names{$_};

    die "This config unknown: \"$_\" ($mcu)\n" if (! defined($conf));

    $conf->{OPTIONS} = [ @{$Options} ];
    }

  @{$Queue}   = ();
  @{$Options} = ();
  }

#-------------------------------------------------------------------------------

        # Reads the entire content of the $File.

sub read_content_from_header($)
  {
  my $File = $_[0];
  my ($state, $name, $addr);
  my @queue;
  my @array;

  open(IN, '<', $File) || die "Can not open the $File header file!\n";

  $state = ST_NONE;
  @queue = ();
  @array = ();

  foreach (grep(! /^\s*$/o, <IN>))
    {
    chomp;
    s/\r$//o;
    s/^\s*|\s*$//go;

    my $line = $_;

    Log("#### \"$line\"", 8);

    given ($state)
      {
      when (ST_NONE)
        {
        Log(":::: ST_NONE ($line) ($mcu)", 4);

        $state = ST_REG_ADDR if ($line =~ /^;-+\s*Register\s+Files\s*-+$/io);
        }

     when (ST_REG_ADDR)
        {
        Log(":::: ST_REG_ADDR ($line) ($mcu)", 4);

        if ($line =~ /^;-+\s*(.+)Bits\s*-+$/io)
          {
        # ;----- STKPTR Bits --------------------------------------------------------
        # ;----- UIR/UIE Bits -------------------------------------------------------
        # ;----- TXSTA, TXSTA1 and TXSTA2 Bits --------------------------------------
        #

        # Therefore need the queue because more register names can be on one line.

          @queue = ($1 =~ /([^\s,\/]+)/go);
          $state = ST_REG1_DEF;
          }
        elsif ($line =~ /^(\w+)\s+EQU\s+([\w']+)$/io)  #'
          {
        # PORTC     EQU  H'0007'
        #

          new_register($1, str2int($2));
          }
        } # when (ST_REG_ADDR)

      when (ST_REG1_DEF)
        {
        Log(":::: ST_REG1_DEF ($line) ($mcu)", 4);

        if ($line =~ /^;\s*I\/O\s+Pin\s+Name\s+Definitions?$/io)
          {
        # ;       I/O Pin Name Definitions
        #

          Log("1 +++ add_reg_bits()", 6);
          add_reg_bits(\@queue, \@array);
          $state = ST_REG2_DEF;
          }
        elsif ($line =~ /^;\s*RAM\s+Definitions?$/io)
          {
        # ;       RAM Definition
        # ;       RAM Definitions
        #

          Log("2 +++ add_reg_bits()", 6);
          add_reg_bits(\@queue, \@array);
          $state = ST_RAM_DEF;
          }
        elsif ($line =~ /^;-+\s*(.+)Bits\s*-+$/io)
          {
          my $name = $1;

          Log("3 +++ add_reg_bits()", 6);
          add_reg_bits(\@queue, \@array);

        # Therefore need the queue because more register names can be on one line.

          @queue = ($name =~ /([^\s,\/]+)/go);
          }
        elsif ($line =~ /^(\w+)\s+EQU\s+([\w']+)$/io)  #'
          {
        # VR2       EQU  H'0002'
        #

          push(@{$array[str2int($2)]}, $1);
          }
        } # when (ST_REG1_DEF)

      when (ST_REG2_DEF)
        {
        Log(":::: ST_REG2_DEF ($line) ($mcu)", 4);

        if ($line =~ /^;\s*RAM\s+Definitions?$/io)
          {
        # ;       RAM Definition
        # ;       RAM Definitions
        #

          Log("4 +++ add_reg_bits()", 6);
          add_reg_bits(\@queue, \@array);
          $state = ST_RAM_DEF;
          }
        elsif ($line =~ /^;-+\s*([^-]+)\s*-+$/io)
          {
          my $name = $1;

          Log("5 +++ add_reg_bits()", 6);
          add_reg_bits(\@queue, \@array);

        # Therefore need the queue because more register names can be on one line.

          @queue = ($name =~ /([^\s,\/]+)/go);
          }
        elsif ($line =~ /^(\w+)\s+EQU\s+([\w']+)$/io)  #'
          {
        # RE3      EQU  3
        #

          push(@{$array[int($2)]}, $1);
          }
        } # when (ST_REG2_DEF)

      when (ST_RAM_DEF)
        {
        Log(":::: ST_RAM_DEF ($line) ($mcu)", 4);

        $state = ST_CONFIG_DEF if ($line =~ /^;\s*Configuration\s+Bits$/io);
        }

      when (ST_CONFIG_DEF)
        {
        Log(":::: ST_CONFIG_DEF ($line) ($mcu)", 4);

        if ($line =~ /^_(DEVID\d*)\s+EQU\s+([\w']+)$/io)  #'
          {
          add_conf_options(\@queue, \@array);

          Log("DEVID: $line", 6);
          push(@devids, { NAME => $1, ADDRESS => str2int($2) });
          $state = ST_DEVID_DEF;
          }
        elsif ($line =~ /^_(IDLOC\d*)\s+EQU\s+([\w']+)$/io)  #'
          {
          add_conf_options(\@queue, \@array);

          Log("IDLOC: $line", 6);
          push(@idlocs, { NAME => $1, ADDRESS => str2int($2) });
          $state = ST_IDLOC_DEF;
          }
        elsif ($line =~ /^_(CONFIG\d*\w*)\s+EQU\s+([\w']+)$/io)  #'
          {
          Log("CONFIG: $line", 6);
          add_conf_word(uc($1), str2int($2));
          }
        elsif ($line =~ /^;\s*-+\s*(Config\d*\w*)\s+Options\s*-+$/io)
          {
          my $name = uc($1);

          Log("1. Config: $line", 6);
          add_conf_options(\@queue, \@array);
          add_conf_word($name, -1);
          push(@queue, $name);
          }
        elsif ($line =~ /^;\s*-+\s*Config\s+Word(\d+)\s+Options\s*-+$/io ||
               $line =~ /^;\s*Configuration\s+Byte\s+(\d+[HL])\s+Options$/io)
          {
          my $name = "CONFIG$1";

          Log("2. Config: $line", 6);
          add_conf_options(\@queue, \@array);
          add_conf_word($name, 0);
          push(@queue, $name);
          }
        elsif ($line =~ /^(\w+)\s+EQU\s+([\w']+)(.+)?$/io)  #'
          {
          my ($name, $value) = ($1, str2int($2));
          my $expl = '';

          if (defined($3))
            {
            $expl = $3;
            $expl =~ s/\s*;\s*//;
            }

          Log("Config option: $line", 6);
          push(@array, { NAME => $name, VALUE => $value, EXPLANATION => $expl });
          }
        } # when (ST_CONFIG_DEF)

      when (ST_DEVID_DEF)
        {
        Log(":::: ST_DEVID_DEF ($line) ($mcu)", 4);

        if ($line =~ /^_(IDLOC\d*)\s+EQU\s+([\w']+)$/io)  #'
          {
          Log("IDLOC: $line", 6);
          push(@idlocs, { NAME => $1, ADDRESS => str2int($2) });
          $state = ST_IDLOC_DEF;
          }
        elsif ($line =~ /^_(DEVID\d*)\s+EQU\s+([\w']+)$/io)  #'
          {
          Log("DEVID: $line", 6);
          push(@devids, { NAME => $1, ADDRESS => str2int($2) });
          }
        }

      when (ST_IDLOC_DEF)
        {
        Log(":::: ST_IDLOC_DEF ($line) ($mcu)", 4);

        if ($line =~ /^_(IDLOC\d*)\s+EQU\s+([\w']+)$/io)  #'
          {
          Log("IDLOC: $line", 6);
          push(@idlocs, { NAME => $1, ADDRESS => str2int($2) });
          }
        }
      } # given ($state)
    } # foreach (grep(! /^\s*$/o, <IN>))

  add_conf_options(\@queue, \@array);

  close(IN);

  return if (! scalar(@registers));

        # Within the array sorts by address the registers.

  @registers = sort {$a->{ADDRESS} <=> $b->{ADDRESS}} @registers;
  }

#-------------------------------------------------------------------------------

        # Reads the bounds of config area from the gpprocesor.c file.

sub extract_config_area($$)
  {
  my ($Conf_start, $Conf_end) = @_;

  open(LIB, '<', $gpproc_path) || die "extract_config_area(): Can not open. -> \"$gpproc_path\"\n";

        # static struct px pics[] = {
        #   { PROC_CLASS_PIC12E   , "__12F529T39A"  , { "pic12f529t39a"  , "p12f529t39a"    , "12f529t39a"      }, 0xE529,  3,    8, 0x00E0, { 0x07, 0x0F }, 0x06F, {     -1,     -1 }, 0x00FF, 0x0005FF, 0x000600, {       -1,       -1 }, { 0x000640, 0x000643 }, { 0x000FFF, 0x000FFF }, { 0x000600, 0x00063F }, 0x0FF0, "p12f529t39a.inc"  , "12f529t39a_g.lkr"  , 0 },
        #   { PROC_CLASS_PIC14E   , "__16LF1517"    , { "pic16lf1517"    , "p16lf1517"      , "16lf1517"        }, 0xA517,  4,   32, 0x0F80, { 0x70, 0x7F },    -1, { 0x2000, 0x21EF }, 0x0FFF, 0x001FFF, 0x002000, {       -1,       -1 }, { 0x008000, 0x008003 }, { 0x008007, 0x008008 }, {       -1,       -1 }, 0x3F80, "p16lf1517.inc"    , "16lf1517_g.lkr"    , 0 },

  my $in_table = FALSE;

  while (<LIB>)
    {
    chomp;

    if (! $in_table)
      {
      $in_table = TRUE if (/^\s*static\s+struct\s+px\s+pics\[\s*\]\s*=\s*\{\s*$/io);
      }
    elsif (/\{\s*PROC_CLASS_\w+\s*,\s*"\w+"\s*,\s*\{\s*"\w+"\s*,\s*"\w+"\s*,\s*"(\w+)"\s*}\s*,\s*[\w-]+\s*,\s*[\w-]+\s*,\s*[\w-]+\s*,\s*[\w-]+\s*,\s*\{\s*\S+\s*,\s*\S+\s*\}\s*,\s*\S+\s*,\s*\{\s*\S+\s*,\s*\S+\s*\}\s*,\s*\S+\s*,\s*\S+\s*,\s*\S+\s*,\s*\{\s*\S+\s*,\s*\S+\s*\}\s*,\s*{\s*\S+\s*,\s*\S+\s*\}\s*,\s*{\s*(\S+)\s*,\s*(\S+)\s*\}\s*,\s*{\s*\S+\s*,\s*\S+\s*\}\s*,\s*\w+\s*,\s*\"?[\.\w]+\"?\s*,\s*\"?[\.\w]+\"?\s*,\s*\d+\s*\}/iop)
      {
      my ($name, $c_start, $c_end) = ($1, $2, $3);

      if ($short_mcu_name eq $name)
        {
        ${$Conf_start} = str2int($c_start);
        ${$Conf_end}   = str2int($c_end);
        last;
        }
      }
    else
      {
      last;
      }
    }

  close(LIB);
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@                                  @@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@  Prints the register definitions.  @@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@                                  @@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        # A bit may be more than one name. This procedure counts that how
        # many the most of name.

sub max_count_of_names_of_bit($)
  {
  my $Bits = $_[0];
  my $num = 0;

  for (my $i = 0; $i < 8; ++$i)
    {
    my $array = $Bits->[$i];

    next if (! defined($array));

    my $l = scalar(@{$array});

    $num = $l if ($num < $l);
    }

  return $num;
  }

#-------------------------------------------------------------------------------

        # There are certain bits that have the same name in other registers
        # also. In this case, in the definitions of bit names need apply
        # a prefix, that allows the bits to distinguish from each other.
        # This function this need is recorded in the affected registers.

sub set_bit_prefix()
  {
  foreach my $register (sort {versionSort($a->{NAME}, $b->{NAME})} @registers)
    {
    my $bits = $register->{BITNAMES};

    next if (! defined($bits));

    for (my $i = 0; $i < 8; ++$i)
      {
      my $array = $bits->[$i];

      next if (! defined($array));

      foreach (@{$array})
        {
        my $reg = $reg_refs_by_bits{$_};

        if (defined($reg))
          {
          Log("The $_ bit of the $register->{NAME} register is occupied in $reg->{NAME} register. ($mcu)", 3);
          $register->{NEED_PREFIX} = TRUE;
          }
        else
          {
          $reg_refs_by_bits{$_} = $register;
          }
        }
      }
    }
  }

#-------------------------------------------------------------------------------

        # Prints the $Index numbered $Bits of a register.

sub print_bits($$$)
  {
  my ($Bits, $Index, $Align) = @_;
  my $al = ' ' x $Align;

  for (my $i = 0; $i < 8; ++$i)
    {
    my $array = $Bits->[$i];
    my $str;
    my $bit = (defined($array) && defined($str = $array->[$Index])) ? " $str" : '';

    Outl(align("${al}unsigned$bit", DIST_BITSIZE), ': 1;');
    }
  }

#-------------------------------------------------------------------------------

        # Prints all bits of a register.

sub print_local_bitdefs($)
  {
  my $Register = $_[0];
  my $bits = $Register->{BITNAMES};
  my $head = ($Register->{NEED_PREFIX}) ? "$Register->{NAME}_" : '';

  for (my $i = 0; $i < 8; ++$i)
    {
    my $array = $bits->[$i];

    next if (! defined($array));

    foreach (@{$array})
      {
      Outl(align("#define _${head}$_", DIST_DEFSIZE), sprintf('0x%02X', 1 << $i));
      }
    }
  }

#-------------------------------------------------------------------------------

        # Deletes the false or fragmentary bitfields.

sub bitfield_filtration($)
  {
  my ($bits, $fields) = ($_[0]->{BITNAMES}, $_[0]->{BITFIELDS});

        # Does not have a bitfields.

  return if (! defined($fields));

  foreach my $field_name (keys(%{$fields}))
    {
    my ($first_addr, $last_addr, $last_index);
    my $gr = $fields->{$field_name};

    Log("bitfield_filtration() -- $field_name", 8);

    if (find_bit($bits, $field_name))
      {
        # The $field_name already exists in the $bits array.
        # Name conflict.

      delete($fields->{$field_name});
      next;
      }

    @{$gr->{ADDRESSES}} = sort {$a <=> $b} @{$gr->{ADDRESSES}};

    my ($addresses, $indexes) = ($gr->{ADDRESSES}, $gr->{INDEXES});

    $first_addr = $addresses->[0];

    if ((scalar(@{$addresses}) < 2) || ($indexes->[$first_addr] != 0))
      {
        # This is not field, for only one member of there is. The other
        # possibility is that the index of the first member is not zero.

      delete($fields->{$field_name});
      next;
      }

    $last_addr  = -1;
    $last_index = -1;

    foreach (@{$addresses})
      {
      my $idx = $indexes->[$_];

      if ($last_addr >= 0)
        {
        if ((($last_addr + 1) != $_) || (($last_index + 1) != $idx))
          {
        # This bitfield is fragmented (not uniform).

          $last_addr = -1;
          last;
          }
        }

      $last_addr  = $_;
      $last_index = $idx;
      }

        # This is the width of the bitfield.

    my $width = $last_addr - $first_addr + 1;

    if (($width > 0) && ($width < 8))
      {
      $gr->{WIDTH} = $width;
      }
    else
      {
      delete($fields->{$field_name});
      }
    } # foreach my $field_name (keys(%{$fields}))
  }

#-------------------------------------------------------------------------------

sub print_bitfield($$$)
  {
  my ($Name, $Group, $Align) = @_;
  my ($addr, $width) = ($Group->{ADDRESSES}->[0], $Group->{WIDTH});
  my $al = ' ' x $Align;

  Outl(align("${al}unsigned", DIST_BITSIZE), ": $addr;") if ($addr > 0);
  Outl(align("${al}unsigned $Name", DIST_BITSIZE), ": $width;");
  $width = 8 - ($addr + $width);
  Outl(align("${al}unsigned", DIST_BITSIZE), ": $width;") if ($width > 0);
  }

#-------------------------------------------------------------------------------

        # Prints all bits of all registers.

sub print_all_registers()
  {
  my $fields;
  my @field_names;
  my ($bit_struct_num, $field_struct_num, $all_struct_num);
  my ($alias, $i, $r, $text, $type, $v);

  $v = @registers;
  for ($r = 0; $r < $v;)
    {
    my $reg = $registers[$r];
    ++$r;

    my ($name, $addr, $bits) = ($reg->{NAME}, $reg->{ADDRESS}, $reg->{BITNAMES});

    if ($addr >= 0)
      {
      bitfield_filtration($reg) if ($create_bitfields);

      $text = sprintf("__at(0x%04X)", $addr);
      $device_registers .= "$text __sfr $name;\n";

      $alias = $register_aliases{$name};
      $alias = undef if (defined($alias) && defined($reg_refs_by_names{$alias}));

      if (defined($bits) && (scalar(@{$bits}) > 0))
        {
        $type = "__$name$btype_t";
        Outl("\n$section\n//", (' ' x 8), "$name Bits\n\nextern $text __sfr $name;");
        Outl("\n#define $alias $name") if (defined($alias));

        $bit_struct_num = max_count_of_names_of_bit($bits);

        if ($create_bitfields)
          {
          $fields = $reg->{BITFIELDS};
          @field_names = sort {$fields->{$a}->{ADDRESSES}->[0] <=> $fields->{$b}->{ADDRESSES}->[0]} keys(%{$fields});
          $field_struct_num = @field_names;
          $all_struct_num = $bit_struct_num + $field_struct_num - 1;
          }
        else
          {
          $all_struct_num = $bit_struct_num - 1;
          }

        if ($all_struct_num > 0)
          {
          Outl("\ntypedef union\n  {");

          for ($i = 0; $i < $bit_struct_num; ++$i)
            {
            Outl("  struct\n    {");
            print_bits($bits, $i, 4);
            Outl('    };');
            Outl() if ($all_struct_num);
            --$all_struct_num;
            }

          if ($create_bitfields)
            {
            for ($i = 0; $i < $field_struct_num; ++$i)
              {
              my $flname = $field_names[$i];

              Outl("  struct\n    {");
              print_bitfield($flname, $fields->{$flname}, 4);
              Outl('    };');
              Outl() if ($all_struct_num);
              --$all_struct_num;
              }
            }
          }
        else
          {
          Outl("\ntypedef struct\n  {");
          print_bits($bits, 0, 2);
          }

        Outl("  } $type;");
        Outl("\nextern $text volatile $type $name$btail;");
        Outl("\n#define $alias$btail $name$btail") if (defined($alias));
        Outl();
        print_local_bitdefs($reg);
        Outl("\n$section\n");

        $device_registers .= "$text volatile $type $name$btail;\n";
        } # if (defined($bits) && (scalar(@{$bits}) > 0))
      else
        {
        Outl("extern $text __sfr $name;");
        Outl("#define $alias $name") if (defined($alias));
        }

      $device_registers .= "\n" if ($r < $v);
      } # if ($addr >= 0)
    elsif (defined($bits) && (scalar(@{$bits}) > 0))
      {
        # This is a register which can not be achieved directly, but the bits has name.

      Outl("\n$section\n//", (' ' x 8), "$name Bits\n");
      print_local_bitdefs($reg);
      Outl("\n$section\n");
      }
    } # for ($r = 0; $r < $v;)
  }

#-------------------------------------------------------------------------------

sub print_configuration_words()
  {
  if (! scalar(@configs))
    {
        # PIC18FxxJ

    my ($start, $end) = (-1, -1);

    extract_config_area(\$start, \$end);
    return if (($start < 0) || ($end < 0));

    Outl("\n$section\n//\n//", (' ' x 8), "Configuration Addresses\n//\n$section\n");

    my $i = 0;
    while ($start <= $end)
      {
      my $n = int($i / 2) + 1;
      my $h = ($i & 1) ? 'H' : 'L';

      Out(align("#define ${conf_head}CONFIG$n$h", DIST_BITSIZE));
      Outl(sprintf("0x%0${caddr_size}X", $start));
      ++$i;
      ++$start;
      }

    Outl("\n$section\n");
    return;
    }

  Outl("\n$section\n//\n//", (' ' x 8), "Configuration Bits\n//\n$section\n");

  foreach (@configs)
    {
    Out(align("#define $conf_head$_->{NAME}", DIST_BITSIZE));
    Outl(sprintf("0x%0${caddr_size}X", $_->{ADDRESS}));
    }

  foreach (@configs)
    {
    next if (! @{$_->{OPTIONS}});

    Outl("\n//", ('-' x 29), " $_->{NAME} Options ", ('-' x 31), "\n");

    foreach (@{$_->{OPTIONS}})
      {
      my $expl = $_->{EXPLANATION};

        # Improve a spelling error: On the end of a sentence a point must be.
      $expl .= '.' if (($expl ne '') && ($expl !~ /\.$/o));

      Out(align("#define $_->{NAME}", DIST_BITSIZE));
      Out(align(sprintf("0x%0${conf_size}X", $_->{VALUE}), 8));
      Out("// $expl") if (defined($expl) && ($expl ne ''));
      Outl();
      }
    }

  Outl("\n$section\n");
  }

#-------------------------------------------------------------------------------

sub print_devids_and_idlocs()
  {
  foreach (\@devids, \@idlocs)
    {
    if (scalar(@{$_}) > 0)
      {
      foreach (@{$_})
        {
        Out(align("#define $conf_head$_->{NAME}", DIST_BITSIZE));
        Outl(sprintf("0x%0${caddr_size}X", $_->{ADDRESS}));
        }

      Outl();
      }
    }
  }

#-------------------------------------------------------------------------------

sub print_license($)
  {
  print $out_handler <<EOT
/*
 * This $_[0] of the $mcu MCU.
 *
 * This file is part of the GNU PIC library for SDCC, originally
 * created by Molnar Karoly <molnarkaroly\@users.sf.net> $year.
 *
 * This file is generated automatically by the $PROGRAM${time_str}.
 *
 * SDCC is licensed under the GNU Public license (GPL) v2. Note that
 * this license covers the code to the compiler and other executables,
 * but explicitly does not cover any code or objects generated by sdcc.
 *
 * For pic device libraries and header files which are derived from
 * Microchip header (.inc) and linker script (.lkr) files Microchip
 * requires that "The header files should state that they are only to be
 * used with authentic Microchip devices" which makes them incompatible
 * with the GPL. Pic device libraries and header files are located at
 * non-free/lib and non-free/include directories respectively.
 * Sdcc should be run with the --use-non-free command line option in
 * order to include non-free header files and libraries.
 *
 * See http://sdcc.sourceforge.net/ for the latest information on sdcc.
 */

EOT
;
  }

#-------------------------------------------------------------------------------

        # This procedure generates the pic14-specific information.

sub make_pic14_dependent_defs()
  {
  foreach (sort {versionSort($a->{NAME}, $b->{NAME})} @registers)
    {
    my ($name, $bits) = ($_->{NAME}, $_->{BITNAMES});
    my $prefix = "$name$btail";

    if ($emit_legacy_names)
      {
      $legacy_names .= align("#define ${name}_$btail", DIST_DEFSIZE);
      $legacy_names .= "$prefix\n";
      }

    next if ($_->{NEED_PREFIX} || ! defined($bits));

    for (my $i = 0; $i < 8; ++$i)
      {
      my $array = $bits->[$i];

      next if (! defined($array));

      my $shadow = (scalar(@{$array}) > 1) ? ", shadows bit in $prefix" : '';

      foreach (@{$array})
        {
        $full_bitdefs .= align("#define $_", DIST_DEFSIZE);
        $full_bitdefs .= align("$prefix.$_", DIST_COMSIZE);
        $full_bitdefs .= "// bit $i$shadow\n";
        }
      }

    $full_bitdefs .= "\n";
    }

  $legacy_names .= "\n";
  }

#-------------------------------------------------------------------------------

        # Prints all informations to the header file.

sub print_to_header_file()
  {
  my ($text, $name, $address, $str);

  print_license('declarations');
  Outl("#ifndef __${mcu}_H__\n#define __${mcu}_H__\n\n$section");

  if (! $is_pic16)
    {
    $text = '#ifndef NO_ADDR_DEFINES';

    Outl("//\n//\tRegister Addresses\n//\n$section\n\n$text\n");

    foreach (sort { $a->{ADDRESS} <=> $b->{ADDRESS} } @registers)
      {
      ($name, $address) = ($_->{NAME}, $_->{ADDRESS});
      next if ($address < 0);

      $str = sprintf('0x%04X', $address);
      Outl(align("#define ${name}_ADDR", DIST_ADDRSIZE), $str);
      }

    Outl("\n#endif // $text");
    }

  Outl("\n$section\n//\n//\tRegister Definitions\n//\n$section\n");
  set_bit_prefix();
  print_all_registers();
  print_configuration_words();
  print_devids_and_idlocs();

  if (! $is_pic16)
    {
    make_pic14_dependent_defs();
    Outl("$section\n");

    if ($full_bitdefs ne '')
      {
      $text = '#ifndef NO_BIT_DEFINES';
      Outl("$text\n\n", $full_bitdefs, "#endif // $text\n");
      }

    if ($emit_legacy_names)
      {
      $text = '#ifndef NO_LEGACY_NAMES';
      Outl("$text\n\n", $legacy_names, "#endif // $text\n");
      }
    }

  Outl("#endif // #ifndef __${mcu}_H__");
  }

#-------------------------------------------------------------------------------

        # Prints name of all registers to the device file.

sub print_to_device_file()
  {
  print_license('definitions');
  Outl("#include <$header_name>\n\n$section\n");
  Out($device_registers) if ($device_registers ne '');
  }

#-------------------------------------------------------------------------------

sub usage()
  {
  print <<EOT
Usage: $PROGRAM [options]

    Options are:

        -gp <path> or --gputils-path <path>

            The program on this path looks for the gputils source package.

        -I <path> or --include <path>

            The program on this path looks for the headers (inc files). If this
            not specified, then the "header" directory in the local repository
            will be the default.

        -p <p12f1822> or --processor <p12f1822>

            The name of MCU. The prefix of name can be: 'p', 'pic' or nothing

        -o <path> or --out-path <path>

            Here to writes the output files. (default: "./")
            Attention! The program overwrites the existing files without asking.

        -v <level> or --verbose <level>

            It provides information on from the own operation.
            Possible value of the level between 0 and 10. (default: 0)

        -cb or --create-bitfields

            Create bit fields. In some register, can be found such bits which
            belong together. For example: CVR0, CVR1, CVR2, CVR3
            These may be useful, to merge during a common field name: CVR
            The compiler helps handle these bit fields. (default: no)

        -e or --emit-legacy-names

            Creates the legacy names also. (default: no)

        -nt or --no-timestamp

            There will be no timestamp in the header and device files. (default: yes)

        -h or --help

            This text.

    For example: $PROGRAM -p 12f1840 -cb
EOT
;
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@  The main program.  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

$PROGRAM = basename($0);
$gp_header_path = '';
$mcu = '';

for (my $i = 0; $i < scalar(@ARGV); )
  {
  my $opt = $ARGV[$i++];

  given ($opt)
    {
    when (/^-(gp|-gputils-path)$/o)
      {
      param_exist($opt, $i);
      $gputils_path = $ARGV[$i++];
      }

    when (/^-(I|-include)$/o)
      {
      param_exist($opt, $i);
      $gp_header_path = $ARGV[$i++];
      }

    when (/^-(p|-processor)$/o)
      {
      param_exist($opt, $i);
      $mcu = $ARGV[$i++];
      }

    when (/^-(o|-out-path)$/o)
      {
      param_exist($opt, $i);
      $out_path = $ARGV[$i++];
      }

    when (/^-(v|-verbose)$/o)
      {
      param_exist($opt, $i);
      $verbose = int($ARGV[$i++]);
      $verbose = 0 if (! defined($verbose) || ($verbose < 0));
      $verbose = 10 if ($verbose > 10);
      }

    when (/^-(cb|-create-bitfields)$/o)
      {
      $create_bitfields = TRUE;
      }

    when (/^-(e|-emit-legacy-names)$/o)
      {
      $emit_legacy_names = TRUE;
      }

    when (/^-(nt|-no-timestamp)$/o)
      {
      $no_timestamp = TRUE;
      }

    when (/^-(h|-help)$/o)
      {
      usage();
      exit(0);
      } # when ('-h' || '--help')
    } # given ($opt)
  }

die "Miss the name of MCU!\n" if ($mcu eq '');
die "This name is wrong: \"$mcu\"\n" if ($mcu !~ /^(p(ic)?)?$name_filter$/io);

die "This directory - $gputils_path - not exist!" if (! -d $gputils_path);

$gp_header_path = "$gputils_path/header" if ($gp_header_path eq '');    # The default value.
$gpproc_path    = "$gputils_path/libgputils/$gpprocessor_c";

$mcu = lc($mcu);
$mcu =~ s/^p(ic)?//o;

if ($mcu =~ /^18/)
  {
  $is_pic16   = TRUE;
  $conf_size  = 2;
  $caddr_size = 6;
  $conf_head  = '__';
  }

$short_mcu_name = $mcu;
my $fname = "p${mcu}.inc";

die "The MCU: $mcu unknown!\n" if (! -f "$gp_header_path/$fname");

$mcu = 'PIC' . uc($mcu);
$header_name = lc($mcu) . '.h';
$device_name = lc($mcu) . '.c';

read_content_from_header("$gp_header_path/$fname");

$year     = strftime('%Y', gmtime);
$time_str = strftime(', %F %T UTC', gmtime) if (! $no_timestamp);

        # Creates the directory structure.

my $path = ($is_pic16) ? "$out_path/$p16_out_path" : "$out_path/$p14_out_path";
my $head_dir = "$path/header";

if (! -e $head_dir)
  {
  Log("Creates the \"$head_dir\" dir.", 4);
  make_path($head_dir) || die "Could not create the \"$head_dir\" dir!";
  }

my $dev_dir = "$path/device";

if (! -e $dev_dir)
  {
  Log("Creates the \"$dev_dir\" dir.", 4);
  make_path($dev_dir) || die "Could not create the \"$dev_dir\" dir!";
  }

        # Creates the pic1xxxx.h file.

my $fpath = "$head_dir/$header_name";
open($out_handler, '>', $fpath) || die "Could not create the \"$fpath\" file!\n";
Log("Creates the $header_name", 1);
print_to_header_file();
close($out_handler);

        # Creates the pic1xxxx.c file.

$fpath = "$dev_dir/$device_name";
open($out_handler, '>', $fpath) || die "Could not create the \"$fpath\" file!\n";
Log("Creates the $device_name", 1);
print_to_device_file();
close($out_handler);
