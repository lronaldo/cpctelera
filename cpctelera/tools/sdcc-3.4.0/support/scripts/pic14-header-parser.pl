#!/usr/bin/perl -w

=back

  Copyright (C) 2012 Molnar Karoly <molnarkaroly@users.sf.net>

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

    The program creates five files in the actual directory:

        adc.tables
        ccp.tables
        pwm.tables
        ssp.tables
        usart.tables

    In these the MCUs can be seen in groups, according to a periphery.
    These informations helps to realize the handling of periphery.
    Of course necessary to study the data sheets as well.

    $Id: pic14-header-parser.pl 8233 2012-12-02 11:16:16Z molnarkaroly $
=cut

use strict;
use warnings;
use 5.10.1;
use feature 'switch';           # Starting from 5.10.1.
use POSIX qw(ULONG_MAX);

use constant FALSE => 0;
use constant TRUE  => 1;

use constant SECT_NONE     => 0;
use constant SECT_BEGIN    => 1;
use constant SECT_REGISTER => 2;

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
my $out_tail = 'tables';
my $table_border = (' ' x 17) . '+' . ('--------+' x 8);

my %reg_addresses = ();

my $state = SECT_NONE;

my @periphery_table = ();

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

#-----------------------------------------------

my $verbose = 0;
my $make_groups;
my $only_prime;
my $out_handler;
my $initial_border;

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

