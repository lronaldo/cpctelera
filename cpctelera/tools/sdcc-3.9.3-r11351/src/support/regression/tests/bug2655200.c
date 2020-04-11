/*
 * [ 2655200 ] pointer to pdata memory not correctly initialized
 *
 * test_array[..] = &thing_pdata
 * was incorrect
 *
 * SDCCclue.c(initPointer)
 * ... SPEC_SCLS (expr->left->etype) == S_PDATA)
 * was not handeled
 */

#include <testfwk.h>
#include <stdint.h>

char thing;
#if defined (__SDCC_mcs51) || defined (__SDCC_ds390)
__code char thing_code = 0;
__data char thing_data;
__idata char thing_idata;
__xdata char thing_xdata;
__pdata char thing_pdata;
__pdata char thing_apdata[2];
#endif


const char * __code test_array[] = {
 &thing
#if defined (__SDCC_mcs51) || defined (__SDCC_ds390)
 , &thing_code
 , &thing_data, &thing_idata, &thing_xdata, &thing_pdata
 , thing_apdata, (char *)thing_apdata
#endif
};


const char *gime_thing() { return &thing; }
#if defined (__SDCC_mcs51) || defined (__SDCC_ds390)
const char *gime_thing_code() { return &thing_code; }
const char *gime_thing_data() { return &thing_data; }
const char *gime_thing_idata() { return &thing_idata; }
const char *gime_thing_xdata() { return &thing_xdata; }
const char *gime_thing_pdata() { return &thing_pdata; }
const char *gime_thing_apdata() { return thing_apdata; }
#endif


void
testBug(void)
{
 ASSERT(test_array[0] == gime_thing());

#if defined (__SDCC_mcs51) || defined (__SDCC_ds390)
 ASSERT(test_array[1] == gime_thing_code());
 ASSERT(test_array[2] == gime_thing_data());
 ASSERT(test_array[3] == gime_thing_idata());
 ASSERT(test_array[4] == gime_thing_xdata());
 ASSERT(test_array[5] == gime_thing_pdata());
 ASSERT(test_array[6] == gime_thing_apdata());
 ASSERT(test_array[7] == gime_thing_apdata());
#endif
}
