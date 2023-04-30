

The ADFlib is a free, portable and open implementation of the Amiga filesystem.

It supports :
- floppy dumps
- multiple partitions harddisk dumps
- UAE hardfiles
- WinNT devices with the 'native driver' written by Dan Sutherland
- mount/unmount/create a device (real one or a dump file),
- mount/unmount/create a volume (partition),
- create/open/close/delete/rename/undel a file,
- read/write bytes from/to a file,
- create/delete/rename/move/undel a directory,
- get directory contents, change current directory, get parent directory
- use dir cache to get directory contents.


It is written in portable C, and support the WinNT platform to access
real drives.

---

unADF is a unzip like for .ADF files :


unadf [-lrcsp -v n] dumpname.adf [files-with-path] [-d extractdir]
    -l : lists root directory contents
    -r : lists directory tree contents
    -c : use dircache data (must be used with -l)
    -s : display entries logical block pointer (must be used with -l)

    -v n : mount volume #n instead of default #0 volume

    -p : send extracted files to pipe (unadf -p dump.adf Pics/pic1.gif | xv -)
    -d dir : extract to 'dir' directory



Credits:
--------

main design and code             Laurent Clevy
Bug fixes and C++ wrapper        Bjarke Viksoe (adfwrapper.h)
WinNT native driver              Dan Sutherland and Gary Harris


New versions and contact e-mail can be found at : 

http://lclevy.free.fr/adflib



COMPILATION
-----------

It had been tested on Intel/Linux with gcc 2.7.2, Solaris 2.6, and
Win32.

Update (march 2006): 
 Makefiles has been modified to compile under Cygwin and gcc 3.4.4. (still 6 ISO C warning : normal)

The size of long must be 4, the size of short 2.
The library reads disk sectors written with the big endian (Motorola) byte
ordering.

You have to type :

make clean
make dep
make lib

A 'lidadf.a' should be created.


* Byte ordering

'make clean' remove the temporary files and the 'defendian.h'. In this file,
LITT_ENDIAN must be defined if the target machine uses the little endian 
byte ordering, like this :

#ifndef LITT_ENDIAN
#define LITT_ENDIAN 1
#endif /* LITT_ENDIAN */

This should be done automatically by the 'myconf' shell script. myconf 
autocompiles a C file which detects the byte ordering. The 'defendian.h'
is generated in 'myconf'. 'defendian.h' should be included in every .c file
which uses the LITT_ENDIAN define is used, otherwise the compiler could think
it is not defined, and the target machine is (always) using the big endian
byte ordering.

'myconf' is launched by 'make depend'.


* Native driver

The NATIV_DIR variable is used to choose the (only one) target platform
of the native driver. The default is :

NATIV_DIR = ./Generic

This one do not give access to any real device. The other one available is
Win32, to access real devices under WinNT.


* Win32DLL

The 'prefix.h' is used to create the Win32 DLL version of the library.
If the WIN32DLL variable is defined in the library code, public functions
are preceded by the '__declspec(dllexport)' directive. If this same
variable is defined, the '__declspec(dllimport)' is put before the functions
prototypes in the 'adflib.h' library include file.




FILES
-----

AUTHORS   Contributors
README			The file you are reading
TODO			Future improvements and bugfixes
CHANGES			Detailed changes
Lib/			main library files
Lib/Win32/		WinNT native driver
Lib/Generic/    native files templates
Boot/			Bootblocks that might by used to put on floppy disks
Docs/			The library developpers documentation 
Faq/			The Amiga Filesystem explained
Test/			Test files and examples (not perfect)
Dumps/			.ADF used to test the library
Refs/			text outputs of the test files
Check/			files stored in the test dumps, used with 'diff'
Bonus/			Additional useful .ADF dumps
Demo/           unadf.c


Possible bugs
-------------

- in dircache updates
- when a volume is becoming full
- lost memory releases


Please report any bugs or mistakes in the documentation !



Have fun anyway !
