/*
   bug3389647.c
   memory: __code, _STATMEM
*/

#include <testfwk.h>

#if defined (__SDCC)
  #pragma disable_warning 196 //no warning about pointer const qualifier (W_TARGET_LOST_QUALIFIER)
  #include <sdcc-lib.h> /* just to get _STATMEM */
#endif

typedef unsigned char hid_report_descriptor[8];

{memory} char dummy = 0; /*prevent hid_report_descriptor to land at address 0 */
{memory} hid_report_descriptor HIDREPORTDESC =
{
    0x06, 0x00, 0xff,   // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,         // USAGE (Vendor Usage 1)
    0xa1, 0x01,         // COLLECTION (Application)
};

unsigned char* DATAPTR1 = ({memory} unsigned char*)&HIDREPORTDESC;
unsigned char* DATAPTR2 = (         unsigned char*)&HIDREPORTDESC;

void testBug(void)
{
    unsigned char* DATAPTR3 = ({memory} unsigned char*)&HIDREPORTDESC;
    unsigned char* DATAPTR4 = (         unsigned char*)&HIDREPORTDESC;

    ASSERT(DATAPTR1 == DATAPTR2);
    ASSERT(DATAPTR2 == DATAPTR3);
    ASSERT(DATAPTR3 == DATAPTR4);
}
