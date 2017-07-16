CC		= sdcc 

MODEL		= large
CPPFLAGS	=
CFLAGS		= --debug --model-$(MODEL)
LDFLAGS		=
LIBS		=

ALL		= $(MAIN) $(OTHERS)
OBJECTS		= $(MAIN).rel $(OTHERS:=.rel)

APP		?= $(MAIN)

all: $(APP).hex

dep: $(APP).dep

$(APP).dep: $(OBJECTS:.rel=.c) *.h
	@>$(APP).dep
	@for c in $(OBJECTS:.rel=.c); do \
		$(CC) -MM $(CPPFALGS) $$c >>$(APP).dep ;\
	done

include $(APP).dep

$(APP).ihx: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

.SUFFIXES: .rel .ihx .hex

.c.rel:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

.ihx.hex:
	packihx $< >$@

clean:
	rm -f $(ALL:=.rel) $(ALL:=.asm) $(ALL:=.lst) $(ALL:=.rst) $(ALL:=.sym) $(ALL:=.adb)
	rm -f $(MAIN).ihx $(MAIN).hex $(MAIN).lk $(MAIN).map $(MAIN).mem $(MAIN).cdb $(MAIN).omf $(MAIN).noi
	rm -f $(APP).ihx $(APP).hex $(APP).lk $(APP).map $(APP).mem $(APP).cdb $(APP).omf $(APP).noi
	rm -f *~
	rm -f $(MAIN).dep $(APP).dep
	rm -f $(MAIN).sim $(APP).sim


# End of sdcc.mk
