# These are our test locations to be probed.
var loc1 rom[0x0000]
var loc2 rom[0x0001]

# Check the initial state of the default vcd.
info hw vcd

# Run the initialization code and stop when we reach the
# first test loop.
break 0x800c
run

# Set up a vcd to monitor the bit changes in loc1 and the value of loc2.
set hw vcd output "out/test.vcd"
set hw vcd add loc1.0
set hw vcd add loc1.1
set hw vcd add loc1.2
set hw vcd add loc1.3
set hw vcd add loc1.4
set hw vcd add loc1.5
set hw vcd add loc1.6
set hw vcd add loc1.7
set hw vcd add loc2
set hw vcd start

# Make sure what it is going to do is what we asked.
info hw vcd

# Run the first loop and stop when we hit the breakpoint again.
cont

# Remove the probe from every second bit.
# Removing probes while started is ok - we just stop logging changes.
set hw vcd del loc1.1
set hw vcd del loc1.3
set hw vcd del loc1.5
set hw vcd del loc1.7

# We can't add anything though because that would require a change to the
# header and it's far too late for that.
set hw vcd add loc1.3

# Run the second loop and stop when we hit the breakpoint again.
cont

# Don't forget to close the vcd properly!
set hw vcd stop
