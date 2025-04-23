STARTYEAR	= 1997

SHELL		= /bin/sh
CXX		= @CXX@
CXXCPP		= @CXXCPP@
RANLIB		= @RANLIB@
INSTALL		= @INSTALL@
STRIP		= @STRIP@
MAKEDEP         = @MAKEDEP@
AR		= @AR@
LEX		= @LEX@
YACC		= @YACC@
transform	= @program_transform_name@

UTLDIR		= src/core/utils.src
SIMDIR		= src/core/sim.src
CMDDIR		= src/core/cmd.src
GUIDIR		= src/core/gui.src
MOTDIR		= src/core/motorola.src

PICOPT		= @PICOPT@
CPPFLAGS        = @CPPFLAGS@ \
		  -I. -I$(srcdir) \
                  -I$(top_builddir)/$(UTLDIR) \
                  -I$(top_srcdir)/$(UTLDIR) \
                  -I$(top_srcdir)/$(SIMDIR) \
		  -I$(top_srcdir)/$(CMDDIR) \
		  -I$(top_srcdir)/$(GUIDIR) \
		  -I$(top_srcdir)/$(MOTDIR)

SILENT		= @SILENT@
OPT		?= 2
CFLAGS          = @WALL_FLAG@ @CFLAGS@ -O$(OPT)
CXXFLAGS        = @WALL_FLAG@ @CXXFLAGS@ -O$(OPT) $(PICOPT)
LDFLAGS		= @LDFLAGS@
LIBS		= @LIBS@
CURSES_LIBS	= @CURSES_LIBS@

WINSOCK_AVAIL   = @WINSOCK_AVAIL@
LDFLAGS		= @LDFLAGS@

EXEEXT		= @EXEEXT@
UCSIM_LIBS	= -lmotorola -lsim -lucsimutil -lgui -lcmd -lsim \
		  $(CURSES_LIBS) $(LIBS)
UCSIM_LIB_PREREQUES = \
		  $(top_builddir)/libmotorola.a \
		  $(top_builddir)/libcmd.a \
		  $(top_builddir)/libgui.a \
		  $(top_builddir)/libsim.a \
		  $(top_builddir)/libucsimutil.a

UCSIM_LIB_TARGETS = \
		motorola_lib \
		cmd_lib \
		gui_lib \
		sim_lib \
		ucsimutil_lib

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

curses_ok	= @curses_ok@
enable_dlso	= @enable_dlso@
dlso_ok		= @dlso_ok@

ALL_OBJECTS	= $(OBJECTS) $(LOCAL_OBJECTS)
SOURCES		= $(patsubst %.o,%.cc,$(ALL_OBJECTS))

.cc.o:
ifeq ($(SILENT),yes)
	@echo CXX $(PKG)-$@
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@
else
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c $< -o $@
endif

.l.cc:
	rm -f $@
	$(LEX) -t $< > $@

.y.cc:
	$(YACC) -d $<
	mv y.tab.c $*.cc
	mv y.tab.h $*.hh


SEN		= s$(PKG)$(EXEEXT)
LEN		= ucsim_$(PKG)$(EXEEXT)
SONAME		= libucsim_$(PKG).so
