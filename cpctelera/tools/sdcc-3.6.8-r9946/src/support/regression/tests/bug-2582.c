/*
   bug-2195.c

   A z80 parameter passing bug.
*/

#include <testfwk.h>

typedef unsigned char u8;

typedef struct WindowPositionStruct
{
    u8 mX;
    u8 mY;
} WindowPosition;

typedef struct WindowSizeStruct
{
    u8 mW;
    u8 mH;
} WindowSize;

typedef struct WindowPositionAndSizeStruct
{
    WindowPosition mPosition;
    WindowSize mSize;
} WindowPositionAndSize;

typedef struct WindowRenderStyleStruct
{
    u8 mStateColor[4];
} WindowRenderStyle;

typedef struct WindowStruct
{
    WindowPositionAndSize mPositionAndSize;

    WindowRenderStyle* mpStyle;
} Window;

void BG_FillRect( u8 x, u8 y, u8 w, u8 h, u8 f)
{
    ASSERT(x == 1);
    ASSERT(y == 2);
    ASSERT(w == 3);
    ASSERT(h == 4);
    ASSERT(f == 0);
}

static Window gWindow;
static WindowRenderStyle gWindowRenderStyle;

void testBug(void)
{
    u8 state = 0;
    gWindow.mpStyle = &gWindowRenderStyle;
    gWindow.mPositionAndSize.mPosition.mX = 1;
    gWindow.mPositionAndSize.mPosition.mY = 2;
    gWindow.mPositionAndSize.mSize.mW = 3;
    gWindow.mPositionAndSize.mSize.mH = 4;

    {
        Window* pWindow = &gWindow;

        BG_FillRect( 
            pWindow->mPositionAndSize.mPosition.mX,
            pWindow->mPositionAndSize.mPosition.mY,
            pWindow->mPositionAndSize.mSize.mW,
            pWindow->mPositionAndSize.mSize.mH, 
            pWindow->mpStyle->mStateColor[state]
        );
    }
}

