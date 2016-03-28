// >> GPL info
// Macro: cpct_mirrorSpriteMasked
//   Mirrors a sprite with interlaced mask, left-to-right and viceversa
//
// C definition:
//   #define <cpct_mirrorSpriteMasked> (MODE, Sprite, Width, Height)
//
// Input parameters:
//    MODE (M0, M1, M2) - Capitals -
//    Sprite - u8* to the sprite
//    Width  - height / width
//
// Known limitations:
//   * This is a C macro, and it is not possible to use it from Assembly. If you 
// wanted to use it from assembly, call the equivalent cpct_mirrorSpriteXX function
// with double width for the sprite (as it has pairs of bytes with colours and mask
// instead of only colours). 
//
// Use example:
// (start code)
//   // Draws a car over a road in a required direction in mode 1
//   void drawCar(u8 x, u8 y) {
//      u8 pmem;
//
//      // Check if the user has pressed reverse key
//      if(cpct_isKeyPressed(Key_Space))
//         cpct_mirrorSpriteMasked(M1, carSprite, 8, 8);
//
//      // Draw the car on the road, at (x,y) location
//      pmem = cpct_getScreenPtr(CPCT_VMEM, x, y);
//      cpct_drawSprite(characterSprite, pmem, 8, 8);
//   }
// (end code)
//
#define cpct_mirrorSpriteMasked(MODE, SP, W, H) cpct_mirrorSprite ## MODE ((SP),2*(W),(H))
