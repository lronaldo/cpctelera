/* Tests new wide character conversion functions.
 */
#include <testfwk.h>

#include <string.h>

#if defined(__SDCC) || defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__APPLE__) && !defined(__OpenBSD__) // As of 2023, macOS and OpenBSD are still not fully C11-compliant: they lack uchar.h.
#include <uchar.h>
#endif

#define BUF_MAX 40

void testW(void)
{
#ifndef PORT_HOST
#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Not enough memory
	char mb_buffer[BUF_MAX + 1];
	char16_t c16_buffer[BUF_MAX + 1];

	ASSERT(__c16stombs(mb_buffer, u"teststring", BUF_MAX) == strlen("teststring"));

	ASSERT(!strcmp(mb_buffer, "teststring"));

	ASSERT(__mbstoc16s(c16_buffer, "teststring2", BUF_MAX) == strlen("teststring2"));

	ASSERT(!memcmp (c16_buffer, u"teststring2", 12 * 2));

	ASSERT(__c16stombs(mb_buffer, c16_buffer, BUF_MAX) == strlen("teststring2"));

	ASSERT(!strcmp(mb_buffer, "teststring2"));

	__c16stombs(mb_buffer, u"Größere Körbe kosten 10 € mehr", BUF_MAX);
	ASSERT(!strcmp(mb_buffer, "Größere Körbe kosten 10 € mehr"));

	__mbstoc16s(c16_buffer, "Größere Körbe kosten 12 € mehr", BUF_MAX);

	ASSERT(!memcmp (c16_buffer, u"Größere Körbe kosten 12 € mehr", 20 * 2));

	__c16stombs(mb_buffer, c16_buffer, BUF_MAX);

	ASSERT(!strcmp(mb_buffer, "Größere Körbe kosten 12 € mehr"));
#endif
#endif
}

