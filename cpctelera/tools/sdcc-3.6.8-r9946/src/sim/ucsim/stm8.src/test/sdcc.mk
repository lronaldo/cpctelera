TARGET		= stm8

CC		= sdcc -m$(TARGET) --std-c99

CPPFLAGS	=
CFLAGS		= --debug
LDFLAGS		=
LIBS		=

ALL		= $(MAIN) $(OTHERS)
OBJECTS		= $(MAIN).rel $(OTHERS:=.rel)

APP		?= $(MAIN)

all: del_serial_rel $(APP).hex

del_serial_rel:
	rm -f serial.rel

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
	rm -f $(MAIN).ihx $(MAIN).hex $(MAIN).lk $(MAIN).map $(MAIN).mem $(MAIN).cdb $(MAIN).omf $(MAIN).noi $(MAIN).adb $(MAIN).sym $(MAIN).cdb
	rm -f *.ihx *.hex
	rm -f *~
	rm -f $(MAIN).dep $(APP).dep


# End of sdcc.mk
