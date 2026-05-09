# General Makefile for ADFlib files

LIBDIR=Lib
TESTDIR=Test
DEMODIR=Demo
LIBNAME=libadf.a

LIBVERSION=0.7.11a
LIBDATE="January 20th, 2007"

#to generate unadf-1.0-1.i386.rpm
VERSION=1.0
RELEASE=1
INSTDIR=/usr/local/unadf

all: demo tests lib

lib: $(LIBDIR)/$(LIBNAME)
	cp $(LIBDIR)/$(LIBNAME) ./Bin/Linux/

$(LIBDIR)/$(LIBNAME): Lib/defendian.h
	cd $(LIBDIR) && $(MAKE)

clean:
	cd $(LIBDIR) && ($(MAKE) clean || true)
	cd $(TESTDIR) && ($(MAKE) clean || true)
	cd $(DEMODIR) && ($(MAKE) clean || true)
	rm -rf PKG *~ unadf*.rpm
	rm -f unadf-${VERSION}.spec

dep Lib/defendian.h:
	cd $(LIBDIR) && ($(MAKE) dep || true)
	cd $(TESTDIR) && ($(MAKE) dep || true)
	cd $(DEMODIR) && ($(MAKE) dep || true)

zip:
	($(MAKE) clean) || true
	(rm adflib.zip) || true
	zip -9r adflib.zip Lib Docs Faq Bin/Linux gen_spec.sh FilesToInstall \
		Boot Demo Makefile README.txt COPYING.txt CHANGES.txt AUTHORS.txt *.dsp *.dsw

dist:	zip	
	cd Dist && dist.sh

preprpm: demo
	mkdir -p PKG/${INSTDIR}
	cp Demo/unadf PKG/${INSTDIR}
	strip PKG/${INSTDIR}/unadf

rpm:	unadf-${VERSION}.spec preprpm 
	rpmbuild -bb unadf-${VERSION}.spec
	cp /usr/src/redhat/RPMS/i386/unadf-${VERSION}-${RELEASE}.i386.rpm .

unadf-${VERSION}.spec: gen_spec.sh
	./gen_spec.sh ${VERSION} ${RELEASE} unadf FilesToInstall >unadf-${VERSION}.spec

zipdump:
	(rm adfdump.zip) || true
	zip -9r dumps.zip Dumps Test

binw32:
	(rm binw32.zip) || true
	zip -9r binw32.zip Bin/Win32

backup:
	cp -f adflib.zip dumps.zip /mnt/winc/Laurent/

demo $(DEMODIR)/unadf: $(LIBDIR)/$(LIBNAME)
	cd $(DEMODIR) &&  $(MAKE)

tests: $(LIBDIR)/$(LIBNAME)
	cd $(TESTDIR) && $(MAKE)

	echo
	echo "floppy tests"
	cd $(TESTDIR) && ./floppy.sh

	echo 
	echo "big devices tests"
	cd $(TESTDIR) && ./bigdev.sh


