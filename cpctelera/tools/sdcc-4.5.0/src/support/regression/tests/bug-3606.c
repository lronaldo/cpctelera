/* bug-3606.c
   Codegen for sm83 could geneate code for a left shift of a 16-bit variable that would result in stack corruption.
 */

#include <testfwk.h>

#include <stdint.h>

#define FALSE 0
#define TRUE 1

typedef enum {
    DIR_UP = 0U,
    DIR_DOWN = 24U,
    DIR_LEFT = 36U,
    DIR_RIGHT = 12U
} DIRECTION;

#define PLAYER_X_LEFT_BOUND_PX   8U
#define PLAYER_X_CENTER_BOUND_PX 88U
#define PLAYER_X_RIGHT_BOUND_PX  160U
#define PLAYER_Y_UP_BOUND_PX     16U
#define PLAYER_Y_CENTER_BOUND_PX 80U
#define PLAYER_Y_DOWN_BOUND_PX   152U
#define PLAYER_SPEED    21
#define LEFT_BOUND 48
#define RIGHT_BOUND 652
#define TOP_BOUND 48
#define BOTTOM_BOUND 652
 
typedef struct PlayerObject {
    uint8_t spriteId;
    uint8_t animFrame;
    uint16_t xSpr;
    uint16_t ySpr;
    int16_t xMap;
    int16_t yMap;
    uint8_t xTile;
    uint8_t yTile;
    uint8_t topOffsetInPx;
    uint8_t bottomOffsetInPx;
    uint8_t leftOffsetInPx;
    uint8_t rightOffsetInPx;
    DIRECTION dir;
    uint8_t moveSpeed;
    int8_t xVel;
    int8_t yVel;
    uint8_t hpMax;
    uint8_t hpCur;
} PlayerObject;


uint8_t i;
uint8_t j;
uint8_t k;

#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory
static uint8_t playGrid[32U][32U]; // We need the array sizes of 32 to trigger the bug.
#endif

#define STARTPOS 0U
#define STARTCAM 0U

static uint8_t roomId;
static uint8_t gridW;
static uint8_t gridH;
static uint16_t camera_max_x = 10U * 16U;
static uint16_t camera_max_y = 9U * 16U;
static uint8_t walkableTileCount = 6U;

static uint16_t camera_x = STARTCAM, camera_y = STARTCAM, new_camera_x = STARTCAM, new_camera_y = STARTCAM;
static uint8_t map_pos_x = STARTPOS, map_pos_y = STARTPOS, new_map_pos_x = STARTPOS, new_map_pos_y = STARTPOS;
static uint8_t redraw;

PlayerObject player;

static void func_1(void);
static void func_2(void);

void
testBug (void)
{
    player.ySpr = 16;
    func_1();
}

static void func_1(void)
{
    player.xVel = player.moveSpeed;
    player.yVel = 0;
    player.dir = DIR_RIGHT;

    func_2();
}

static void func_2(void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory
    int16_t x = player.xSpr + player.xVel;
    int16_t y = player.ySpr + player.yVel;

    uint8_t topOffset = 8;
    uint8_t bottomOffset = 1;
    uint8_t leftOffset = 4;
    uint8_t rightOffset = 5;

    // If you comment/uncomment one of these...
    static uint8_t playerTopMetatileIndex;
    playerTopMetatileIndex = (y) - 16U;

    uint8_t playerBottomMetatileIndex = y;
    uint8_t playerLeftMetatileIndex = x;
    uint8_t playerRightMetatileIndex = x;

    uint8_t collided = TRUE;
    switch (player.dir)
    {
        case DIR_UP:
            if ((playGrid[playerTopMetatileIndex][playerLeftMetatileIndex] < walkableTileCount)
             && (playGrid[playerTopMetatileIndex][playerRightMetatileIndex] < walkableTileCount))
                collided = FALSE;
            break;
        case DIR_DOWN:
            if ((playGrid[playerBottomMetatileIndex][playerLeftMetatileIndex] < walkableTileCount)
             && (playGrid[playerBottomMetatileIndex][playerRightMetatileIndex] < walkableTileCount))
                collided = FALSE;
            break;
        case DIR_LEFT:
            if ((playGrid[playerTopMetatileIndex][playerLeftMetatileIndex] < walkableTileCount)
             && (playGrid[playerBottomMetatileIndex][playerLeftMetatileIndex] < walkableTileCount))
                collided = FALSE;
            break;
        case DIR_RIGHT:
            if ((playGrid[playerTopMetatileIndex][playerRightMetatileIndex] < walkableTileCount)
             && (playGrid[playerBottomMetatileIndex][playerRightMetatileIndex] < walkableTileCount))
                collided = FALSE;
            break;
    }


    i = (x) - 8U;
    j = (y) - 16U;

    if (collided == FALSE)
    {
        player.xSpr = x;
        player.ySpr = y;
    }
    else
    {
        switch (player.dir)
        {
            case DIR_UP:    player.ySpr = (playerTopMetatileIndex);   break;
            case DIR_DOWN:  player.ySpr = (playerBottomMetatileIndex);  break;
            case DIR_LEFT:  player.xSpr = (playerLeftMetatileIndex + 1U); break;
            case DIR_RIGHT: player.xSpr = (playerRightMetatileIndex);     break;
            default: break;
        }

        player.xVel = 0U;
        player.yVel = 0U;
    }
#endif
}

