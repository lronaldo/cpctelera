#!/usr/bin/perl -w

=back

  Copyright (C) 2012-2014, Molnar Karoly <molnarkaroly@users.sf.net>

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

    Proposal for use: ./pic16-header-parser.pl -gp

    This program creates seven files in the current directory:

        adc.h.gen
        ccp.h.gen
        pwm.h.gen
        i2c.h.gen
        spi.h.gen
        usart.h.gen
        peripheral.groups

    In these the MCUs can be seen in groups, according to a periphery.
    These informations helps to realize the handling of periphery.
    Of course necessary to study the data sheets as well.

    $Id: pic16-header-parser.pl 9071 2014-09-17 08:32:55Z molnarkaroly $
=cut

use strict;
use warnings;
no if $] >= 5.018, warnings => "experimental::smartmatch";        # perl 5.16
use 5.10.1;
use feature 'switch';           # Starting from 5.10.1.
use POSIX 'ULONG_MAX';

use constant FALSE => 0;
use constant TRUE  => 1;

use constant P_NO_SHOW_BITS     => 0;
use constant P_NO_SHOW_IF_EMPTY => 1;
use constant P_ALWAYS_SHOW      => 2;
use constant P_SHOW_ONLY_NAME   => 3;

my @default_paths =
  (
  '/usr/share/sdcc/non-free/include',
  '/usr/share/sdcc/include',
  '/usr/local/share/sdcc/non-free/include',
  '/usr/local/share/sdcc/include'
  );

my $default_port = 'pic16';
my $header_name_filter = 'pic18f\d+[a-z]*\d+\.h';

my $include_path;
my $out_tail = '.gen';
my $peri_group = 'peripheral.groups';
my $table_border = (' ' x 19) . '+' . ('---------+' x 8);

my %reg_addresses = ();

my @some_ADC_registers =
  (
  'ADCON\d+[HL]?',
  'ADCTMUEN\d+[HL]',
  'ADCHIT\d+[HL]',
  'ADC[HS]S\d+[HL]',
  'ADRES[HL]?|ADCBUF\d+[HL]?',
  'ANCON\d+',
  'ANSEL[\dHL]?',
  'ANSEL[A-O]'
  );

#-----------------------------------------------

=back
        The structure of one element of the @periphery_table array.

        {
        NAME => '',
        REGS => [
                  {
                  VALID_REGS => '',
                  VALID_BITS => '',
                  PRINT_MODE => P_ALWAYS_SHOW
                  },

                       ...

                  {
                  }
                ]
        }

=cut

my @periphery_table = ();

#-----------------------------------------------

=back
        The structure of one element of the @io_table_by_mcu hash:

        {
        'ADC'   => {
                   'AN0' => [],

                   ...

                   'AN4' => []
                   },

        ...

        'USART' => {}
        }
=cut

my %io_table_by_mcu = ();

#-----------------------------------------------

=back
        The structure of one element of the @io_dir_table_by_mcu hash:

        {
        'ADC'   => {},

        ...

        'USART' => {
                   'RX' => 1,
                   'TX' => 0
                   }
        }
=cut

my %io_dir_table_by_mcu = ();

#-----------------------------------------------

=back
        The structure of one element of the @mcu_raw array:

        {       Descriptor of MCU.
        NAME      => '',     The name of MCU.
        REG_REFS  => {       Accelerate searching of the registers.
                     'PIR1'  => register_reference, ---+
                     'TRISD' => register_reference, ---|---+
                     ...                               |   |
                     ...                               |   .
                     },                                |   .
        REG_ARRAY => [      The array of registers.    |   .
                       {                A register. <--+
                       NAME     => 'PIR1',  The name of register.
                       ADDRESS  => 0,       The address of register.
                       BITNAMES => [        The bits of register.
                                     [],      The names of bit.
                                     [],
                                     [],
                                     [],
                                     [],
                                     [],
                                     [],
                                     []
                                   ]
                       },

                       ...

                       {
                       }
                     ]
        }
=cut

my @mcu_raw = ();

#-----------------------------------------------

=back
        The structure of one element of the @mcu_filtered and @mcu_groups arrays:

        {       Descriptor of MCU.
        NAME       => '',     The name of MCU.
        IN_GROUP   => 0,      This member of a MCU group.
        REG_REFS   => {       Accelerate searching of the registers.
                      'PIR1'  => register_reference, ----------------------+
                      'TRISD' => register_reference, ----------------------|---+
                      ...                                                  |   |
                      ...                                                  |   |
                      },                                                   |   .
        REG_GROUPS => [      The group of all necessary register.          |   .
                        {                The first register group.         |   .
                        VALID_REGS => '',    The valid names of registers. |
                        VALID_BITS => '',    The valid names of bits.      |
                        PRINT_MODE => 0,     The mode of print.            |
                        REG_ARRAY  => [      The array of registers.       |
                                        {                A register. <-----+
                                        NAME     => 'PIR1',  The name of register.
                                        ADDRESS  => 0,       The address of register.
                                        GROUP    => undef,   Back reference of REG_GROUPS.
                                        TOUCHED  => 0,       Touched register during the search.
                                        EMPTY    => 0,       True if the register became empty after the filtering.
                                        BITNAMES => [        The bits of register.
                                                      [],      The names of bit.
                                                      [],
                                                      [],
                                                      [],
                                                      [],
                                                      [],
                                                      [],
                                                      []
                                                    ]
                                        },

                                        ...

                                        {
                                        }
                                      ]
                        },

                        ...

                        {                The last register group.
                        }
                      ]
        }
=cut

my @mcu_filtered = ();

my @mcu_groups = ();

my %definitions = ();
my %io_groups = ();

#-----------------------------------------------

my $verbose = 0;
my $make_groups;
my $only_prime;
my $out_handler;
my $initial_border;

my $peri_groups = '';

################################################################################
################################################################################
################################################################################
################################################################################

sub basename($)
  {
  return ($_[0] =~ /([^\/]+)$/) ? $1 : '';
  }

#-------------------------------------------------------------------------------

sub Log
  {
  return if (pop(@_) > $verbose);
  foreach (@_) { print STDERR $_; }
  print STDERR "\n";
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

sub Outf
  {
  printf $out_handler (shift(@_), @_);
  }

#-------------------------------------------------------------------------------

sub Outfl
  {
  Outf(@_);
  print $out_handler "\n";
  }

#-------------------------------------------------------------------------------

sub smartCompare($$)
  {
  my ($Str1, $Str2) = @_;

  if (${$Str1} =~ /^\d/o && ${$Str2} =~ /^\d/o)
    {
        # $Str1 number and $Str2 number
    return (int(${$Str1}) <=> int(${$Str2}));
    }

  return (${$Str1} cmp ${$Str2});
  }

#-------------------------------------------------------------------------------

sub smartSort($$)
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
    $k = smartCompare(\$a_s[$i], \$b_s[$i]);

    return $k if ($k != 0);
    }

  return $ret;
  }

#-------------------------------------------------------------------------------

