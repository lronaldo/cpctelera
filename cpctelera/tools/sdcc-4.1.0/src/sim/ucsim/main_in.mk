#
# S51 main.mk
#
# (c) Drotos Daniel, Talker Bt. 1997,99
#

STARTYEAR	= 1997

SHELL		= /bin/sh
CXX		= @CXX@
#CPP		= @CPP@
CXXCPP		= @CXXCPP@
RANLIB		= @RANLIB@
INSTALL		= @INSTALL@
MAKEDEP         = @MAKEDEP@
AR		= @AR@

top_builddir	= @top_builddir@
top_srcdir	= @top_srcdir@
SIMDIR		= sim.src
CMDDIR		= cmd.src
GUIDIR		= gui.src

DEFS            = $(subs -DHAVE_CONFIG_H,,@DEFS@)
# FIXME: -Imcs51 must be removed!!!
CPPFLAGS        = @CPPFLAGS@ -I$(top_builddir) -I$(srcdir) \
                  -I$(top_srcdir)/$(SIMDIR) \
		  -I$(top_srcdir)/$(CMDDIR) -I$(top_srcdir)/$(GUIDIR)
CFLAGS          = @CFLAGS@ -I$(top_builddir) @WALL_FLAG@
CXXFLAGS        = @CXXFLAGS@ -I$(top_builddir) @WALL_FLAG@
WINSOCK_AVAIL   = @WINSOCK_AVAIL@
LDFLAGS		= @LDFLAGS@

EXEEXT		= @EXEEXT@

LIB_LIST	= sim ucsimutil guiucsim cmd ucsimutil sim
UCSIM_LIBS	= $(patsubst %,-l%,$(LIB_LIST)) @LIBS@
UCSIM_LIB_FILES	= $(patsubst %,lib%.a,$(LIB_LIST))

RELAY_LIBS	= $(patsubst %,-l%,ucsimutil) @LIBS@
RELAY_LIB_FILES	= $(patsubst %,lib%.a,ucsimutil)

prefix          = @prefix@
exec_prefix     = @exec_prefix@
bindir          = @bindir@
libdir          = @libdir@
datadir         = @datadir@
datarootdir     = @datarootdir@
includedir      = @includedir@
mandir          = @mandir@
man1dir         = $(mandir)/man1
man2dir         = $(mandir)/man2
infodir         = @infodir@
srcdir          = @srcdir@
VPATH           = @srcdir@

OBJECTS         = pobj.o globals.o utils.o error.o app.o option.o chars.o fio.o
ifeq ($(WINSOCK_AVAIL), 1)
OBJECTS		+= fwio.o
else
OBJECTS		+= fuio.o
endif
SOURCES		= $(patsubst %.o,%.cc,$(OBJECTS))
UCSIM_OBJECTS	= ucsim.o
UCSIM_SOURCES	= $(patsubst %.o,%.cc,$(UCSIM_OBJECTS))
RELAY_OBJECTS	= relay.o
RELAY_SOURCES	= $(patsubst %.o,%.cc,$(RELAY_OBJECTS))
ALL_SOURCES	= $(SOURCES) $(UCSIM_SOURCES) $(RELAY_SSOURCES)

enable_ucsim	= @enable_ucsim@


# Compiling entire program or any subproject
# ------------------------------------------
all: checkconf libs

libs: libucsimutil.a

main_app: checkconf ucsim_app relay_app

# Compiling and installing everything and runing test
# ---------------------------------------------------
install: all installdirs


# Deleting all the installed files
# --------------------------------
uninstall:
	rm -f $(DESTDIR)$(bindir)/s51$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/savr$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/serialview$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/portmon$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/sstm8$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/sst7$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/sp1516$(EXEEXT)
	rm -f $(DESTDIR)$(bindir)/sm6809$(EXEEXT)


# Performing self-test
# --------------------
check:

test:

# Performing installation test
# ----------------------------
installcheck:


# Creating installation directories
# ---------------------------------
installdirs:


# Creating dependencies
# ---------------------
dep: main.dep

main.dep: $(ALL_SOURCES) *.h $(srcdir)/*.h
	$(MAKEDEP) $(CPPFLAGS) $(filter %.cc,$^) >main.dep

-include main.dep
include $(srcdir)/clean.mk

#parser.cc: parser.y

#plex.cc: plex.l

# My rules
# --------
libucsimutil.a: $(OBJECTS)
	$(AR) -rc $@ $(OBJECTS)
	$(RANLIB) $@


ifeq ($(enable_ucsim),yes)
ucsim_app: libs ucsim$(EXEEXT)
else
ucsim_app:
endif

ftest_app: libs ftest$(EXEEXT)

relay_app: libs relay$(EXEEXT)

ucsim$(EXEEXT): $(UCSIM_OBJECTS) $(UCSIM_LIB_FILES)
	echo $(UCSIM_LIB_FILES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -L$(top_builddir) $(UCSIM_LIBS) -o $@ 

ftest$(EXEEXT): ftest.o libucsimutil.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -L$(top_builddir) -lucsimutil @LIBS@

relay$(EXEEXT): $(RELAY_OBJECTS) $(RELAY_LIB_FILES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -L$(top_builddir) $(RELAY_LIBS) -o $@


ptt: ptt.o
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread
.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@


# Remaking configuration
# ----------------------
checkconf:
	@if [ -f devel ]; then\
	  $(top_srcdir)/mkecho $(top_builddir) "MAIN.MK checkconf";\
	  $(MAKE) -f conf.mk srcdir="$(srcdir)" freshconf;\
	fi

# End of main_in.mk/main.mk
