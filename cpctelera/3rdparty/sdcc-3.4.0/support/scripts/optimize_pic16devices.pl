#!/usr/bin/perl -w

=back

  Copyright (C) 2012 Molnar Karoly <molnarkaroly@users.sf.net>

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

================================================================================

    This program optimizes or unoptimizes the pic16devices.txt file.
    For more explanation: optimize_pic16devices.pl -h

  $Id: optimize_pic16devices.pl 8813 2013-08-24 23:04:13Z tecodev $

=cut

use strict;
use warnings;
use 5.12.0;                     # when (regex)

use constant FALSE => 0;
use constant TRUE  => 1;

my $PROGRAM = '';
my $verbose = 0;
my $file = '';

use constant OP_NULL       => 0;
use constant OP_OPTIMIZE   => 1;
use constant OP_UNOPTIMIZE => 2;

my $operation = OP_NULL;

my @devices_header = ();
my @device_names = ();

#-----------------------------------------------

=back
        The structure of one element of the %devices_by_name:

        {
        NAME   => '',
        RAM    => {
                  SIZE  => 0,
                  SPLIT => 0
                  },
        CONFIG => {
                  FIRST => 0,
                  LAST  => 0,
                  WORDS => [
                             {
                             ADDRESS  => 0,
                             MASK     => 0,
                             VALUE    => 0,
                             AND_MASK => 0
                             },
                             ...
                             {
                             }
                           ]
                  },
        ID     => {
                  FIRST => 0,
                  LAST  => 0,
                  WORDS => [
                             {
                             ADDRESS  => 0,
                             VALUE    => 0
                             },
                             ...
                             {
                             }
                           ]
                  },
        XINST  => 0
        }
=cut

my %devices_by_name = ();

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@                          @@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@  Some auxiliary function.  @@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@                          @@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

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

  return hex($1)   if ($Str =~ /^0x([[:xdigit:]]+)$/io);
  return oct($1)   if ($Str =~ /^(0[0-7]+)$/o);
  return int($Str) if ($Str =~ /^-?\d+$/o);

  die "This string not integer: \"$Str\"";
  }

#-------------------------------------------------------------------------------

sub Log
  {
  return if (pop(@_) > $verbose);
  foreach (@_) { print STDERR $_; }
  print STDERR "\n";
  }

#-------------------------------------------------------------------------------

