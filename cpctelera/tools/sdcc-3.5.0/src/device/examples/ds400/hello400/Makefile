CC = ../../../../bin/sdcc

MFLAGS = -mds400
LFLAGS = --xram-loc 0x10000 --code-loc 0x400000 -Wl-r

OBJECTS = hello.rel

all: hello.hex

clean:
	rm -f *~ \#* *.asm *.cdb *.rel *.hex *.ihx *.lst *.map *.rst *.sym *.lnk

hello.hex: hello.ihx
	packihx hello.ihx >hello.hex

hello.ihx: $(OBJECTS)
	$(CC) $(MFLAGS) $(LFLAGS) $(OBJECTS)

%.rel: %.c
	$(CC) -c $(MFLAGS) $<
