/* bug-3459.c
   A bug in code generation for restoring registers after a builtin call.
 */

#include <testfwk.h>

#pragma disable_warning 85

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <string.h>
#include <stdint.h>
#include <stdarg.h>

const char TextPunc[] = ":%! ";

#define TEXT_SPACE ( TEXT_PUNC + 3 )
#define TEXT_UPPER ( TEXT_NUM + 10 )
#define TEXT_PUNC ( TEXT_UPPER + 26 )
#define TEXT_NUM ( TEXT_START + 0 )
#define TEXT_START 0

int myprintf( const char *restrict format, ... );

void ImageCompleteText( uint8_t x, uint8_t y, const char *text ) {

    while( *text ) {

        uint8_t ch = *text++;

        uint8_t index = 0;
        uint8_t start = TEXT_SPACE;

        if ( ch == ' ' ) ;
        else{

            char *ptr = strchr( TextPunc, ch ); // One of the two bytes of the result of this builtin call got corrupted.
            if ( ptr )
            {
                index = ptr - TextPunc;
                start = TEXT_PUNC;
            }
        }

        myprintf( "%d %d %c\n", index, start, ch );

        x += 2;
    }
}

int myprintf ( const char *restrict format, ... )
{
	int index, start, ch;
	va_list v;
	va_start ( v, format );
	index = va_arg ( v, int );
	ASSERT (index == 2);
	start = va_arg ( v, int );
	ASSERT (start == 36);
	ch = va_arg ( v, int );
	ASSERT (ch == '!');
	va_end ( v );
}

void
testBug( void) {
    ImageCompleteText(0,0,"!");
}

