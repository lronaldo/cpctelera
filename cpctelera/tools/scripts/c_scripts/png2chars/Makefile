APPNAME :=cpct_png2chars
SHELL   :=/bin/bash
SRCDIR  :=src
OBJDIR  :=obj
BINDIR  :=bin
LIBDIR  :=libs
INCDIRS :=src
MKDIR   :=mkdir -p
RM      :=rm -rf

CC      :=gcc
CXX     :=g++
CXXFLAGS:=-std=c++14 -Wall -fsigned-char -pedantic
LDLIBS  :=
SUBLIBS :=

.PHONY: showVars clean cleanall all

vpath %.hpp $(INCDIRS)
vpath %.h   $(INCDIRS)
vpath %.cpp $(SRCDIR)
vpath %.o $(OBJDIR)

HEADERS := $(foreach D,$(INCDIRS),$(shell find $(D)/ -type f -name '*.h') $(shell find $(D)/ -type f -name '*.hpp'))
SRCS    := $(shell find $(SRCDIR) -type f -iname '*.cpp')
OBJS    := $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.cpp,%.o,$(SRCS)))
APP     := $(BINDIR)/${APPNAME}
INCMODS := $(foreach D,$(INCDIRS),-I$(D))
SUBLIBOBJS := $(foreach L,$(SUBLIBS),$(LIBDIR)/$(L)/bin/$(L).a)
SRCSUBDIRS := $(filter-out $(SRCDIR),$(shell find $(SRCDIR) -type d))
OBJSUBDIRS := $(OBJDIR) $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(SRCSUBDIRS))

all: cygwinstaticbuild $(APP)

$(APP): $(SUBLIBOBJS) $(BINDIR) $(OBJSUBDIRS) $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) $(SUBLIBOBJS)

$(OBJDIR)/%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -o $@ -c $(INCMODS) $<

%.a:
	@echo "*****************************************************************"
	@echo "*** COMPILANDO LIBRERIA: $(basename $(notdir $@))"
	@echo
	$(MAKE) -C $(LIBDIR)/$(basename $(notdir $@))/
	@echo 
	@echo "*** $(basename $(notdir $@)) COMPILADA CON ÉXITO"
	@echo "*****************************************************************"
	@echo

cygwinstaticbuild:
	$(eval LDLIBS+=$(shell if [[ "$$(uname)" =~ "CYGWIN" ]]; then echo '-static-libstdc++'; fi))

showVars:
	$(info %.o)
	$(info $(HEADERS))
	$(info $(SRCS))
	$(info $(OBJS))
	$(info $(SRCSUBDIRS))
	$(info $(OBJSUBDIRS))

$(BINDIR): 
	@$(MKDIR) $(BINDIR)

$(OBJSUBDIRS): 
	@$(MKDIR) "$@"
	
clean:
	$(RM) $(OBJDIR)

cleanall: clean
	$(RM) $(APP)
	$(foreach L,$(SUBLIBS),$(MAKE) cleanall -C $(LIBDIR)/$(L)/)
