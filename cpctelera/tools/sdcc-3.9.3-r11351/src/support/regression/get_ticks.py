import sys, re, io
import string

"""Simple script that scans all of the simulator output text fed in
through stdin and summarises the total number of system clock ticks."""

# Read in everything
if sys.version_info[0]<3:
    safe_stdin = sys.stdin
else:
    safe_stdin = io.TextIOWrapper(sys.stdin.buffer, encoding="latin-1")
lines = safe_stdin.readlines()

# Declare globals
bytes = 0
ticks = 0

for line in lines:
    # 'n words read from ...', where = # bytes in hex file
    if (re.search(r'words read from', line)):
        (data, post) = re.split(r'words', line, 1)
        data = re.sub(r'[^0-9]',' ',data).strip().split();
        if len(data)>0:
          bytes = int(data[-1])
        else:
          bytes = 0 # wrong size, but better than blowing up

    # 'Total time since last reset= 0.102021 sec (i clks)',
    # where i = # system clock ticks.
    if (re.search(r'^Total time', line)):
        (pre, data) = re.split(r'\(', line)
        (nticks, post) = re.split(r' ', data)
        ticks = int(nticks)

print("\n--- Simulator: %d/%d: %d bytes, %d ticks" % (bytes, ticks, bytes, ticks))
