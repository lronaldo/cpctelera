#!/usr/bin/perl -w

=back

  Copyright (C) 2012-2015 Molnar Karoly <molnarkaroly@users.sf.net>

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

  $Id: optimize_pic16devices.pl 9450 2016-01-09 16:47:43Z molnarkaroly $
=cut

use strict;
use warnings;
no if $] >= 5.018, warnings => "experimental::smartmatch";        # perl 5.16
use 5.12.0;                     # when (regex)
use POSIX 'ULONG_MAX';

use constant FALSE => 0;
use constant TRUE  => 1;

use constant FNV1A32_INIT  => 0x811C9DC5;
use constant FNV1A32_PRIME => 0x01000193;

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
        NAME     => '',
        COMMENTS => '',
        RAM      => {
                    SIZE      => 0,
                    SPLIT     => 0,
                    HASH      => 0,
                    DIFF      => 0
                    },
        CONFIG   => {
                    FIRST     => 0,
                    LAST      => 0,
                    WORDS     => {},
                    ORD_WORDS => [],
                    HASH      => 0,
                    DIFF      => 0
                    },
        ID       => {
                    FIRST     => 0,
                    LAST      => 0,
                    WORDS     => {},
                    ORD_WORDS => [],
                    HASH      => 0,
                    DIFF      => 0
                    },
        XINST    => 0
        CHILD    => 0
        }
=cut

use constant RELEVANCE_RAM      => 2;
use constant RELEVANCE_CONFWORD => 4;
use constant RELEVANCE_IDWORD   => 2;
use constant RELEVANCE_FATAL    => 1000;

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

#---------------------------------------------------------------------------------------------------

