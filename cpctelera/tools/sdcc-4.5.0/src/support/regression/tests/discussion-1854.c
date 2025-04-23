/* discussion-1854.c
 * Real-world code that triggers an unusual combination of peephole optimizer rules.
 * There was no bug here, but I still want to protect from regressions.
 */
 
#include <testfwk.h>
 
void twoDigitsFromNum( char* dest, unsigned number ) {
    dest[0] = '0' + ( number / 10 ) % 10;
    dest[1] = '0' +   number % 10;
}

void
testBug(void)
{
    char dest[2];
    twoDigitsFromNum( dest, 0 );
    ASSERT ( dest[0] == '0' && dest[1] == '0');
    twoDigitsFromNum( dest, 10 );
    ASSERT ( dest[0] == '1' && dest[1] == '0');
    twoDigitsFromNum( dest, 42 );
    ASSERT ( dest[0] == '4' && dest[1] == '2');
}

