#!/usr/bin/env python
#---------------------------------------------------------------------------
#  valdiag.py - Validate diagnostic messages from SDCC/GCC
#	  Written By -  Erik Petrich . epetrich@users.sourceforge.net (2003)
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2, or (at your option) any
#   later version.
#   
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#   
#   In other words, you are welcome to use, share and improve this program.
#   You are forbidden to forbid anyone else to use, share and improve
#   what you give them.   Help stamp out software-hoarding!  
#---------------------------------------------------------------------------

import sys, string, os, popen2, re

macrodefs = {}

gcc = {
    "CC":"gcc",
    "CCFLAGS":"-c -Wall -DPORT_HOST=1",
    "CCDEF":"-D",
    "CCOUTPUT":"-o",
    "C89":"-std=c89",
    "C99":"-std=c99",
    "defined": {
        "__GNUC__":"1",
        "GCC":"1"
    },
    "ignoremsg": [
    ]
}

sdcc = {
    "CC":"../../bin/sdcc",
    "CCFLAGS":"-c -m{port}",
    "CCDEF":"-D",
    "CCOUTPUT":"-o",
    "CCINCLUDEDIR":"-I",
    "C89":"--std-sdcc89",
    "C99":"--std-sdcc99",
    "defined": {
        "SDCC":"1",
        "SDCC_{port}":"1",
        "__{port}":"1"
    },
    "ignoremsg": [
        "code not generated.*due to previous errors",
        "unreferenced function argument"
    ]
}

testmodes = {
    "host":{
        "compiler":gcc,
        "port":"host",
        "defined": {
            "PORT_HOST":"1"
        }
    },
    "mcs51":{
        "compiler":sdcc,
        "port":"mcs51"
    },
    "mcs51-large":{
        "compiler":sdcc,
        "port":"mcs51",
        "flags":"--model-large",
        "defined": {
            "SDCC_MODEL_LARGE":"1"
        }
    },
    "mcs51-stack-auto":{
        "compiler":sdcc,
        "port":"mcs51",
        "flags":"--stack-auto",
        "defined": {
            "SDCC_STACK_AUTO":"1"
        }
    },
    "ds390":{
        "compiler":sdcc,
        "port":"ds390"
    },
    "z80":{
        "compiler":sdcc,
        "port":"z80"
    },
    "gbz80":{
        "compiler":sdcc,
        "port":"gbz80"
    },
    "hc08":{
        "compiler":sdcc,
        "port":"hc08"
    },
    "pic14":{
        "compiler":sdcc,
        "port":"pic14"
    },
    "pic16":{
        "compiler":sdcc,
        "port":"pic16"
    }
}


def evalQualifier(expr):
    global macrodefs
    tokens = re.split("([^0-9A-Za-z_])", expr)
    for tokenindex in range(len(tokens)):
        token = tokens[tokenindex]
        if token in macrodefs:
            tokens[tokenindex] = macrodefs[token]
        elif token == "defined":
            tokens[tokenindex] = ""
            if tokens[tokenindex+2] in macrodefs:
                tokens[tokenindex+2] = "1"
            else:
                tokens[tokenindex+2] = "0"
        elif len(token)>0:
            if token[0]=="_" or token[0] in string.ascii_letters:
                tokens[tokenindex] = "0"
    expr = string.join(tokens,"")
    expr = string.replace(expr,"&&"," and ");
    expr = string.replace(expr,"||"," or ");
    expr = string.replace(expr,"!"," not ");
    return eval(expr)

def expandPyExpr(expr):
    tokens = re.split("({|})", expr)
    for tokenindex in range(1,len(tokens)):
        if tokens[tokenindex-1]=="{":
            tokens[tokenindex]=eval(tokens[tokenindex])
            tokens[tokenindex-1]=""
            tokens[tokenindex+1]=""
    expandedExpr = string.join(tokens,"")
    return expandedExpr

def addDefines(deflist):
    for define in deflist.keys():
        expandeddef = expandPyExpr(define)
        macrodefs[expandeddef] = expandPyExpr(deflist[define])

def parseInputfile(inputfilename):
    inputfile = open(inputfilename, "r")
    testcases = {}
    testname = ""
    linenumber = 1

    # Find the test cases and tests in this file
    for line in inputfile.readlines():

        # See if a new testcase is being defined
        p = string.find(line, "TEST")
        if p>=0:
            testname = string.split(line[p:])[0]
            if not testcases.has_key(testname):
                testcases[testname] = {}

        # See if a new test is being defined
        for testtype in ["ERROR", "WARNING", "IGNORE"]:
            p = string.find(line, testtype);
            if p>=0:
                # Found a test definition
                qualifier = string.strip(line[p+len(testtype):])
                p = string.find(qualifier, "*/")
                if p>=0:
                    qualifier = string.strip(qualifier[:p])
                if len(qualifier)==0:
                    qualifier="1"
                qualifier = evalQualifier(qualifier)
                if qualifier:
                    if not linenumber in testcases[testname]:
                        testcases[testname][linenumber]=[]
                    testcases[testname][linenumber].append(testtype)

        linenumber = linenumber + 1

    inputfile.close()
    return testcases

