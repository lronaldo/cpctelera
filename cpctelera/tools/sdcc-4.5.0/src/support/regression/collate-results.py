from __future__ import print_function

import sys, re, io
#import string

"""Simple script that scans all of the test suite results text fed in
through stdin and summarises the total number of failures, test
points, and test cases."""

# Function to parse numbers
def number_from_string(str_value):
  if str_value.find("x") != -1:
    return int(str_value.replace("0x", "").replace("x", ""), 16);
  else:
    return int(str_value)

# Read in everything
if sys.version_info[0]<3:
    safe_stdin = sys.stdin
else:
    safe_stdin = io.TextIOWrapper(sys.stdin.buffer, encoding="latin-1")
lines = safe_stdin.readlines()

# Init the running totals
failures = 0
cases = 0
tests = 0
bytes = 0
ticks = 0
invalid = 0
stack_overflow = 0
halt = 0
unmatch = 0
flag = 0
messagelog = []
exlist = ["bug663539"]

# hack for valdiag
name = ""

for line in lines:

    m = re.match(r'^Simulation started,', line)
    if (m):
        flag = 0
        name = ""

    # --- Running: gen/ucz80/longor/longor
    m = re.match(r'^--- Running: (.*)$', line)
    if (m):
        #take the name only if not a whitespace, this happens if simulator stops when calling to print the name (stack overflow)
        stripped_name = m.group(1).strip()
        if stripped_name != "":
          name = stripped_name

    # in case the test program crashes before the "--- Running" message
    m = re.match(r'^[0-9]+ words read from (.*).ihx', line)
    if (m):
        name = m.group(1)

    # '--- Summary: f/t/c: ...', where f = # failures, t = # test points,
    # c = # test cases.
    if (re.search(r'^--- Summary:', line)):
        try:
            if line.count(':') == 1:
              (summary, data) = re.split(r':', line)
            else:
              (summary, data, rest) = re.split(r':', line)

            (nfailures, ntests, ncases) = re.split(r'/', data)
            failures = failures + number_from_string(nfailures)
            tests = tests + number_from_string(ntests)
            cases = cases + number_from_string(ncases)
        except ValueError:
            print("Parsing error at ", name)
            print("Bad summary line: ", line)
            nfailures = '1'
        if (number_from_string(nfailures)):
            messagelog.append("Failure: %s" % name)
        flag = 1

    # '--- Simulator: b/t: ...', where b = # bytes, t = # ticks
    if (re.search(r'^--- Simulator:', line)):
        try:
            (simulator, data, rest) = re.split(r':', line)
            (nbytes, nticks) = re.split(r'/', data)
        except ValueError:
            print("Bad simulator line", line)
        else:
            bytes = bytes + int(nbytes)
            ticks = ticks + int(nticks)
        if (flag != 1):
            for e in exlist:
                if (e in name):
                    flag = 2
        if (flag == 0):
            unmatch += 1
            messagelog.append("abnormal stop: %s" % name)
        flag = 0

    # Stop at 0x000228: (106) Invalid instruction 0x00fd
    if (re.search(r'Invalid instruction', line) or re.search(r'unknown instruction', line)):
        invalid += 1
        messagelog.append("Invalid instruction: %s" % name)

    # Stop at 0xXXXXXX: (103) Stack overflow
    if (re.search(r'Stack overflow', line)):
        stack_overflow += 1
        messagelog.append("Stack overflow: %s" % name)

    # HALT instruction
    if (re.search(r'HALT instruction', line) or re.search(r'Halt instruction', line) or re.search(r'halt instruction', line)):
        halt += 1
        messagelog.append("HALT instruction: %s" % name)

    # --- FAIL: "timeout, simulation killed" in xx/xx/testfile.c
    m = re.search(r'simulation killed',line)
    if (m):
      name = line.split()[-1]
      name = '.'.join(name.split('.')[0:-1])


if (len(sys.argv) > 1):
    print("Summary for '%s':" % sys.argv[1], end=' ')
if (unmatch > 0):
    print("%d abnormal stops (" % unmatch, end=' ')
    if (invalid > 0):
        print("%d invalid instructions," % invalid, end=' ')
    if (stack_overflow > 0):
        print("%d Stack overflows," % stack_overflow, end=' ')
    if (halt > 0):
        print("%d HALT instructions," % halt, end=' ')
    print("),", end=' ')
print("%.0f failures, %.0f tests, %.0f test cases, %.0f bytes, %.0f ticks" % (failures, tests, cases, bytes, ticks))
for msg in messagelog:
  print("  ",msg)
print()
