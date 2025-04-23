// dummies for unused functions in cpp and cc1

#ifdef CC1_DUMMIES
#define SDCPP_DUMMY_FCT() fprintf(stderr, "unreachable dummy in cc1%s\n", __func__); gcc_assert(false)
#define SDCPP_DUMMY_FCT_() // fprintf(stderr, "dummy in cc1%s\n", __func__)
#else
#define SDCPP_DUMMY_FCT() fprintf(stderr, "unreachable dummy %s\n", __func__); gcc_assert(false)
#define SDCPP_DUMMY_FCT_()  // fprintf(stderr, "dummy %s\n", __func__)
#endif

#include "config.h"
#include "system.h"
#include "coretypes.h"

// #include "errors.h" // BUG?

// #include "tree-core.h"
// #include "langhooks.h"
// #include "langhooks-def.h"

#include <cassert>

#include "opt-suggestions.h"
option_proposer::~option_proposer ()
{ SDCPP_DUMMY_FCT_();
}
const char *host_detect_local_cpu (int, const char **)
{
  return NULL;
}

