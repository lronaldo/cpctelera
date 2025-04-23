from __future__ import print_function

import sys, re, io
import string

"""Simple script that scans all of the test suite results text fed in
through stdin and summarises the total number of failures, test
points, and test cases."""

# Function to parse numbers
def number_from_string(str_value):
  if str_value.find("x") != -1:
    return int(str_value.replace("0x", "").replace("x", ""), 16);
  else:
    return int(str_value)

def running_name_valid(name):
  if (name != "") and (name.find(" ") == -1) and ((name.find("/") != -1) or re.match(r'^(.*).c$', name)):
    return True
  else:
    return False

# Read in everything
if sys.version_info[0]<3:
    safe_stdin = sys.stdin
else:
    safe_stdin = io.TextIOWrapper(sys.stdin.buffer, encoding="latin-1")
lines = safe_stdin.readlines()

summary_found = False
for line in lines:
    if (re.search(r'^--- Summary:', line)):
        summary_found = True
        break


outname = ""
if len(sys.argv) > 1:
  outname = sys.argv[1]

# Create lines not present in simulation log
if not summary_found:
    print("- Added by compact-results script: %s\n" % sys.argv[1])
    m = re.match(r'^(.*).out', outname)
    if (m):
      outname = m.group(1)
    print("--- Running: %s\n" % outname)
    print("--- Summary: 1/0/0: 1 failed of 0 tests in 0 cases.\n")

# Init the running totals
failures = 0 if summary_found else 1
cases = 0
tests = 0
bytes = 0
ticks = 0
invalid = 0
stack_overflow = 0

# hack for valdiag
name = ""
base = ""

for line in lines:
    # --- Running: gen/ucz80/longor/longor
    m = re.match(r'^--- Running: (.*)$', line)
    if (m):
        #take the name only if not a whitespace, this happens if simulator stops when calling to print the name (stack overflow)
        stripped_name = m.group(1).strip()
        if running_name_valid(stripped_name):
          name = stripped_name

    # In case the test program crashes before the "--- Running" message
    m = re.match(r'^[0-9]+ words read from (.*).ihx', line)
    if (m):
        name = m.group(1)

    safe_name = name if name != "" else outname

    # Get base name from name
    base = safe_name
    m = re.match(r'([^/]*)/([^/]*)/([^/]*)/(.*)$', base)
    if (m):
        base = m.group(3)

    # '--- Summary: f/t/c: ...', where f = # failures, t = # test points,
    # c = # test cases.
    if (re.search(r'^--- Summary:', line)):
        try:
            if line.count(':') == 1:
              (summary, data) = re.split(r':', line)
            else:
              (summary, data, rest) = re.split(r':', line)

            (nfailures, ntests, ncases) = re.split(r'/', data)
            tests = tests + number_from_string(ntests)
            cases = cases + number_from_string(ncases)
        except ValueError:
            print("Bad summary line:", line)
            nfailures = '1'
        failures = failures + number_from_string(nfailures)
        if (number_from_string(nfailures)):
            print("Failure: %s" % safe_name)

    # '--- Simulator: b/t: ...', where b = # bytes, t = # ticks
    if (re.search(r'^--- Simulator:', line)):
        try:
            (simulator, data, rest) = re.split(r':', line)
            (nbytes, nticks) = re.split(r'/', data)
        except ValueError:
            print("Bad simulator line:", line)
        else:
            bytes = bytes + float(nbytes)
            ticks = ticks + float(nticks)

    # Stop at 0x000228: (106) Invalid instruction 0x00fd
    if (re.search(r'Invalid instruction', line) or re.search(r'unknown instruction', line)):
        invalid += 1;
        print("Invalid instruction: %s" % safe_name)

    # Stop at 0xXXXXXX: (103) Stack overflow
    if (re.search(r'Stack overflow', line)):
        stack_overflow += 1
        print("Stack overflow: %s" % safe_name)


print("%-35.35s" % sys.argv[1], end=' ')

if (invalid > 0):
    print("%d invalid instructions," % invalid, end=' ')
print("(f: %2.0f, t:%4.0f, c: %4.0f, b: %7.0f, T: %9.0f)" % (failures, tests, cases, bytes, ticks))
