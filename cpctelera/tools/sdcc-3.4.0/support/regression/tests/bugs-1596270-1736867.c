/*
        bugs 1596270 and 1736867.
*/

#include <testfwk.h>


typedef unsigned char UINT8;
typedef unsigned char S_GAMES_SI_CHARS;

typedef struct
{
  S_GAMES_SI_CHARS c;
  UINT8 x, y;
}S_GAMES_SI_BLOCK_STRUCT;
#define S_GAMES_SI_BLOCK_STRUCT_size sizeof(S_GAMES_SI_BLOCK_STRUCT)


static void
s_Games_SI_BlockInit(S_GAMES_SI_BLOCK_STRUCT *bl, UINT8
i, UINT8 x, UINT8 y)
{
  bl[i ].x = x - 1; bl[i ].y = y - 1;
  bl[i + 1].x = x ; bl[i + 1].y = y - 1;
  bl[i + 2].x = x + 1;
  bl[i + 2].y = y - 1;
  bl[i + 3].x = x - 1; bl[i + 3].y = y ;
  bl[i + 4].x = x ; bl[i + 4].y = y ;
  bl[i + 5].x = x + 1; bl[i + 5].y = y ;
  bl[i + 6].x = x - 1; bl[i + 6].y = y + 1;
  bl[i + 7].x = x ; bl[i + 7].y = y + 1;
  bl[i + 8].x = x + 1; bl[i + 8].y = y + 1;
}

void
testBug156270(void)
{
  S_GAMES_SI_BLOCK_STRUCT b[9];
  s_Games_SI_BlockInit(b, 0, 1, 1);
  ASSERT(b[1].x == 1); /* 1596270 */
  ASSERT(b[1].y == 0); /* 1596270 */
  ASSERT(b[2].x == 2); /* 1596270 */
  ASSERT(b[2].y == 0); /* 1736867 */
}

