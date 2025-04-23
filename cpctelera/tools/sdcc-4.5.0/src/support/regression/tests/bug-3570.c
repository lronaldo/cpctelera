/* bug-3570.c
   A comparison between pointers resulting in an invalid compile time error during AST proccessing.
 */

#include <testfwk.h>

unsigned char *LevelMapGet(void) { return 0; }

typedef struct game_data {
    unsigned char *current;
} MapData;


MapData mapData;

void
testBug(void) {
    if ( mapData.current + 5 >= LevelMapGet() )
        return;

    if ( &mapData.current[5] >= LevelMapGet() ) // This comparison failed to compile
        return;
}

