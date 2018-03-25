SRCDIR  = src
BINDIR  = bin
OBJDIR  = obj
COMP    = g++
TARGET  = $(BINDIR)/iDSK
SRCFILES= $(wildcard $(SRCDIR)/*.cpp)
OBJFILES= $(foreach F, $(SRCFILES), $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(F:%.cpp=%.o)))
CPFLAGS = -std=c++98 -O3 -Wall -pedantic -fsigned-char
LDLIBS  = 

.PHONY: clean cleanall

# Compile (Add -static-libstdc++ only on Cygwin)
$(TARGET): $(OBJFILES) Makefile
	$(eval LDLIBS+=$(shell if [[ "$$(uname)" =~ "CYGWIN" ]]; then echo '-static-libstdc++'; fi))
	$(COMP) $(CPFLAGS) $(LDLIBS) -o $(TARGET) $(OBJFILES)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(COMP) -c $(CPFLAGS) $< -o $@

$(SRCDIR)/%.cpp: $(OBJDIR)
	
$(OBJDIR): 
	@echo "Creating directory $(OBJDIR)..."
	@mkdir -p $(OBJDIR)

clean:
	@echo "Removing object directory $(OBJDIR)/..."
	@rm -rf $(OBJDIR)

cleanall: clean
	@echo "Removing binaries from $(BINDIR)/..."
	@rm -rf $(BINDIR)/*
