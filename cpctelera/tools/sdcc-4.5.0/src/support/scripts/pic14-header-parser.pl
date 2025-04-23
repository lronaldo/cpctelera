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

    Proposal for use: ./pic14-header-parser.pl -gp

    This program creates seven files in the actual directory:

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

    $Id: pic14-header-parser.pl 9072 2014-09-17 14:00:11Z molnarkaroly $
=cut

use Data::Dumper;
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

my $default_port = 'pic14';
my $header_name_filter = 'pic1[026][a-z]+\d+([a-z]|([a-z]+\d+)?).h';

my $include_path;
my $out_tail = '.gen';
my $peri_group = 'peripheral.groups';
my $table_border = (' ' x 19) . '+' . ('---------+' x 8);

my %reg_addresses = ();

my @some_ADC_registers =
  (
  'ADCON\d+[HL]?|AD\d*CON\d*',
  'ADCOMCON',
  'AD\d*RES[HL]?',
  'ANSEL[\dHL]?',
  'ANSEL[A-Z]'
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
        ENHANCED  => 0,      This is enhanced MCU.
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
        ENHANCED   => 0,      This is enhanced MCU.
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

my @regular = ();
my @enhanced = ();
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
        # This is a section. E.g.: "RI[0-8]"

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
        # This is a name. E.g.: "RD7" or "RO2"

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
                ENHANCED  => FALSE,
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

  $Mcu_raw->{ENHANCED} = TRUE if ($Name eq 'STATUS_SHAD');

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

  if ($Mcu_ref->{ENHANCED})
    {
    push(@enhanced, lc($Mcu_ref->{NAME}));
    }
  else
    {
    push(@regular, lc($Mcu_ref->{NAME}));
    }

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
            ENHANCED   => $Mcu_raw->{ENHANCED},
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

  return FALSE if ($Prime->{ENHANCED} != $Candidate->{ENHANCED});

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

  Outl("This is an enhanced 14 bit MCU.") if ($Mcu->{ENHANCED});

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
  my ($sidx, $group_index, $border);
  my ($family, $tech, $group_name, $len);

  Outl("\n#ifndef $lock\n#define $lock\n\n#include \"pic16fam.h\"");

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

      ($family, $tech, $group_name) = ($_->[0]->{NAME} =~ /^(1[026])(c|cr|f|hv|lf)(\w+)$/io);

      $group_name =~ s/\D//go;
      $len = length($group_name);

      if ($len < 3)
        {
        $group_name = "00$group_name";
        }
      elsif ($len < 4)
        {
        $group_name = "0$group_name";
        }

      given ($tech)
        {
        # C CR
        when (/c|cr/io) { $sidx = '0'; }

        # F HV LF
        default         { $sidx = '1'; }
        }

      $peri_groups .= "$family$group_name$Index$sidx:" . join(',', map { lc($_->{NAME}); } @{$_}) . "\n";

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

#-------------------------------------------------------------------------------

sub print_listing($$$)
  {
  my ($Out, $Device_per_line, $List_ref) = @_;
  my ($i, $len);

  $len = scalar(@{$List_ref});

  return if ($len <= 0);

  $i = 0;
  while (1)
    {
    print $Out $List_ref->[$i];
    ++$i;

    if ($i >= $len)
      {
      print $Out "\n";
      last;
      }

    print $Out ((($i % $Device_per_line) == 0) ? "\n" : ',');
    }
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

@regular = ();
@enhanced = ();
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

if (scalar(@regular) > 0)
  {
  print GR "\n    SECTION=REGULAR\n\n";
  print_listing(*GR, 10, \@regular);
  }

if (scalar(@enhanced) > 0)
  {
  print GR "\n    SECTION=ENHANCED\n\n";
  print_listing(*GR, 10, \@enhanced);
  }

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

BEGIN=PERIPHERY:ADC         # ADC --> adc.h.gen

  BEGIN=REGISTER
    VALID_REGS="AD\d*CON\d*|ADCOMCON"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="AD\d*RES\d*[HL]?"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-Z]*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="(FVR|REF)CON"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="DAC\d*"
    VALID_BITS=""
    PRINT_MODE=P_SHOW_ONLY_NAME
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="DAC(ON\d*|CON\d+)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="INTCON\d?"
    VALID_BITS="(G|PE|AD)IE"
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_REGS="ANSEL[A-Z]*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="TRIS([A-Z]|IO)"
    VALID_BITS="TRIS([A-Z]|IO)?\d"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

END=PERIPHERY

################################################################################
#
#       The CCP module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:CCP         # CCP --> ccp.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="(CCP(SEL\d|\dSEL)|P\d[A-D]SEL)"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSLOCK"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCP\d+PPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="R[A-Z]\dPPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCP(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCP(AS\d*|\d+AS)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCPR([HL]\d*|\d+[HL])"
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
    VALID_REGS="PWM(CON\d*|\d+CON)"
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
    VALID_BITS="(G|PE)IE"
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIE\d+"
    VALID_BITS="CCP\d+IE"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PIR\d+"
    VALID_BITS="CCP\d+IF"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ANSEL[A-Z]*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+"
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
#       The PWM(CCP) module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:PWM         # PWM --> pwm.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="(CCP(SEL\d|\dSEL)|P\d[A-D]SEL)"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSLOCK"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCP\d+PPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="R[A-Z]\dPPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCP(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCP(AS\d*|\d+AS)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="CCPR([HL]\d*|\d+[HL])"
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
    VALID_REGS="PWM(CON\d*|\d+CON)"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PWM(DC[HL]\d*|\d+DC[HL])"
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
    VALID_BITS="(G|PE)IE"
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_REGS="ANSEL[A-Z]*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+"
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
    VALID_REGS="PPSLOCK"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP(CLK|DAT)PPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="R[A-Z]\dPPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_BITS="(G|PE)IE"
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_REGS="ANSEL[A-Z]*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+"
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
    VALID_REGS="PPSLOCK"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="SSP\w+PPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="R[A-Z]\dPPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_BITS="(G|PE)IE"
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_REGS="ANSEL[A-Z]*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+"
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
#       The USART module related registers.
#       (There is so, which only indirectly connected to the module.)
#

BEGIN=PERIPHERY:USART       # USART --> usart.h.gen

  BEGIN=REGISTER
    VALID_REGS="APFCON\d*"
    VALID_BITS="(RX(DT)?|TX(CK)?)SEL"
    PRINT_MODE=P_NO_SHOW_IF_EMPTY
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="PPSLOCK"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="R[A-Z]\dPPS"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_BITS="(G|PE)IE"
    PRINT_MODE=P_ALWAYS_SHOW
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
    VALID_REGS="ANSEL[A-Z]*"
    VALID_BITS=""
    PRINT_MODE=P_ALWAYS_SHOW
  END=REGISTER

  BEGIN=REGISTER
    VALID_REGS="ADCON\d+"
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
#       This table describes that which onto port pins connected a peripheral.
#

BEGIN=IO_TABLE

  BEGIN=MCU:10f320,10f322
    ADC=AN0:RA0,AN1:RA1,AN2:RA2
    PWM=PWM1:RA0,PWM2:RA1
  END=MCU

  BEGIN=MCU:12f615,12f617,12f683
    ADC=AN0:GP0,AN1:GP1,AN2:GP2,AN3:GP4
    CCP=CCP:GP2
    PWM=PWM:GP2
  END=MCU

  BEGIN=MCU:12f675
    ADC=AN0:GP0,AN1:GP1,AN2:GP2,AN3:GP4
  END=MCU

  BEGIN=MCU:12f752
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4
    CCP=CCP:RA2
    PWM=PWM:RA2
  END=MCU

  BEGIN=MCU:12f1501
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4
    PWM=PWM1:RA2,PWM2:RA0,PWM3:RA4,PWM4:RA5
  END=MCU

  BEGIN=MCU:12f1571
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4
    PWM=PWM1:RA1/RA5,PWM2:RA0/RA4,PWM3:RA2
  END=MCU

  BEGIN=MCU:12f1572
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4
    PWM=PWM1:RA1/RA5,PWM2:RA0/RA4,PWM3:RA2
    USART=RX:RA1/RA5,TX:RA0/RA4
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:12f1612
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4
    CCP=CCP1:RA2/RA5,CCP2:RA0
    PWM=PWM1:RA2/RA5,PWM2:RA0
  END=MCU

  BEGIN=MCU:12f1822
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4
    CCP=CCP:RA2/RA5
    PWM=PWM:RA2/RA5
    I2C=SDA:RA2,SCL:RA1
    SPI=SDI:RA2,SDO:RA0,SCK:RA1,SS:RA0/RA3
    USART=RX:RA1/RA5,TX:RA0/RA4
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:12lf1552
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RA5
    I2C=SDA:RA2/RA3,SCL:RA1
    SPI=SDI:RA2/RA3,SDO:RA0/RA4,SCK:RA1,SS:RA0/RA3
  END=MCU

  BEGIN=MCU:16c62
    CCP=CCP:RC2
    PWM=PWM:RC2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
  END=MCU

  BEGIN=MCU:16c63a,16c65b
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16c71,16c710,16c711,16c715
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3
  END=MCU

  BEGIN=MCU:16c72,16f72
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP:RC2
    PWM=PWM:RC2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
  END=MCU

  BEGIN=MCU:16c73b,16c74b,16f73,16f76
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16c433
    ADC=AN0:GP0,AN1:GP1,AN2:GP2,AN3:GP4
  END=MCU

  BEGIN=MCU:16c717,16c770,16c771
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RB0,AN5:RB1
    CCP=CCP:RB3
    PWM=PWM:RB3
    I2C=SDA:RB4,SCL:RB2
    SPI=SDI:RB4,SDO:RB5,SCK:RB2,SS:RB1
  END=MCU

  BEGIN=MCU:16c745,16c765
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16c773
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN8:RB3,AN9:RB3
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RB1
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16c774
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB3,AN9:RB3
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RB1
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16c781,16c782
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RB0,AN5:RB1,AN6:RB2,AN7:RB3
  END=MCU

  BEGIN=MCU:16c925,16c926
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP:RC2
    PWM=PWM:RC2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
  END=MCU

  BEGIN=MCU:16f74,16f77
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f87
    CCP=CCP:RB0/RB3
    PWM=PWM:RB0/RB3
    I2C=SDA:RB1,SCL:RB4
    SPI=SDI:RB1,SDO:RB2,SCK:RB4,SS:RB5
    USART=RX:RB2,TX:RB5
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f88
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA4,AN5:RB6,AN6:RB7
    CCP=CCP:RB0/RB3
    PWM=PWM:RB0/RB3
    I2C=SDA:RB1,SCL:RB4
    SPI=SDI:RB1,SDO:RB2,SCK:RB4,SS:RB5
    USART=RX:RB2,TX:RB5
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f616,16hv616,16f684
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    CCP=CCP:RC5
    PWM=PWM:RC5
  END=MCU

  BEGIN=MCU:16f627,16f627a,16f628,16f628a,16f648a
    CCP=CCP:RB3
    PWM=PWM:RB3
    USART=RX:RB1,TX:RB2
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f676
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
  END=MCU

  BEGIN=MCU:16f677
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RC6
  END=MCU

  BEGIN=MCU:16f685
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP:RC5
    PWM=PWM:RC5
  END=MCU

  BEGIN=MCU:16f688
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    USART=RX:RC5,TX:RC4
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f687,16f689,16f690
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP:RC5
    PWM=PWM:RC5
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RC6
    USART=RX:RB5,TX:RB7
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16f707
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f716
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3
    CCP=CCP:RB3
    PWM=PWM:RB3
  END=MCU

  BEGIN=MCU:16f720,16f721
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP:RC5
    PWM=PWM:RC5
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RC6
    USART=RX:RB5,TX:RB7
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f722,16f722a,16f723,16f723a,16f726
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f724,16f727
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f737,16f767
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3,CCP3:RB5
    PWM=PWM1:RC2,PWM2:RC1/RB3,PWM3:RB5
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16f747,16f777
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3,CCP3:RB5
    PWM=PWM1:RC2,PWM2:RC1/RB3,PWM3:RB5
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16f753,16hv753
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    CCP=CCP:RC5
    PWM=PWM:RC5
  END=MCU

  BEGIN=MCU:16f785
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP:RC5
    PWM=PWM:RC5
  END=MCU

  BEGIN=MCU:16f818,16f819
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA4
    CCP=CCP:RB3
    PWM=PWM:RB3
    I2C=SDA:RB1,SCL:RB4
    SPI=SDI:RB1,SDO:RB2,SCK:RB4,SS:RB5
  END=MCU

  BEGIN=MCU:16f870
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP:RC2
    PWM=PWM:RC2
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16f871
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    CCP=CCP:RC2
    PWM=PWM:RC2
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16f872
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP:RC2
    PWM=PWM:RC2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
  END=MCU

  BEGIN=MCU:16f873,16f873a,16f876,16f876a
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16f874,16f874a,16f877,16f877a
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16f882,16f883,16f886
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f884,16f887
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1
    PWM=PWM1:RC2,PWM2:RC1
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f913,16f916
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3
    CCP=CCP:RC5
    PWM=PWM:RC5
    I2C=SDA:RC7,SCL:RC6
    SPI=SDI:RC7,SDO:RC4,SCK:RC6,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f914,16f917,16f946
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2
    CCP=CCP1:RC5,CCP2:RD2
    PWM=PWM1:RC5,PWM2:RD2
    I2C=SDA:RC7,SCL:RC6
    SPI=SDI:RC7,SDO:RC4,SCK:RC6,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1454
    PWM=PWM1:RC5,PWM2:RA5/RC3
    I2C=SDA:RC1,SCL:RC0
    SPI=SDI:RC1,SDO:RA4/RC2,SCK:RC0,SS:RA3/RC3
    USART=RX:RC5,TX:RC4
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1455
    ADC=AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    PWM=PWM1:RC5,PWM2:RA5/RC3
    I2C=SDA:RC1,SCL:RC0
    SPI=SDI:RC1,SDO:RA4/RC2,SCK:RC0,SS:RA3/RC3
    USART=RX:RC5,TX:RC4
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1459
    ADC=AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    PWM=PWM1:RC5,PWM2:RC6
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RA3/RC6
    USART=RX:RB5,TX:RB7
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1503
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    PWM=PWM1:RC5,PWM2:RC3,PWM3:RA2,PWM4:RC1
    I2C=SDA:RC1,SCL:RC0
    SPI=SDI:RC1,SDO:RA4/RC2,SCK:RC0,SS:RA3/RC3
  END=MCU

  BEGIN=MCU:16f1507
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    PWM=PWM1:RC5,PWM2:RC3,PWM3:RA2,PWM4:RC1
  END=MCU

  BEGIN=MCU:16f1508,16f1509
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    PWM=PWM1:RC5,PWM2:RC3,PWM3:RA2,PWM4:RC1
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RA3/RC6
    USART=RX:RB5,TX:RB7
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1512,16f1513
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1516,16f1518
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5,AN14:RC2,AN15:RC3,AN16:RC4,AN17:RC5,AN18:RC6,AN19:RC7
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1517,16f1519
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5,AN14:RC2,AN15:RC3,AN16:RC4,AN17:RC5,AN18:RC6,AN19:RC7,AN20:RD0,AN21:RD1,AN22:RD2,AN23:RD3,AN24:RD4,AN25:RD5,AN26:RD6,AN27:RD7
    CCP=CCP1:RC2,CCP2:RC1/RB3
    PWM=PWM1:RC2,PWM2:RC1/RB3
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1526,16f1527
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6,AN12:RG4,AN13:RG3,AN14:RG2,AN15:RG1,AN16:RF0,AN17:RB0,AN18:RB1,AN19:RB2,AN20:RB3,AN21:RB4,AN22:RB5,AN23:RD0,AN24:RD1,AN25:RD2,AN26:RD3,AN27:RE0,AN28:RE1,AN29:RE2
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4,CCP6:RE6,CCP7:RE5,CCP8:RE4,CCP9:RE3,CCP10:RE2
    CCP=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4,PWM6:RE6,PWM7:RE5,PWM8:RE4,PWM9:RE3,PWM10:RE2
    I2C=SDA1:RC4,SCL1:RC3,SDA2:RD5,SCL2:RD6
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7,SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6,RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:16lf1554,16lf1559
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN10:RA4,AN11:RC4,AN12:RC2,AN13:RC0,AN20:RA5,AN21:RC5,AN22:RC3,AN23:RC1
    PWM=PWM1:RC2,PWM2:RC3
    I2C=SDA:RC1/RA3,SCL:RC0
    SPI=SDI:RC1/RA3,SDO:RC2/RA4,SCK:RC0,SS:RC3/RA3
    USART=RX:RC5/RA4,TX:RC4/RC3
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1613
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    CCP=CCP1:RC5,CCP2:RA5/RC3
    PWM=PWM1:RC5,PWM2:RA5/RC3
  END=MCU

#
# Remappable peripheral pins. (PPS)
# (The definition valid until a newer definition overwrites the members.)
#
  BEGIN=DEFINE
    RI0=RA0
    RI1=RA1
    RI2=RA2
    RI3=RA3
    RI4=RA4
    RI5=RA5
    RI6=RC0
    RI7=RC1
    RI8=RC2
    RI9=RC3
    RI10=RC4
    RI11=RC5

    RO0=RA0
    RO1=RA1
    RO2=RA2
    RO3=RA4
    RO4=RA5
    RO5=RC0
    RO6=RC1
    RO7=RC2
    RO8=RC3
    RO9=RC4
    RO10=RC5
  END=DEFINE

  BEGIN=MCU:16f1703
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    CCP=CCP1:RI[0-11],CCP2:RI[0-11]
    PWM=PWM1:RO[0-10],PWM2:RO[0-10]
    I2C=SDA:RO[0-10],SCL:RO[0-10]
    SPI=SDI:RI[0-11],SDO:RO[0-10],SCK:RO[0-10],SS:RI[0-11]
  END=MCU

  BEGIN=MCU:16f1704,16lf1704,16f1705
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    CCP=CCP1:RI[0-11],CCP2:RI[0-11]
    PWM=PWM1:RO[0-10],PWM2:RO[0-10]
    I2C=SDA:RO[0-10],SCL:RO[0-10]
    SPI=SDI:RI[0-11],SDO:RO[0-10],SCK:RO[0-10],SS:RI[0-11]
    USART=RX:RI[0-11],TX:RO[0-10]
    USART=IO_DIR=RX:1,TX:1
  END=MCU

#
# Remappable peripheral pins. (PPS)
# (The definition valid until a newer definition overwrites the members.)
#
  BEGIN=DEFINE
    RI0=RA0
    RI1=RA1
    RI2=RA2
    RI3=RA3
    RI4=RA4
    RI5=RA5
    RI6=RB4
    RI7=RB5
    RI8=RB6
    RI9=RB7
    RI10=RC0
    RI11=RC1
    RI12=RC2
    RI13=RC3
    RI14=RC4
    RI15=RC5
    RI16=RC6
    RI17=RC7
  END=DEFINE

  BEGIN=MCU:16f1707
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP1:RI[0-17],CCP2:RI[0-17]
    PWM=PWM1:RO[0-10],PWM2:RO[0-10]
    I2C=SDA:RO[0-10],SCL:RO[0-10]
    SPI=SDI:RI[0-17],SDO:RO[0-10],SCK:RO[0-10],SS:RI[0-17]
  END=MCU

  BEGIN=MCU:16f1708,16lf1708,16f1709
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP1:RI[0-17],CCP2:RI[0-17]
    PWM=PWM1:RO[0-10],PWM2:RO[0-10]
    I2C=SDA:RO[0-10],SCL:RO[0-10]
    SPI=SDI:RI[0-17],SDO:RO[0-10],SCK:RO[0-10],SS:RI[0-17]
    USART=RX:RI[0-17],TX:RO[0-10]
    USART=IO_DIR=RX:1,TX:1
  END=MCU

#
# Remappable peripheral pins. (PPS)
# (The definition valid until a newer definition overwrites the members.)
#
  BEGIN=DEFINE
    RI0=RB0
    RI1=RB1
    RI2=RB2
    RI3=RB3
    RI4=RB4
    RI5=RB5
    RI6=RB6
    RI7=RB7
    RI8=RC0
    RI9=RC1
    RI10=RC2
    RI11=RC3
    RI12=RC4
    RI13=RC5
    RI14=RC6
    RI15=RC7

    RO0=RB0
    RO1=RB1
    RO2=RB2
    RO3=RB3
    RO4=RB4
    RO5=RB5
    RO6=RB6
    RO7=RB7
    RO8=RC0
    RO9=RC1
    RO10=RC2
    RO11=RC3
    RO12=RC4
    RO13=RC5
    RO14=RC6
    RO15=RC7
  END=DEFINE

  BEGIN=MCU:16f1713,16f1716,16f1718
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5,AN14:RC2,AN15:RC3,AN16:RC4:AN17:RC5,AN18:RC6,AN19:RC7
    CCP=CCP1:RI[0-15],CCP2:RI[0-15]
    PWM=PWM1:RO[0-15],PWM2:RO[0-15]
    I2C=SDA:RO[0-15],SCL:RO[0-15]
    SPI=SDI:RI[0-15],SDO:RO[8-15],SCK:RO[0-15],SS:RI[8-15]
    USART=RX:RI[0-15],TX:RO[0-15]
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1717,16f1719
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5,AN14:RC2,AN15:RC3,AN16:RC4:AN17:RC5,AN18:RC6,AN19:RC7,AN20:RD0,AN21:RD1,AN22:RD2,AN23:RD3,AN24:RD4,AN25:RD5,AN26:RD6,AN27:RD7
    CCP=CCP1:RI[0-15],CCP2:RI[0-15]
    PWM=PWM1:RO[0-15],PWM2:RO[0-15]
    I2C=SDA:RO[0-15],SCL:RO[0-15]
    SPI=SDI:RI[0-15],SDO:RO[8-15],SCK:RO[0-15],SS:RI[8-15]
    USART=RX:RI[0-15],TX:RO[0-15]
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1782,16f1783
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2/RB0,CCP2:RC1/RB3
    PWM=PWM1:RC2/RB0,PWM2:RC1/RB3
    I2C=SDA:RC4/RB6,SCL:RC3/RB7
    SPI=SDI:RC4/RB6,SDO:RC5/RB5,SCK:RC3/RB7,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1786
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2/RB0,CCP2:RC1/RB3,CCP3:RC6/RB5
    PWM=PWM1:RC2/RB0,PWM2:RC1/RB3,PWM3:RC6/RB5
    I2C=SDA:RC4/RB6,SCL:RC3/RB7
    SPI=SDI:RC4/RB6,SDO:RC5/RB5,SCK:RC3/RB7,SS:RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1784,16f1787
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5,AN21:RD1
    CCP=CCP1:RC2/RB0,CCP2:RC1/RB3,CCP3:RE0/RB5
    PWM=PWM1:RC2/RB0,PWM2:RC1/RB3,PWM3:RE0/RB5
    I2C=SDA:RC4/RB6,SCL:RC3/RB7
    SPI=SDI:RC4/RB6,SDO:RC5/RB5,SCK:RC3/RB7,SS:RA5
    USART=RX:RC7/RB7,TX:RC6/RB6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1788
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2/RB0,CCP2:RC1/RB3,CCP3:RC6/RB5
    PWM=PWM1:RC2/RB0,PWM2:RC1/RB3,PWM3:RC6/RB5
    I2C=SDA:RC4/RB6,SCL:RC3/RB7
    SPI=SDI:RC4/RB6,SDO:RC5/RB5,SCK:RC3/RB7,SS:RA0/RA5/RB4
    USART=RX:RC7/RB7,TX:RC6/RB6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1789
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5,AN21:RD1
    CCP=CCP1:RC2/RB0,CCP2:RC1/RB3,CCP3:RE0/RB5
    PWM=PWM1:RC2/RB0,PWM2:RC1/RB3,PWM3:RE0/RB5
    I2C=SDA:RC4/RB6,SCL:RC3/RB7
    SPI=SDI:RC4/RB6,SDO:RC5/RB5,SCK:RC3/RB7,SS:RA0/RA5/RB4
    USART=RX:RC7/RB7,TX:RC6/RB6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1823
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    CCP=CCP:RC5
    PWM=PWM:RC5
    I2C=SDA:RC1,SCL:RC0
    SPI=SDI:RC1,SDO:RA4/RC2,SCK:RC0,SS:RA3/RC3
    USART=RX:RC5/RA1,TX:RC4/RA0
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1824,16f1825
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3
    CCP=CCP1:RC5,CCP2:RC3/RA5,CCP3:RA2,CCP4:RC1
    PWM=PWM1:RC5,PWM2:RC3/RA5,PWM3:RA2,PWM4:RC1
    I2C=SDA:RC1,SCL:RC0
    SPI=SDI:RC1,SDO:RA4/RC2,SCK:RC0,SS:RA3/RC3
    USART=RX:RC5/RA1,TX:RC4/RA0
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1826
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA4,AN5:RB6,AN6:RB7,AN7:RB5,AN8:RB4,AN9:RB3,AN10:RB2,AN11:RB1
    CCP=CCP:RB3/RB0
    PWM=PWM:RB3/RB0
    I2C=SDA:RB1,SCL:RB4
    SPI=SDI:RB1,SDO:RB2/RA6,SCK:RB4,SS:RB5/RA5
    USART=RX:RB1/RB2,TX:RB2/RB5
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1827,16f1847
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA4,AN5:RB6,AN6:RB7,AN7:RB5,AN8:RB4,AN9:RB3,AN10:RB2,AN11:RB1
    CCP=CCP1:RB3/RB0,CCP2:RB6/RA7,CCP3:RA3,CCP4:RA4
    PWM=PWM1:RB3/RB0,PWM2:RB6/RA7,PWM3:RA3,PWM4:RA4
    I2C=SDA1:RB1,SCL1:RB4,SDA2:RB2,SCL2:RB5
    SPI=SDI1:RB1,SDO1:RB2/RA6,SCK1:RB4,SS1:RB5/RA5,SDI2:RB2,SDO2:RA0,SCK2:RB5,SS2:RA1
    USART=RX:RB1/RB2,TX:RB2/RB5
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1828
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP1:RC5,CCP2:RC3/RA5,CCP3:RA2,CCP4:RC6
    PWM=PWM1:RC5,PWM2:RC3/RA5,PWM3:RA2,PWM4:RC6
    I2C=SDA:RB4,SCL:RB6
    SPI=SDI:RB4,SDO:RC7,SCK:RB6,SS:RC6
    USART=RX:RC5/RB5,TX:RC4/RB7
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1829
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA4,AN4:RC0,AN5:RC1,AN6:RC2,AN7:RC3,AN8:RC6,AN9:RC7,AN10:RB4,AN11:RB5
    CCP=CCP1:RC5,CCP2:RC3/RA5,CCP3:RA2,CCP4:RC6
    PWM=PWM1:RC5,PWM2:RC3/RA5,PWM3:RA2,PWM4:RC6
    I2C=SDA1:RB4,SCL1:RB6,SDA2:RB5,SCL2:RB7
    SPI=SDI1:RB4,SDO1:RC7,SCK1:RB6,SS1:RC6,SDI2:RB5,SDO2:RC1/RA5,SCK2:RB7,SS2:RC0/RA4
    USART=RX:RC5/RB5,TX:RC4/RB7
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1933,16f1936,16f1938
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3,CCP3:RC6/RB5,CCP4:RB0,CCP5:RA4
    PWM=PWM1:RC2,PWM2:RC1/RB3,PWM3:RC6/RB5,PWM4:RB0,PWM5:RA4
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1934,16f1937,16f1939
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    CCP=CCP1:RC2,CCP2:RC1/RB3,CCP3:RE0/RB5,CCP4:RD1,CCP5:RE2
    PWM=PWM1:RC2,PWM2:RC1/RB3,PWM3:RE0/RB5,PWM4:RD1,PWM5:RE2
    I2C=SDA:RC4,SCL:RC3
    SPI=SDI:RC4,SDO:RC5,SCK:RC3,SS:RA0/RA5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:1
  END=MCU

  BEGIN=MCU:16f1946,16f1947
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RF7,AN6:RF1,AN7:RF2,AN8:RF3,AN9:RF4,AN10:RF5,AN11:RF6,AN12:RG4,AN13:RG3,AN14:RG2,AN15:RG1,AN16:RF0
    CCP=CCP1:RC2,CCP2:RC1/RE7,CCP3:RG0,CCP4:RG3,CCP5:RG4
    PWM=PWM1:RC2,PWM2:RC1/RE7,PWM3:RG0,PWM4:RG3,PWM5:RG4
    I2C=SDA1:RC4,SCL1:RC3,SDA2:RD5,SCL2:RD6
    SPI=SDI1:RC4,SDO1:RC5,SCK1:RC3,SS1:RF7,SDI2:RD5,SDO2:RD4,SCK2:RD6,SS2:RD7
    USART=RX1:RC7,TX1:RC6,RX2:RG2,TX2:RG1
    USART=IO_DIR=RX1:1,TX1:0,RX2:1,TX2:0
  END=MCU

  BEGIN=MCU:16lf1902,16lf1903
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
  END=MCU

  BEGIN=MCU:16lf1906
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

  BEGIN=MCU:16lf1904,16lf1907
    ADC=AN0:RA0,AN1:RA1,AN2:RA2,AN3:RA3,AN4:RA5,AN5:RE0,AN6:RE1,AN7:RE2,AN8:RB2,AN9:RB3,AN10:RB1,AN11:RB4,AN12:RB0,AN13:RB5
    USART=RX:RC7,TX:RC6
    USART=IO_DIR=RX:1,TX:0
  END=MCU

END=IO_TABLE