sub load_periphery_data()
  {
  my $periphery = undef;

  foreach (grep(! /^\s*$|^\s*#/o, <DATA>))
    {
    chomp;
    s/#.*$//o;                  # Remove ending comment.
    s/^\s*|\s*$//go;            # Remove starting and ending whitespaces.

    my $line = $_;

    if ($state == SECT_NONE)
      {
      if ($line =~ /^SECTION=TABLE_BEGIN:(\w+)$/o)
        {
        $periphery = add_periphery($1);
        $state = SECT_BEGIN;
        next;
        }
      }
    elsif ($state == SECT_BEGIN)
      {
      if ($line eq 'SECTION=REGISTER')
        {
        die "There is no periphery to which can be assigned the following register." if (! defined($periphery));

        add_reg_def($periphery);
        $state = SECT_REGISTER;
        next;
        }

      $state = SECT_NONE;
      }
    elsif ($state == SECT_REGISTER)
      {
      if ($line eq 'SECTION=REGISTER')
        {
        die "There is no periphery to which can be assigned the following register." if (! defined($periphery));

        add_reg_def($periphery);
        next;
        }
      elsif ($line eq 'SECTION=TABLE_END')
        {
        $state = SECT_NONE;
        $periphery = undef;
        next;
        }

      if ($line =~ /^([^:]+):(.*)$/o)
        {
        # This a key -- value pair.

        die "No entry of register table!" if (! scalar(@{$periphery}));

        my $key = $1;
        my $val = $2;
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
      } # elsif ($state == SECT_REGISTER)
    } # foreach (grep(! /^\s*$|^\s*#/o, <DATA>))
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
  my $mcu_ref = {
                NAME      => uc($_[0]),
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
  my ($fh, $mcu_name, $name, $addr, $bit_addr, $bitnames);

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

    Log(">>>>: $line", 6);

    if ($line =~ /^#include\s+"(\S+)"$/o)
      {
      &read_regs_from_header($Mcu_ref, $1);
      }
    elsif ($line =~ /^#\s*define\s+(\w+_ADDR)\s+0[xX]([[:xdigit:]]+)$/o)
      {
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

      Log("\tadd_register_raw($Mcu_ref, $name, hex($addr))", 7);
      add_register_raw($Mcu_ref, $name, hex($addr));
      }
    elsif ($line =~ /^extern\s+__sfr\s+__at\s*\((\w+_ADDR)\)\s+(\w+);$/o)
      {
        # extern __sfr  __at (EEDATA_ADDR)  EEDATA;
        #

      die "This register: $2 not have address!" if (! defined($reg_addresses{$1}));

      Log("\tadd_register_raw($Mcu_ref, $2, reg_addresses\{$1\})", 7);
      add_register_raw($Mcu_ref, $2, $reg_addresses{$1});
      }
    elsif ($line =~ /\bstruct\b/o)
      {
      Log("\tbit_addr = 0", 7);
      $bit_addr = 0;
      }
    elsif ($line =~ /^unsigned(?:\s+char|\s+int)?\s*:\s*(\d+)\s*;$/o)
      {
        # unsigned char :1;
        # unsigned int : 4;
        #

      Log("\tbit_addr += int($1)", 7);
      $bit_addr += int($1);
      }
    elsif ($line =~ /^unsigned(?:\s+char|\s+int)?\s*(\w+)\s*:\s*1\s*;$/o)
      {
        # unsigned char PCFG2:1;
        # unsigned int PSA:1;
        #

      Log("\tpush(bitnames->\[$bit_addr\], $1)", 7);
      push(@{$bitnames->[$bit_addr]}, $1);
      ++$bit_addr;
      }
    elsif ($line =~ /^\}\s*(?:__)?(\w+)bits_t\s*;$/o || $line =~ /^\}\s*(?:__)?(\w+)_t\s*;$/o)
      {
      my $reg_ref = $Mcu_ref->{REG_REFS}->{$1};

      if (! defined($reg_ref))
        {
        print STDERR "This register: $1 not have data structure!\n";
        exit(1);
        }

      Log("\treg_ref : $reg_ref)", 7);
      $reg_ref->{BITNAMES} = $bitnames;
      $bitnames = [];
      }
    } # foreach (grep(! /^\s*$/o, <$fh>))

  close($fh);

  my $array = \@{$Mcu_ref->{REG_ARRAY}};

  return if (! scalar(@{$array}));

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
  my $Mcu_raw = $_[1];

  my $mcu = {
            NAME       => $Mcu_raw->{NAME},
            ENHANCED   => $Mcu_raw->{ENHANCED},
            REG_REFS   => {},
            IN_GROUP   => FALSE,
            REG_GROUPS => []
            };

        # Copies the master periphery table.

  foreach (@{$_[0]})
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

  foreach (@{$Mcu->{REG_GROUPS}})
    {
    if ($name =~ /^$_->{VALID_REGS}$/)
      {
        # This register fits into this group.

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

sub bitfilter($$)
  {
  my ($Regname, $Bits) = @_;

  return if (! defined($Bits));

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

sub filter_regs_from_raw($)
  {
  my $Periphery = $_[0];

  foreach (@mcu_raw)
    {
    my $mcu_ref = add_mcu($Periphery, $_);

    foreach my $reg_raw (@{$_->{REG_ARRAY}})
      {
      my $reg_dst = add_register($mcu_ref, $reg_raw);

      next if (! defined($reg_dst));

      my $bits_src = $reg_raw->{BITNAMES};
      my $bits_dst = $reg_dst->{BITNAMES};
      my $valid_bits = $reg_dst->{GROUP}->{VALID_BITS};
      my $empty = TRUE;

      if (defined($valid_bits) && $valid_bits ne '')
        {
        # Filtering follows.

        for (my $i = 0; $i < 8; ++$i)
          {
          my $new_bits = [];

          bitfilter($reg_dst->{NAME}, \@{$bits_src->[$i]});
          foreach (@{$bits_src->[$i]})
            {
        # Only those names notes, which passed through the filter.

            push(@{$new_bits}, $_) if (defined($_) && $_ =~ /^$valid_bits$/);
            }

          if (@{$new_bits})
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
          bitfilter($reg_dst->{NAME}, \@{$bits_src->[$i]});
          $empty = FALSE if (defined($bits_src->[$i]));

          $bits_dst->[$i] = $bits_src->[$i];
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

  return TRUE  if (! defined(@{$Bits1}) && ! defined(@{$Bits2}));

  return FALSE if (! defined(@{$Bits1}) || ! defined(@{$Bits2}));

  foreach (@{$Bits1})
    {
    return TRUE if (/^$_$/ ~~ @{$Bits2});
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

sub find_equivalent_register($$$)
  {
  my $Prime_reg = $_[1];
  my $cand_reg;
  my $prime_bits;
  my $cand_bits;

  $cand_reg = find_register($_[0], $Prime_reg);

  return FALSE if (! defined($cand_reg));

  $cand_reg->{TOUCHED} = TRUE;

        # Not performs comparison, if the bits not must be displayed.
  return TRUE if ($_[2] == P_SHOW_ONLY_NAME);

  $prime_bits = \@{$Prime_reg->{BITNAMES}};
  $cand_bits  = \@{$cand_reg->{BITNAMES}};

  for (my $i = 0; $i < 8; ++$i)
    {
    return FALSE if (! find_equivalent_bit(\@{$cand_bits->[$i]}, \@{$prime_bits->[$i]}));
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

  return FALSE if (find_not_touched_reg($Prime));
  return FALSE if (find_not_touched_reg($Candidate));

  return TRUE;
  }

#-------------------------------------------------------------------------------

sub make_mcu_groups()
  {
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

      if (find_equivalent_mcu($prime, $_))
        {
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

sub dump_registers($$)
  {
  my $Print_mode = $_[1];
  my $i;
  my $bits;
  my $show_closing_border = FALSE;

  foreach (@{$_[0]})
    {
    if ($_->{EMPTY} && $Print_mode == P_NO_SHOW_IF_EMPTY)
      {
        # Sole bit not have name and the empty register is not must show.
      next;
      }

    if ($Print_mode == P_SHOW_ONLY_NAME)
      {
      Outfl("%-8s (%03Xh)", $_->{NAME}, $_->{ADDRESS});
      $initial_border = FALSE;
      next;
      }

    if (! $initial_border)
      {
      Outl($table_border);
      $initial_border = TRUE;
      }

    Outf("%-8s (%03Xh)  |", $_->{NAME}, $_->{ADDRESS});

    $bits = \@{$_->{BITNAMES}};
    for ($i = 7; $i >= 0; --$i)
      {
      if (defined($bits->[$i]) && $Print_mode != P_NO_SHOW_BITS)
        {
        Outf("%-8s|", find_shortest_name(\@{$bits->[$i]}));
        }
      else
        {
        Out('        |');
        }
      }

    Outl();
    $show_closing_border = TRUE;
    } # foreach (@{$_[0]})

  Outl($table_border) if ($show_closing_border);
  }

#-------------------------------------------------------------------------------

sub dump_mcu($)
  {
  my $Mcu = $_[0];

  Outl("This is an enhanced 14 bit MCU.") if ($Mcu->{ENHANCED});

  $initial_border = FALSE;

  foreach (@{$Mcu->{REG_GROUPS}})
    {
    next if (! scalar(@{$_->{REG_ARRAY}}));

    dump_registers(\@{$_->{REG_ARRAY}}, $_->{PRINT_MODE});
    }
  }

#-------------------------------------------------------------------------------

sub dump_all_data()
  {
  if ($make_groups)
    {
    my $group_index = 1;
    my $border = '#' x 40;

    make_mcu_groups();

    foreach (@mcu_groups)
      {
      next if (! scalar(@{$_}));

      Outl("\n//$border ${group_index}th group $border");

      if ($only_prime)
        {
        Outl("\n/*");

        # Prints the name of the group members.

        foreach (@{$_})
          {
          Outl("PIC$_->{NAME}");
          }

        # Only contents of the first it shows, because content of the others same.

        dump_mcu($_->[0]);
        Outl('*/');
        }
      else
        {
        # Displays full contents of each member of the group.

        foreach (@{$_})
          {
          Outl("\n\n/*\nPIC$_->{NAME}");
          dump_mcu($_);
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
      dump_mcu($_);
      Outl('*/');
      }
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

  die "Can not find the directory of headers!" if ($include_path eq '');
  }

opendir(DIR, $include_path) || die "Can not open. -> \"$include_path\"";

print "Include path: \"$include_path\"\n";

my @filelist = grep(-f "$include_path/$_" && /^$header_name_filter$/, readdir(DIR));
closedir(DIR);

@mcu_raw = ();
foreach (sort {smartSort($a, $b)} @filelist)
  {
  my $name = $_;

  print STDERR "Reading the registers from $_ header ...";

  $name =~ s/^pic//io;
  $name =~ s/\.\S+$//o;
  read_regs_from_header(add_mcu_raw($name), $_);
  print STDERR " done.\n";
  }

foreach (@periphery_table)
  {
  my $out_name = "$_->{NAME}.$out_tail";

  open($out_handler, '>', $out_name) || die "Can not create the $out_name output!";

  print STDERR "Filtering of registers the aspects of $_->{NAME} according to ...";
  @mcu_filtered = ();
  @mcu_groups = ();
  filter_regs_from_raw($_->{REGS});
  print STDERR " done.\n";

  print STDERR "Creating the $out_name ...";
  dump_all_data();
  print STDERR " done.\n";

  close($out_handler);
  }

__END__
################################################################################
#
#   The following rules determine to which registers belong to an peripheral.
#   The description of a periphery is bounded by the SECTION=TABLE_XXX flags.
#   The description of a register begin after the SECTION=REGISTER flag.
#
#   SECTION=TABLE_BEGIN:ADC
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

SECTION=TABLE_BEGIN:ADC         # ADC --> adc.tables

  SECTION=REGISTER
    VALID_REGS:"ADCON\d+"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"ADRES[HL]?"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"ANSEL[A-Z]*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"(FVR|REF)CON"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"DAC\d*"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"DAC(ON\d*|CON\d+)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"INTCON\d?"
    VALID_BITS:"(G|PE|AD)IE"
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PIE\d+"
    VALID_BITS:"ADIE"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"PIR\d+"
    VALID_BITS:"ADIF"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"ANSEL[A-Z]*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"TRIS([A-Z]|IO)"
    VALID_BITS:"TRIS([A-Z]|IO)\d"
    PRINT_MODE:P_ALWAYS_SHOW

SECTION=TABLE_END

################################################################################
#
#       The CCP module related registers.
#       (There is so, which only indirectly connected to the module.)
#

SECTION=TABLE_BEGIN:CCP         # CCP --> ccp.tables

  SECTION=REGISTER
    VALID_REGS:"APFCON\d*"
    VALID_BITS:"(CCP(SEL\d|\dSEL)|P\d[A-D]SEL)"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"CCP(CON\d*|\d+CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"CCP(AS\d*|\d+AS)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"CCPR([HL]\d*|\d+[HL])"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"CCPTMRS\d*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PSTR(CON\d*|\d+CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PWM(CON\d*|\d+CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"(T[2468]CON|T10CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"(TMR[2468]|TMR10)"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"(PR[2468]|PR10)"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"INTCON\d?"
    VALID_BITS:"(G|PE)IE"
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PIE\d+"
    VALID_BITS:"CCP\d+IE"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"PIR\d+"
    VALID_BITS:"CCP\d+IF"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"ANSEL[A-Z]*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"ADCON\d+"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"TRIS([A-Z]|IO)"
    VALID_BITS:"TRIS([A-Z]|IO)\d"
    PRINT_MODE:P_ALWAYS_SHOW

SECTION=TABLE_END

################################################################################
#
#       The PWM(CCP) module related registers.
#       (There is so, which only indirectly connected to the module.)
#

SECTION=TABLE_BEGIN:PWM         # PWM --> pwm.tables

  SECTION=REGISTER
    VALID_REGS:"APFCON\d*"
    VALID_BITS:"(CCP(SEL\d|\dSEL)|P\d[A-D]SEL)"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"CCP(CON\d*|\d+CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"CCP(AS\d*|\d+AS)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"CCPR([HL]\d*|\d+[HL])"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"CCPTMRS\d*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PSTR(CON\d*|\d+CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PWM(CON\d*|\d+CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PWM(DC[HL]\d*|\d+DC[HL])"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"(T[2468]CON|T10CON)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"(TMR[2468]|TMR10)"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"(PR[2468]|PR10)"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"INTCON\d?"
    VALID_BITS:"(G|PE)IE"
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PIE\d+"
    VALID_BITS:"((CCP\d+|TMR[2468])IE|TMR10IE)"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"PIR\d+"
    VALID_BITS:"((CCP\d+|TMR[2468])IF|TMR10IF)"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"ANSEL[A-Z]*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"ADCON\d+"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"TRIS([A-Z]|IO)"
    VALID_BITS:"TRIS([A-Z]|IO)\d"
    PRINT_MODE:P_ALWAYS_SHOW

SECTION=TABLE_END

################################################################################
#
#       The SSP module related registers.
#       (There is so, which only indirectly connected to the module.)
#

SECTION=TABLE_BEGIN:SSP         # SSP --> ssp.tables

  SECTION=REGISTER
    VALID_REGS:"APFCON\d*"
    VALID_BITS:"S(DI|DO|CK|S)\d*SEL"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"SSP\d*CON\d*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"SSP\d*ADD\d*"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"SSP\d*BUF\d*"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"SSP\d*MSK\d*"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"SSP\d*STAT\d*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"INTCON\d?"
    VALID_BITS:"(G|PE)IE"
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PIE\d+"
    VALID_BITS:"(BCL|SSP)\d*IE"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"PIR\d+"
    VALID_BITS:"(BCL|SSP)\d*IF"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"ANSEL[A-Z]*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"ADCON\d+"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"TRIS([A-Z]|IO)"
    VALID_BITS:"TRIS([A-Z]|IO)\d"
    PRINT_MODE:P_ALWAYS_SHOW

SECTION=TABLE_END

################################################################################
#
#       The USART module related registers.
#       (There is so, which only indirectly connected to the module.)
#

SECTION=TABLE_BEGIN:USART       # USART --> usart.tables

  SECTION=REGISTER
    VALID_REGS:"APFCON\d*"
    VALID_BITS:"(RX(DT)?|TX(CK)?)SEL"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"RC(STA\d*|\d+STA)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"TX(STA\d*|\d+STA)"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"BAUD(C(ON|TL)\d*|\d+C(ON|TL))"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"SP(BRG[HL]?\d?|\d+BRG[HL]?)"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"RC(REG\d*|\d+REG)"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"TX(REG\d*|\d+REG)"
    VALID_BITS:""
    PRINT_MODE:P_SHOW_ONLY_NAME

  SECTION=REGISTER
    VALID_REGS:"INTCON\d?"
    VALID_BITS:"(G|PE)IE"
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"PIE\d+"
    VALID_BITS:"(RC|TX)\d*IE"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"PIR\d+"
    VALID_BITS:"(RC|TX)\d*IF"
    PRINT_MODE:P_NO_SHOW_IF_EMPTY

  SECTION=REGISTER
    VALID_REGS:"ANSEL[A-Z]*"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"ADCON\d+"
    VALID_BITS:""
    PRINT_MODE:P_ALWAYS_SHOW

  SECTION=REGISTER
    VALID_REGS:"TRIS([A-Z]|IO)"
    VALID_BITS:"TRIS([A-Z]|IO)\d"
    PRINT_MODE:P_ALWAYS_SHOW

SECTION=TABLE_END
