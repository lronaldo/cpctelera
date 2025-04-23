/* bug-3456.c
   Invalid asm generated when using -mz80 --allow-undocumented-instructions.
 */

#include <testfwk.h>

#pragma disable_warning 85
#pragma disable_warning 155

/// Test derived from code by SF user "Under4Mhz" in 2022.
// GPL 2.0
#include <stdbool.h>

typedef struct  { int x; int y; } maths_point3d;

bool maths_clip_visible( const maths_point3d *point ) __z88dk_fastcall;
void maths_clip_points( maths_point3d *first, maths_point3d *second );

///< \brief Draw lines using two points
bool maths_clip_line( maths_point3d *first, maths_point3d *second )
{
    bool c1 = maths_clip_visible( first );
    bool c2 = maths_clip_visible( second );
    bool onscreen = true;

    // On screen
    if ( c1 && c2 ) {

    // Off screen
    } else if ( !c1 && !c2 ) {

        onscreen = false;

        // Clip
    } else {

        maths_clip_points( first, second );
    }

    return onscreen;
}

void testBug( void ) {
	maths_point3d p;
    ASSERT ( maths_clip_line (&p, &p) );
    ASSERT ( !maths_clip_line (0, 0) );
}

bool maths_clip_visible( const maths_point3d *point ) __z88dk_fastcall
{
	return point;
}

void maths_clip_points( maths_point3d *first, maths_point3d *second )
{
	ASSERT ( 0 );
}

