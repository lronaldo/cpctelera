#include <gstring.h>
#include <iostream.h>
#include <fstream.h>

/**
 * Basic uses of the gstring.
 */
int main(void)
{
	gstring a = "Hello";

	cout << "Variable is set to: " << a << endl;

	a = a + " ";
	a += "World!";
	cout << "String is added to variable: " << a << endl;

	return 0;
}