sub exist_in_list($$)
  {
  my ($List, $Member) = @_;

  foreach (@{$List})
    {
    return TRUE if ($Member =~ /^$_$/);
    }

  return FALSE;
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@                                 @@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@  Populates the peripheral table.  @@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@                                 @@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub add_periphery($)
  {
  my $peri = {
             NAME => lc($_[0]),
             REGS => []
             };

  push(@periphery_table, $peri);
  return $peri->{REGS};
  }

#-------------------------------------------------------------------------------

sub add_reg_def($)
  {
  my $def = {
            VALID_REGS => '',
            VALID_BITS => '',
            PRINT_MODE => P_ALWAYS_SHOW
            };

  push(@{$_[0]}, $def);
  }

#-------------------------------------------------------------------------------

sub resolve_define($)
  {
  my $Name = $_[0];
  my $ig = \@{$io_groups{$Name}};
  my @array = ();
  my $r;

  if (defined($ig) && scalar(@{$ig}) > 0)
    {
    foreach (@{$ig})
      {
      $r = $definitions{$_};
      push(@array, ((defined($r)) ? $r : $_));
      }
    }
  else
    {
    $r = $definitions{$Name};
    push(@array, ((defined($r)) ? $r : $Name));
    }

  return \@array;
  }

#-------------------------------------------------------------------------------

sub add_io_pins($$$)
  {
  my ($Mcu_group, $Peri, $Pins) = @_;
  my %pin_groups = ();
  my ($mcu, $io);
  my @ports;

  foreach (@{$Pins})
    {
    if ($_ !~ /^(\w+):(\S+)$/o)
      {
      print STDERR "This piece is wrong: \"$_\" (" . join(',', @{$Mcu_group}) . ")\n";
      exit(1);
      }

    $io = $1;
    @ports = ();

    foreach (split('/', $2))
      {
      if ($_ =~ /^(\w+)\[(\d+)-(\d+)\]$/o)
        {
        # This is a section. E.g.: "RP[0-18]"

        my ($name, $first, $second) = ($1, int($2), int($3));

        if ($first > $second)
          {
          print STDERR "\"$_\" The first number ($first) greather than the second number ($second)!\n";
          exit(1);
          }

        while ($first <= $second)
          {
          push(@ports, @{resolve_define("$name$first")});
          ++$first;
          }
        }
      else
        {
        # This is a name. E.g.: "RD7" or "RP22"

        push(@ports, @{resolve_define($_)});
        }
      } # foreach (split('/', $2))

    $pin_groups{$io} = [ sort {$a cmp $b} @ports ];
    } # foreach (@{$Pins})

  foreach $mcu (@{$Mcu_group})
    {
    foreach $io (keys(%pin_groups))
      {
      $io_table_by_mcu{$mcu}->{$Peri}{$io} = [ @{$pin_groups{$io}} ];
      }
    }
  }

#-------------------------------------------------------------------------------

sub add_io_dir($$$)
  {
  my ($Mcu_group, $Peri, $Pins) = @_;
  my %pin_groups = ();
  my ($mcu, $io, $dir);
  my @ports;

  foreach (@{$Pins})
    {
    if ($_ !~ /^(\w+):([01])$/o)
      {
      print STDERR "This piece is wrong: \"$_\" (" . join(',', @{$Mcu_group}) . ")\n";
      exit(1);
      }

    $dir = int($2);
    @ports = @{resolve_define($1)};

    if (@ports > 1)
      {
      print STDERR "Only one I/O line can be specified: \"$_\" (" . join(',', @{$Mcu_group}) . ")\n";
      exit(1);
      }

    $pin_groups{$ports[0]} = $dir;
    } # foreach (@{$Pins})

  foreach $mcu (@{$Mcu_group})
    {
    foreach $io (keys(%pin_groups))
      {
      $io_dir_table_by_mcu{$mcu}->{$Peri}{$io} = $pin_groups{$io};
      }
    }
  }

#-------------------------------------------------------------------------------

sub load_periphery_data()
  {
  my $periphery = undef;
  my @mcu_group = ();
  my @blocks = ();
  my ($line, $block, $key, $val);

  foreach (grep(! /^\s*$|^\s*#/o, <DATA>))
    {
    chomp;
    s/#.*$//o;                  # Remove ending comment.
    s/^\s*|\s*$//go;            # Remove starting and ending whitespaces.
    $line = $_;

    if ($line =~ /^BEGIN=(\S+)$/o)
      {
      $block = $1;

      given ($block)
        {
        when (['IO_TABLE', 'DEFINE', 'GROUP'])
          {
          }

        when (/^(PERIPHERY):(\w+)$/o)
          {
          $block = $1;
          $periphery = add_periphery($2);
          }

        when ('REGISTER')
          {
          if (! defined($periphery))
            {
            print STDERR "There is no periphery to which can be assigned the following register.\n";
            exit(1);
            }

          add_reg_def($periphery);
          }

        when (/^(MCU):(\S+)$/o)
          {
          $block = $1;
          @mcu_group = split(',', $2);
          }

        default
          {
          print STDERR "Unknown block: \"$block\"\n";
          exit(1);
          }
        } # given ($block)

      push(@blocks, $block);
      next;
      } # if ($line =~ /^BEGIN=(\S+)$/o)
    elsif ($line =~ /^END=(\S+)$/o)
      {
      $block = $1;

      if (scalar(@blocks) == 0 || $blocks[$#blocks] ne $block)
        {
        print STDERR "The \"$block\" block has no beginning!\n";
        exit(1);
        }

      given ($block)
        {
        when ('PERIPHERY') { $periphery = undef; }
        when ('MCU')       { @mcu_group = (); }
        }

      $block = pop(@blocks);
      next;
      } # elsif ($line =~ /^END=(\w+)$/o)

        #...................................

    $block = (scalar(@blocks) > 0) ? $blocks[$#blocks] : '';

    given ($block)
      {
      when ('REGISTER')
        {
        if ($line =~ /^([^=]+)=(.*)$/o)
          {
        # This a key -- value pair.

          if (scalar(@{$periphery}) == 0)
            {
            print STDERR "No entry of the register table!\n";
            exit(1);
            }

          ($key, $val) = ($1, $2);
        # Reference of the last member.
          my $peri_r = \%{$periphery->[$#{$periphery}]};

          if ($val =~ /^['"]([^'"]*)['"]$/o)
            {
        # This a text.

            $peri_r->{$key} = $1;
            }
          else
            {
        # This a constant.

            $peri_r->{$key} = eval($val);
            }
          }
        } # when ('REGISTER')

      when ('DEFINE')
        {
        if ($line !~ /^([^=]+)=(.*)$/o)
          {
          print STDERR "This is not key -- value pair: $line\n";
          exit(1);
          }

        ($key, $val) = ($1, $2);

        if (defined($val) && $val ne '')
          {
          $definitions{$key} = $val;
          }
        else
          {
        # Undefine the $key.

          delete($definitions{$key});
          }
        } # when ('DEFINE')

      when ('GROUP')
        {
        if ($line !~ /^([^=]+)=(.*)$/o)
          {
          print STDERR "This is not group -- members definition: $line\n";
          exit(1);
          }

        @{$io_groups{$1}} = split(',', $2);
        } # when ('GROUP')

      when ('MCU')
        {
        if ($line =~ /^([^=]+)=(.*)$/o)
          {
          my ($peri, $val) = (lc($1), $2);

          if ($val =~ /^([^=]+)=(.*)$/o)
            {
        # It is possible that this is a property.

            my $prop = $1;
            $val = $2;

            given ($prop)
              {
              when ('IO_DIR')
                {
                my @pins = split(',', $val);

                add_io_dir(\@mcu_group, $peri, \@pins);
                }

              default
                {
                print STDERR "This is unknown property definition: $line\n";
                exit(1);
                }
              }
            }
          else
            {
            my @pins = split(',', $val);

            add_io_pins(\@mcu_group, $peri, \@pins);
            }
          } # if ($line =~ /^([^=]+)=(.*)$/o)
        } # when ('MCU')
      } # given ($block)
    } # foreach (grep(! /^\s*$|^\s*#/o, <DATA>))

  if (scalar(@blocks) > 0)
    {
    print STDERR "The \"$blocks[$#blocks]\" block has no ending!\n";
    exit(1);
    }
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@                                                  @@@@@@@@@@@@@@@
#@@@@@@@@@@@@@  Load all registers, which will find in a header.  @@@@@@@@@@@@@@
#@@@@@@@@@@@@@@                                                  @@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub add_mcu_raw($)
  {
  my $Name = uc($_[0]);

  Log("add_mcu_raw(): $Name", 9);

  my $mcu_ref = {
                NAME      => $Name,
                REG_REFS  => {},
                REG_ARRAY => []
                };

  push(@mcu_raw, $mcu_ref);
  return $mcu_ref;
  }

#-------------------------------------------------------------------------------

sub add_register_raw($$$)
  {
  my ($Mcu_raw, $Name, $Address) = @_;

  Log(sprintf("add_register_raw(): $Name, 0x%04X", $Address), 9);

  my $reg = {
            NAME     => $Name,
            ADDRESS  => $Address,
            BITNAMES => []
            };

  push(@{$Mcu_raw->{REG_ARRAY}}, $reg);
  $Mcu_raw->{REG_REFS}->{$Name} = $reg;
  }

#-------------------------------------------------------------------------------

sub read_regs_from_header($$)
  {
  my ($Mcu_ref, $Fname) = @_;
  my $path = "$include_path/$Fname";
  my ($fh, $name, $addr, $bit_addr, $bitnames, $width);

  if (! open($fh, '<', $path))
    {
    print STDERR "\a\t$0 : Can not open the $path header file!\n";
    exit(1);
    }

  Log("read_regs_from_header(): $path", 6);
  $bitnames = [];

  foreach (grep(! /^\s*$/o, <$fh>))
    {
    chomp;
    s/\r$//o;
    s/^\s*|\s*$//go;

    my $line = $_;

    Log(">>>>: $line", 7);

    if ($line =~ /^#include\s+"(\S+)"$/o)
      {
      &read_regs_from_header($Mcu_ref, $1);
      }
    elsif ($line =~ /^#\s*define\s+(\w+_ADDR)\s+0[xX]([[:xdigit:]]+)$/o)
      {
      Log("reg_addresses\{$1\} = hex($2)", 8);
      $reg_addresses{$1} = hex($2);
      }
    elsif ($line =~ /^extern\b/o &&
           $line =~ /\b__sfr\b/o &&
           (($addr) = ($line =~ /\b__at\s*\(\s*0[xX]([[:xdigit:]]+)\s*\)/o)) &&
           (($name) = ($line =~ /\b(\w+)\s*;$/o)))
      {
        # extern __at(0x0000) __sfr INDF;
        # extern __sfr __at(0x0003) STATUS;
        #

      add_register_raw($Mcu_ref, $name, hex($addr));
      }
    elsif ($line =~ /^extern\s+__sfr\s+__at\s*\((\w+_ADDR)\)\s+(\w+);$/o)
      {
        # extern __sfr  __at (EEDATA_ADDR)  EEDATA;
        #

      if (! defined($reg_addresses{$1}))
        {
        print STDERR "This register: $2 not have address!\n";
        exit(1);
        }

      add_register_raw($Mcu_ref, $2, $reg_addresses{$1});
      }
    elsif ($line =~ /\bstruct\b/o)
      {
      Log("\tbit_addr = 0", 8);
      $bit_addr = 0;
      }
    elsif ($line =~ /^unsigned(?:\s+char|\s+int)?\s*:\s*(\d+)\s*;$/o)
      {
        # unsigned char :1;
        # unsigned int : 4;
        #

      $width = int($1);
      $bit_addr += $width;
      Log("\tbit_addr += $width ($bit_addr)", 8);
      }
    elsif ($line =~ /^unsigned(?:\s+char|\s+int)?\s*(\w+)\s*:\s*(\d+)\s*;$/o)
      {
        # unsigned char PCFG2:1;
        # unsigned int PSA:1;
        # unsigned TRISG  :5;
        #

      ($name, $width) = ($1, int($2));

      if ($width == 1)
        {
        Log("\tpush(bitnames->\[$bit_addr\], $name)", 8);
        push(@{$bitnames->[$bit_addr]}, $name);
        }
      else
        {
        Log("\t$name", 8);
        }

      $bit_addr += $width;
      Log("\tbit_addr += $width ($bit_addr)", 8);
      }
    elsif ($line =~ /^\}\s*(?:__)?(\w+)bits_t\s*;$/o || $line =~ /^\}\s*(?:__)?(\w+)_t\s*;$/o)
      {
      my $reg_ref = $Mcu_ref->{REG_REFS}->{$1};

      if (! defined($reg_ref))
        {
        print STDERR "This register: $1 not have data structure!\n";
        exit(1);
        }

      Log("\treg_ref : $reg_ref)", 8);
      $reg_ref->{BITNAMES} = $bitnames;
      $bitnames = [];
      }
    } # foreach (grep(! /^\s*$/o, <$fh>))

  close($fh);

  my $array = \@{$Mcu_ref->{REG_ARRAY}};

  return if (scalar(@{$array}) == 0);

        # Within the array sorts by name the registers.

  @{$array} = sort {smartSort($a->{NAME}, $b->{NAME})} @{$array};
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@                                                          @@@@@@@@@@@
#@@@@@@@@@  To periphery fitting in a manner, filters the registers.  @@@@@@@@@@
#@@@@@@@@@@                                                          @@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub add_mcu($$)
  {
  my ($Peri_regs, $Mcu_raw) = @_;

  my $mcu = {
            NAME       => $Mcu_raw->{NAME},
            REG_REFS   => {},
            IN_GROUP   => FALSE,
            REG_GROUPS => []
            };

  Log("add_mcu($mcu->{NAME})", 8);

        # Copies the master periphery table.

  foreach (@{$Peri_regs})
    {
    my $group = {
                VALID_REGS => $_->{VALID_REGS},
                VALID_BITS => $_->{VALID_BITS},
                PRINT_MODE => $_->{PRINT_MODE},
                REG_ARRAY  => []
                };

    push(@{$mcu->{REG_GROUPS}}, $group);
    }

  push(@mcu_filtered, $mcu);
  return $mcu;
  }

#-------------------------------------------------------------------------------

sub add_register($$)
  {
  my ($Mcu, $Reg_raw) = @_;
  my $name = $Reg_raw->{NAME};

  Log("add_register($Mcu->{NAME}, $name)", 8);

  foreach (@{$Mcu->{REG_GROUPS}})
    {
    if ($name =~ /^$_->{VALID_REGS}$/)
      {
        # This register fits into this group.

      if ($name =~ /^([\D_]+)1$/ && defined($Mcu->{REG_REFS}->{$1}))
        {
        # This register already exists. E.g.: RCREG1 --> RCREG
        return undef;
        }

      my $reg = {
                NAME     => $name,
                ADDRESS  => $Reg_raw->{ADDRESS},
                GROUP    => $_,
                TOUCHED  => FALSE,
                EMPTY    => FALSE,
                BITNAMES => []
                };

      push(@{$_->{REG_ARRAY}}, $reg);
      $Mcu->{REG_REFS}->{$name} = $reg;
      return $reg;
      }
    }

  return undef;
  }

#-------------------------------------------------------------------------------

        # Cut down frippery from the bit names.

sub cut_frippery_from_bitnames($$)
  {
  my ($Regname, $Bits) = @_;

  return if (! defined($Bits) || scalar(@{$Bits}) <= 0);

  my $changed = 0;
  my $new_bits = [];

  foreach (@{$Bits})
    {
    $changed += ($_ =~ s/^${Regname}_|_${Regname}$//);
    $changed += ($_ =~ s/^(\d+)$/bit_$1/o);
    push(@{$new_bits}, $_);
    }

  $Bits = $new_bits if ($changed);
  }

#-------------------------------------------------------------------------------

sub tris_ansel_filter($$)
  {
  my ($Peri_pins, $Pin_name) = @_;

  return TRUE if (! defined($Peri_pins) ||
                  ($Pin_name !~ /^TRIS[A-O]\d+$/io &&
                   $Pin_name !~ /^ANS[A-O]\d+$/io));

  foreach (keys(%{$Peri_pins}))
    {
    foreach (@{$Peri_pins->{$_}})
      {
      if ($_ =~ /^R([A-O]\d+)$/io)
        {
        # E.g.: "RC7" --> "TRISC7"; "RC7" --> "ANSC7"
        my $tail  = uc($1);
        my $trisx = "TRIS$tail";
        my $ansx  = "ANS$tail";

        return TRUE if ($Pin_name eq $trisx || $Pin_name eq $ansx);
        }
      }
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

sub filter_regs_from_raw($)
  {
  my $Periphery = $_[0];
  my ($peri_name, $peri_regs) = ($Periphery->{NAME}, $Periphery->{REGS});

  foreach (@mcu_raw)
    {
    my $mcu       = lc($_->{NAME});
    my $io_ref    = \%{$io_table_by_mcu{$mcu}};
    my $peri_pins = (defined($io_ref)) ? $io_ref->{$peri_name} : undef;

        # The MCU not have this periphery.
    next if (! defined($peri_pins));

    my $mcu_ref   = add_mcu($peri_regs, $_);

    foreach my $reg_raw (@{$_->{REG_ARRAY}})
      {
      my $reg_dst = add_register($mcu_ref, $reg_raw);

      next if (! defined($reg_dst));

      my $bits_src   = $reg_raw->{BITNAMES};
      my $bits_dst   = $reg_dst->{BITNAMES};
      my $valid_bits = $reg_dst->{GROUP}->{VALID_BITS};
      my $empty      = TRUE;

      if (defined($valid_bits) && $valid_bits ne '')
        {
        # Filtering follows.

        for (my $i = 0; $i < 8; ++$i)
          {
          my $new_bits = [];

          cut_frippery_from_bitnames($reg_dst->{NAME}, \@{$bits_src->[$i]});
          foreach (@{$bits_src->[$i]})
            {
        # Only those names notes, which passed through the filter.

            push(@{$new_bits}, $_) if (defined($_) && $_ =~ /^$valid_bits$/ &&
                                       tris_ansel_filter($peri_pins, $_));
            }

          if (scalar(@{$new_bits}) > 0)
            {
            $bits_dst->[$i] = $new_bits;
            $empty = FALSE;
            }
          else
            {
            $bits_dst->[$i] = undef;
            }
          }
        }
      else
        {
        # No filtering.

        for (my $i = 0; $i < 8; ++$i)
          {
          cut_frippery_from_bitnames($reg_dst->{NAME}, \@{$bits_src->[$i]});
          $empty = FALSE if (defined($bits_src->[$i]));

          $bits_dst->[$i] = [ @{$bits_src->[$i]} ];
          }
        }

      $reg_dst->{EMPTY} = $empty;
      }
    } # foreach (@mcu_raw)
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@                                                               @@@@@@@@
#@@@@@@@  Collects same group them MCU which, have the same peripheral.  @@@@@@@
#@@@@@@@@                                                               @@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub load_touched_flags($$)
  {
  my $Level = $_[1];

  foreach (@{$_[0]->{REG_GROUPS}})
    {
    foreach (@{$_->{REG_ARRAY}})
      {
      $_->{TOUCHED} = $Level;
      }
    }
  }

#-------------------------------------------------------------------------------

sub find_not_touched_reg($)
  {
  foreach (@{$_[0]->{REG_GROUPS}})
    {
    foreach (@{$_->{REG_ARRAY}})
      {
      next if ($_->{EMPTY});            # It does not take into account the empty register.

      return TRUE if (! $_->{TOUCHED}); # This register left out from a previous comparison.
      }
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

sub find_register($$)
  {
  my $Prime_reg = $_[1];
  my $cand_reg = $_[0]->{REG_REFS}->{$Prime_reg->{NAME}};

  return $cand_reg if (defined($cand_reg) && $cand_reg->{ADDRESS} == $Prime_reg->{ADDRESS});

  return undef;
  }

#-------------------------------------------------------------------------------

sub find_equivalent_bit($$)
  {
  my ($Bits1, $Bits2) = @_;
  my $d = (defined($Bits1) && scalar(@{$Bits1}) > 0) + (defined($Bits2) && scalar(@{$Bits2}) > 0);

  return TRUE if ($d == 0);

  if ($d != 2)
    {
    Log("find_equivalent_bit(): Only one bits defined.", 6);
    return FALSE;
    }

  foreach (@{$Bits1})
    {
    return TRUE if (/^$_$/ ~~ @{$Bits2});

    Log("find_equivalent_bit(): The $_ bit not defined.", 7);
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

sub find_equivalent_register($$$)
  {
  my ($Candidate, $Prime_reg, $Print_mode) = @_;
  my ($cand_reg, $prime_bits, $cand_bits);

  $cand_reg = find_register($Candidate, $Prime_reg);

  if (! defined($cand_reg))
    {
    Log("find_equivalent_register(): Not exists candidate reg: $Prime_reg->{NAME} in $Candidate->{NAME} MCU", 5);
    return FALSE;
    }

  $cand_reg->{TOUCHED} = TRUE;

        # Not performs comparison, if the bits not must be displayed.
  return TRUE if ($Print_mode == P_SHOW_ONLY_NAME);

  $prime_bits = \@{$Prime_reg->{BITNAMES}};
  $cand_bits  = \@{$cand_reg->{BITNAMES}};

  for (my $i = 0; $i < 8; ++$i)
    {
    if (! find_equivalent_bit(\@{$cand_bits->[$i]}, \@{$prime_bits->[$i]}))
      {
      Log("find_equivalent_register(): Not finds equivalent bit: $cand_reg->{NAME} != $Prime_reg->{NAME}", 5);
      return FALSE;
      }
    }

  return TRUE;
  }

#-------------------------------------------------------------------------------

sub find_equivalent_mcu($$)
  {
  my ($Prime, $Candidate) = @_;

  load_touched_flags($Prime, FALSE);
  load_touched_flags($Candidate, FALSE);

  foreach (@{$Prime->{REG_GROUPS}})
    {
    my $pmode = $_->{PRINT_MODE};

    foreach (@{$_->{REG_ARRAY}})
      {
      $_->{TOUCHED} = TRUE;
      next if ($_->{EMPTY});

      return FALSE if (! find_equivalent_register($Candidate, $_, $pmode));
      }
    }

  if (find_not_touched_reg($Prime))
    {
    Log("find_equivalent_mcu(): Finds not touched register: $Prime->{NAME}", 5);
    return FALSE;
    }

  if (find_not_touched_reg($Candidate))
    {
    Log("find_equivalent_mcu(): Finds not touched register: $Candidate->{NAME}", 5);
    return FALSE;
    }

  return TRUE;
  }

#-------------------------------------------------------------------------------

sub cmp_io_dirs($$$)
  {
  my ($Prime, $Candidate, $Periphery) = @_;
  my $prime_io = $io_dir_table_by_mcu{lc($Prime->{NAME})};
  my $cand_io  = $io_dir_table_by_mcu{lc($Candidate->{NAME})};
  my $d = (defined($prime_io) && scalar(keys %{$prime_io}) > 0) + (defined($cand_io) && scalar(keys %{$cand_io}) > 0);

  return TRUE  if ($d == 0);
  return FALSE if ($d != 2);

  my ($pr, $ca) = ($prime_io->{$Periphery}, $cand_io->{$Periphery});

  $d = (defined($pr) && scalar(keys %{$pr}) > 0) + (defined($ca) && scalar(keys %{$ca}) > 0);

  return TRUE  if ($d == 0);
  return FALSE if ($d != 2);

  foreach (keys(%{$pr}))
    {
    $d = $ca->{$_};

    return FALSE if (! defined($d) || $d != $pr->{$_});
    }

  return TRUE;
  }

#-------------------------------------------------------------------------------

sub make_mcu_groups($)
  {
  my $Periphery = $_[0];
  my $index = 0;

  foreach (@mcu_filtered)
    {
    next if ($_->{IN_GROUP});

    my $group = \@{$mcu_groups[$index]};
    my $prime = $_;

        # The prime - reference - member of group;
    push(@{$group}, $_);
    $_->{IN_GROUP} = TRUE;

    foreach (@mcu_filtered)
      {
      next if ($_->{IN_GROUP} || $prime == $_);

      if (find_equivalent_mcu($prime, $_) && cmp_io_dirs($prime, $_, $Periphery))
        {
        Log("make_mcu_groups(): $prime->{NAME} == $_->{NAME}\n", 5);
        push(@{$group}, $_);
        $_->{IN_GROUP} = TRUE;
        }
      }

    @{$group} = sort {smartSort($a->{NAME}, $b->{NAME})} @{$group};
    ++$index;
    }
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@  Prints the register tables.  @@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

=back
        Sometimes a bit has more name. This procedure selects these from among
        the shortest.
=cut

sub find_shortest_name($)
  {
  my $min = ULONG_MAX;
  my $str = '';

  foreach (@{$_[0]})
    {
    my $l = length;

    if ($l < $min)
      {
      $min = $l;
      $str = $_;
      }
    }

  return $str;
  }

#-------------------------------------------------------------------------------

sub print_registers($$$)
  {
  my ($Reg_array, $Print_mode, $No_ADC) = @_;
  my ($i, $bits);
  my $show_closing_border = FALSE;

  foreach (@{$Reg_array})
    {
    next if ($No_ADC && exist_in_list(\@some_ADC_registers, $_->{NAME}));

        # Sole bit not have name and the empty register is not must show.
    next if ($_->{EMPTY} && $Print_mode == P_NO_SHOW_IF_EMPTY);

    if ($Print_mode == P_SHOW_ONLY_NAME)
      {
      Outfl("%-10s  (%03Xh)", $_->{NAME}, $_->{ADDRESS});
      $initial_border = FALSE;
      next;
      }

    if (! $initial_border)
      {
      Outl($table_border);
      $initial_border = TRUE;
      }

    Outf("%-10s  (%03Xh) |", $_->{NAME}, $_->{ADDRESS});

    $bits = \@{$_->{BITNAMES}};
    for ($i = 7; $i >= 0; --$i)
      {
      if (defined($bits->[$i]) && $Print_mode != P_NO_SHOW_BITS)
        {
        Outf("%-9s|", find_shortest_name(\@{$bits->[$i]}));
        }
      else
        {
        Out('         |');
        }
      }

    Outl();
    $show_closing_border = TRUE;
    } # foreach (@{$_[0]})

  Outl($table_border) if ($show_closing_border);
  }

#-------------------------------------------------------------------------------

        #
        # Collects into a list the inputs of ADC, which are uses another
        # periphery also.
        #

sub filter_off_adc_inputs($$)
  {
  my ($Peri_pins, $Adc) = @_;
  my @adc_pins = ();

  if (defined($Adc))
    {
    foreach my $adc_pin_name (keys(%{$Adc}))
      {
      my $adc_pin_io = $Adc->{$adc_pin_name};

      foreach my $peri_pin_name (keys(%{$Peri_pins}))
        {
        foreach (@{$Peri_pins->{$peri_pin_name}})
          {
          if (! (/^$adc_pin_name$/ ~~ @adc_pins) && /^$_$/ ~~ @{$adc_pin_io})
            {
            push(@adc_pins, $adc_pin_name)
            }
          }
        }
      }
    }

  return \@adc_pins;
  }

#-------------------------------------------------------------------------------

sub print_mcu($$)
  {
  my ($Mcu, $Peri_name) = @_;
  my $name       = lc($Mcu->{NAME});
  my $io_ref     = \%{$io_table_by_mcu{$name}};
  my $io_dir_ref = \%{$io_dir_table_by_mcu{$name}};
  my $peri_pins  = (defined($io_ref)) ? $io_ref->{$Peri_name} : undef;
  my $peri_dirs  = (defined($io_dir_ref)) ? $io_dir_ref->{$Peri_name} : undef;
  my ($adc, $adc_pins, $io, $pin_name, $drop_adc_pins);
  my $suppl_info = '';

  $drop_adc_pins = FALSE;

  if (defined($peri_pins))
    {
    if ($Peri_name ne 'adc')
      {
      $adc = \%{$io_ref->{'adc'}};
      $adc_pins = filter_off_adc_inputs($peri_pins, $adc);

      if (scalar(@{$adc_pins}) > 0)
        {
        # Supplementary information: Displays inputs of the ADC periphery.

        $suppl_info .= "\n";

        foreach $pin_name (sort {smartSort($a, $b)} @{$adc_pins})
          {
          $suppl_info .= "\t$pin_name:";

          foreach (@{$adc->{$pin_name}})
            {
            $suppl_info .= " $_";
            }

          $suppl_info .= "\n";
          }
        }
      else
        {
        $drop_adc_pins = TRUE;
        }
      } # if ($Peri_name ne 'adc')

    $suppl_info .= "\n";

    foreach $pin_name (sort {smartSort($a, $b)} keys(%{$peri_pins}))
      {
      $suppl_info .= "\t$pin_name:";

      foreach (@{$peri_pins->{$pin_name}})
        {
        $suppl_info .= " $_";
        }

      $suppl_info .= "\n";
      }
    } # if (defined($peri_pins))
  else
    {
    print STDERR "print_mcu(): This MCU $name not have $Peri_name pin!\n";
    }

  if (defined($peri_dirs))
    {
    $suppl_info .= "\n    I/O directions after initialization:\n\n";

    foreach (sort {smartSort($a, $b)} keys %{$peri_dirs})
      {
      $io = ($peri_dirs->{$_} == 0) ? '0 (output)' : '1 (input)';
      $suppl_info .= "\t$_: $io\n";
      }
    }

  $initial_border = FALSE;

  foreach (@{$Mcu->{REG_GROUPS}})
    {
    next if (scalar(@{$_->{REG_ARRAY}}) == 0);

    print_registers(\@{$_->{REG_ARRAY}}, $_->{PRINT_MODE}, $drop_adc_pins);
    }

  Out($suppl_info);
  }

#-------------------------------------------------------------------------------

sub print_all_data($$)
  {
  my ($Periphery, $Index) = @_;
  my $peri_name = $Periphery->{NAME};
  my $lock = '__' . uc($peri_name) . '__H__';
  my ($sidx, $group_index, $border, $group_name);

  Outl("\n#ifndef $lock\n#define $lock\n\n#include \"pic18fam.h\"");

  $peri_groups .= "\n    SECTION=" . uc($peri_name) . "\n\n";

  if ($make_groups)
    {
    $group_index = 1;
    $border = '#' x 45;

    make_mcu_groups($peri_name);

    foreach (@mcu_groups)
      {
      next if (scalar(@{$_}) == 0);

      Outl("\n//$border ${group_index}th group $border");

      ($group_name) = ($_->[0]->{NAME} =~ /^18f(\w+)$/io);

      given ($group_name)
        {
        # 18fxxJyy
        when (/j/io) { $sidx = '1'; }

        # 18fxxKyy
        when (/k/io) { $sidx = '2'; }

        # 18fxxxx
        default      { $sidx = '0'; }
        }

      $group_name =~ s/\D//go;
      $group_name = "0$group_name" if (length($group_name) < 4);

      $peri_groups .= "18$group_name$Index$sidx:" . join(',', map { lc($_->{NAME}); } @{$_}) . "\n";

      if ($only_prime)
        {
        Outl("\n/*");

        # Prints the name of the group members.

        foreach (@{$_})
          {
          Outl("PIC$_->{NAME}");
          }

        # Only contents of the first it shows, because content of the others same.

        print_mcu($_->[0], $peri_name);
        Outl('*/');
        }
      else
        {
        # Displays full contents of each member of the group.

        foreach (@{$_})
          {
          Outl("\n\n/*\nPIC$_->{NAME}");
          print_mcu($_, $peri_name);
          Outl('*/');
          }
        }

      ++$group_index;
      }
    }
  else
    {
        # Displays full contents of each MCU.

    foreach (@mcu_filtered)
      {
      Outl("\n\n/*\nPIC$_->{NAME}");
      print_mcu($_, $peri_name);
      Outl('*/');
      }
    }

  Outl("\n#endif // $lock");
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@  The main program.  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

load_periphery_data();

$include_path = '';
$make_groups = FALSE;
$only_prime = FALSE;

for (my $i = 0; $i < @ARGV; )
  {
  my $opt = $ARGV[$i++];

  given ($opt)
    {
    when (/^-(I|-include)$/o)
      {
      die "This option \"$opt\" requires a parameter." if ($i > $#ARGV);

      $include_path = $ARGV[$i++];
      }

    when (/^-(g|-make-groups)$/o)
      {
      $make_groups = TRUE;
      }

    when (/^-(p|-only-prime)$/o)
      {
      $only_prime = TRUE;
      }

    when (/^-(gp|pg)$/o)
      {
      $make_groups = TRUE;
      $only_prime = TRUE;
      }

    when (/^-(v|-verbose)$/o)
      {
      die "This option \"$opt\" requires a parameter.\n" if ($i > $#ARGV);

      $verbose = int($ARGV[$i++]);
      $verbose = 0 if (! defined($verbose) || $verbose < 0);
      $verbose = 10 if ($verbose > 10);
      }

    when (/^-(h|-help)$/o)
      {
      print <<EOT
Usage: $0 [options]

    Options are:

        -I <path> or --include <path>

            The program on this path looks for the headers.
            If this is not specified, then looking for a installed
            sdcc copy in the system.

        -g or --make-groups

            This command creates groups of MCUs.

        -p or --only-prime

            Prints only the prime member of an MCU group.

        -v <level> or --verbose <level>

            It provides information on from the own operation.
            Possible value of the level between 0 and 10. (default: 0)

        -h or --help

            This text.
EOT
;
      exit(0);
      }
    } # given ($opt)
  }

if ($include_path eq '')
  {
  foreach (@default_paths)
    {
    if (-d $_)
      {
      $include_path = "$_/$default_port";
      last;
      }
    }

  die "Can not find the directory of sdcc headers!" if ($include_path eq '');
  }

opendir(DIR, $include_path) || die "Can not open. -> \"$include_path\"";

print "Include path: \"$include_path\"\n";

my @filelist = grep(-f "$include_path/$_" && /^$header_name_filter$/, readdir(DIR));
closedir(DIR);

@mcu_raw = ();
foreach (sort {smartSort($a, $b)} @filelist)
  {
  my $name = $_;

  print STDERR "Reading the registers from the $_ header ...";

  $name =~ s/^pic//io;
  $name =~ s/\.\S+$//o;
  read_regs_from_header(add_mcu_raw($name), $_);
  print STDERR " done.\n";
  }

my $p_idx = 0;
foreach (@periphery_table)
  {
  my $out_name = "$_->{NAME}.h$out_tail";

  open($out_handler, '>', $out_name) || die "Can not create the $out_name output!";

  print STDERR "Filtering of registers the aspects of $_->{NAME} according to ...";
  @mcu_filtered = ();
  @mcu_groups = ();
  filter_regs_from_raw($_);
  print STDERR " done.\n";

  print STDERR "Creating the $out_name ...";
  print_all_data($_, $p_idx);
  print STDERR " done.\n";

  close($out_handler);
  ++$p_idx;
  }

open(GR, '>', $peri_group) || die "Can not create the $peri_group output!";
print GR $peri_groups;
close(GR);

__END__
################################################################################
#
#   The following rules determine to which registers belong to an peripheral.
#   The description of a periphery is bounded by the BEGIN=TABLE_XXX flags.
#   The description of a register begin after the BEGIN=REGISTER flag.
#
#   BEGIN=PERIPHERY:ADC
#       The "ADC" effect of: An file will be created under the name "adc.tables".
#
#   VALID_REGS -- This a regular expression. Specifies which one a register
#                 applies to this entries.
#
#   VALID_BITS -- This a regular expression. Specifies which bits are interesting,
#                 are important. If an empty string is in there will be no filtering.
#
#   PRINT_MODE -- This a constant. The following values can be:
#
#                 P_NO_SHOW_BITS     -- Does not shows the bits.
#
#                 P_NO_SHOW_IF_EMPTY -- Not shows the register if it is empty.
#                                       (This it could be because there are no bits
#                                        or because the filter thrown out them.)
#
#                 P_ALWAYS_SHOW      -- All conditions shows the register.
#
#                 P_SHOW_ONLY_NAME   -- Only shows the register name and address.
#

################################################################################
#
#       The ADC module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:ADC     # ADC --> adc.h.gen

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANCON\d+"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCTMUEN\d+[HL]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCHIT\d+[HL]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCSS\d+[HL]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCHS\d+[HL]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADRES[HL]?|ADCBUF\d+[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[\dHL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-O]"
    VALID_BITS="ANS[A-O]\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(FVR|REF|VREF)CON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="DAC(|ON\d*|CON\d+)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="INTCON\d?"
    VALID_BITS="((G|PE)IE|GIE[HL])"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="IPR\d+"
    VALID_BITS="ADIP"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIE\d+"
    VALID_BITS="ADIE"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIR\d+"
    VALID_BITS="ADIF"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PMD\d+"
    VALID_BITS="ADC\d*MD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CO?M(CON\d*|\d*CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TRIS([A-Z]|IO)"
    VALID_BITS="TRIS([A-Z]|IO)\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

END=PERIPHERY

################################################################################
#
#       The CCP module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:CCP     # CCP --> ccp.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="(CCP(SEL\d|\dSEL)|P\d[A-D]SEL)"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="E?CCP(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TCLK(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="E?CCP(AS\d*|\d+AS)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="E?CCPR([HL]\d*|\d+[HL])"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCPTMRS\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PSTR(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(PWM(CON\d*|\d+CON)|ECCP\d*DEL)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(T[2468]CON|T10CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(TMR[2468]|TMR10)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(PR[2468]|PR10)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="INTCON\d?"
    VALID_BITS="((G|PE)IE|GIE[HL])"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIE\d+"
    VALID_BITS="CCP\d+IE"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="IPR\d+"
    VALID_BITS="CCP\d+IP"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIR\d+"
    VALID_BITS="CCP\d+IF"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PMD\d+"
    VALID_BITS="(CCP|TMR)\d+MD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[\dHL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-O]"
    VALID_BITS="ANS[A-O]\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANCON\d+"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSCON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPINR[78]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPINR(14_15|16_17|32_33|34_35|36_37|38_39)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPOR\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="OD(\d*CON|CON\d*)"
    VALID_BITS="CCP\d*OD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TRIS([A-Z]|IO)"
    VALID_BITS="TRIS([A-Z]|IO)\d|CCP\d*OD"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

END=PERIPHERY

################################################################################
#
#       The PWM(CCP) module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:PWM     # PWM --> pwm.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="(CCP(SEL\d|\dSEL)|P\d[A-D]SEL)"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="E?CCP(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TCLK(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="E?CCP(AS\d*|\d+AS)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="E?CCPR([HL]\d*|\d+[HL])"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCPTMRS\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(PT(CON\d*|\d+CON)|PSTR(CON\d*|\d+CON))"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(PWM(CON\d*|\d+CON)|ECCP\d*DEL)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="DTCON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="OVDCON[DS]\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="FLTCONFIG"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="P(DCL\d*|DC\d*L)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="P(DCH\d*|DC\d*H)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PT(MR|PER)L\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PT(MR|PER)H\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SEVTCMPL\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SEVTCMPH\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(T[2468]CON|T10CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(TMR[2468]|TMR10)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(PR[2468]|PR10)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="INTCON\d?"
    VALID_BITS="((G|PE)IE|GIE[HL])"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="IPR\d+"
    VALID_BITS="((CCP\d+|TMR[2468])IP|TMR10IP)"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIE\d+"
    VALID_BITS="((CCP\d+|TMR[2468])IE|TMR10IE)"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIR\d+"
    VALID_BITS="((CCP\d+|TMR[2468])IF|TMR10IF)"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PMD\d+"
    VALID_BITS="(CCP|TMR)\d+MD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[\dHL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-O]"
    VALID_BITS="ANS[A-O]\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANCON\d+"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSCON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPOR\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="OD(\d*CON|CON\d*)"
    VALID_BITS="CCP\d*OD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TRIS([A-Z]|IO)"
    VALID_BITS="TRIS([A-Z]|IO)\d|CCP\d*OD"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

END=PERIPHERY

################################################################################
#
#       The I2C(SSP) module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:I2C     # I2C --> i2c.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="S(DI|DO|CK|S)\d*SEL"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*CON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*ADD\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*BUF\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*MSK\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*STAT\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="INTCON\d?"
    VALID_BITS="((G|PE)IE|GIE[HL])"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="IPR\d+"
    VALID_BITS="(BCL|SSP)\d*IP"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIE\d+"
    VALID_BITS="(BCL|SSP)\d*IE"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIR\d+"
    VALID_BITS="(BCL|SSP)\d*IF"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PMD\d+"
    VALID_BITS="MSSP\d+MD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[\dHL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-O]"
    VALID_BITS="ANS[A-O]\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANCON\d+"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSCON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPINR2[123]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPOR\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TRIS([A-Z]|IO)"
    VALID_BITS="TRIS([A-Z]|IO)\d|SPIOD"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

END=PERIPHERY

################################################################################
#
#       The SPI(SSP) module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:SPI     # SPI --> spi.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="S(DI|DO|CK|S)\d*SEL"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*CON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*ADD\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*BUF\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*MSK\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\d*STAT\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="INTCON\d?"
    VALID_BITS="((G|PE)IE|GIE[HL])"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="IPR\d+"
    VALID_BITS="(BCL|SSP)\d*IP"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIE\d+"
    VALID_BITS="(BCL|SSP)\d*IE"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIR\d+"
    VALID_BITS="(BCL|SSP)\d*IF"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PMD\d+"
    VALID_BITS="MSSP\d+MD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[\dHL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-O]"
    VALID_BITS="ANS[A-O]\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANCON\d+"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSCON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPINR2[123]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPINR(8_9|10_11|12_13)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPOR\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TRIS([A-Z]|IO)"
    VALID_BITS="TRIS([A-Z]|IO)\d|SPIOD"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

END=PERIPHERY

################################################################################
#
#       The USART module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:USART   # USART --> usart.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="(RX(DT)?|TX(CK)?)SEL"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RC(STA\d*|\d+STA)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TX(STA\d*|\d+STA)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="BAUD(C(ON|TL)\d*|\d+C(ON|TL))"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SP(BRG[HL]?\d?|\d+BRG[HL]?)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RC(REG\d*|\d+REG)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TX(REG\d*|\d+REG)"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="INTCON\d?"
    VALID_BITS="((G|PE)IE|GIE[HL])"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="IPR\d+"
    VALID_BITS="(RC|TX)\d*IP"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIE\d+"
    VALID_BITS="(RC|TX)\d*IE"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIR\d+"
    VALID_BITS="(RC|TX)\d*IF"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PMD\d+"
    VALID_BITS="UART\d+MD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[\dHL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-O]"
    VALID_BITS="ANS[A-O]\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANCON\d+"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSCON\d*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPINR1[67]"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPINR(0_1|2_3|4_5|6_7)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="RPOR\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="OD(\d*CON|CON\d*)"
    VALID_BITS="U\d*OD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="LATG"
    VALID_BITS="U\d*OD"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TRIS([A-Z]|IO)"
    VALID_BITS="TRIS([A-Z]|IO)\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

END=PERIPHERY

################################################################################
#
#       This table describes that which onto port pins connected a peripheral.
#

BEGIN=IO_TABLE

  BEGIN=MCU:18f13k22,18f14k22
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    ADC=AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP1:RC5
    PWM=PWM:RC5
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RC6
    USART=RX:RB5,TX:RB7
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f13k50,18f14k50
    ADC=AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    ADC=AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP1:RC5
    PWM=PWM:RC5
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RC6
    USART=RX:RB5,TX:RB7
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f23k20,18f24k20,18f25k20,18f26k20
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f43k20,18f44k20,18f45k20,18f46k20
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f23k22,18f24k22,18f25k22,18f26k22
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    ADC=AN13:RB5,AN14:RC2,AN15:RC3,AN16:RC4,AN17:RC5,AN18:RC6,AN19:RC7
    CCP=CCP1:RC2,CCP2:RC1/RB3,CCP3:RB5,CCP4:RB0,CCP5:RA4
    PWM=PWM1:RC2,PWM2:RC1/RB3,PWM3:RB5,PWM4:RB0,PWM5:RA4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    I2C=SDA2:RB2,SCL2:RB1
    SPI=SDI2:RB2,SDO2:RB3,SCK2:RB1,SS2:RB0
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RB7,TX2:RB6
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:1
  END=MCU

  BEGIN=MCU:18f43k22,18f44k22,18f45k22,18f46k22
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    ADC=AN13:RB5,AN14:RC2,AN15:RC3,AN16:RC4,AN17:RC5,AN18:RC6,AN19:RC7
    ADC=AN20:RD0,AN21:RD1,AN22:RD2,AN23:RD3,AN24:RD4,AN25:RD5,AN26:RD6,AN27:RD7
    CCP=CCP1:RC2,CCP2:RC1/RB3,CCP3:RB5,CCP4:RD1,CCP5:RE2
    PWM=PWM1:RC2,PWM2:RC1/RB3,PWM3:RB5,PWM4:RD1,PWM5:RE2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    I2C=SDA2:RB2,SCL2:RB1
    SPI=SDI2:RB2,SDO2:RB3,SCK2:RB1,SS2:RB0
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RB7,TX2:RB6
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:1
  END=MCU

  BEGIN=MCU:18f24j10,18f25j10
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f44j10,18f45j10
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    I2C=SDA2:RD1,SCL2:RD0
    SPI=SDI2:RD1,SDO2:RD2,SCK2:RD0,SS2:RD3
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

#
# Remappable peripheral pins. (PPS)
# (The definition valid until a newer definition overwrites the members.)
#
  BEGIN=DEFINE
    RP0=RA0
    RP1=RA1
    RP2=RA5
    RP3=RB0
    RP4=RB1
    RP5=RB2
    RP6=RB3
    RP7=RB4
    RP8=RB5
    RP9=RB6
    RP10=RB7
    RP11=RC0
    RP12=RC1
    RP13=RC2
    RP14=RC3
    RP15=RC4
    RP16=RC5
    RP17=RC6
    RP18=RC7
  END=DEFINE

  BEGIN=MCU:18f24j11,18f25j11,18f26j11
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP1:RP[0-18]
    CCP=CCP2:RP[0-18]
    PWM=PWM1:RP[0-18]
    PWM=PWM2:RP[0-18]
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    SPI=SDI2:RP[0-18]
    SPI=SDO2:RP[0-18]
    SPI=SCK2:RP[0-18]
    SPI=SS2:RP[0-18]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-18]
    USART=TX2:RP[0-18]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

#
# Remappable peripheral pins. (PPS)
# (The definition valid until a newer definition overwrites the members.)
#
  BEGIN=DEFINE
    RP19=RD2
    RP20=RD3
    RP21=RD4
    RP22=RD5
    RP23=RD6
    RP24=RD7
  END=DEFINE

  BEGIN=MCU:18f44j11,18f45j11,18f46j11
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP1:RP[0-24]
    CCP=CCP2:RP[0-24]
    PWM=PWM1:RP[0-24]
    PWM=PWM2:RP[0-24]
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    I2C=SDA2:RD1,SCL2:RD0
    SPI=SDI2:RP[0-24]
    SPI=SDO2:RP[0-24]
    SPI=SCK2:RP[0-24]
    SPI=SS2:RP[0-24]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-24]
    USART=TX2:RP[0-24]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f24j50,18f25j50,18f26j50
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP1:RP[0-13]/RP[17-18]
    CCP=CCP2:RP[0-13]/RP[17-18]
    PWM=PWM1:RP[0-13]/RP[17-18]
    PWM=PWM2:RP[0-13]/RP[17-18]
    I2C=SDA1:RB5,SCL1:RB4
    SPI=SDI1:RB5,SDO1:RC7,SCK1:RB4,SS1:RA5
    SPI=SDI2:RP[0-13]/RP[17-18]
    SPI=SDO2:RP[0-13]/RP[17-18]
    SPI=SCK2:RP[0-13]/RP[17-18]
    SPI=SS2:RP[0-13]/RP[17-18]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-13]/RP[17-18]
    USART=TX2:RP[0-13]/RP[17-18]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f44j50,18f45j50,18f46j50
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP1:RP[0-13]/RP[17-24]
    CCP=CCP2:RP[0-13]/RP[17-24]
    PWM=PWM1:RP[0-13]/RP[17-24]
    PWM=PWM2:RP[0-13]/RP[17-24]
    I2C=SDA1:RB5,SCL1:RB4
    SPI=SDI1:RB5,SDO1:RC7,SCK1:RB4,SS1:RA5
    I2C=SDA2:RD1,SCL2:RD0
    SPI=SDI2:RP[0-13]/RP[17-24]
    SPI=SDO2:RP[0-13]/RP[17-24]
    SPI=SCK2:RP[0-13]/RP[17-24]
    SPI=SS2:RP[0-13]/RP[17-24]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-13]/RP[17-24]
    USART=TX2:RP[0-13]/RP[17-24]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f24k50,18f25k50
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    ADC=AN14:RC2,AN18:RC6,AN19:RC7
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RB0,SCL:RB1
    SPI=SDI:RB0,SDO:RB3/RC7,SCK:RB1,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f45k50
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    ADC=AN14:RC2,AN18:RC6,AN19:RC7
    ADC=AN20:RD0,AN21:RD1,AN22:RD2,AN23:RD3,AN24:RD4,AN25:RD5,AN26:RD6,AN27:RD7
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RB0,SCL:RB1
    SPI=SDI:RB0,SDO:RB3/RC7,SCK:RB1,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f25k80,18f26k80
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB1,AN9:RB4,AN10:RB0
    CCP=CCP1:RB4,CCP2:RC2,CCP3:RC6,CCP4:RC7,CCP5:RB5
    PWM=PWM1:RB4,PWM2:RC2,PWM3:RC6,PWM4:RC7,PWM5:RB5
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RB7,TX2:RB6
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f45k80,18f46k80
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB1,AN9:RB4,AN10:RB0
    CCP=CCP1:RD4,CCP2:RC2,CCP3:RC6,CCP4:RC7,CCP5:RB5
    PWM=PWM1:RD4,PWM2:RC2,PWM3:RC6,PWM4:RC7,PWM5:RB5
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RD7,TX2:RD6
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f65k80,18f66k80
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB1,AN9:RB4,AN10:RB0
    CCP=CCP1:RD4,CCP2:RC2,CCP3:RC6,CCP4:RC7,CCP5:RB5
    PWM=PWM1:RD4,PWM2:RC2,PWM3:RC6,PWM4:RC7,PWM5:RB5
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX1:RG0,TX1:RG3
    USART=RX2:RE6,TX2:RE7
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f26j13,18f27j13
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP4:RB4,CCP5:RB5,CCP6:RB6,CCP7:RB7,CCP8:RC1,CCP9:RC6,CCP10:RC7
    PWM=PWM4:RB4,PWM5:RB5,PWM6:RB6,PWM7:RB7,PWM8:RC1,PWM9:RC6,PWM10:RC7
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    SPI=SDI2:RP[0-18]
    SPI=SDO2:RP[0-18]
    SPI=SCK2:RP[0-18]
    SPI=SS2:RP[0-18]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-18]
    USART=TX2:RP[0-18]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f46j13,18f47j13
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP4:RB4,CCP5:RB5,CCP6:RB6,CCP7:RB7,CCP8:RC1,CCP9:RC6,CCP10:RC7
    PWM=PWM4:RB4,PWM5:RB5,PWM6:RB6,PWM7:RB7,PWM8:RC1,PWM9:RC6,PWM10:RC7
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RA5
    I2C=SDA2:RD1,SCL2:RD0
    SPI=SDI2:RP[0-24]
    SPI=SDO2:RP[0-24]
    SPI=SCK2:RP[0-24]
    SPI=SS2:RP[0-24]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-24]
    USART=TX2:RP[0-24]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f26j53,18f27j53
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP4:RB4,CCP5:RB5,CCP6:RB6,CCP7:RB7,CCP8:RC1,CCP9:RC6,CCP10:RC7
    PWM=PWM4:RB4,PWM5:RB5,PWM6:RB6,PWM7:RB7,PWM8:RC1,PWM9:RC6,PWM10:RC7
    I2C=SDA1:RB5,SCL1:RB4
    SPI=SDI1:RB5,SDO1:RC7,SCK1:RB4,SS1:RA5
    SPI=SDI2:RP[0-13]/RP[17-18]
    SPI=SDO2:RP[0-13]/RP[17-18]
    SPI=SCK2:RP[0-13]/RP[17-18]
    SPI=SS2:RP[0-13]/RP[17-18]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-13]/RP[17-18]
    USART=TX2:RP[0-13]/RP[17-18]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f46j53,18f47j53
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RC2,AN12:RB0
    CCP=CCP4:RB4,CCP5:RB5,CCP6:RB6,CCP7:RB7,CCP8:RC1,CCP9:RC6,CCP10:RC7
    PWM=PWM4:RB4,PWM5:RB5,PWM6:RB6,PWM7:RB7,PWM8:RC1,PWM9:RC6,PWM10:RC7
    I2C=SDA1:RB5,SCL1:RB4
    SPI=SDI1:RB5,SDO1:RC7,SCK1:RB4,SS1:RA5
    I2C=SDA2:RD1,SCL2:RD0
    SPI=SDI2:RP[0-13]/RP[17-24]
    SPI=SDO2:RP[0-13]/RP[17-24]
    SPI=SCK2:RP[0-13]/RP[17-24]
    SPI=SS2:RP[0-13]/RP[17-24]
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RP[0-13]/RP[17-24]
    USART=TX2:RP[0-13]/RP[17-24]
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f63j11,18f64j11,18f65j11
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f83j11,18f84j11,18f85j11
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f66j11,18f66j16,18f67j11
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f86j11,18f86j16,18f87j11
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f63j90,18f64j90,18f65j90
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f66j90,18f66j93,18f67j90,18f67j93
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f83j90,18f84j90,18f85j90
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN9:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f86j90,18f86j93,18f87j90,18f87j93
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN9:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f65j10,18f65j15,18f66j10,18f66j15,18f67j10
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f85j10,18f85j15,18f86j10,18f86j15,18f87j10
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f65j50,18f66j50,18f66j55,18f67j50
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f85j50,18f86j50,18f86j55,18f87j50
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f65k22
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6,CCP7:RE5,CCP8:RE4
    PWM=PWM6:RE6,PWM7:RE5,PWM8:RE4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f66k22,18f67k22
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6,CCP7:RE5,CCP8:RE4,CCP9:RE3,CCP10:RE2
    PWM=PWM6:RE6,PWM7:RE5,PWM8:RE4,PWM9:RE3,PWM10:RE2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f85k22
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    ADC=AN20:RH3,AN21:RH2,AN22:RH1,AN23:RH0
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6/RH7,CCP7:RE5/RH6,CCP8:RE4/RH5
    PWM=PWM6:RE6/RH7,PWM7:RE5/RH6,PWM8:RE4/RH5
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f86k22,18f87k22
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    ADC=AN20:RH3,AN21:RH2,AN22:RH1,AN23:RH0
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6/RH7,CCP7:RE5/RH6,CCP8:RE4/RH5,CCP9:RE3/RH4,CCP10:RE2
    PWM=PWM6:RE6/RH7,PWM7:RE5/RH6,PWM8:RE4/RH5,PWM9:RE3/RH4,PWM10:RE2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f65k90
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6,CCP7:RE5,CCP8:RE4
    PWM=PWM6:RE6,PWM7:RE5,PWM8:RE4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f66k90,18f67k90
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6,CCP7:RE5,CCP8:RE4,CCP9:RE3,CCP10:RE2
    PWM=PWM6:RE6,PWM7:RE5,PWM8:RE4,PWM9:RE3,PWM10:RE2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f85k90
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    ADC=AN20:RH3,AN21:RH2,AN22:RH1,AN23:RH0
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6/RH7,CCP7:RE5/RH6,CCP8:RE4/RH5
    PWM=PWM6:RE6/RH7,PWM7:RE5/RH6,PWM8:RE4/RH5
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f86k90,18f87k90
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    ADC=AN20:RH3,AN21:RH2,AN22:RH1,AN23:RH0
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    CCP=CCP6:RE6/RH7,CCP7:RE5/RH6,CCP8:RE4/RH5,CCP9:RE3/RH4,CCP10:RE2
    PWM=PWM6:RE6/RH7,PWM7:RE5/RH6,PWM8:RE4/RH5,PWM9:RE3/RH4,PWM10:RE2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f66j60,18f66j65,18f67j60
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1,CCP3:RD1,CCP4:RD2,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1,PWM3:RD1,PWM4:RD2,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    USART=RX1:RC7,TX1:RC6
    USART=IO_DIR=RX1:1,TX1:0
  END=MCU

  BEGIN=MCU:18f86j60,18f86j65,18f87j60
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f96j60,18f96j65,18f97j60
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f86j72,18f87j72
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF7,AN6:RF1,AN7:RF2,AN9:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f242,18f252
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f442,18f452
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f248,18f258
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f448,18f458
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f1220,18f1320
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RB0,AN5:RB1,AN6:RB4
    CCP=CCP:RB3
    PWM=PWM:RB3
    USART=RX:RB4,TX:RB1
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f1230,18f1330
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA6
    USART=RX:RA3,TX:RA2
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f2220,18f2320
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f4220,18f4320
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f2221,18f2321
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f4221,18f4321
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f2331,18f2431
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA4
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC5
    SPI=SDI:RC4,SDO:RC7,SCK:RC5,SS:RC6
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f4331,18f4431
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA4,AN5:RA5,AN6:RE0,AN7:RE1,AN8:RE2
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4/RD2,SCL:RC5/RD3
    SPI=SDI:RC4/RD2,SDO:RC7,SCK:RC5/RD3,SS:RC6
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f2410,18f2510,18f2515,18f2610
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f4410,18f4510,18f4515,18f4610
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f2420,18f2423,18f2520,18f2523
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f4420,18f4423,18f4520,18f4523
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f2439,18f2539
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    PWM=PWM1:PWM1,PWM2:PWM2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f4439,18f4539
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    PWM=PWM1:PWM1,PWM2:PWM2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f2450
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP:RC2
    PWM=PWM:RC2
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f4450
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP:RC2
    PWM=PWM:RC2
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f2455,18f2458,18f2550,18f2553
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RB0,SCL:RB1
    SPI=SDI:RB0,SDO:RC7,SCK:RB1,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f4455,18f4458,18f4550,18f4553
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RB0,SCL:RB1
    SPI=SDI:RB0,SDO:RC7,SCK:RB1,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f2480,18f2580,18f2585,18f2680,18f2682,18f2685
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB1,AN9:RB4,AN10:RB0
    CCP=CCP:RC2
    PWM=PWM:RC2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f4480,18f4580,18f4585,18f4680,18f4682,18f4685
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB1,AN9:RB4,AN10:RB0
    CCP=CCP1:RC2,CCP2:RD4
    PWM=PWM1:RC2,PWM2:RD4
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:18f2525,18f2620
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f4525,18f4620
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    ADC=AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f6310,18f6410
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f8310,18f8410
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f6390,18f6393,18f6490,18f6493
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f8390,18f8393,18f8490,18f8493
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f6520,18f6525,18f6620,18f6621,18f6720
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f8520,18f8525,18f8620,18f8621,18f8720
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f6527,18f6622,18f6627,18f6628,18f6722,18f6723
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f8527,18f8622,18f8627,18f8628,18f8722,18f8723
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6
    USART=RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:18f6585,18f6680
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    CCP=CCP1:RC2,CCP2:RC1/RE7
    PWM=PWM1:RC2,PWM2:RC1/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:18f8585,18f8680
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    ADC=AN5:RF0,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    CCP=CCP1:RC2,CCP2:RC1/RB3/RE7
    PWM=PWM1:RC2,PWM2:RC1/RB3/RE7
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RF7
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

#
# Remappable peripheral pins. (PPS)
# (The definition valid until a newer definition overwrites the members.)
#
  BEGIN=DEFINE
    RP0=RA0
    RP1=RA1
    RP2=RA2
    RP3=RA3
    RP4=RA4
    RP5=RA5
    RP6=RA6
    RP7=RB3
    RP8=RB0
    RP9=RB1
    RP10=RA7
    RP11=RC2
    RP12=RB4
    RP13=RB5
    RP14=RB2
    RP15=RC3
    RP16=RC5
    RP17=RC4
    RP18=RC6
    RP19=RC7
    RP20=RD0
    RP21=RD1
    RP22=RD2
    RP23=RD3
    RP24=RD4
    RP25=RD5
    RP26=RD6
    RP27=RD7
    RP28=RE0
    RP29=RE1
    RP30=RE2
    RP31=RE7
    RP32=RE4
    RP33=RE3
    RP34=RE6
    RP35=RF5
    RP36=RF2
    RP37=RE5
    RP38=RF7
    RP39=RG1
    RP40=RF6
    RP41=RF3
    RP42=RG2
    RP43=RG3
    RP44=RG4
    RP45=RF4
    RP46=RG0
  END=DEFINE

#
# PPS-Lite
#
  BEGIN=GROUP
    RP_GROUP0=RP0,RP4,RP8,RP12,RP16,RP20,RP24,RP28,RP32,RP36,RP40,RP44
    RP_GROUP1=RP1,RP5,RP9,RP13,RP17,RP21,RP25,RP29,RP33,RP37,RP41,RP45
    RP_GROUP2=RP2,RP6,RP10,RP14,RP18,RP22,RP26,RP30,RP34,RP38,RP42,RP46
    RP_GROUP3=RP3,RP7,RP11,RP15,RP19,RP23,RP27,RP31,RP35,RP39,RP43
  END=GROUP

  BEGIN=MCU:18f65j94,18f66j94,18f66j99,18f67j94
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RF7,AN6:RA4
    ADC=AN7:RF2,AN8:RG0,AN9:RC2,AN10:RF5,AN11:RF6
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    CCP=CCP1:RP_GROUP3
    CCP=CCP2:RP_GROUP3
    CCP=CCP3:RP_GROUP2
    CCP=CCP4:RP_GROUP3
    CCP=CCP5:RP_GROUP0
    CCP=CCP6:RP_GROUP2
    CCP=CCP7:RP_GROUP1
    CCP=CCP8:RP_GROUP0
    CCP=CCP9:RP_GROUP1
    CCP=CCP10:RP_GROUP2
    PWM=PWM1:RP_GROUP3
    PWM=PWM2:RP_GROUP3
    PWM=PWM3:RP_GROUP2
    PWM=PWM4:RP_GROUP3
    PWM=PWM5:RP_GROUP0
    PWM=PWM6:RP_GROUP2
    PWM=PWM7:RP_GROUP1
    PWM=PWM8:RP_GROUP0
    PWM=PWM9:RP_GROUP1
    PWM=PWM10:RP_GROUP2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RP_GROUP0,SDO1:RP_GROUP1,SCK1:RP_GROUP3,SS1:RP_GROUP2
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RP_GROUP1,SDO2:RP_GROUP0,SCK2:RP_GROUP2,SS2:RP_GROUP3
    USART=RX1:RP_GROUP3,TX1:RP_GROUP2
    USART=RX2:RP_GROUP2,TX2:RP_GROUP3
    USART=RX3:RP_GROUP0,TX3:RP_GROUP1
    USART=RX4:RP_GROUP0,TX4:RP_GROUP1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:1,RX3:1,TX3:1,RX4:1,TX4:1
  END=MCU

  BEGIN=MCU:18f85j94,18f86j94,18f86j99,18f87j94
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RF7,AN6:RA4
    ADC=AN7:RF2,AN8:RG0,AN9:RC2,AN10:RF5,AN11:RF6
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    ADC=AN20:RH3,AN21:RH2,AN22:RH1,AN23:RH0
    CCP=CCP1:RP_GROUP3
    CCP=CCP2:RP_GROUP3
    CCP=CCP3:RP_GROUP2
    CCP=CCP4:RP_GROUP3
    CCP=CCP5:RP_GROUP0
    CCP=CCP6:RP_GROUP2
    CCP=CCP7:RP_GROUP1
    CCP=CCP8:RP_GROUP0
    CCP=CCP9:RP_GROUP1
    CCP=CCP10:RP_GROUP2
    PWM=PWM1:RP_GROUP3
    PWM=PWM2:RP_GROUP3
    PWM=PWM3:RP_GROUP2
    PWM=PWM4:RP_GROUP3
    PWM=PWM5:RP_GROUP0
    PWM=PWM6:RP_GROUP2
    PWM=PWM7:RP_GROUP1
    PWM=PWM8:RP_GROUP0
    PWM=PWM9:RP_GROUP1
    PWM=PWM10:RP_GROUP2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RP_GROUP0,SDO1:RP_GROUP1,SCK1:RP_GROUP3,SS1:RP_GROUP2
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RP_GROUP1,SDO2:RP_GROUP0,SCK2:RP_GROUP2,SS2:RP_GROUP3
    USART=RX1:RP_GROUP3,TX1:RP_GROUP2
    USART=RX2:RP_GROUP2,TX2:RP_GROUP3
    USART=RX3:RP_GROUP0,TX3:RP_GROUP1
    USART=RX4:RP_GROUP0,TX4:RP_GROUP1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:1,RX3:1,TX3:1,RX4:1,TX4:1
  END=MCU

  BEGIN=MCU:18f95j94,18f96j94,18f96j99,18f97j94
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RF7,AN6:RA4
    ADC=AN7:RF2,AN8:RG0,AN9:RC2,AN10:RF5,AN11:RF6
    ADC=AN16:RG4,AN17:RG3,AN18:RG2,AN19:RG1
    ADC=AN12:RH4,AN13:RH5,AN14:RH6,AN15:RH7
    ADC=AN20:RH3,AN21:RH2,AN22:RH1,AN23:RH0
    CCP=CCP1:RP_GROUP3
    CCP=CCP2:RP_GROUP3
    CCP=CCP3:RP_GROUP2
    CCP=CCP4:RP_GROUP3
    CCP=CCP5:RP_GROUP0
    CCP=CCP6:RP_GROUP2
    CCP=CCP7:RP_GROUP1
    CCP=CCP8:RP_GROUP0
    CCP=CCP9:RP_GROUP1
    CCP=CCP10:RP_GROUP2
    PWM=PWM1:RP_GROUP3
    PWM=PWM2:RP_GROUP3
    PWM=PWM3:RP_GROUP2
    PWM=PWM4:RP_GROUP3
    PWM=PWM5:RP_GROUP0
    PWM=PWM6:RP_GROUP2
    PWM=PWM7:RP_GROUP1
    PWM=PWM8:RP_GROUP0
    PWM=PWM9:RP_GROUP1
    PWM=PWM10:RP_GROUP2
    I2C=SDA1:RC4,SCL1:RC3
    SPI=SDI1:RP_GROUP0,SDO1:RP_GROUP1,SCK1:RP_GROUP3,SS1:RP_GROUP2
    I2C=SDA2:RD5,SCL2:RD6
    SPI=SDI2:RP_GROUP1,SDO2:RP_GROUP0,SCK2:RP_GROUP2,SS2:RP_GROUP3
    USART=RX1:RP_GROUP3,TX1:RP_GROUP2
    USART=RX2:RP_GROUP2,TX2:RP_GROUP3
    USART=RX3:RP_GROUP0,TX3:RP_GROUP1
    USART=RX4:RP_GROUP0,TX4:RP_GROUP1
    USART=IO_DIR=RX1:1,TX1:1,RX2:1,TX2:1,RX3:1,TX3:1,RX4:1,TX4:1
  END=MCU

END=IO_TABLE
