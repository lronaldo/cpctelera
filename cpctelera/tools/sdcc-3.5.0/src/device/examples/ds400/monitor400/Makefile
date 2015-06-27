CC = ../../../../bin/sdcc

MFLAGS = -mds400
LFLAGS = --xram-loc 0x10000 --code-loc 0x400000 -Wl-r

OBJECTS = mon400.rel

all: mon400.hex

clean:
	rm -f *~ \#* *.asm *.cdb *.rel *.hex *.ihx *.lst *.map *.rst *.sym *.lnk

mon400.hex: mon400.ihx
	packihx mon400.ihx >mon400.hex

mon400.ihx: $(OBJECTS)
	$(CC) $(MFLAGS) $(LFLAGS) $(OBJECTS)

%.rel: %.c
	$(CC) -c $(MFLAGS) $<

