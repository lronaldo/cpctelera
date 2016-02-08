SHELL:=/bin/bash
INCDIR=include
SRCDIR=src
OBJDIR=obj
BINDIR=bin
LIBDIR=lib
MKDIR=mkdir -p
RM=rm -rf

CC=gcc
CXX=g++
CXXFLAGS=-std=c++0x -O3 -Werror -Wall -fsigned-char
LDLIBS=

.PHONY: showVars clean cleanall all

vpath %.hpp $(INCDIR)
vpath %.cpp $(SRCDIR)
vpath %.cc $(SRCDIR)
vpath %.o $(OBJDIR)

HEADERS = $(wildcard $(INCDIR)/*)
SRCS = $(wildcard $(SRCDIR)/*.cc) $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(SRCS))))
APPNAME = $(BINDIR)/dskgen

all: cygwinstaticbuild $(APPNAME)

$(APPNAME): $(BINDIR) $(OBJDIR) $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJDIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c -I$(INCDIR) $<

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c -I$(INCDIR) $<

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
	$(RM) $(BINDIR)
