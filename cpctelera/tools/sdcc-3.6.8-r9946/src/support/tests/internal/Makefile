TOPDIR = ../../..

LIBSOURCE = \
	$(TOPDIR)/src/SDCCutil.c \
	$(TOPDIR)/src/SDCChasht.c \
	$(TOPDIR)/support/util/NewAlloc.c \
	$(TOPDIR)/support/util/SDCCerr.c \
	$(TOPDIR)/src/SDCCmacro.c \
	stubs.c

SOURCES = testpaths.c $(LIBSOURCE)

include $(TOPDIR)/Makefile.common

CFLAGS += -I$(TOPDIR)/src -I$(TOPDIR)/support/util -I$(TOPDIR)

all: all-tests

all-tests: $(OBJ)
	$(CC) -o $@ $(OBJ)

