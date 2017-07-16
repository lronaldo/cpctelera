import sys, re
import string

"""Simple script that scans all of the test suite results text fed in
through stdin and summarises the total number of failures, test
points, and test cases."""

# Read in everything
lines = sys.stdin.readlines()

found = False
for line in lines:
    if (re.search(r'^--- Summary:', line)):
        found = True
        break

if not found:
    fp = open(sys.argv[1], "w")
    fp.write("--- Running: %s\n" % sys.argv[1])
    fp.write("--- Summary: 1/0/0: 1 failed of 0 tests in 0 cases.\n")
    fp.close()

# Init the running totals
failures = 0
cases = 0
tests = 0
bytes = 0
ticks = 0
invalid = 0

# hack for valdiag
name = ""
base = ""

for line in lines:
    # --- Running: gen/ucz80/longor/longor
    m = re.match(r'^--- Running: (.*)$', line)
    if (m):
        name = m.group(1)

    # in case the test program crashes before the "--- Running" message
    m = re.match(r'^[0-9]+ words read from (.*)\.ihx$',line)
    if (m):
        name = m.group(1)

    base = name
    m = re.match(r'([^/]*)/([^/]*)/([^/]*)/(.*)$', name)
    if (m >= 3):
        base = m.group(3)

    # '--- Summary: f/t/c: ...', where f = # failures, t = # test points,
    # c = # test cases.
    if (re.search(r'^--- Summary:', line)):
        try:
            (summary, data, rest) = re.split(r':', line)
            (nfailures, ntests, ncases) = re.split(r'/', data)
            tests = tests + string.atof(ntests)
            cases = cases + string.atof(ncases)
        except ValueError:
            print "Bad summary line:", line
            nfailures = '1'
        failures = failures + string.atof(nfailures)
        if (string.atof(nfailures)):
            print "Failure: %s" % name

    # '--- Simulator: b/t: ...', where b = # bytes, t = # ticks
    if (re.search(r'^--- Simulator:', line)):
        try:
            (simulator, data, rest) = re.split(r':', line)
            (nbytes, nticks) = re.split(r'/', data)
        except ValueError:
            print "Bad simulator line:", line
        else:
            bytes = bytes + string.atof(nbytes)
            ticks = ticks + string.atof(nticks)

    # Stop at 0x000228: (106) Invalid instruction 0x00fd
    if (re.search(r'Invalid instruction', line) or re.search(r'unknown instruction', line)):
        invalid += 1;
        print "Invalid instruction: %s" % name

print "%-35.35s" % base,

if (invalid > 0):
    print "%d invalid instructions," % invalid,
print "(f: %2.0f, t:%4.0f, c: %2.0f, b: %6.0f, T: %8.0f)" % (failures, tests, cases, bytes, ticks)
