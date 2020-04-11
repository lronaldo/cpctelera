#include <gstring.h>
#include <iostream.h>
#include <fstream.h>

/**
 * Append and prepend data to a string.
 */
int main(void)
{
	gstring a;
	gstring foo = "Hello";
	gstring bar = "World!";

	cout << "Variable is initialized to nothing: " << a << endl;

	a.append(bar);
	cout << "String is appended to variable: " << a << endl;

	a.prepend(foo + " ");
	cout << "String is prepended to variable: " << a << endl;

	return 0;
}
