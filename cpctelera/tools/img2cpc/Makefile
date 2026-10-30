##
## img2cpc Makefile. No external libraries required.
## Builds on Linux, macOS and Cygwin with any C++20 compiler (GCC 10+, Clang 10+).
##
INCDIR=include
SRCDIR=src
OBJDIR=obj
BINDIR=bin
MKDIR=mkdir

CXXFLAGS=-std=c++20 -O3 -Wall -fsigned-char
LDLIBS=

## Cygwin: link libstdc++ statically so img2cpc.exe does not depend on its DLL
ifneq ($(findstring CYGWIN,$(shell uname -s)),)
LDLIBS+=-static-libstdc++
endif

vpath %.hpp $(INCDIR)
vpath %.cpp $(SRCDIR)
vpath %.cc $(SRCDIR)
vpath %.o $(OBJDIR)

HEADERS = $(wildcard $(INCDIR)/*.hpp $(INCDIR)/*.h $(INCDIR)/json/*.h)
SRCS = $(wildcard $(SRCDIR)/*.cc) $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(SRCS))))
APPNAME = $(BINDIR)/img2cpc

.PHONY: all createFolders showVars clean cleanall

all: createFolders $(APPNAME)

$(APPNAME): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OBJDIR)/%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c -I$(INCDIR) $<

$(OBJDIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ -c -I$(INCDIR) $<

$(OBJS): | createFolders

showVars:
	$(info $(HEADERS))
	$(info $(SRCS))
	$(info $(OBJS))

createFolders:
	@$(MKDIR) -p $(OBJDIR)
	@$(MKDIR) -p $(BINDIR)

clean:
	$(RM) $(OBJDIR)/*

cleanall: clean
	$(RM) $(BINDIR)/*
