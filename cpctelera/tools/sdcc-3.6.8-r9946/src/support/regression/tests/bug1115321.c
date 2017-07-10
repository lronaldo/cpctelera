/*
 *  simple, small and self contained snprintf() function.
 *  Copyright (C) 2005  Weston Schmidt
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <testfwk.h>
#include <stdarg.h>
#include <string.h>

/*
 *  General information about this file:
 *
 *  This file has been created with the ability to be compiled using sdcc
 *  and gcc both.  If gcc is used, the output file can be executed and
 *  the built in test vectors will be run.
 *
 *  The test vectors I created are very simple and in no way test
 *  everything that should be tested or really come close.  The output
 *  is visually inspected for correctness.  Additionally, I have not
 *  done a thorough job testing this to ensure that no buffer overruns
 *  take place.  Logically, everything looks pretty safe, and it works
 *  for the debug generation that I needed it to work for.
 *
 *  I ran into problems when running this on my pic because I was allocating
 *  80 byte long arrays on the stack (which was only 128 bytes is size).
 *  The end result was "it just didn't work" with no real hints.  So if
 *  this happens to you, think about up-ing the stack size or moving your
 *  arrays out of automatic stack variables (defined inside {}s, which
 *  scope the variable) and out to staticly declared memory (aka "global"
 *  variables).
 *
 *  I hope this saves you the day it took me to write and debug.
 */

#include <stdio.h>

#define SUPPORT_CHAR_C

/* Normally a char is promoted to int when passed as varargs parameter */
/* but SDCC leaves it as char. */
#ifndef __SDCC
#   define VA_ARG_CHAR int
#   define VA_ARG(args,type) va_arg((args),type)
#   define SDCC_SNPRINTF    sdcc_snprintf
#else
#   define VA_ARG_CHAR char
#   define VA_ARG(args,type) va_arg((args),type)
#   define SDCC_SNPRINTF    snprintf
#endif

/*
 *  snprintf is a bound version of the popular sprintf() function.
 *  The purpose of snprintf is to format just like sprintf(), but
 *  to make sure that memory is never accidentally overwritten by
 *  a buffer overrun.
 *
 *  A stripped down version of snprintf for the pic16 platform of the
 *  sdcc project.  Depending on the compile flags set/unset above this
 *  function varies in size from ~16k down to ~5.  Below is the
 *  breakdown:
 *
 *  SUPPORT_CHAR_C  - c  format  - ~0.5k of program space required
 *  SUPPORT_STRING  - s  format  - ~1.6k of program space required
 *
 *  I don't think I have any type of optimization on when I did these
 *  measurements.  They were taken with a snapshot build of sdcc from
 *  January 22, 2005 (end of day).
 *
 *  Supported formatting:
 *  %%, %c, %[ 0][1-9][0-9]*s
 *             \
 *              \-- optional padding value
 *
 *  For more detail on what this formatting means, google 'man printf'
 *
 *  Parameters:
 *  ----------------------------------------------------------------------
 *  char *buffer             -- the buffer to write into (assumed to
 *                              not be NULL)
 *  const unsigned char size -- the maximum number of bytes to write
 *                              into the buffer
 *  const char *format       -- the printf style formatter string (see
 *                              above for supported formats.
 *
 *  Return Values:
 *  ----------------------------------------------------------------------
 *  The actual number of characters written into the buffer.  The count
 *  includes the '\0' located at the end of the string.  If there is
 *  not enough room for the entire string, the '\0' value is written in
 *  as the buffer[end - 1] value.
 */
