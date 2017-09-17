/* Tests wide character conversion functions.
 */
#include <testfwk.h>

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L
#include <wchar.h>
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <uchar.h>
#endif

static void
testwcharnorestart(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL))
	wchar_t w;
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbtowc(&w, c, 1) == 1);
	ASSERT(wctomb(c, w) == 1);
	ASSERT(c[0] == 'C');
	ASSERT(wctob(btowc('C')) == 'C');
	ASSERT(btowc(EOF) == WEOF);
	ASSERT(wctob(WEOF) == EOF);
#endif
}

static void
testwcharstringnorestart(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L && !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) // Not enough memory
	wchar_t wcs1[5] = L"Test";
	wchar_t wcs2[5];
	char mbs[5 * MB_LEN_MAX];

	// Test basic functionality
	ASSERT(wcslen (wcs1) == 4);
	ASSERT(wcstombs(mbs, wcs1, 5 * MB_LEN_MAX) > 0);
	ASSERT(mbstowcs(wcs2, mbs, 5) > 0);
	ASSERT(wcs1[3] == L't');
	ASSERT(wcs2[3] == L't');
	ASSERT(!wcscmp(wcs1, wcs2));

	// Test for 0-terminated strings
	ASSERT(wcstombs(mbs, wcs1, 1000) > 0);
	ASSERT(mbstowcs(wcs2, mbs, 1000) > 0);
	ASSERT(!wcscmp(wcs1, wcs2));

	// Test for unterminated strings
	mbs[2] = 0;
	wcs2[2] = 0;
	ASSERT(wcstombs(mbs, wcs1, 2) == 2);
	ASSERT(!strcmp("Te", mbs));
	ASSERT(mbstowcs(wcs2, mbs, 2) == 2);
	ASSERT(!wcscmp(L"Te", wcs2));
#endif
}

static void
testwcharrestart(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL))
	static mbstate_t ps;
	wchar_t w;
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbrtowc(&w, c, 1, &ps) == 1);
	ASSERT(wcrtomb(c, w, &ps) == 1);
	ASSERT(c[0] == 'C');
	ASSERT(mbrtowc(&w, c, 1, &ps) == mbrlen(c, 1, &ps));
#endif
}

static void
testchar16restart(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL))
	static mbstate_t ps;
	char16_t c16[2];
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbrtoc16(c16, c, 1, &ps) == 1);
	ASSERT(c16rtomb(c, c16[0], &ps) == 1);
	ASSERT(c[0] == 'C');
#endif
}

static void
testchar32restart(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL))
	static mbstate_t ps;
	char32_t c32[2];
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbrtoc32(c32, c, 1, &ps) == 1);
	ASSERT(c32rtomb(c, c32[0], &ps) == 1);
	ASSERT(c[0] == 'C');
#endif
}

