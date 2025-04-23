/* bug-223113.c
   PENDING
 */
#include <testfwk.h>

int putch( int Ch )
{
  return( Ch );
}

int puts( const char *Str )
{
  const char *Ptr;

  for( Ptr = Str; *Ptr != '\0'; Ptr++ ) {
    putch( *Ptr );
  }

  return( (Ptr - Str) );
}

void __main( void )
{
  puts( "hello world\n" );
}

void testBug(void)
{
}
