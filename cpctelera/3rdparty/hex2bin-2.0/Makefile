# Makefile hex2bin/mot2bin
SRCDIR  = src
BINDIR  = bin
OBJDIR  = obj
TGTDIR  = $(BINDIR)
B_SRCFILES= $(foreach F, hex2bin.c common.c libcrc.c binary.c, $(SRCDIR)/$(F))
B_OBJFILES= $(foreach F, hex2bin.o common.o libcrc.o binary.o, $(OBJDIR)/$(F))
M_SRCFILES= $(foreach F, mot2bin.c common.c libcrc.c binary.c, $(SRCDIR)/$(F))
M_OBJFILES= $(foreach F, mot2bin.o common.o libcrc.o binary.o, $(OBJDIR)/$(F))
TGT_FILES = $(foreach F, hex2bin mot2bin hex2bin.1, $(TGTDIR)/$(F))
CPFLAGS = -std=gnu99 -O2 -Wall -pedantic
# Compile
all: objectdir $(TGT_FILES)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc -c $(CPFLAGS) $< -o $@

objectdir:
	@echo "Creating directory $(OBJDIR)..."
	mkdir -p $(OBJDIR)

$(TGTDIR)/hex2bin.1: $(SRCDIR)/hex2bin.pod
	pod2man $(SRCDIR)/hex2bin.pod > $(TGTDIR)/hex2bin.1

$(TGTDIR)/hex2bin: $(B_OBJFILES)
	gcc $(CPFLAGS) -o $(TGTDIR)/hex2bin $(B_OBJFILES)

$(TGTDIR)/mot2bin: $(M_OBJFILES)
	gcc $(CPFLAGS) -o $(TGTDIR)/mot2bin $(M_OBJFILES)

clean:
	rm -rf $(OBJDIR)
