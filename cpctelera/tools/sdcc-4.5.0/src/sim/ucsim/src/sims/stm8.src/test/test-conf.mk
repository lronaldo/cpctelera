CC = sdcc -mstm8
AS = sdasstm8
LD = sdldstm8
LINKFLAGS = -b _CODE=0x8000 -b _DATA=0x0000
