#
# uCsim  simulator Makefile
#
# (c) Drotos Daniel, Talker Bt. 1997
#

srcdir          = @srcdir@
VPATH           = @srcdir@
top_builddir	= @top_builddir@
top_srcdir	= @top_srcdir@

include $(srcdir)/objs.mk
include ../../common.mk


# Compiling entire program or any subproject
# ------------------------------------------
all: $(LEN)

include $(top_srcdir)/src/app.mk

local_install:

local_uninstall:

# Creating dependencies
# ---------------------
dep: Makefile.dep

Makefile.dep: $(srcdir)/*.cc $(srcdir)/*.h objs.mk
	$(MAKEDEP) $(CPPFLAGS) $(filter %.cc,$^) >Makefile.dep

-include Makefile.dep
include $(srcdir)/clean.mk


# End of simulator Makefile
