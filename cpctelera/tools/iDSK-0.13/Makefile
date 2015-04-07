SRCDIR  = src
BINDIR  = bin
OBJDIR  = obj
COMP    = g++
TARGET  = $(BINDIR)/iDSK
SRCFILES= $(wildcard $(SRCDIR)/*.cpp)
OBJFILES= $(foreach F, $(SRCFILES), $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(F:%.cpp=%.o)))
CPFLAGS = -O2 -Wall -pedantic

# Compile
all: objectdir $(OBJFILES) Makefile
	$(COMP) $(CPFLAGS) -o $(TARGET) $(OBJFILES)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(COMP) -c $(CPFLAGS) $< -o $@

objectdir:
	@echo "Creating directory $(OBJDIR)..."
	@mkdir -p $(OBJDIR)

clean:
	@echo "Removing object directory $(OBJDIR)/..."
	@rm -rf $(OBJDIR)

cleanall: clean
	@echo "Removing binaries from $(BINDIR)/..."
	@rm -rf $(BINDIR)/*
