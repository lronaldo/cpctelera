# Memory contents are random initially so set some recognizable content.
set mem rom 0x1000 0x01 0x02 0x04 0x08 0x10 0x20 0x40 0x80

# Dump using old format <as> <start> <end>
dump rom 0x1000 0x1007

# Dump using new format <as>[<start>] <end>
dump rom[0x1000] 0x1007

# We can use new format for both start and end
dump rom[0x1000] rom[0x1007]

# But we can't cross address spaces!
dump rom[0x1000] regs8[0x1007]

# Bits can be expressed individually
dump rom[0x1003].1

# Or as sets
dump rom[0x1003][3:1]

# We can set bits individually
set mem rom[0x1000].7 1

# Or in groups
set mem rom[0x1001][6:5] 0b11

# Vars can be set...
set mem rom 0x1100 0xff
var test rom[0x1100]
info var test

# ...and changed...
var test rom[0x1100][3:0]
info var test
var test rom[0x1100][5:4]
info var test
var test rom[0x1100][7:6]
info var test
var test rom[0x1100][6:2]
info var test

# ...and deleted
rmvar test
info var test

# Add some labels
var test0 rom[0x1000]
var test2 rom[0x1002]
var test3a rom[0x1003]
var test3b rom[0x1003]

# And name some bits
var set0  rom[0x1003][0:1]
var set1  rom[0x1003].1
var set2  rom[0x1003].2
var set3  rom[0x1003].3
var set4  rom[0x1003].4
var set5  rom[0x1003].5
var set6  rom[0x1003].6
var set7  rom[0x1003].7

var test4bit0  rom[0x1004].0
var test4bit2  rom[0x1004].2
var test4bit4  rom[0x1004].4
var test4bit6  rom[0x1004].6

# And name an odd bit in a location between two locations
# that have neither labels nor named bits
var lone1 rom[0x1006].6

# Now when we dump the region the labels for addresses are shown,
# breaking up the lines of hex data. Where there are names for
# bits we automatically switch to displaying the data in binary.
dump rom[0x1000] 0x100f

# Dumping some specific bits from a location works
dump rom[0x1003][6:2]

# Specifying a format disables all the smarts
dump /h rom[0x1000] 0x100f

# Dumping a region with every other bit labelled...
dump 0x1004

# And with partial views...
dump 0x1004[6:4]
dump 0x1004[5:3]
dump 0x1004[7:2]

# If the first label was qualified due to a request for a bit
# range subsequent labels may be considered identical even if
# they aren't strictly identical.
var some_bits test0[5:3]
d some_bits
var more_bits test0[6:2]
d some_bits
var less_bits test0.4
d some_bits
d less_bits
d more_bits

# Finally the expression evaluater handles bits in the same way
rom[0x1000]
rom[0x1000].7
rom[0x1001][6:5]
