/* bug-3401.c
   Compiler crash in the frontend.
 */

#include <testfwk.h>

#pragma disable_warning 84
#pragma disable_warning 85

#include <stdint.h>
#define DAP_TRANSFER_RnW (1U << 1)

static uint8_t DAP_TransferBlock(const uint8_t *req, uint8_t *res)
{
    uint8_t num;

    num |= (4U + (((uint8_t)(*(req + 1)) | (uint8_t)(*(req + 2) << 8)) * 4)) << 16;

    return (num);
}

void testBug(void)
{
}

