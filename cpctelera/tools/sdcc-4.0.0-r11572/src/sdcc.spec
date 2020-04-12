#
# spec file for package sdcc
#

Name:          sdcc
Requires:      /bin/sh
License:       GPL
Group:         Applications/Engineering
Summary:       Small Device C Compiler
Requires:      sdcc-common
Version:       4.0.0
Release:       2
Source:        %{name}-src-%{version}.tar.gz
URL:           http://sdcc.sourceforge.net/
Packager:      Maarten Brock <sourceforge.brock AT dse.nl>
BuildRoot:     %{_tmppath}/%{name}-%{version}-build

%description
SDCC is a free open source, retargettable, optimizing ANSI C compiler
suite that targets a growing list of processors including the Intel
MCS51 based microprocessors (8031, 8032, 8051, 8052, etc.), Maxim
(formerly Dallas) DS80C390 variants, Freescale (formerly Motorola)
HC08 based (hc08, s08), Zilog Z80 based MCUs (z80, z180, gbz80,
Rabbit 2000/3000, Rabbit 3000A, TLCS-90) and the STM8.
Work is in progress on supporting the Microchip PIC16 and PIC18 targets.
It can be retargeted for other microprocessors.

%package common
License:       GPL, LGPL
Group:         Application/Engineering
Summary:       Libraries and Header Files for the SDCC C compiler
Requires:      sdcc

%description common
SDCC is a free open source, retargettable, optimizing ANSI C compiler
suite that targets a growing list of processors including the Intel
MCS51 based microprocessors (8031, 8032, 8051, 8052, etc.), Maxim
(formerly Dallas) DS80C390 variants, Freescale (formerly Motorola)
HC08 based (hc08, s08) and Zilog Z80 based MCUs (z80, z180, gbz80,
Rabbit 2000/3000, Rabbit 3000A). Work is in progress on supporting
the Microchip PIC16 and PIC18 targets. It can be retargeted for other
microprocessors.

%package doc
License:       GPL
Group:         Applications/Engineering
Summary:       Documentation for the SDCC C compiler

%description doc
SDCC is a free open source, retargettable, optimizing ANSI C compiler
suite that targets a growing list of processors including the Intel
MCS51 based microprocessors (8031, 8032, 8051, 8052, etc.), Maxim
(formerly Dallas) DS80C390 variants, Freescale (formerly Motorola)
HC08 based (hc08, s08), Zilog Z80 based MCUs (z80, z180, gbz80,
Rabbit 2000/3000, Rabbit 3000A, TLCS-90) and the STM8.
Work is in progress on supporting the Microchip PIC16 and PIC18 targets.
It can be retargeted for other microprocessors.

%prep
%setup -n sdcc

%build
./configure CFLAGS="$RPM_OPT_FLAGS" --prefix=/usr --enable-doc docdir=$RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}
make
cd doc
make
cd ..

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT/usr install
cd doc
make install
cd ..

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin

%files common
%defattr(-,root,root)
/usr/share/sdcc/include
/usr/share/sdcc/non-free/include
/usr/share/sdcc/lib
/usr/share/sdcc/non-free/lib

%files doc
%defattr(-,root,root)
%doc %{_defaultdocdir}

%changelog
* Sat Jan 18 2029 - pkk AT spth.de
- version updated to 4.0.0
* Fri Apr 05 2019 - krauseph AT informatik.uni-freiburg.de
- version updated to 3.9.0
* Sun Sep 16 2018 - pkk AT spth.de
- version updated to 3.8.0
* Wed Feb 07 2018 - pkk AT spth.de
- version updated to 3.7.0
* Sun May 29 2016 - sourceforge.brock AT dse.nl
- version updated to 3.6.0
* Fri Jun 06 2015 - sourceforge.brock AT dse.nl
- version updated to 3.5.0
* Fri Feb 28 2014 - sourceforge.brock AT dse.nl
- version updated to 3.4.0
* Sat May 04 2013 - sourceforge.brock AT dse.nl
- version updated to 3.3.0
* Fri Jun 12 2012 - borut.razem AT siol.net
- version updated to 3.2.0
* Sat Oct 08 2010 - borut.razem AT siol.net
- added non-free include and lib directories
- version updated to 3.1.0
* Sun Dec 26 2006 - bernhard AT bernhardheld.de
- version updated
* Sun Sep 03 2006 - Christer Weinigel
- fixed build of doc
* Tue Mar 09 2004 - bernhard AT bernhardheld.de
- split into two packages
* Wed Feb 26 2004 - bernhard AT bernhardheld.de
- version updated
* Sun Jan 04 2004 - bernhard AT bernhardheld.de
- updated
* Sat Apr 27 2002 - steve AT icarus.com
- first version from Stephen Williams
