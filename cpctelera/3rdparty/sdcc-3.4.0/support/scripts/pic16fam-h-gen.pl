#!/usr/bin/perl -w

#
# This script generates a C header file that maps the target device (as
# indicated via the sdcc generated -Dpic16cxxx or -Dpic12fxxx or -Dpic16fxxx
# macro) to its device family and the device families to their respective
# style of ADC, USART and SSP programming for use in the SDCC PIC14 I/O library.
#
# Copyright 2010 Raphael Neider <rneider AT web.de>
# Very modified and expanded to the pic14 series: Molnár Károly, 2012 <molnarkaroly@users.sf.net>
#
# This file is part of SDCC.
# 
# SDCC is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 2 of the License, or (at your
# option) any later version.
#
# SDCC is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with SDCC.  If not, see <http://www.gnu.org/licenses/>.
#

#
# Usage: perl pic16fam-h-gen.pl [-i|-p|-h]
#
# This will create pic16fam.h.gen in your current directory.
# Check sanity of the file and move it to .../include/pic14/pic16fam.h.
# If you assigned new I/O styles, implement them in
# .../include/pic14/{adc,i2c,pwm,spi,usart}.h and
# .../lib/pic14/libio/*/*.c
#

use strict;
use 5.10.1;
use feature 'switch';           # Starting from 5.10.1.

use constant FALSE => 0;
use constant TRUE  => 1;

use constant SECT_NONE => 0;
use constant SECT_REG  => 1;
use constant SECT_ENH  => 2;
use constant SECT_PER  => 3;

my $head = '__SDCC_PIC';

my $gen_ignore_lists = FALSE;
my $gen_mcu_lists = FALSE;

my @regular_mcu = ();
my @enhanced_mcu = ();

my %peripherals_by_names = ();
my $actual_peripheral;

my $section;

my $line;

my $tail = '.gen';
my $update = 'Please update your pic14/pic16fam.h manually and/or inform the maintainer.';

my $fname;

#-------------------------------------------------------------------------------

sub align($$)
  {
  my $Text = $_[0];
  my $al   = $_[1] - length($Text);

        # One space will surely becomes behind it.
  $al = 1 if ($al < 1);

  return ($Text . (' ' x $al));
  }

#-------------------------------------------------------------------------------

sub find_in_supported($$)
  {
  my ($Hash, $Name) = @_;

  for (keys %{$Hash})
    {
    return TRUE if ($_ > 0 && $Name ~~ @{${$Hash}{$_}});
    }

  return FALSE;
  }

#-------------------------------------------------------------------------------

=back
	This procedure give a list (id=0) to the peripherals, in which are
	included the not supported devices.
=cut

