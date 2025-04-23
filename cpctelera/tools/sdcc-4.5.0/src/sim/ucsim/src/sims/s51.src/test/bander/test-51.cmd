# Bander maps bits one at a time starting from iram_chip[0x20] into
# the bits address space starting at 0x00
info mem

# Set iram_chip[0x20] and iram_chip[0x21] to 0xff
set mem iram_chip 0x20 0xff 0xff

# First 16 addresses of bits should all read as 0x01
dump bits 0 15

# Set iram_chip[0x20] and iram_chip[0x21] to 0x00
set mem iram_chip 0x20 0x00 0x00

# First 16 addresses of bits should all read as 0x00
dump bits 0 15

# Now set bits in turn starting with the lowest and the
# corresponding location in bits should change to 0x01
set mem iram_chip[0x20].0 1
dump bits 0 15
set mem iram_chip[0x20].1 1
dump bits 0 15
set mem iram_chip[0x20].2 1
dump bits 0 15
set mem iram_chip[0x20].3 1
dump bits 0 15
set mem iram_chip[0x20].4 1
dump bits 0 15
set mem iram_chip[0x20].5 1
dump bits 0 15
set mem iram_chip[0x20].6 1
dump bits 0 15
set mem iram_chip[0x20].7 1
dump bits 0 15
set mem iram_chip[0x21].0 1
dump bits 0 15
set mem iram_chip[0x21].1 1
dump bits 0 15
set mem iram_chip[0x21].2 1
dump bits 0 15
set mem iram_chip[0x21].3 1
dump bits 0 15
set mem iram_chip[0x21].4 1
dump bits 0 15
set mem iram_chip[0x21].5 1
dump bits 0 15
set mem iram_chip[0x21].6 1
dump bits 0 15
set mem iram_chip[0x21].7 1
dump bits 0 15

# If we set bits[0x08] to 0 that should clear bit 0 of iram_chip[0x21]
set mem bits[0x08] 0
dump iram_chip 0x20 0x21
