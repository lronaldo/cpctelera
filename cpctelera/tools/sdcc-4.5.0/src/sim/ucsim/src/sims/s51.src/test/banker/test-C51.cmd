# There are four banks of registers available. Which bank is
# currently in use is selected via two bits in sfr[0xd0]
info mem

# The bank switcher should have an operator on sfr[0xd0]
memory cell sfr[0xd0]

# Define the control bits
var bank sfr[0xd0][4:3]

# Now the current bank is 0 and we have a set of registers
#dump regs 0 7

# Note that the R<n> vars are defined on the "regs" space
info var R3

# Let's label the banks in the underlying chip
var bank0 iram_chip[0x00]
var bank1 iram_chip[0x08]
var bank2 iram_chip[0x10]
var bank3 iram_chip[0x18]

# and prime the banks with recognisable data
set mem iram_chip 0x00 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
set mem iram_chip 0x08 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17
set mem iram_chip 0x10 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
set mem iram_chip 0x18 0x30 0x31 0x32 0x33 0x34 0x35 0x36 0x37

# Now when we dump the regs we can see the label on the
# current bank of the underlying chip as well
#dump regs 0 7

# Change to bank 1
set mem bank 1

# and check the state
info mem

# If we dump regs again we still see the R<n> labels because
# they are on the address space however "bank0" has changed
# to "bank1" because they are on the underlying chip.
#dump regs 0 7

# Repeat for banks 2 and 3
set mem bank 2
info mem
#dump regs 0 7
set mem bank 3
info mem
#dump regs 0 7

# And finally check that the registers follow the bank switching correctly
set mem bank 0
info reg
set mem bank 1
info reg
set mem bank 2
info reg
set mem bank 3
info reg