sub create_ignore_lists()
  {
  my @full_mcu_list = (@regular_mcu, @enhanced_mcu);

  foreach (keys %peripherals_by_names)
    {
    my $peri = $peripherals_by_names{$_};
    my @ignore_list = ();

    foreach (@full_mcu_list)
      {
      push(@ignore_list, $_) if (! find_in_supported($peri, $_) && ! ($_ ~~ @ignore_list));
      }

    @{${$peri}{0}} = @ignore_list;
    }
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

sub smartSort
  {
  my @a_s = ($a =~ /(\d+|\D+)/go);
  my @b_s = ($b =~ /(\d+|\D+)/go);
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

sub collector($)
  {
  my $id;
  my @fields = split(':', $line);

  die "Invalid record: >$line<" if (@fields != 2);

  $id = int($fields[0]);
  push(@{${$_[0]}{$id}}, map { uc($_); } split(',', $fields[1])) if ($id > 0);
  }

#-------------------------------------------------------------------------------

=back
	Creates the ignore files of peripherals from the ignore lists.
=cut

sub generate_ignore_files()
  {
  foreach (keys %peripherals_by_names)
    {
    $fname = "$_.ignore$tail";
    open(FH, '>', $fname) or die "Could not open: \"$fname\"";
    print FH join("\n", map { lc($_); } sort smartSort @{${$peripherals_by_names{$_}}{0}}) . "\n";
    close(FH);
    }
  }

#-------------------------------------------------------------------------------

=back
	Creates the lists of MCUs.
=cut

sub generate_mcu_files()
      {
  $fname = "mcu.regular$tail";
  open(FH, '>', $fname) or die "Could not open: \"$fname\"";
  print FH join("\n", map { lc($_); } sort smartSort @regular_mcu) . "\n";
  close(FH);

  $fname = "mcu.enhanced$tail";
  open(FH, '>', $fname) or die "Could not open: \"$fname\"";
  print FH join("\n", map { lc($_); } sort smartSort @enhanced_mcu) . "\n";
  close(FH);
      }

#-------------------------------------------------------------------------------

sub print_peripheral($)
  {
  my $Name = $_[0];
  my $array = $peripherals_by_names{$_};
  my $def = align("#define __SDCC_${Name}_STYLE", 30);
  my $cpp;

  print FH <<EOT
/*
 * Define $Name style per device family.
 */
#undef  __SDCC_${Name}_STYLE

EOT
;
  $cpp = '#if   ';
  foreach (sort keys %{$array})
    {
    my $fams = "defined($head" . join(") || \\\n      defined($head", sort smartSort @{$array->{$_}}) . ')';

    print FH "$cpp$fams\n$def$_\n\n";
    $cpp = '#elif ';
    }

  print FH <<EOT
#else
#warning No $Name style associated with the target device.
#warning $update
#endif

EOT
;
  }

#-------------------------------------------------------------------------------

for (my $i = 0; $i < @ARGV; )
  {
  my $opt = $ARGV[$i++];

  given ($opt)
    {
    when (/^-(i|-gen-ignore)$/o)
      {
	# This command creates the PERIPHERAL.ignore.gen files.
      $gen_ignore_lists = TRUE;
      }

    when (/^-(p|-gen-processor-lists)$/o)
      {
	# This command creates the mcu.{enhanced,regular}.gen files.
      $gen_mcu_lists = TRUE;
      }

    when (/^-(ip|pi)$/o)
      {
	# This command creates the PERIPHERAL.ignore.gen and
	# the mcu.{enhanced,regular}.gen files.
      $gen_ignore_lists = TRUE;
      $gen_mcu_lists    = TRUE;
      }

    when (/^-(h|-help)$/o)
      {
      print <<EOT
Usage: $0 [options]

    Options are:
	-i or --gen-ignore
		This command creates the PERIPHERAL.ignore$tail files.

	-p or --gen-processor-lists
		This command creates the mcu.{enhanced,regular}$tail files.

	-h or --help
		This text.
EOT
;
      exit(0);
      }
    } # given ($opt)
  }

$section = SECT_NONE;

	# While reading skips the blank or comment lines.
foreach (grep(! /^\s*$|^\s*#/o, <DATA>))
  {
    chomp;
  s/\s*//go;                  # strip whitespace

  $line = $_;

  if ($line =~ /^SECTION=(\S+)$/o)
    {
    given ($1)
      {
      when ('REGULAR')  { $section = SECT_REG; }
      when ('ENHANCED') { $section = SECT_ENH; }

      default
        {
        my $name = uc($1);

        die "The $name peripheral already exist!" if (defined($peripherals_by_names{$name}));

        $actual_peripheral = {};
        $peripherals_by_names{$name} = $actual_peripheral;
        $section = SECT_PER;
        }
      }

    next;
    }

  given ($section)
    {
    when (SECT_REG)
      {
      push(@regular_mcu, map { uc($_); } split(',', $line));
    }

    when (SECT_ENH)
      {
      push(@enhanced_mcu, map { uc($_); } split(',', $line));
      }

    when (SECT_PER)
      {
      collector($actual_peripheral);
      }
    }
  } # foreach (grep(! /^\s*$|^\s*#/o, <DATA>))

create_ignore_lists();

generate_ignore_files() if ($gen_ignore_lists);
generate_mcu_files() if ($gen_mcu_lists);

$fname = "pic16fam.h$tail";
open(FH, '>', $fname) or die "Could not open: \"$fname\"";

print FH <<EOT
/*
 * pic16fam.h - PIC14 families
 *
 * This file is has been generated using $0 .
 */
#ifndef __SDCC_PIC16FAM_H__
#define __SDCC_PIC16FAM_H__ 1

/*
 * Define device class.
 */
#undef  __SDCC_PIC14_ENHANCED

EOT
;

my $memb = "defined($head" . join(") || \\\n      defined($head", sort smartSort @enhanced_mcu) . ')';
print FH <<EOT
#if   $memb
#define __SDCC_PIC14_ENHANCED 1

#endif

EOT
;

foreach (sort smartSort keys %peripherals_by_names)
  {
  print_peripheral($_);
  }

print FH "#endif /* !__SDCC_PIC16FAM_H__ */\n";

close(FH);

__END__
################################################################################
#
# This list contains the names of the regular devices.
# (Of course only them which are the sdcc also know.)
#

    SECTION=REGULAR

10f320,10f322
12f609,12f615,12f617,12f629,12f635,12f675,12f683,12f752
16c62,16c63a,16c65b,16c71,16c72,16c73b,16c74b
16c432,16c433
16c554,16c557,16c558
16c620,16c620a,16c621,16c621a,16c622,16c622a
16c710,16c711,16c715,16c717,16c745,16c765,16c770,16c771,16c773,16c774,16c781,16c782
16c925,16c926
16f72,16f73,16f74,16f76,16f77
16f84,16f84a,16f87,16f88
16f610,16f616,16f627,16f627a,16f628,16f628a,16f630,16f631,16f636,16f639,16f648a
16f676,16f677,16f684,16f685,16f687,16f688,16f689,16f690
16f707,16f716,16f720,16f721,16f722,16f722a,16f723,16f723a,16f724,16f726,16f727,16f737,16f747,16f767,16f777,16f785
16f818,16f819,16f870,16f871,16f872,16f873,16f873a,16f874,16f874a,16f876,16f876a,16f877,16f877a
16f882,16f883,16f884,16f886,16f887
16f913,16f914,16f916,16f917,16f946
16hv616

################################################################################
#
# This list in turn exclusively contains the names of the enhanced devices.
# (Only them which are the sdcc also know.)
#

    SECTION=ENHANCED

12f1501,12lf1552,12f1822,12f1840
16f1454,16f1455,16f1458,16f1459
16f1503,16f1507,16f1508,16f1509,16f1512,16f1513,16f1516,16f1517,16f1518,16f1519,16f1526,16f1527
16f1782,16f1783,16f1784,16f1786,16f1787
16f1823,16f1824,16f1825,16f1826,16f1827,16f1828,16f1829,16f1847
16f1933,16f1934,16f1936,16f1937,16f1938,16f1939,16f1946,16f1947
16lf1902,16lf1903,16lf1904,16lf1906,16lf1907

################################################################################
#
# <id>:<head>{,<member>}
#
# Each line provides a colon separated list of
#
#  * a numeric family name, derived from the first family member as follows:
#
#    ADC     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u1", <num1>, <num2>)
#    PWM     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u2", <num1>, <num2>)
#    SSP     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u3", <num1>, <num2>)
#    USART   : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u4", <num1>, <num2>)
#
#  * a comma-separated list of members of a device family.
#
#    The rules basis of which members of a family belong together:
#
#       a.) The connectors of peripheral are located on the same pin.
#
#       b.) In the periphery - in context with other peripherals - should be
#           used in the same way. (Pin relocation. Other peripheral to use
#           the same pin? Peripheral initialization. ...)
#
# This data has been gathered manually from data sheets published by
# Microchip Technology Inc.
#

    SECTION=ADC

1003201:10f320,10f322
1206151:12f615,12f617
1206751:12f675,12f683
1207521:12f752
1600711:16c71,16c710,16c711
1600721:16c72,16f72,16c73b,16f73,16f76
1600741:16c74b,16f74,16f77
1604331:16c433
1607151:16c715
1607171:16c717,16c770,16c771
1607451:16c745
1607651:16c765
1607731:16c773
1607741:16c774
1607811:16c781,16c782
1609251:16c925,16c926,16f870,16f872,16f873,16f873a,16f876,16f876a
1600881:16f88
1606161:16f616,16hv616
1606761:16f676,16f684,16f688
1606771:16f677,16f685,16f687,16f689,16f690
1607071:16f707
1607161:16f716
1607201:16f720,16f721
1607221:16f722,16f722a,16f723,16f723a,16f726
1607241:16f724,16f727
1607371:16f737,16f767
1607471:16f747,16f777
1607851:16f785
1608181:16f818,16f819
1608711:16f871,16f874,16f874a,16f877,16f877a
1608821:16f882,16f883,16f886
1608841:16f884,16f887
1609131:16f913,16f916
1609141:16f914,16f917,16f946
1215011:12f1501
1218221:12f1822,12f1840
1614551:16f1455
1614591:16f1459
1615031:16f1503
1615071:16f1507,16f1508,16f1509
1615121:16f1512,16f1513,16f1516,16f1518
1615171:16f1517,16f1519
1615261:16f1526,16f1527
1617821:16f1782,16f1783,16f1786
1617841:16f1784,16f1787
1618231:16f1823
1618241:16f1824,16f1825
1618261:16f1826,16f1827,16f1847
1618281:16f1828,16f1829
1619331:16f1933,16f1936,16f1938
1619341:16f1934,16f1937,16f1939
1619461:16f1946,16f1947
1619021:16lf1902,16lf1903,16lf1906
1619041:16lf1904,16lf1907

    SECTION=PWM

1003202:10f320,10f322
1206152:12f615,12f617
1206832:12f683
1207522:12f752
1600622:16c62,16c72,16c925,16c926,16f72,16f870,16f871,16f872
1600632:16c63a,16c65b,16c73b,16c74b,16c745,16c765,16c773,16c774,16f73,16f74,16f76,16f77,16f873,16f873a,16f874,16f874a,16f876,16f876a,16f877,16f877a
1608822:16f882,16f883,16f886,16f884,16f887
1607172:16c717,16c770,16c771,16f716
1606272:16f627,16f627a,16f628,16f628a,16f648a
1600872:16f87,16f88
1606162:16f616,16hv616,16f684
1606852:16f685,16f690
1607072:16f707
1607222:16f722,16f722a,16f723,16f723a,16f724,16f726,16f727
1607202:16f720,16f721,16f913,16f916
1607372:16f737,16f747,16f767,16f777
1607852:16f785
1608182:16f818,16f819
1609142:16f914,16f917,16f946
1215012:12f1501
1218222:12f1822,12f1840
1614542:16f1454,16f1455
1614592:16f1459
1615032:16f1503,16f1507,16f1508,16f1509
1615122:16f1512,16f1513,16f1516,16f1517,16f1518,16f1519
1615262:16f1526,16f1527
1617822:16f1782,16f1783
1617842:16f1784,16f1787
1617862:16f1786
1618232:16f1823
1618242:16f1824,16f1825
1618262:16f1826
1618272:16f1827,16f1847
1618282:16f1828,16f1829
1619332:16f1933,16f1936,16f1938
1619342:16f1934,16f1937,16f1939
1619462:16f1946,16f1947

    SECTION=SSP

1600623:16c62,16c72
1600633:16c63a,16c65b
1600733:16c73b,16c74b
1607173:16c717,16c770,16c771
1607733:16c773,16c774
1609253:16c925,16c926
1600723:16f72,16f73,16f74,16f76,16f77
1600873:16f87,16f88,16f818,16f819
1606773:16f677,16f687,16f689,16f690
1607073:16f707
1607223:16f722,16f722a,16f723,16f723a,16f724,16f726,16f727
1607203:16f720,16f721
1607373:16f737,16f747,16f767,16f777
1608723:16f872,16f873,16f873a,16f874,16f874a,16f876,16f876a,16f877,16f877a
1608823:16f882,16f883,16f884,16f886,16f887
1609133:16f913,16f914,16f916,16f917,16f946
1218223:12f1822,12f1840
1614543:16f1454,16f1455
1614593:16f1459
1615033:16f1503
1615083:16f1508,16f1509
1615123:16f1512,16f1513,16f1516,16f1517,16f1518,16f1519
1615263:16f1526,16f1527
1617823:16f1782,16f1783,16f1784,16f1786,16f1787
1618233:16f1823,16f1824,16f1825
1618263:16f1826
1618273:16f1827,16f1847
1618283:16f1828
1618293:16f1829
1619333:16f1933,16f1934,16f1936,16f1937,16f1938,16f1939
1619463:16f1946,16f1947

    SECTION=USART

1600634:16c63a,16c65b,16c73b,16c74b,16c745,16c765,16f73,16f74,16f76,16f77
1600874:16f87,16f88
1606274:16f627,16f627a,16f628,16f628a,16f648a
1606874:16f687,16f689,16f690
1606884:16f688
1607074:16f707
1607204:16f720,16f721
1607734:16c773,16c774,16f722,16f722a,16f723,16f723a,16f724,16f726,16f727,16f737,16f747,16f767,16f777,16f870,16f871,16f873,16f873a,16f874,16f874a,16f876,16f876a,16f877,16f877a,16f913,16f914,16f916,16f917,16f946
1608824:16f882,16f883,16f884,16f886,16f887
1218224:12f1822,12f1840
1614544:16f1454,16f1455
1614594:16f1459
1615084:16f1508,16f1509
1615124:16f1512,16f1513,16f1516,16f1517,16f1518,16f1519
1615264:16f1526,16f1527
1617824:16f1782,16f1783,16f1784,16f1786,16f1787
1618234:16f1823,16f1824,16f1825
1618264:16f1826,16f1827,16f1847
1618284:16f1828,16f1829
1619334:16f1933,16f1934,16f1936,16f1937,16f1938,16f1939,16lf1904,16lf1906,16lf1907
1619464:16f1946,16f1947
