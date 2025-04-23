from HTMLgen import TemplateDocument
import sys, re, tempfile, os

"""See InstanceGenerator for a description of this file"""

# Globals
# Directory that the generated files should be placed into
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
  return "{testcase}";
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
        self.replacements = []
        # Initalise the function list hash.
        self.functions = []
        # Emit the suite wrapper into a temporary file
        self.tmpname = tempfile.mktemp()
        (self.dirname, self.filename) = os.path.split(self.inname)
        (self.basename, self.ext) = os.path.splitext (self.filename)
        if self.ext == ".in":
            (self.basename, self.ext) = os.path.splitext (self.filename[:-3])

    def permute(self, basename, repl, trans = {}):
        """Permutes across all of the names.  For each value, recursively creates
        a mangled form of the name, this value, and all the combinations of
        the remaining values.  At the tail of the recursion when one full
        combination is built, generates an instance of the test case from
        the template."""
        basepath = os.path.join(outdir, basename)
        if len(repl) == 0:
            # End of the recursion.
            # Set the runtime substitutions.
            trans['testcase'] = re.sub(r'\\', r'\\\\', basename)
            # Create the instance from the template
            T = TemplateDocument(self.tmpname)
            T.substitutions = trans
            T.write(basepath + self.ext)
        else:
            # Pull off this key, then recursively iterate through the rest.
            key = repl[0][0]
            for part in repl[0][1]:
                trans[key] = part
                # Turn a empty string into something decent for a filename
                if not part:
                    part = 'none'
                # Remove any bad characters from the filename.
                part = re.sub(r'\s+', r'_', part)
                # The slice operator (_[1:]) creates a copy of the list missing the
                # first element.
                # Can't use '-' as a seperator due to the mcs51 assembler.
                self.permute(basename + '_' + key + '_' + part, repl[1:], trans)

    def writetemplate(self):
        """Given a template file and a temporary name writes out a verbatim copy
        of the source file and adds the suite table and functions."""
        if sys.version_info[0]<3:
            fout = open(self.tmpname, 'w')
        else:
            fout = open(self.tmpname, 'w', encoding="latin-1")

        for line in self.lines:
            fout.write(line)

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
        fout.write(testfunsuite);
        
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
                    
                    self.replacements.append((name, values))
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
        if self.writetemplate() == 0:
            sys.stderr.write("Empty function list in " + self.inname + "!\n")

        # Create the output directory if it doesn't exist
        createdir(outdir)

        # Generate
        self.permute(self.basename, self.replacements)

        # Remove the temporary file
        os.remove(self.tmpname)

def main():
    # Check and parse the command line arguments
    if len(sys.argv) < 3:
        print("usage: generate-cases.py template.c outdir")
        sys.exit(-1)
        
    # Input name is the first arg.

    s = InstanceGenerator(sys.argv[1])
    s.generate()

if __name__ == '__main__':
    main()
