VPATH		= ..

vpath		%.mk $(VPATH)

TARGET		= stm8

CC		= sdcc -m$(TARGET) --std-c99

CPPFLAGS	=
CFLAGS		= --debug
LDFLAGS		=
LIBS		=

-include	$(MAIN).mk

DEVICES		?= S208

ALL		= $(MAIN) $(OTHERS)

OBJECTS		= $(MAIN).rel $(OTHERS:=.rel)

CPPFLAGS	= -DDEVICE=DEV_STM8$(DEVICE) -I$(VPATH)

.SUFFIXES: .rel .ihx .hex

.PHONY: $(DEVICES)

all: $(DEVICES)

$(DEVICES):
	test -d $@ || mkdir $@
	$(MAKE) -C $@ DEVICE=$@ REAL=yes MAIN=$(MAIN) -I$(VPATH) -f$(VPATH)/sdcc.mk compile copy_result

copy_result: $(VPATH)/$(MAIN)_$(DEVICE).hex $(VPATH)/$(MAIN)_$(DEVICE).cdb

$(VPATH)/$(MAIN)_$(DEVICE).hex: $(MAIN).hex
	cp $< $@

$(VPATH)/$(MAIN)_$(DEVICE).cdb: $(MAIN).cdb
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
	rm -rf $(DEVICES)

ifeq ($(REAL),yes)
include $(MAIN).dep
endif
