from HTMLgen import TemplateDocument
import sys, re, os

"""
Build a test driver for various C tests without main function,
Derived from generate-cases, and far too complicated for the task.
"""

# Globals
# Directory that the generated files should be placed into
infile = sys.argv[1]
outdir = sys.argv[2]

# Start of the test function table definition
testfuntableheader = """
void
__runSuite(void)
{
"""

# End of the test function table definition
testfuntablefooter = """}
"""

# Code to generate the suite function
testfunsuite = """
__code const char *
__getSuiteName(void)
{
  return "%s";
}
""" 

# Utility functions
def createdir(path):
    """Creates a directory if it doesn't exist"""
    if not os.path.isdir(path):
        os.mkdir(path)

class InstanceGenerator:
    """Test case iteration generator.
    Takes the template given as the first argument, pulls out all the meta
    iteration information, and generates an instance for each combination
    of the names and types.

    See doc/test_suite_spec.tex for more information on the template file
    format."""

    def __init__(self, inname):
        self.inname = inname
        # Initalise the replacements hash.
        # Map of name to values.
        self.replacements = { }
        # Initalise the function list hash.
        self.functions = []
        # Emit the suite wrapper into a temporary file
        (self.dirname, self.filename) = os.path.split(self.inname)
        (self.basename, self.ext) = os.path.splitext (self.filename)
        if self.ext == ".in":
            (self.basename, self.ext) = os.path.splitext (self.filename[:-3])

    def writetemplate(self, fn):
        """Given a template file and a temporary name writes out a verbatim copy
        of the source file and adds the suite table and functions."""

        """print("writing", fn)"""
        if sys.version_info[0]<3:
            fout = open(fn, 'w')
        else:
            fout = open(fn, 'w', encoding="latin-1")

        fout.write('#include "')
        fout.write(infile)
        fout.write('"\n')

        # Emmit the suite table
        fout.write(testfuntableheader)

        n = 0;
        for fun in self.functions:
            # Turn the function definition into a function call
            fout.write("  __prints(\"Running " + fun + "\\n\");\n");
            fout.write('  ' + fun + "();\n")
            n += 1;

        fout.write(testfuntablefooter)
        fout.write("\nconst int __numCases = " + str(n) + ";\n")
        fout.write(testfunsuite % ( self.basename + self.ext ));

        fout.close()
        return n

    def readfile(self):
        """Read in all of the input file."""
        if sys.version_info[0]<3:
            fin = open(self.inname)
        else:
            fin = open(self.inname, encoding="latin-1")
        self.lines = fin.readlines()
        fin.close()

    def parse(self):
        # Start off in the header.
        inheader = 1;

        # Iterate over the source file and pull out the meta data.
        for line in self.lines:
            line = line.strip()

            # If we are still in the header, see if this is a substitution line
            if inheader:
                # A substitution line has a ':' in it
                if re.search(r':', line) != None:
                    # Split out the name from the values
                    (name, rawvalues) = re.split(r':', line)
                    # Split the values at the commas
                    values = re.split(r',', rawvalues)

                    # Trim the name
                    name = name.strip()
                    # Trim all the values
                    values = [value.strip() for value in values]

                    self.replacements[name] = values
                elif re.search(r'\*/', line) != None:
                    # Hit the end of the comments
                    inheader = 0;
                else:
                    # Do nothing.
                    None
            else:
                # Pull out any test function names
                m = re.match(r'^(?:\W*void\W+)?\W*(test\w*)\W*\(\W*void\W*\)', line)
                if m != None:
                    self.functions.append(m.group(1))

    def generate(self):
        """Main function.  Generates all of the instances."""
        self.readfile()
        self.parse()
        outfn = sys.argv[2]
        if self.writetemplate(outfn) == 0:
            sys.stderr.write("Empty function list in " + self.inname + "!\n")

def main():
    # Check and parse the command line arguments
    if len(sys.argv) < 3:
        print("usage: mkdrv.py infile outfile")
        sys.exit(-1)

    # Input name is the first arg.

    s = InstanceGenerator(sys.argv[1])
    s.generate()

if __name__ == '__main__':
    main()
