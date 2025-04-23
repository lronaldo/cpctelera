# These are the test locations named in the input vcd.
var loc1 rom[0x0000]
var loc2 rom[0x0001]

# Run the initialization code and stop when we reach the
# first test loop.
break 0x800c
run

# Start the vcd (we set the input file via the Makefile) and
# have it stop execution each time an event occurs.
set hw vcd start
set hw vcd break

# The VCD data starts at time 0 which will, by default, align
# with the current simulator time. We can adjust that by setting
# starttime. This can be done before or after starting the vcd,
# even part way through. Here we push the vcd 750 ns into the
# future.
set hw vcd starttime 750 ns

# Make sure what it is going to do is what we asked.
conf
info hw vcd

# Continue until an event stops us then dump the locations
# that the vcd is driving according to the input file.
# Repeat a sufficient number of times.
cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
timer get
dump rom 0 1

set hw vcd pause
conf
step
step
step
step
set hw vcd pause
conf
timer get

cont
dump rom 0 1

cont
timer get
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

cont
dump rom 0 1

# Stop now
set hw vcd stop
conf
