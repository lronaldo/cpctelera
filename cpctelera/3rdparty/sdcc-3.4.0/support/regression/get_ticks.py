import sys, re
import string

"""Simple script that scans all of the simulator output text fed in
through stdin and summarises the total number of system clock ticks."""

# Read in everything
lines = sys.stdin.readlines()

# Declare globals
bytes = 0
ticks = 0

for line in lines:
    # 'n words read from ...', where = # bytes in hex file
    if (re.search(r'words read from', line)):
        (data, post) = re.split(r'w', line, 1)
        bytes = string.atoi(data)

    # 'Total time since last reset= 0.102021 sec (i clks)',
    # where i = # system clock ticks.
    if (re.search(r'^Total time', line)):
        (pre, data) = re.split(r'\(', line)
        (nticks, post) = re.split(r' ', data)
        ticks = string.atoi(nticks)

print "\n--- Simulator: %d/%d: %d bytes, %d ticks" % (bytes, ticks, bytes, ticks)