sub param_exist($$)
  {
  die "This option \"$_[0]\" requires a parameter.\n" if ($_[1] > $#ARGV);
  }

#---------------------------------------------------------------------------------------------------

sub str2int($)
  {
  my $Str = $_[0];

  return hex($1)   if ($Str =~ /^0x([[:xdigit:]]+)$/io);
  return oct($1)   if ($Str =~ /^(0[0-7]+)$/o);
  return int($Str) if ($Str =~ /^-?\d+$/o);

  die "This string not integer: \"$Str\"";
  }

#---------------------------------------------------------------------------------------------------

sub Log
  {
  return if (pop(@_) > $verbose);
  foreach (@_) { print STDERR $_; }
  print STDERR "\n";
  }

#---------------------------------------------------------------------------------------------------

sub Open($$)
  {
  my ($File, $Function) = @_;
  my $handle;

  open($handle, '<', $File) || die "${Function}(): Could not open the \"$File\" file.\n";
  return $handle;
  }

#---------------------------------------------------------------------------------------------------

sub fnv1a32_str($$)
  {
  my ($String, $Hash) = @_;

  foreach (unpack('C*', $String))
    {
    $Hash ^= $_;
    $Hash *= FNV1A32_PRIME;
    $Hash &= 0xFFFFFFFF;
    }

  return $Hash;
  }

#---------------------------------------------------------------------------------------------------

sub fnv1a32_int32($$)
  {
  my ($Int, $Hash) = @_;
  my $i;

  for ($i = 4; $i; --$i)
    {
    $Hash ^= $Int & 0xFF;
    $Hash *= FNV1A32_PRIME;
    $Hash &= 0xFFFFFFFF;
    $Int >>= 8;
    }

  return $Hash;
  }

#---------------------------------------------------------------------------------------------------

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

#---------------------------------------------------------------------------------------------------

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

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   The important procedures.   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                             @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        #
        # Reads the entire pic16devices.txt file.
        #

sub read_pic16devices_txt($)
  {
  my $File = $_[0];
  my ($parent, $first, $last, $txt, $ref, $is_using);
  my $in = Open($File, 'read_pic16devices_txt');
  my $header = TRUE;
  my $device = undef;

  Log("Reads the $File file.", 4);

  while (<$in>)
    {
    chomp;
    s/\r$//o;
    s/\s+$//o;

    $header = FALSE if (/^\s*name\b/io);

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
                  NAME      => $1,
                  COMMENTS  => undef,
                  RAM       => {
                               SIZE  => -1,
                               SPLIT => -1,
                               HASH  => 0,
                               DIFF  => 0
                               },
                  CONFIG    => {
                               FIRST     => -1,
                               LAST      => -1,
                               WORDS     => {},
                               ORD_WORDS => [],
                               HASH      => 0,
                               DIFF      => 0
                               },
                  ID        => {
                               FIRST     => -1,
                               LAST      => -1,
                               WORDS     => {},
                               ORD_WORDS => [],
                               HASH      => 0,
                               DIFF      => 0
                               },
                  XINST     => -1,
                  CHILD     => FALSE
                  };

        Log("name       : $1", 7);
        $devices_by_name{$1} = $device;
        $is_using = FALSE;
        }

      when (/^\s*using\s+(\w+)$/io)
        {
        die "Device not exists." if (! defined($device));

        $parent = $devices_by_name{$1};

        die "In device - \"$device->{NAME}\" - not exists the parent: \"$1\"\n" if (! defined($parent));

        # Unlock the "using" keyword.

        Log("using      : $1", 7);
        %{$device->{RAM}}             = %{$parent->{RAM}};
        $device->{CONFIG}->{FIRST}    = $parent->{CONFIG}->{FIRST};
        $device->{CONFIG}->{LAST}     = $parent->{CONFIG}->{LAST};
        %{$device->{CONFIG}->{WORDS}} = %{$parent->{CONFIG}->{WORDS}};
        $device->{ID}->{FIRST}        = $parent->{ID}->{FIRST};
        $device->{ID}->{LAST}         = $parent->{ID}->{LAST};
        %{$device->{ID}->{WORDS}}     = %{$parent->{ID}->{WORDS}};
        $device->{XINST}              = $parent->{XINST};
        $is_using = TRUE;
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
        Log("configrange: $first, $last", 7);

        if (($device->{CONFIG}->{FIRST} >= 0) || ($device->{CONFIG}->{LAST} >= 0))
          {
          Log("The configrange already exists in the \"$device->{NAME}\".", 0);

          if (($device->{CONFIG}->{FIRST} != $first) || ($device->{CONFIG}->{LAST} != $last))
            {
            Log("  In addition the previous values different from the new values.", 0);
            Log(sprintf("  previous: 0x%06X - 0x%06X, new: 0x%06X - 0x%06X",
                         $device->{CONFIG}->{FIRST}, $device->{CONFIG}->{LAST},
                         $first, $last), 0);
            }

        # The previous values invalid.

          $device->{CONFIG}->{WORDS} = {};
          }

        $device->{CONFIG}->{FIRST} = $first;
        $device->{CONFIG}->{LAST}  = $last;
        }

      when (/^\s*configword\s+(\w+)\s+(\w+)\s+(\w+)(?:\s+(\w+))?$/io)
        {
        my ($addr, $mask, $val, $amask, $hash);

        die "Device not exists." if (! defined($device));

        ($addr, $mask, $val) = (str2int($1), str2int($2), str2int($3));

        if (defined($4))
          {
          $amask = str2int($4);
          Log("configword : $addr, $mask, $val, $amask", 7);
          }
        else
          {
          $amask = -1;
          Log("configword : $addr, $mask, $val", 7);
          }

        $hash = fnv1a32_int32($addr, FNV1A32_INIT);
        $hash = fnv1a32_int32($mask, $hash);
        $hash = fnv1a32_int32($val, $hash);
        $hash = fnv1a32_int32($amask, $hash);
        $ref  = {
                ADDRESS  => $addr,
                MASK     => $mask,
                VALUE    => $val,
                AND_MASK => $amask,
                HASH     => $hash
                };

        if ($is_using && ! defined($device->{CONFIG}->{WORDS}->{$addr}))
          {
          printf STDERR "Database error: The 0x%06X config word not exist in the ancestor MCU!\n", $addr;
          exit(1);
          }

        $device->{CONFIG}->{WORDS}->{$addr} = $ref;
        }

      when (/^\s*XINST\s+(\w+)\s*$/io)
        {
        die "Device not exists." if (! defined($device));

        Log("XINST    : $1", 7);
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
        my ($addr, $val, $hash);

        die "Device not exists." if (! defined($device));

        ($addr, $val) = (str2int($1), str2int($2));
        Log("idword     : $1 $2", 7);
        $hash = fnv1a32_int32($addr, FNV1A32_INIT);
        $hash = fnv1a32_int32($val, $hash);
        $ref  = {
                ADDRESS  => $addr,
                VALUE    => $val,
                HASH     => $hash
                };

        if ($is_using && ! defined($device->{ID}->{WORDS}->{$addr}))
          {
          printf STDERR "Database error: The 0x%06X id word not exist in the ancestor MCU!\n", $addr;
          exit(1);
          }

        $device->{ID}->{WORDS}->{$addr} = $ref;
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

#---------------------------------------------------------------------------------------------------

sub make_hashes($)
  {
  my $DevRef = $_[0];
  my ($ref1, $ref2, $hash);

  $ref1 = $DevRef->{RAM};
  $hash = fnv1a32_int32($ref1->{SIZE}, FNV1A32_INIT);
  $ref1->{HASH} = fnv1a32_int32($ref1->{SPLIT}, $hash);

  #.................

  $ref1 = $DevRef->{CONFIG};
  $hash = fnv1a32_int32($ref1->{FIRST}, FNV1A32_INIT);
  $hash = fnv1a32_int32($ref1->{LAST}, $hash);

  @{$ref1->{ORD_WORDS}} = sort {$a->{ADDRESS} <=> $b->{ADDRESS}} values %{$ref1->{WORDS}};

  foreach $ref2 (@{$ref1->{ORD_WORDS}})
    {
    $hash = fnv1a32_int32($ref2->{ADDRESS}, $hash);
    $hash = fnv1a32_int32($ref2->{MASK}, $hash);
    $hash = fnv1a32_int32($ref2->{VALUE}, $hash);
    $hash = fnv1a32_int32($ref2->{AND_MASK}, $hash);
    }

  $ref1->{HASH} = $hash;

  #.................

  $ref1 = $DevRef->{ID};

  if (defined($ref1))
    {
    $hash = fnv1a32_int32($ref1->{FIRST}, FNV1A32_INIT);
    $hash = fnv1a32_int32($ref1->{LAST}, $hash);

    if (defined($ref1->{WORDS}))
      {
      @{$ref1->{ORD_WORDS}} = sort {$a->{ADDRESS} <=> $b->{ADDRESS}} values %{$ref1->{WORDS}};

      foreach $ref2 (@{$ref1->{ORD_WORDS}})
        {
        $hash = fnv1a32_int32($ref2->{ADDRESS}, $hash);
        $hash = fnv1a32_int32($ref2->{VALUE}, $hash);
        }
      }

    $ref1->{HASH} = $hash;
    }
  }

#---------------------------------------------------------------------------------------------------

sub difference_of_arrays($$)
  {
  my ($ArrayRef1, $ArrayRef2) = @_;
  my ($diff, $len, $i);

  $len = @{$ArrayRef1};
  # The lenght of two arrays must be of equal.
  return RELEVANCE_FATAL if ($len != scalar(@{$ArrayRef2}));

  $diff = 0;
  for ($i = 0; $i < $len; ++$i)
    {
    if ($ArrayRef1->[$i]->{ADDRESS} != $ArrayRef2->[$i]->{ADDRESS})
      {
      $diff += RELEVANCE_FATAL;
      $ArrayRef1->[$i]->{DIFF} = TRUE;
      }
    elsif ($ArrayRef1->[$i]->{HASH} != $ArrayRef2->[$i]->{HASH})
      {
      $diff += RELEVANCE_CONFWORD;
      $ArrayRef1->[$i]->{DIFF} = TRUE;
      }
    else
      {
      $ArrayRef1->[$i]->{DIFF} = FALSE;
      }
    }
  
  return $diff;
  }

#---------------------------------------------------------------------------------------------------

        #
        # Compares the $Dev1 and the $Dev2.
        #

sub difference_of_devices($$)
  {
  my ($DevRef1, $DevRef2) = @_;
  my ($diff, $r1, $r2, $aref1, $aref2, $len1, $len2, $min, $i);

  $i = defined($DevRef1) + defined($DevRef2);

  return RELEVANCE_FATAL if ($i != 2);

  $diff  = 0;
  $r1 = $DevRef1->{RAM};
  $r2 = $DevRef2->{RAM};

  if ($r1->{HASH} != $r2->{HASH})
    {
    $diff += RELEVANCE_RAM;
    $r1->{DIFF} = TRUE;
    }
  else
    {
    $r1->{DIFF} = FALSE;
    }

  $r1 = $DevRef1->{CONFIG};
  $r2 = $DevRef2->{CONFIG};

  if ($r1->{HASH} != $r2->{HASH})
    {
    $diff += RELEVANCE_FATAL if ($r1->{FIRST} != $r2->{FIRST});
    $diff += RELEVANCE_FATAL if ($r1->{LAST}  != $r2->{LAST});
    $diff += difference_of_arrays($r1->{ORD_WORDS}, $r2->{ORD_WORDS});
    $r1->{DIFF} = TRUE;
    }
  else
    {
    $r1->{DIFF} = FALSE;
    }

  $r1 = $DevRef1->{ID};
  $r2 = $DevRef2->{ID};

  if ($r1->{HASH} != $r2->{HASH})
    {
    $diff += RELEVANCE_FATAL if ($r1->{FIRST} != $r2->{FIRST});
    $diff += RELEVANCE_FATAL if ($r1->{LAST}  != $r2->{LAST});
    $diff += difference_of_arrays($r1->{ORD_WORDS}, $r2->{ORD_WORDS});
    $r1->{DIFF} = TRUE;
    }
  else
    {
    $r1->{DIFF} = FALSE;
    }

  # The value of two XINST elements must be of equal.
  $diff += RELEVANCE_FATAL if ($DevRef1->{XINST} != $DevRef1->{XINST});
  return $diff;
  }

#---------------------------------------------------------------------------------------------------

sub print_config_words($)
  {
  my $Words = $_[0];

  return if (! defined($Words));

  foreach (@{$Words})
    {
    printf "configword  0x%06X 0x%02X 0x%02X", $_->{ADDRESS}, $_->{MASK}, $_->{VALUE};
    printf " 0x%02X", $_->{AND_MASK} if ($_->{AND_MASK} > 0);
    print  "\n";
    }
  }

#---------------------------------------------------------------------------------------------------

sub print_id_words($)
  {
  my $Words = $_[0];

  return if (! defined($Words));

  foreach (@{$Words})
    {
    printf "idword      0x%06X 0x%02X\n", $_->{ADDRESS}, $_->{VALUE};
    }
  }

#---------------------------------------------------------------------------------------------------

sub print_diff_config_words($)
  {
  my $ArrayRef = $_[0];

  foreach (@{$ArrayRef})
    {
    next if (! $_->{DIFF});

    printf "configword  0x%06X 0x%02X 0x%02X", $_->{ADDRESS}, $_->{MASK}, $_->{VALUE};
    printf " 0x%02X", $_->{AND_MASK} if ($_->{AND_MASK} > 0);
    print  "\n";
    }
  }

#---------------------------------------------------------------------------------------------------

sub print_diff_id_words($)
  {
  my $ArrayRef = $_[0];

  foreach (@{$ArrayRef})
    {
    next if (! $_->{DIFF});

    printf "idword      0x%06X 0x%02X\n", $_->{ADDRESS}, $_->{VALUE};
    }
  }

#---------------------------------------------------------------------------------------------------

sub print_device($)
  {
  my $Index = $_[0];
  my $mcu = $device_names[$Index];
  my $dev = $devices_by_name{$mcu};
  my ($min_diff, $diff);
  my ($ac, $ancestor, $i, $ref1, $ref2);

  return if (! defined($dev));

  Log("Prints the $mcu MCU.", 4);

  $ancestor = undef;

  if ($operation == OP_OPTIMIZE)
    {
        # Optimized writing is required.

    $min_diff = ULONG_MAX;
    for ($i = 0; $i < scalar(@device_names); ++$i)
      {
      $ac = $devices_by_name{$device_names[$i]};

      last if ($Index == $i);
      next if ($ac->{CHILD});

      $diff = difference_of_devices($dev, $ac);

      if ($min_diff > $diff)
        {
        $min_diff = $diff;
        $ancestor = $ac;
        }
      }

    $ancestor = undef if ($min_diff > 15);
    }

  print "name        $dev->{NAME}\n";

  if ($dev->{COMMENTS})
    {
    foreach (@{$dev->{COMMENTS}})
      {
      print "$_\n";
      }
    }

  $ref1 = $dev->{RAM};

  if (defined($ancestor))
    {
    $dev->{CHILD} = TRUE;

    print  "using       $ancestor->{NAME}\n";
    difference_of_devices($dev, $ancestor);

    if ($ref1->{DIFF})
      {
      $ref2 = $ancestor->{RAM};

      if ($ref1->{SIZE} != $ref2->{SIZE})
        {
        print  "ramsize     $ref1->{SIZE}\n";
        }

      if ($ref1->{SPLIT} != $ref2->{SPLIT})
        {
        printf "split       0x%02X\n", $ref1->{SPLIT};
        }
      }

    $ref1 = $dev->{CONFIG};

    if ($ref1->{DIFF})
      {
      print_diff_config_words($ref1->{ORD_WORDS});
      }

    printf "XINST       $dev->{XINST}\n" if (($ancestor->{XINST} < 0) && ($dev->{XINST} > 0));

    $ref1 = $dev->{ID};

    if ($ref1->{DIFF})
      {
      print_diff_id_words($ref1->{ORD_WORDS});
      }
    }
  else
    {
    print  "ramsize     $ref1->{SIZE}\n";
    printf "split       0x%02X\n", $ref1->{SPLIT};

    $ref1 = $dev->{CONFIG};
    printf "configrange 0x%06X 0x%06X\n", $ref1->{FIRST}, $ref1->{LAST};
    print_config_words($ref1->{ORD_WORDS});

    printf "XINST       $dev->{XINST}\n" if ($dev->{XINST} > 0);

    $ref1 = $dev->{ID};

    if (($ref1->{FIRST} > 0) && ($ref1->{LAST} > 0))
      {
      printf "idlocrange  0x%06X 0x%06X\n", $ref1->{FIRST}, $ref1->{LAST};
      print_id_words($ref1->{ORD_WORDS});
      }
    }
  }

#---------------------------------------------------------------------------------------------------

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

            Unlocks the "using" keywords and displays the full original
            content.

        -v <level> or --verbose <level>

            It provides information on from the own operation.
            Possible value of the level between 0 and 10. (default: 0)

        -h or --help

            This text.
EOT
;
  }

#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   The main program.   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
# @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

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

if (($file eq '') || ($operation == OP_NULL))
  {
  usage();
  exit(0);
  }

(-f $file) || die "This file - \"$file\" - not exist!";

read_pic16devices_txt($file);

@device_names = sort {versionSort($a, $b)} keys(%devices_by_name);

foreach (@device_names)
  {
  make_hashes($devices_by_name{$_});
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
