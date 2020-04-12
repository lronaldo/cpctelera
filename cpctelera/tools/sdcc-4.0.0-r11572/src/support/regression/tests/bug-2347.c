/* bug-2347.c
   An bug in marking rematerializeable variables
 */

#include <testfwk.h>

#include <string.h>

const unsigned char star_star_filename[] = { '*', '.', '*', 0xff };

void
dos_catalog(const char *filename)
{
        char    filename_copy[10];
        int     len = 10;

        if (filename == NULL)
                filename = star_star_filename;

        len = strlen(filename);

        memcpy(filename_copy, filename, len); // This memcpy() always used star_star_filename as source.

	ASSERT(filename_copy[0] == 'X');
}

void testBug(void)
{
	dos_catalog("X");
}