unsigned char SDCC_SNPRINTF( char *buffer, const unsigned char size,
                             const char *format, ... )
{
    va_list args;
    char *start = buffer;
    char *end   = &buffer[size];

    va_start( args, format );

    while( (buffer <  end) && ('\0' != *format) ) {
        if( '%' == *format ) {
            format++;
            switch( *format ) {
#ifdef SUPPORT_CHAR_C
                case 'c':
                    *buffer = VA_ARG( args, VA_ARG_CHAR );
                    buffer++;
                    break;
#endif /* SUPPORT_CHAR_C */

                case '%':
                    *buffer = '%';
                    buffer++;
                    break;

                default:
                {
                    char padding = ' ';
                    short digits = 0;

                    /* Determine the padding */
                    switch( *format ) {
                        case '0':
                            padding = '0';
                        case ' ':
                            format++;
                            break;
                    }

                    /* Determine how many digits to display */
                    while(    ('\0' != *format)
                           && ((*format >= '0') && (*format <= '9')) )
                    {
                        digits = digits * 10 + (*format - '0');
                        format++;
                    }

                    switch( *format ) {
                        case 's':
                        {
                            char *val, *val_end;
                            short length;

                            val = VA_ARG( args, char * );
                            val_end = val;

                            /* Find the end of the string. */
                            while( '\0' != *val_end ) { val_end++; }

                            length = val_end - val;

                            /* Optionally crop the output string */
                            if( (digits > 0) && (length > digits) ) {
                                length = digits;
                            }

                            /* Add padding */
                            while( (digits > length) && (buffer < end) ) {
                                *buffer = padding;
                                buffer++;
                                digits--;
                            }

                            /* Copy string */
                            while( (length > 0) && (buffer < end) ) {
                                *buffer = *val;
                                buffer++;
                                val++;
                                length--;
                            }

                            break;
                        }

                        default:
                            goto clean_and_bail;
                    }
                    break;
                }
            }
        } else {
            *buffer = *format;
            buffer++;
        }

        format++;
    }

clean_and_bail:
    if( buffer < end ) {
        *buffer = '\0';
        buffer++;
    } else {
        *(buffer-1) = '\0';
    }

    va_end( args );

    return (buffer - start);
}

void test_s( void )
{
    char buffer[32];
    int ret;
    int i = 0;

    for( i = 0; i < 31; i++ ) {
        buffer[i] = 'P';
    }
    buffer[31] = '\0';

    ret = SDCC_SNPRINTF( buffer, 32, "->|%%|<-" );
//    printf( "->|%s|<- %d \n", buffer, ret );
    ASSERT (strcmp(buffer, "->|%|<-") == 0);
    ASSERT (ret == 8);
    ret = SDCC_SNPRINTF( buffer, 32, "%s, %s.", "Hello", "world" );
//    printf( "->|%s|<- %d \n", buffer, ret );
    ASSERT (strcmp(buffer, "Hello, world.") == 0);
    ASSERT (ret == 14);
    ret = SDCC_SNPRINTF( buffer, 32, "% 10s, % 10s.", "Hello", "world" );
//    printf( "->|%s|<- %d \n", buffer, ret );
    ASSERT (strcmp(buffer, "     Hello,      world.") == 0);
    ASSERT (ret == 24);
    ret = SDCC_SNPRINTF( buffer, 32, "% 3s, % 3s.", "Hello", "world" );
//    printf( "->|%s|<- %d \n", buffer, ret );
    ASSERT (strcmp(buffer, "Hel, wor.") == 0);
    ASSERT (ret == 10);
    ret = SDCC_SNPRINTF( buffer, 32, "%03s, %03s.", "Hello", "world" );
//    printf( "->|%s|<- %d \n", buffer, ret );
    ASSERT (strcmp(buffer, "Hel, wor.") == 0);
    ASSERT (ret == 10);
    ret = SDCC_SNPRINTF( buffer, 10, "%s", "Hello, world" );
//    printf( "->|%s|<- %d \n", buffer, ret );
    ASSERT (strcmp(buffer, "Hello, wo") == 0);
    ASSERT (ret == 10);
}

#if defined SDCC
extern void _putchar(char c);

int putchar(int c)
{
	_putchar(c);
	return(c);
}
#endif

