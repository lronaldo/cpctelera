#!/usr/bin/perl -w

=back

    This script generates a C header file that maps the target device (as
    indicated via the sdcc generated -Dpic16cxxx or -Dpic12fxxx or -Dpic16fxxx
    macro) to its device family and the device families to their respective
    style of ADC, USART and SSP programming for use in the SDCC PIC14 I/O library.

    Copyright 2010 Raphael Neider <rneider AT web.de>
    PIC14 port:
    Copyright 2012-2014 Molnar Karoly <molnarkaroly@users.sf.net>

    This file is part of SDCC.

    SDCC is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 2 of the License, or (at your
    option) any later version.

    SDCC is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with SDCC.  If not, see <http://www.gnu.org/licenses/>.

    Usage: perl pic16fam-h-gen.pl [-i|-p|-h]

	This will create pic16fam.h.gen in your current directory.
	Check sanity of the file and move it to .../include/pic14/pic16fam.h.
	If you assigned new I/O styles, implement them in
	.../include/pic14/{adc,i2c,pwm,spi,usart}.h and
	.../lib/pic14/libio/*/*.c

    $Id: pic16fam-h-gen.pl 9072 2014-09-17 14:00:11Z molnarkaroly $
=cut


use strict;
use warnings;
no if $] >= 5.018, warnings => "experimental::smartmatch";        # perl 5.16
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

10f320,10f322,12f609,12f615,12f617,12f629,12f635,12f675,12f683,12f752
16c62,16c63a,16c65b,16c71,16c72,16c73b,16c74b,16c432,16c433,16c554
16c557,16c558,16c620,16c620a,16c621,16c621a,16c622,16c622a,16c710,16c711
16c715,16c717,16c745,16c765,16c770,16c771,16c773,16c774,16c781,16c782
16c925,16c926,16f72,16f73,16f74,16f76,16f77,16f84,16f84a,16f87
16f88,16f610,16f616,16f627,16f627a,16f628,16f628a,16f630,16f631,16f636
16f639,16f648a,16f676,16f677,16f684,16f685,16f687,16f688,16f689,16f690
16f707,16f716,16f720,16f721,16f722,16f722a,16f723,16f723a,16f724,16f726
16f727,16f737,16f747,16f753,16f767,16f777,16f785,16f818,16f819,16f870
16f871,16f872,16f873,16f873a,16f874,16f874a,16f876,16f876a,16f877,16f877a
16f882,16f883,16f884,16f886,16f887,16f913,16f914,16f916,16f917,16f946
16hv616,16hv753

################################################################################
#
# This list in turn exclusively contains the names of the enhanced devices.
# (Only them which are the sdcc also know.)
#

    SECTION=ENHANCED

12f1501,12f1571,12f1572,12f1612,12f1822,12f1840,12lf1552,16f1454,16f1455,16f1458
16f1459,16f1503,16f1507,16f1508,16f1509,16f1512,16f1513,16f1516,16f1517,16f1518
16f1519,16f1526,16f1527,16f1613,16f1703,16f1704,16f1705,16f1707,16f1708,16f1709
16f1713,16f1716,16f1717,16f1718,16f1719,16f1782,16f1783,16f1784,16f1786,16f1787
16f1788,16f1789,16f1823,16f1824,16f1825,16f1826,16f1827,16f1828,16f1829,16f1847
16f1933,16f1934,16f1936,16f1937,16f1938,16f1939,16f1946,16f1947,16lf1554,16lf1559
16lf1704,16lf1708,16lf1902,16lf1903,16lf1904,16lf1906,16lf1907

################################################################################
#
# <id>:<head>{,<member>}
#
# Each line provides a colon separated list of
#
#  * a numeric family name, derived from the first family member as follows:
#
#    ADC     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u1", <num1>, <num2>)
#    CCP     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u2", <num1>, <num2>)
#    PWM     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u3", <num1>, <num2>)
#    I2C     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u4", <num1>, <num2>)
#    SPI     : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u5", <num1>, <num2>)
#    USART   : <num1>(c|f|hv|lf)<num2>.? -> printf("%u%04u6", <num1>, <num2>)
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

10032001:10f320,10f322
12061501:12f615
12061701:12f617
12067501:12f675
12068301:12f683
12075201:12f752
12150101:12f1501
12157101:12f1571
12157201:12f1572
12161201:12f1612
12182201:12f1822
12155201:12lf1552
16007100:16c71,16c710,16c711
16007200:16c72,16f72
16007300:16c73b,16c74b,16c745,16c765,16f73,16f76
16043300:16c433
16071500:16c715
16071700:16c717,16c770,16c771
16077300:16c773
16077400:16c774
16078100:16c781,16c782
16092500:16c925,16c926,16f872
16007401:16f74,16f77
16008801:16f88
16061601:16f616,16hv616
16067601:16f676,16f684
16067701:16f677,16f685
16068701:16f687,16f689,16f690
16068801:16f688
16070701:16f707
16071601:16f716
16072001:16f720,16f721
16072201:16f722,16f722a,16f723,16f723a,16f726
16072401:16f724,16f727
16073701:16f737,16f767
16074701:16f747,16f777
16075301:16f753,16hv753
16078501:16f785
16081801:16f818,16f819
16087001:16f870,16f873,16f876
16087101:16f871,16f874,16f877
16087301:16f873a,16f876a
16087401:16f874a,16f877a
16088201:16f882,16f883,16f886
16088401:16f884,16f887
16091301:16f913,16f916
16091401:16f914,16f917,16f946
16145501:16f1455
16145901:16f1459
16150301:16f1503
16150701:16f1507
16150801:16f1508,16f1509
16151201:16f1512,16f1513
16151601:16f1516,16f1518
16151701:16f1517,16f1519
16152601:16f1526,16f1527
16161301:16f1613
16170301:16f1703
16170401:16f1704,16f1705,16lf1704
16170701:16f1707
16170801:16f1708,16lf1708
16170901:16f1709
16171301:16f1713,16f1716,16f1718
16171701:16f1717,16f1719
16178201:16f1782,16f1783
16178401:16f1784,16f1787
16178601:16f1786
16178801:16f1788
16178901:16f1789
16182301:16f1823
16182401:16f1824,16f1825
16182601:16f1826,16f1827,16f1847
16182801:16f1828,16f1829
16193301:16f1933,16f1936,16f1938
16193401:16f1934,16f1937,16f1939
16194601:16f1946,16f1947
16155401:16lf1554
16155901:16lf1559
16190201:16lf1902,16lf1903
16190401:16lf1904,16lf1907
16190601:16lf1906

    SECTION=CCP

12061511:12f615
12061711:12f617
12068311:12f683
12075211:12f752
12161211:12f1612
12182211:12f1822
16006210:16c62
16006310:16c63a,16c65b
16007210:16c72,16f72
16007310:16c73b,16c74b,16f73,16f74,16f76,16f77
16071710:16c717,16c770,16c771
16074510:16c745,16c765
16077310:16c773,16c774
16092510:16c925,16c926,16f872
16008711:16f87
16008811:16f88
16061611:16f616,16hv616
16062711:16f627,16f627a,16f628,16f628a,16f648a
16068411:16f684
16068511:16f685
16068711:16f687,16f689
16069011:16f690
16070711:16f707
16071611:16f716
16072011:16f720,16f721
16072211:16f722,16f722a,16f723,16f723a,16f726
16072411:16f724,16f727
16073711:16f737,16f747,16f767,16f777
16075311:16f753,16hv753
16078511:16f785
16081811:16f818,16f819
16087011:16f870,16f871
16087311:16f873,16f874,16f876,16f877
16087311:16f873a,16f874a,16f876a,16f877a
16088211:16f882,16f883,16f886
16088411:16f884,16f887
16091311:16f913,16f916
16091411:16f914,16f917,16f946
16151211:16f1512,16f1513,16f1516,16f1518
16151711:16f1517,16f1519
16152611:16f1526,16f1527
16161311:16f1613
16170311:16f1703
16170411:16f1704,16f1705,16lf1704
16170711:16f1707
16170811:16f1708,16lf1708
16170911:16f1709
16171311:16f1713,16f1716,16f1718
16171711:16f1717,16f1719
16178211:16f1782,16f1783
16178411:16f1784,16f1787
16178611:16f1786
16178811:16f1788
16178911:16f1789
16182311:16f1823
16182411:16f1824
16182511:16f1825
16182611:16f1826
16182711:16f1827,16f1847
16182811:16f1828
16182911:16f1829
16193311:16f1933,16f1936,16f1938
16193411:16f1934,16f1937,16f1939
16194611:16f1946,16f1947

    SECTION=PWM

10032021:10f320,10f322
12061521:12f615
12061721:12f617
12068321:12f683
12075221:12f752
12150121:12f1501
12157121:12f1571
12157221:12f1572
12161221:12f1612
12182221:12f1822
16006220:16c62
16006320:16c63a,16c65b
16007220:16c72,16f72
16007320:16c73b,16c74b,16f73,16f74,16f76,16f77
16071720:16c717,16c770,16c771
16074520:16c745,16c765
16077320:16c773,16c774
16092520:16c925,16c926,16f872
16008721:16f87
16008821:16f88
16061621:16f616,16hv616
16062721:16f627,16f627a,16f628,16f628a,16f648a
16068421:16f684
16068521:16f685
16068721:16f687,16f689
16069021:16f690
16070721:16f707
16071621:16f716
16072021:16f720,16f721
16072221:16f722,16f722a,16f723,16f723a,16f726
16072421:16f724,16f727
16073721:16f737,16f747,16f767,16f777
16075321:16f753,16hv753
16078521:16f785
16081821:16f818,16f819
16087021:16f870,16f871
16087321:16f873,16f874,16f876,16f877
16087321:16f873a,16f874a,16f876a,16f877a
16088221:16f882,16f883,16f886
16088421:16f884,16f887
16091321:16f913,16f916
16091421:16f914,16f917,16f946
16145421:16f1454
16145521:16f1455
16145921:16f1459
16150321:16f1503
16150721:16f1507
16150821:16f1508,16f1509
16151221:16f1512,16f1513,16f1516,16f1518
16151721:16f1517,16f1519
16161321:16f1613
16170321:16f1703
16170421:16f1704,16f1705,16lf1704
16170721:16f1707
16170821:16f1708,16lf1708
16170921:16f1709
16171321:16f1713,16f1716,16f1718
16171721:16f1717,16f1719
16178221:16f1782,16f1783
16178421:16f1784,16f1787
16178621:16f1786
16178821:16f1788
16178921:16f1789
16182321:16f1823
16182421:16f1824
16182521:16f1825
16182621:16f1826
16182721:16f1827,16f1847
16182821:16f1828
16182921:16f1829
16193321:16f1933,16f1936,16f1938
16193421:16f1934,16f1937,16f1939
16194621:16f1946,16f1947
16155421:16lf1554
16155921:16lf1559

    SECTION=I2C

12182231:12f1822
12155231:12lf1552
16006230:16c62
16006330:16c63a,16c65b
16007230:16c72
16007330:16c73b,16c74b,16f73,16f74,16f76,16f77
16071730:16c717,16c770,16c771
16077330:16c773,16c774
16092530:16c925,16c926
16007231:16f72
16008731:16f87
16008831:16f88
16067731:16f677
16068731:16f687,16f689,16f690
16070731:16f707
16072031:16f720,16f721
16072231:16f722,16f722a,16f723,16f723a,16f726
16072431:16f724,16f727
16073731:16f737,16f747,16f767,16f777
16081831:16f818,16f819
16087231:16f872
16087331:16f873,16f874,16f876,16f877
16087331:16f873a,16f874a,16f876a,16f877a
16088231:16f882,16f883,16f886
16088431:16f884,16f887
16091331:16f913,16f916
16091431:16f914,16f917,16f946
16145431:16f1454
16145531:16f1455
16145931:16f1459
16150331:16f1503
16150831:16f1508,16f1509
16151231:16f1512,16f1513,16f1516,16f1518
16151731:16f1517,16f1519
16152631:16f1526,16f1527
16170331:16f1703
16170431:16f1704,16f1705,16lf1704
16170731:16f1707
16170831:16f1708,16lf1708
16170931:16f1709
16171331:16f1713,16f1716,16f1718
16171731:16f1717,16f1719
16178231:16f1782,16f1783
16178431:16f1784,16f1787
16178631:16f1786
16178831:16f1788
16178931:16f1789
16182331:16f1823
16182431:16f1824
16182531:16f1825
16182631:16f1826
16182731:16f1827,16f1847
16182831:16f1828
16182931:16f1829
16193331:16f1933,16f1936,16f1938
16193431:16f1934,16f1937,16f1939
16194631:16f1946,16f1947
16155431:16lf1554
16155931:16lf1559

    SECTION=SPI

12182241:12f1822
12155241:12lf1552
16006240:16c62
16006340:16c63a,16c65b
16007240:16c72
16007340:16c73b,16c74b,16f73,16f74,16f76,16f77
16071740:16c717,16c770,16c771
16077340:16c773,16c774
16092540:16c925,16c926
16007241:16f72
16008741:16f87
16008841:16f88
16067741:16f677
16068741:16f687,16f689,16f690
16070741:16f707
16072041:16f720,16f721
16072241:16f722,16f722a,16f723,16f723a,16f726
16072441:16f724,16f727
16073741:16f737,16f747,16f767,16f777
16081841:16f818,16f819
16087241:16f872
16087341:16f873,16f874,16f876,16f877
16087341:16f873a,16f874a,16f876a,16f877a
16088241:16f882,16f883,16f886
16088441:16f884,16f887
16091341:16f913,16f916
16091441:16f914,16f917,16f946
16145441:16f1454
16145541:16f1455
16145941:16f1459
16150341:16f1503
16150841:16f1508,16f1509
16151241:16f1512,16f1513,16f1516,16f1518
16151741:16f1517,16f1519
16152641:16f1526,16f1527
16170341:16f1703
16170441:16f1704,16f1705,16lf1704
16170741:16f1707
16170841:16f1708,16lf1708
16170941:16f1709
16171341:16f1713,16f1716,16f1718
16171741:16f1717,16f1719
16178241:16f1782,16f1783
16178441:16f1784,16f1787
16178641:16f1786
16178841:16f1788
16178941:16f1789
16182341:16f1823
16182441:16f1824
16182541:16f1825
16182641:16f1826
16182741:16f1827,16f1847
16182841:16f1828
16182941:16f1829
16193341:16f1933,16f1936,16f1938
16193441:16f1934,16f1937,16f1939
16194641:16f1946,16f1947
16155441:16lf1554
16155941:16lf1559

    SECTION=USART

12157251:12f1572
12182251:12f1822
16006350:16c63a,16c65b
16007350:16c73b,16c74b,16c745,16c765,16f73,16f74,16f76,16f77
16077350:16c773,16c774
16008751:16f87
16008851:16f88
16062751:16f627,16f627a,16f628,16f628a,16f648a
16068751:16f687,16f689,16f690
16068851:16f688
16070751:16f707
16072051:16f720,16f721
16072251:16f722,16f722a,16f723,16f723a,16f726
16072451:16f724,16f727
16073751:16f737,16f747,16f767,16f777
16087051:16f870,16f871,16f873,16f874,16f876,16f877
16087351:16f873a,16f874a,16f876a,16f877a
16088251:16f882,16f883,16f886
16088451:16f884,16f887
16091351:16f913,16f916
16091451:16f914,16f917,16f946
16145451:16f1454
16145551:16f1455
16145951:16f1459
16150851:16f1508,16f1509
16151251:16f1512,16f1513,16f1516,16f1518
16151751:16f1517,16f1519
16152651:16f1526,16f1527
16170451:16f1704,16f1705,16lf1704
16170851:16f1708,16lf1708
16170951:16f1709
16171351:16f1713,16f1716,16f1718
16171751:16f1717,16f1719
16178251:16f1782,16f1783
16178451:16f1784,16f1787
16178651:16f1786
16178851:16f1788
16178951:16f1789
16182351:16f1823
16182451:16f1824
16182551:16f1825
16182651:16f1826,16f1827,16f1847
16182851:16f1828
16182951:16f1829
16193351:16f1933,16f1936,16f1938
16193451:16f1934,16f1937,16f1939
16194651:16f1946,16f1947
16155451:16lf1554
16155951:16lf1559
16190451:16lf1904,16lf1907
16190651:16lf1906