def parseResults(output):
    results = {}
    for line in output:
        print line,

        if string.count(line, "SIGSEG"):
            results[0] = ["FAULT", string.strip(line)]
            continue

        # look for something of the form:
        #   filename:line:message
        msg = string.split(line,":",2)
        if len(msg)<3: continue
        if msg[0]!=inputfilename: continue
        if len(msg[1])==0: continue
        if not msg[1][0] in string.digits: continue

        # it's in the right form; parse it
        linenumber = int(msg[1])
        msgtype = "UNKNOWN"
        uppermsg = string.upper(msg[2])
        if string.count(uppermsg,"ERROR"):
            msgtype = "ERROR"
        if string.count(uppermsg,"WARNING"):
            msgtype = "WARNING"
        msgtext = string.strip(msg[2])
        ignore = 0
        for ignoreExpr in ignoreExprList:
           if re.search(ignoreExpr,msgtext)!=None:
               ignore = 1
        if not ignore:
            results[linenumber]=[msgtype,string.strip(msg[2])]
    return results

def showUsage():
    print "Usage: test testmode cfile [objectfile]"
    print "Choices for testmode are:"
    for testmodename in testmodes.keys():
        print "   %s" % testmodename
    sys.exit(1)

# Start here
if len(sys.argv)<3:
    showUsage()

testmodename = sys.argv[1]
if not testmodename in testmodes:
    print "Unknown test mode '%s'" % testmodename
    showUsage()

testmode = testmodes[testmodename]
compilermode = testmode["compiler"]
port = expandPyExpr(testmode["port"])
cc = expandPyExpr(compilermode["CC"])
ccflags = expandPyExpr(compilermode["CCFLAGS"])
if "flags" in testmode:
    ccflags = string.join([ccflags,expandPyExpr(testmode["flags"])])
if len(sys.argv)>=4:
    if "CCOUTPUT" in compilermode:
        ccflags = string.join([ccflags,expandPyExpr(compilermode["CCOUTPUT"]),sys.argv[3]])
if len(sys.argv)>=5:
    if "CCINCLUDEDIR" in compilermode:
        ccflags = string.join([ccflags,expandPyExpr(compilermode["CCINCLUDEDIR"]),sys.argv[4]])
if "defined" in compilermode:
    addDefines(compilermode["defined"])
if "defined" in testmode:
    addDefines(testmode["defined"])
if "ignoremsg" in compilermode:
    ignoreExprList = compilermode["ignoremsg"]
else:
    ignoreExprList = []

inputfilename = sys.argv[2]
inputfilenameshort = os.path.basename(inputfilename)

try:
    testcases = parseInputfile(inputfilename)
except IOError:
    print "Unable to read file '%s'" % inputfilename
    sys.exit(1)

casecount = len(testcases.keys())
testcount = 0
failurecount = 0

print "--- Running: %s " % inputfilenameshort
for testname in testcases.keys():
    if string.find(testname,"DISABLED"):
      continue
    ccdef = compilermode["CCDEF"]+testname
    if testname[-3:] == "C89":
        ccstd = compilermode["C89"]
    elif testname[-3:] == "C99":
        ccstd = compilermode["C99"]
    else:
        ccstd = ""
    cmd = string.join([cc,ccflags,ccstd,ccdef,inputfilename])
    print
    print cmd
    spawn = popen2.Popen4(cmd)
    spawn.wait()
    output = spawn.fromchild.readlines()

    results = parseResults(output)

    if len(testcases[testname])==0:
        testcount = testcount + 1 #implicit test for no errors

    # Go through the tests of this case and make sure
    # the compiler gave a diagnostic
    for checkline in testcases[testname].keys():
        testcount = testcount + 1
        if checkline in results:
            if "IGNORE" in testcases[testname][checkline]:
                testcount = testcount - 1  #this isn't really a test
            del results[checkline]
        else:
            for wanted in testcases[testname][checkline]:
                if not wanted=="IGNORE":
                    print "--- FAIL: expected %s" % wanted,
                    print "at %s:%d" % (inputfilename, checkline)
                    failurecount = failurecount + 1

    # Output any unexpected diagnostics    
    for checkline in results.keys():
        print '--- FAIL: unexpected message "%s" ' % results[checkline][1],
        print "at %s:%d" % (inputfilename, checkline)
        failurecount = failurecount + 1

print
print "--- Summary: %d/%d/%d: " % (failurecount, testcount, casecount),
print "%d failed of %d tests in %d cases." % (failurecount, testcount, casecount)
