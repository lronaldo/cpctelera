LIBNAME :=tmxlite.a
SHELL   :=/bin/bash
INCDIRS :=-Isrc -Iinclude
SRCDIR  :=src
SRCDIRS :=$(SRCDIR) $(SRCDIR)/detail
OBJDIR  :=obj
BINDIR  :=bin
LIBDIR  :=lib
MKDIR   :=mkdir -p
RM      :=rm -rf
LIBGEN  :=ar

CC      :=gcc
CXX     :=g++
CXXFLAGS:=-std=c++14 -O3 -Wall -fsigned-char -static -pedantic
CCFLAGS :=-O3 -Wall -fno-strict-aliasing -fsigned-char -static -pedantic
LDLIBS  :=

.PHONY: showVars clean cleanall all

vpath %.hpp $(INCDIR)
vpath %.h   $(INCDIR)
vpath %.cpp $(SRCDIR)
vpath %.cc $(SRCDIR)
vpath %.c $(SRCDIR)
vpath %.o $(OBJDIR)

HEADERS := $(wildcard $(INCDIR)/*)
SRCS    := $(foreach D,$(SRCDIRS),$(wildcard $(D)/*.cc) $(wildcard $(D)/*.cpp) $(wildcard $(D)/*.c))
OBJS    := $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SRCS)))))
APP     := $(BINDIR)/$(LIBNAME)

all: cygwinstaticbuild $(APP)

$(APP): $(BINDIR) $(OBJDIR) $(OBJS)
#	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	$(LIBGEN) rcs $(APP) $(OBJS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -o $@ -c $(INCDIRS) $<

$(OBJDIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $(INCDIRS) $<

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $(INCDIRS) $<

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
	$(MKDIR) $(foreach D,$(SRCDIRS),$(D:$(SRCDIR)%=$(OBJDIR)%))
	
clean:
	$(RM) $(OBJDIR)

cleanall: clean
	$(RM) $(APP)
