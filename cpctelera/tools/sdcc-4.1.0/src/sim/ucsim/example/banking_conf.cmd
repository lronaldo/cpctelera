memory create chip bank1_chip 0x8000 8
memory create chip bank2_chip 0x8000 8
memory create chip bank3_chip 0x8000 8

memory create banker sfr 0xb1 0x03 rom 0x8000 0xffff

memory create bank rom 0x8000 0 rom_chip 0x8000
memory create bank rom 0x8000 1 bank1_chip 0
memory create bank rom 0x8000 2 bank2_chip 0
memory create bank rom 0x8000 3 bank3_chip 0

fill rom_chip 0x8000 0xffff 0
fill bank1_chip 0 0x7fff 0
fill bank2_chip 0 0x7fff 0
fill bank3_chip 0 0x7fff 0

set hardware simif xram 0xffff
