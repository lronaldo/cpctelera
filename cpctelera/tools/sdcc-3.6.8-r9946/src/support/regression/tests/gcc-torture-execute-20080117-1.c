/*
   20080117-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

typedef struct gs_imager_state_s {
  struct {
    int half_width;
    int cap;
    float miter_limit;
  } line_params;
} gs_imager_state;
static const gs_imager_state gstate_initial = { { 1 } };
void gstate_path_memory(gs_imager_state *pgs) {
  /**pgs = gstate_initial; not yet supported by sdcc */
  memcpy (pgs, &gstate_initial, sizeof (gstate_initial));
}
int gs_state_update_overprint(void)
{
  return gstate_initial.line_params.half_width;
}

void
testTortureExecute (void)
{
  if (gs_state_update_overprint() != 1)
    ASSERT (0);
  return;
}

