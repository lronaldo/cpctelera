APPNAME :=cpct_tmx2data
SHELL   :=/bin/bash
SRCDIR  :=src
OBJDIR  :=obj
BINDIR  :=bin
LIBDIR  :=libs
INCDIRS :=src $(LIBDIR)/tmxlite/include
MKDIR   :=mkdir -p
RM      :=rm -rf

CC      :=gcc
CXX     :=g++
CXXFLAGS:=-std=c++11 -O3 -Werror -Wall -fsigned-char
LDLIBS  :=

.PHONY: showVars clean cleanall all

vpath %.hpp $(INCDIRS)
vpath %.h   $(INCDIRS)
vpath %.cpp $(SRCDIR)
vpath %.cc $(SRCDIR)
vpath %.o $(OBJDIR)

HEADERS := $(foreach D,$(INCDIRS),$(shell find $(D)/ -type f -name '*.h') $(shell find $(D)/ -type f -name '*.hpp'))
SRCS    := $(wildcard $(SRCDIR)/*.cc) $(wildcard $(SRCDIR)/*.cpp)
OBJS    := $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(SRCS))))
APP     := $(BINDIR)/${APPNAME}
INCMODS := $(foreach D,$(INCDIRS),-I$(D))

all: cygwinstaticbuild $(APP)

$(APP): $(BINDIR) $(OBJDIR) $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJDIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $(INCMODS) $<

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $(INCMODS) $<

cygwinstaticbuild:
	$(eval LDLIBS+=$(shell if [[ "$$(uname)" =~ "CYGWIN" ]]; then echo '-static-libstdc++'; fi))

showVars:
	$(info %.o)
	$(info $(HEADERS))
	$(info $(SRCS))
	$(info $(OBJS))

$(BINDIR): 
	@$(MKDIR) $(BINDIR)

$(OBJDIR): 
	@$(MKDIR) $(OBJDIR)
	
clean:
	$(RM) $(OBJDIR)

cleanall: clean
	$(RM) $(APP)
