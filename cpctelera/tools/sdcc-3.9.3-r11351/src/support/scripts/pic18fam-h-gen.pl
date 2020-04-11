#!/usr/bin/perl -w

#
# This script generates a C header file that maps the target device (as
# indicated via the sdcc generated -Dpic18fxxx macro) to its device
# family and the device families to their respective style of ADC and
# USART programming for use in the SDCC PIC16 I/O library.
#
# Copyright 2010 Raphael Neider <rneider AT web.de>
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
# Usage: perl pic18fam-h-gen.pl
#
# This will create pic18fam.h.gen in your current directory.
# Check sanity of the file and move it to .../include/pic16/pic18fam.h.
# If you assigned new I/O styles, implement them in
# .../include/pic16/{adc,i2c,usart}.h and
# .../lib/pic16/libio/*/*.c
#

use strict;

my $head = '__SDCC_PIC';

my %families = ();
my %adc = ();
my %usart = ();
my $update = "Please update your pic16/pic18fam.h manually and/or inform the maintainer.";

while (<DATA>) {
    chomp;
    s/\s*#.*$//;                # remove comments
    s/\s*//g;                   # strip whitespace
    next if (/^\s*$/);         # ignore empty lines

    my $line = $_;

    my @fields = split(/:/, $line);

    die "Invalid record >$line<" if (4 != scalar @fields);

    my ($id, $memberlist, $adcstyle, $usartstyle) = @fields;

    # extract numeric family id
    $id = 0+$id;

    # extract family members
    my @arr = split(/,/, $memberlist);
    @arr = sort(map { uc($_); } @arr);
    $families{$id} = \@arr;

    # ADC style per device family
    $adcstyle = 0+$adcstyle;
    if (not defined $adc{$adcstyle}) {
        $adc{$adcstyle} = [];
    } # if
    push @{$adc{$adcstyle}}, $id;

    # (E)USART style per device family
    $usartstyle = 0+$usartstyle;
    if (not defined $usart{$usartstyle}) {
        $usart{$usartstyle} = [];
    } # if
    push @{$usart{$usartstyle}}, $id;
}

my $fname = "pic18fam.h.gen";
open(FH, ">", "$fname") or die "Could not open >$fname<";

print FH <<EOT
/*
 * pic18fam.h - PIC16 families
 *
 * This file is has been generated using $0 .
 */
#ifndef __SDCC_PIC18FAM_H__
#define __SDCC_PIC18FAM_H__ 1

/*
 * Define device families.
 */
#undef  __SDCC_PIC16_FAMILY

EOT
;
my $pp = "#if   ";
for my $id (sort keys %families) {
    my $list = $families{$id};
    my $memb = "defined($head" . join(") \\\n    || defined($head", @$list) . ")";
    print FH <<EOT
${pp} ${memb}
#define __SDCC_PIC16_FAMILY ${id}

EOT
;
    $pp = "#elif ";
} # for
print FH <<EOT
#else
#warning No family associated with the target device. ${update}
#endif

/*
 * Define ADC style per device family.
 */
#undef  __SDCC_ADC_STYLE

EOT
;
$pp = "#if   ";
for my $s (sort keys %adc) {
    my $fams = join (" \\\n    || ", map { "(__SDCC_PIC16_FAMILY == $_)" } sort @{$adc{$s}});
    print FH <<EOT
${pp} ${fams}
#define __SDCC_ADC_STYLE    ${s}

EOT
;
    $pp = "#elif ";
} # for
print FH <<EOT
#else
#warning No ADC style associated with the target device. ${update}
#endif

/*
 * Define (E)USART style per device family.
 */
#undef  __SDCC_USART_STYLE

EOT
;
$pp = "#if   ";
for my $s (sort keys %usart) {
    my $fams = join (" \\\n    || ", map { "(__SDCC_PIC16_FAMILY == $_)" } sort @{$usart{$s}});
    print FH <<EOT
${pp} ${fams}
#define __SDCC_USART_STYLE  ${s}

EOT
;
    $pp = "#elif ";
} # for
print FH <<EOT
#else
#warning No (E)USART style associated with the target device. ${update}
#endif

EOT
;

print FH <<EOT
#endif /* !__SDCC_PIC18FAM_H__ */
EOT
;
__END__
#
# <id>:<head>{,<member>}:<adc>:<usart>
#
# Each line provides a colon separated list of
#  * a numeric family name, derived from the first family member as follows:
#    - 18F<num>         -> printf("18%04d0", <num>)
#    - 18F<num1>J<num2> -> printf("18%02d%02d1", <num1>, <num2>)
#    - 18F<num1>K<num2> -> printf("18%02d%02d2", <num1>, <num2>)
#  * a comma-separated list of members of a device family,
#    where a family comprises all devices that share a single data sheet,
#  * the ADC style (numeric family name or 0, if not applicable)
#  * the USART style (numeric family name or 0, if not applicable)
#
# This data has been gathered manually from data sheets published by
# Microchip Technology Inc.
#
1812200:18f1220,18f1320:1812200:1812200
1812300:18f1230,18f1330:1812300:1812300
1813502:18f13k50,18f14k50:1813502:1813502
1822200:18f2220,18f2320,18f4220,18f4320:1822200:1822200
1822210:18f2221,18f2321,18f4221,18f4321:1822200:1822210
1823310:18f2331,18f2431,18f4331,18f4431:0:1822210
1823202:18f23k20,18f24k20,18f25k20,18f26k20,18f43k20,18f44k20,18f45k20,18f46k20:1822200:1822210
1823222:18f23k22,18f24k22,18f25k22,18f26k22,18f43k22,18f44k22,18f45k22,18f46k22:1823222:1822210
1824100:18f2410,18f2510,18f2515,18f2610,18f4410,18f4510,18f4515,18f4610:1822200:1822210
1802420:18f242,18f252,18f442,18f452:1802420:1822200                             # TODO: verify family members and USART
1824200:18f2420,18f2520,18f4420,18f4520:1822200:1822210
1824230:18f2423,18f2523,18f4423,18f4523:1822200:1822210
1824500:18f2450,18f4450:1822200:1824500
1824550:18f2455,18f2550,18f4455,18f4550:1822200:1822210
1802480:18f248,18f258,18f448,18f458:1802420:1822200                             # TODO: verify family members and USART
1824800:18f2480,18f2580,18f4480,18f4580:1822200:1824500
1824101:18f24j10,18f25j10,18f44j10,18f45j10:1822200:1822210
1824501:18f24j50,18f25j50,18f26j50,18f44j50,18f45j50,18f46j50:1824501:1824501
1825250:18f2525,18f2620,18f4525,18f4620:1822200:1822210
1825850:18f2585,18f2680,18f4585,18f4680:1822200:1824500
1826820:18f2682,18f2685,18f4682,18f4685:1822200:1824500
1865200:18f6520,18f6620,18f6720,18f8520,18f8620,18f8720:1822200:1865200
1865270:18f6527,18f6622,18f6627,18f6722,18f8527,18f8622,18f8627,18f8722:1822200:1824501
1865850:18f6585,18f6680,18f8585,18f8680:1822200:1865850
1865501:18f65j50,18f66j50,18f66j55,18f67j50,18f85j50,18f86j50,18f86j55,18f87j50:1865501:1824501
1866601:18f66j60,18f66j65,18f67j60,18f86j60,18f86j65,18f87j60,18f96j60,18f96j65,18f97j60:1822200:1824501
