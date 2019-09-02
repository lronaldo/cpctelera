#include <gstring.h>
#include <iostream.h>
#include <fstream.h>

/**
 * Uses of explode and implode.  Note: there are no memory leaks here.
 */
int main(void)
{
	gstring a = "drewpc:x:38241:29:Drew Philip C:/home/cia/drewpc:/usr/local/bin/tcsh";
	gstring token = ":";
	gstring bar;
	gstring* foo;  // Array of gstrings.
	int nfields = a.nfields(token);

	cout << "Variable is initialized to a string: " << a << endl;
	cout << "Variable is seperated by token: '" << token << "'" << endl;

	// explode() allocates memory for each array index automatically.
	foo = a.explode(token);

	for(int i = 0; i < nfields; i++) {
		cout << "Array[" << i << "]: " << foo[i] << endl;
	}

	// implode puts foo back together, separated by token.
	bar = implode(foo, token, nfields);
	cout << "Variable is set to implosion of array: " << bar << endl;

	// you do have to delete foo, or you'll have memory leaks.
	delete [] foo;

	return 0;
}