sub Open($$)
  {
  my ($File, $Function) = @_;
  my $handle;

  open($handle, '<', $File) || die "${Function}(): Could not open the \"$File\" file.\n";
  return $handle;
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
#@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@   The important procedures.   @@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

sub copy_words($$)
  {
  my ($Dst, $Src) = @_;

  return if (! defined(@{$Src}));

  foreach (@{$Src})
    {
    push(@{$Dst}, { %{$_} });
    }
  }

#-------------------------------------------------------------------------------

        #
        # Compares the config $Words1 and the config $Words2.
        #

sub compare_config_words($$)
  {
  my ($Words1, $Words2) = @_;
  my $m = defined($Words1) + defined($Words2);
  my ($i, $v);

  return TRUE  if ($m == 0);
  return FALSE if ($m == 1 || @{$Words1} != @{$Words2});

  $v = @{$Words1};

  for ($i = 0; $i < $v; ++$i)
    {
    return FALSE if ($Words1->[$i]->{ADDRESS} != $Words2->[$i]->{ADDRESS});
    return FALSE if ($Words1->[$i]->{MASK}    != $Words2->[$i]->{MASK});
    return FALSE if ($Words1->[$i]->{VALUE}   != $Words2->[$i]->{VALUE});

    $m = defined($Words1->[$i]->{AND_MASK}) + defined($Words2->[$i]->{AND_MASK});

    next if ($m == 0);

    return FALSE if ($m == 1 || $Words1->[$i]->{AND_MASK} != $Words2->[$i]->{AND_MASK});
    }

  return TRUE;
  }

#-------------------------------------------------------------------------------

        #
        # Compares the id $Words1 and the id $Words2.
        #

sub compare_id_words($$)
  {
  my ($Words1, $Words2) = @_;
  my $m = defined($Words1) + defined($Words2);
  my ($i, $v);

  return TRUE  if ($m == 0);
  return FALSE if ($m == 1 || @{$Words1} != @{$Words2});

  $v = @{$Words1};

  for ($i = 0; $i < $v; ++$i)
    {
    return FALSE if ($Words1->[$i]->{ADDRESS} != $Words2->[$i]->{ADDRESS});
    return FALSE if ($Words1->[$i]->{VALUE}   != $Words2->[$i]->{VALUE});
    }

  return TRUE;
  }

#-------------------------------------------------------------------------------

        #
        # Compares the $Dev1 and the $Dev2.
        #

sub compare_devices($$)
  {
  my ($Dev1, $Dev2) = @_;
  my $m = defined($Dev1) + defined($Dev2);

  return FALSE if ($m < 2);

  return FALSE if ($Dev1->{RAM}->{SPLIT}    != $Dev2->{RAM}->{SPLIT});
  return FALSE if ($Dev1->{CONFIG}->{FIRST} != $Dev2->{CONFIG}->{FIRST});
  return FALSE if ($Dev1->{CONFIG}->{LAST}  != $Dev2->{CONFIG}->{LAST});
  return FALSE if (! compare_config_words(\@{$Dev1->{CONFIG}->{WORDS}}, \@{$Dev2->{CONFIG}->{WORDS}}));
  return FALSE if ($Dev1->{XINST}           != $Dev2->{XINST});

  $m = defined($Dev1->{ID}) + defined($Dev2->{ID});
  return TRUE  if ($m == 0);
  return FALSE if ($m == 1 || $Dev1->{ID}->{FIRST} != $Dev2->{ID}->{FIRST});
  return FALSE if ($Dev1->{ID}->{LAST} != $Dev2->{ID}->{LAST});
  return FALSE if (! compare_id_words(\@{$Dev1->{ID}->{WORDS}}, \@{$Dev2->{ID}->{WORDS}}));
  return TRUE;
  }

#-------------------------------------------------------------------------------

sub print_config_words($)
  {
  my $Words = $_[0];

  return if (! defined($Words));

  foreach (@{$Words})
    {
    printf "configword  0x%06X 0x%02X 0x%02X", $_->{ADDRESS}, $_->{MASK}, $_->{VALUE};
    printf " 0x%02X", $_->{AND_MASK} if (defined($_->{AND_MASK}));
    print  "\n";
    }
  }

#-------------------------------------------------------------------------------

sub print_id_words($)
  {
  my $Words = $_[0];

  return if (! defined($Words));

  foreach (@{$Words})
    {
    printf "idword      0x%06X 0x%02X\n", $_->{ADDRESS}, $_->{VALUE};
    }
  }

#-------------------------------------------------------------------------------

sub print_device($)
  {
  my $Index = $_[0];
  my $mcu = $device_names[$Index];
  my $dev = $devices_by_name{$mcu};

  return if (! defined($dev));

  Log("Prints the $mcu MCU.", 4);

  my $ancestor = undef;
  my ($ac, $i, $ref);

  if ($operation == OP_OPTIMIZE)
    {
        # Optinized writing is required.

    for ($i = 0; $i < @device_names; ++$i)
      {
      last if ($Index == $i);

      $ac = $devices_by_name{$device_names[$i]};

      if (compare_devices($ac, $dev))
        {
        $ancestor = $ac;
        last;
        }
      }
    }

  print "name        $dev->{NAME}\n";

  if ($dev->{COMMENTS})
    {
    foreach (@{$dev->{COMMENTS}})
      {
      print "$_\n";
      }
    }

  $ref = $dev->{RAM};

  if (defined($ancestor))
    {
    print  "using       $ancestor->{NAME}\n";
    print  "ramsize     $ref->{SIZE}\n" if ($ancestor->{RAM}->{SIZE} != $ref->{SIZE});
    }
  else
    {
    print  "ramsize     $ref->{SIZE}\n";
    printf "split       0x%02X\n", $ref->{SPLIT};

    $ref = $dev->{CONFIG};
    printf "configrange 0x%06X 0x%06X\n", $ref->{FIRST}, $ref->{LAST};
    print_config_words(\@{$ref->{WORDS}});

    if (defined($dev->{XINST}))
      {
      printf "XINST       %d\n", $dev->{XINST};
      }

    $ref = $dev->{ID};

    if (defined($ref))
      {
      printf "idlocrange  0x%06X 0x%06X\n", $ref->{FIRST}, $ref->{LAST};
      print_id_words(\@{$ref->{WORDS}}) if (defined($ref->{WORDS}));
      }
    }
  }

#-------------------------------------------------------------------------------

        #
        # Reads the entire pic16devices.txt file.
        #

sub read_pic16devices_txt($)
  {
  my $File = $_[0];
  my $in = Open($File, 'read_pic16devices_txt');
  my $header = TRUE;
  my $device = undef;
  my ($first, $last, $txt);

  Log("Reads the $File file.", 4);

  while (<$in>)
    {
    chomp;
    s/\r$//o;
    s/\s+$//o;

    $header = FALSE if ($_ =~ /^\s*name\b/io);

    if ($header)
      {
      push(@devices_header, $_);
      next;
      }

    given ($_)
      {
      when (/^\s*name\s+(\w+)$/io)
        {
        $device = {
                  NAME     => $1,
                  COMMENTS => undef,
                  RAM      => {},
                  CONFIG   => {},
                  ID       => undef,
                  XINST    => undef,
                  };

        Log("name       : $1", 7);
        $devices_by_name{$1} = $device;
        }

      when (/^\s*using\s+(\w+)$/io)
        {
        die "Device not exists." if (! defined($device));

        my $parent = $devices_by_name{$1};

        die "In device - \"$device->{NAME}\" - not exists the parent: \"$1\"\n" if (! defined($parent));

        # Unlock the "using" keyword.

        Log("using      : $1", 7);

        %{$device->{RAM}} = %{$parent->{RAM}};

        $device->{CONFIG}->{FIRST} = $parent->{CONFIG}->{FIRST};
        $device->{CONFIG}->{LAST}  = $parent->{CONFIG}->{LAST};
        copy_words(\@{$device->{CONFIG}->{WORDS}}, \@{$parent->{CONFIG}->{WORDS}});
        die "XINST overwritten for $device->{NAME}." if (defined($device->{XINST}) && $device->{XINST} != $parent->{XINST});
        printf "XINST reset %d -> %d for %s\n", $device->{XINST}, $parent->{XINST}, $device->{NAME} if defined($device->{XINST});
        $device->{XINST}           = $parent->{XINST};

        if (defined($parent->{ID}))
          {
          $device->{ID}->{FIRST} = $parent->{ID}->{FIRST};
          $device->{ID}->{LAST}  = $parent->{ID}->{LAST};
          copy_words(\@{$device->{ID}->{WORDS}}, \@{$parent->{ID}->{WORDS}});
          }
        }

      when (/^\s*ramsize\s+(\w+)$/io)
        {
        die "Device not exists." if (! defined($device));

        Log("ramsize    : $1", 7);
        $device->{RAM}->{SIZE} = str2int($1);
        }

      when (/^\s*split\s+(\w+)$/io)
        {
        die "Device not exists." if (! defined($device));

        Log("split      : $1", 7);
        $device->{RAM}->{SPLIT} = str2int($1);
        }

      when (/^\s*configrange\s+(\w+)\s+(\w+)$/io)
        {
        die "Device not exists." if (! defined($device));

        ($first, $last) = (str2int($1), str2int($2));
        Log("configrange: $1 $2", 7);

        if (defined($device->{CONFIG}->{FIRST}))
          {
          Log("The configrange already exists in the \"$device->{NAME}\".", 0);

          if ($device->{CONFIG}->{FIRST} != $first || $device->{CONFIG}->{LAST} != $last)
            {
            Log("  In addition the previous values different from the new values.", 0);
            Log(sprintf("  previous: 0x%06X - 0x%06X, new: 0x%06X - 0x%06X",
                         $device->{CONFIG}->{FIRST}, $device->{CONFIG}->{LAST},
                         $first, $last), 0);
            }

        # The previous values invalid.

          $device->{CONFIG}->{WORDS} = [];
          }

        $device->{CONFIG}->{FIRST} = $first;
        $device->{CONFIG}->{LAST}  = $last;
        }

      when (/^\s*configword\s+(\w+)\s+(\w+)\s+(\w+)(?:\s+(\w+))?$/io)
        {
        die "Device not exists." if (! defined($device));

        if (defined($4))
          {
          Log("configword : $1 $2 $3 $4", 7);
          push(@{$device->{CONFIG}->{WORDS}}, {
                                              ADDRESS  => str2int($1),
                                              MASK     => str2int($2),
                                              VALUE    => str2int($3),
                                              AND_MASK => str2int($4)
                                              });
          }
        else
          {
          Log("configword : $1 $2 $3", 7);
          push(@{$device->{CONFIG}->{WORDS}}, {
                                              ADDRESS  => str2int($1),
                                              MASK     => str2int($2),
                                              VALUE    => str2int($3)
                                              });
          }
        }

      when (/^\s*XINST\s+(\w+)\s*$/io)
        {
          die "Device not exists." if (! defined($device));

          Log("XINST    : $1", 7);
          printf "XINST $device->{XINST} -> $1 for $device->{NAME}.\n" if (defined ($device->{XINST}));
          $device->{XINST} = str2int($1);
        }

      when (/^\s*idlocrange\s+(\w+)\s+(\w+)$/io)
        {
        die "Device not exists." if (! defined($device));

        Log("idlocrange : $1 $2", 7);
        $device->{ID}->{FIRST} = str2int($1);
        $device->{ID}->{LAST}  = str2int($2);
        }

      when (/^\s*idword\s+(\w+)\s+(\w+)$/io)
        {
        die "Device not exists." if (! defined($device));

        Log("idword     : $1 $2", 7);
        push(@{$device->{ID}->{WORDS}}, { 
                                        ADDRESS => str2int($1),
                                        VALUE   => str2int($2)
                                        });
        }

      when (/^\s*#/o)
        {
        die "Device not exists." if (! defined($device));

        Log("comment    : \"$_\"", 7);
        push(@{$device->{COMMENTS}}, $_);
        }

      default
        {
        Log("unrecognized line: \"$_\"", 7);
        }
      } # given ($_)
    }

  close($in);
  }

#-------------------------------------------------------------------------------

sub usage()
  {
  print <<EOT
Usage: $PROGRAM <option> path/to/pic16devices.txt > output.txt

    Options are:

        -o or --optimize

            If a MCU features matches with an earlier listed MCU features,
            then use the "using" keyword and with this method significantly
            reduces the file size.

        -u or --unoptimize

            Unlocks the "using" keywords and instead displays the original
            content.

        -v <level> or --verbose <level>

            It provides information on from the own operation.
            Possible value of the level between 0 and 10. (default: 0)

        -h or --help

            This text.
EOT
;
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@                     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@   The main program.   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@                     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

$PROGRAM = basename($0);

for (my $i = 0; $i < @ARGV; )
  {
  my $opt = $ARGV[$i++];

  given ($opt)
    {
    when (/^-(o|-optimize)$/o)   { $operation = OP_OPTIMIZE; }

    when (/^-(u|-unoptimize)$/o) { $operation = OP_UNOPTIMIZE; }

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
      $file = $opt;
      }
    } # given ($opt)
  } # for (my $i = 0; $i < @ARGV; )

if ($file eq '' || $operation == OP_NULL)
  {
  usage();
  exit(0);
  }

(-f $file) || die "This file - \"$file\" - not exist!";

read_pic16devices_txt($file);

@device_names = sort {smartSort($a, $b)} keys(%devices_by_name);

foreach (@device_names)
  {
  my $dev = $devices_by_name{$_};

  @{$dev->{CONFIG}->{WORDS}} = sort {$a->{ADDRESS} <=> $b->{ADDRESS}} @{$dev->{CONFIG}->{WORDS}};

  if (defined($dev->{ID}) && defined($dev->{ID}->{WORDS}))
    {
    @{$dev->{ID}->{WORDS}} = sort {$a->{ADDRESS} <=> $b->{ADDRESS}} @{$dev->{ID}->{WORDS}};
    }
  }

print join("\n", @devices_header) . "\n";

my $i = 0;
my $v = @device_names;
while (TRUE)
  {
  print_device($i);
  ++$i;
  last if ($i == $v);
  print "\n";
  }
