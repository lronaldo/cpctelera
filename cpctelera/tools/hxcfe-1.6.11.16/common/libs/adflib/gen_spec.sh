#!/bin/sh


cat <<EOF
Summary: unzip like for .adf files (Amiga devices dumps)
Name: $3
Version: $1
Release: $2
URL: http://lclevy.free.fr/adflib
Source0: %{Name}-%{Version}.tar.gz
Patch0:/dev/null
License: GPL
Group: Development/Tools
EOF

echo "BuildRoot: `pwd`/PKG"

cat <<EOF
Packager: lclevy@club-internet.fr
Prefix: /usr/local/unadf

%description
unzip like for .adf files (Amiga devices dumps)
powered by ADFLib
EOF

echo "Based on $tag of unadf"

cat <<EOF
%prep
%build
#A binary package is not built
%install
#Here put the command to run the install
%clean
#Binary package does not need to be cleaned
%files -f `pwd`/$4
%defattr(-,root,root)
%post
%postun
#cp -Rf /usr/src/anl/AAs /usr/src/anl/AAs_old
%changelog
* Sat Jan 20 2007 Laurent Clevy <lclevy@club-internet.fr> 1.0:
  - stable version of unadf

