/*
   bug-3322.c
   A z80 code generation (overwriting a value in iy during an assignment) bug found by z88dk
   */

#include <testfwk.h>

#define PIECE_IS_NEXT 255

#define STARTBOARD_X 15
#define STARTBOARD_Y 2

#define POS_NEXT_X 3
#define POS_NEXT_Y 1

#define NEXT_X (POS_NEXT_X+2)
#define NEXT_Y (POS_NEXT_Y+3)

#define EMPTY_GR_CHAR  32
#define EMPTY_GR_COLOR 0
#define FONTS 0

typedef unsigned char byte;

// tetromino sprite
typedef struct {
   int x;          // signed integers to allow negative board positions
   int y;          // -1 means piece is preview ("next")
   byte piece;
   byte angle;
} sprite;

#define gr4_tile(x,y,tile,color,tileset) _gr4_tile_x = (x); _gr4_tile_y = (y); _gr4_tile_tile = (tile); _gr4_tile_color = (color); _gr4_tile_tileset = (tileset); _gr4_tile()

byte _gr4_tile_x;
byte _gr4_tile_y;
byte _gr4_tile_tile;
byte _gr4_tile_color;
byte *_gr4_tile_tileset;

byte *_gr4_tile_source;
byte *_gr4_tile_dest;

void _gr4_tile() {
}

// offset of a single tile composing a piece
typedef struct {
   byte offset_x;
   byte offset_y;
} tile_offset;

tile_offset *get_piece_offsets(byte piece, byte angle);

tile_offset pieces_XY[4] = {  
   {+1,+1}  ,  {+2,+1}  ,  {+3,+1}  ,  {+1,+2}  ,
};

void SLOT1_VIDEO_START() {
}

void SLOT1_END() {
}

// given a piece number and an angle returns the 4 byte "offsets" of the piece
tile_offset *get_piece_offsets(byte piece, byte angle) {
   return &pieces_XY[(piece*sizeof(tile_offset)*4*2)+angle*4];
}

void gr_erasepiece(sprite *p);

// erase piece from the screen
void gr_erasepiece(sprite *p) {
   tile_offset *data = get_piece_offsets(p->piece, p->angle);   // bug if this declaration is here

   int px = p->x;
   int py = p->y;

   // are we erasing the "next" piece ?
   if(py==PIECE_IS_NEXT) {
      px = NEXT_X;
      py = NEXT_Y;
   }
   else {
      px += STARTBOARD_X;
      py += STARTBOARD_Y;
   }

   // tile_offset *data = get_piece_offsets(p->piece, p->angle);   // no bug if this declaration is here

   SLOT1_VIDEO_START();
   for(byte t=0; t<4; t++) {
      int x = px + data->offset_x;
      int y = py + data->offset_y;      
      data++;
      gr4_tile((byte)x,(byte)y,EMPTY_GR_CHAR,EMPTY_GR_COLOR,FONTS);
   }
   SLOT1_END();
}

void testBug(void)
{
   sprite s;

   s.x = 0;
   s.y = 0;
   s.piece = 0;
   s.angle = 0;
   gr_erasepiece(&s);
   ASSERT(_gr4_tile_x == s.x + STARTBOARD_X + pieces_XY[3].offset_x);
   ASSERT(_gr4_tile_y == s.y + STARTBOARD_Y + pieces_XY[3].offset_y);
}

