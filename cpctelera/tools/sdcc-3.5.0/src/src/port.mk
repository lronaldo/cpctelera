# Common Makefile for all ports.
# Input: SOURCES - list of C files included in the project
#        SPECIAL - list of special files that should be included in dependencies
#        PEEPRULES - list of all peephole rules (.rul) derrived files
#        PREBUILD - list of special files to build before deps.

# Ports are always located in sdcc/src/<portname>

# Output
LIB = port.a
# Include the sdcc/src directory
INCLUDEFLAGS = -I$(srcdir)/.. -I..

# If the sources aren't specified, assume all in this directory.
ifndef SOURCES
SOURCES = $(notdir $(wildcard $(srcdir)/*.c))
endif
ifndef CXXSOURCES
CXXSOURCES = $(notdir $(wildcard $(srcdir)/*.cc))
endif

# If the peephole rules aren't specified, assume all.
ifndef PEEPRULES
PEEPDEFS = $(notdir $(wildcard $(srcdir)/*.def))
PEEPRULES = $(PEEPDEFS:.def=.rul)
endif

PREBUILD += $(PEEPRULES)

all: $(PREBUILD) dep $(LIB)

include $(top_builddir)/Makefile.common

$(LIB): $(OBJ)
	rm -f $(LIB)
	$(AR) rc $(LIB) $(OBJ)
	$(RANLIB) $(LIB)

%.rul: %.def
	$(AWK) -f $(srcdir)/../SDCCpeeph.awk $< > $@

dep: Makefile.dep

Makefile.dep: $(PREBUILD) Makefile $(SOURCES) $(CXXSOURCES) $(SPECIAL)
	$(MAKEDEP) $(CPPFLAGS) $(filter %.c %.cc,$^) >Makefile.dep

# don't include Makefile.dep for the listed targets:
ifeq "$(findstring $(MAKECMDGOALS),clean distclean)" ""
  -include Makefile.dep
endif

include $(srcdir)/../port-clean.mk
