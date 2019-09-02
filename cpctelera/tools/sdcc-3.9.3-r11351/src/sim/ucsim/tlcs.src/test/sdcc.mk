TARGET		= tlcs90

CC		= sdcc -m$(TARGET)

CPPFLAGS	=
CFLAGS		= --fverbose-asm -DNO_VARARGS --debug
LDFLAGS		=
LIBS		=

ALL		= $(MAIN) $(OTHERS)
OBJECTS		= $(MAIN).rel $(OTHERS:=.rel)

all: $(MAIN).hex

dep: $(MAIN).dep

$(MAIN).dep: $(OBJECTS:.rel=.c) *.h
	@>$(MAIN).dep
	@for c in $(OBJECTS:.rel=.c); do \
		$(CC) -MM $(CPPFALGS) $$c >>$(MAIN).dep ;\
	done

include $(MAIN).dep

$(MAIN).ihx: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

.SUFFIXES: .rel .ihx .hex

.c.rel:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

.ihx.hex:
	packihx $< >$@

clean:
	rm -f $(ALL:=.rel) $(ALL:=.asm) $(ALL:=.lst) $(ALL:=.rst) $(ALL:=.sym) $(ALL:=.adb)
	rm -f $(MAIN).ihx $(MAIN).hex $(MAIN).lk $(MAIN).map $(MAIN).mem $(MAIN).cdb $(MAIN).omf $(MAIN).noi
	rm -f *~
	rm -f $(MAIN).dep


# End of sdcc.mk
