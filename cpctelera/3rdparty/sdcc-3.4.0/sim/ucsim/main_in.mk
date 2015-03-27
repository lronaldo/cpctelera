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

EXEEXT		= @EXEEXT@

LIB_LIST	= ucsimutil cmd sim
UCSIM_LIBS	= -Wl,--start-group $(patsubst %,-l%,$(LIB_LIST)) -Wl,--end-group
UCSIM_LIB_FILES	= $(patsubst %,lib%.a,$(LIB_LIST))

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

OBJECTS         = pobj.o globals.o utils.o error.o app.o option.o
SOURCES		= $(patsubst %.o,%.cc,$(OBJECTS))
UCSIM_OBJECTS	= ucsim.o
UCSIM_SOURCES	= $(patsubst %.o,%.cc,$(UCSIM_OBJECTS))
ALL_SOURCES	= $(SOURCES) $(UCSIM_SOURCES)

enable_ucsim	= @enable_ucsim@


# Compiling entire program or any subproject
# ------------------------------------------
all: checkconf libs

libs: libucsimutil.a

main_app: checkconf ucsim_app

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
	ar -rcu $*.a $(OBJECTS)
	$(RANLIB) $*.a


ifeq ($(enable_ucsim),yes)
ucsim_app: libs ucsim
else
ucsim_app:
endif

ucsim: $(UCSIM_OBJECTS) $(UCSIM_LIB_FILES)
	echo $(UCSIM_LIB_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $< -L$(top_builddir) $(UCSIM_LIBS)

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
