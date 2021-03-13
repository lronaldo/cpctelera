VPATH		= ..

vpath		%.mk $(VPATH)

CPPFLAGS	=
CFLAGS		= --debug
LDFLAGS		=
LIBS		=

-include	$(MAIN).mk

TARGETS		?= mcs51

CC		= sdcc -m$(CPU) --std-c99 --out-fmt-ihx

ALL		= $(MAIN) $(OTHERS)

OBJECTS		= $(MAIN).rel $(OTHERS:=.rel)

CPPFLAGS	= -I$(VPATH)

.SUFFIXES: .rel .ihx .hex

.PHONY: $(TARGETS)

all: $(TARGETS)

$(TARGETS):
	test -d $@ || mkdir $@
	echo "Compiling $@..."
	$(MAKE) -C $@ CPU=$@ REAL=yes MAIN=$(MAIN) -I$(VPATH) -f$(VPATH)/sdcc.mk compile copy_result
	echo "Done $@"

copy_result: $(VPATH)/$(MAIN)_$(CPU).hex $(VPATH)/$(MAIN)_$(CPU).cdb

$(VPATH)/$(MAIN)_$(CPU).hex: $(MAIN).hex
	cp $< $@

$(VPATH)/$(MAIN)_$(CPU).cdb: $(MAIN).cdb
	cp $(MAIN).cdb $@

compile: dep $(MAIN).hex

.c.rel:
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

.ihx.hex:
	packihx $< >$@

$(MAIN).ihx: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAAGS) $(OBJECTS) -o $@

$(MAIN).cdb: $(MAIN).hex

dep: $(MAIN).dep

$(MAIN).dep: $(addprefix ../,$(OBJECTS:.rel=.c)) ../*.h
	for c in $(addprefix ../,$(OBJECTS:.rel=.c)); do \
		$(CC) -MM $(CPPFLAGS) $$c >>$@; \
	done

clean:
	rm -rf $(TARGETS)

ifeq ($(REAL),yes)
include $(MAIN).dep
endif
