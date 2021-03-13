/* Tests wide character conversion functions.
 */
#include <testfwk.h>

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L
#include <wchar.h>
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <uchar.h>
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdint.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#ifdef __SDCC
_Static_assert(!WCHAR_MIN, "nonzero WCHAR_MIN");
_Static_assert(WEOF <= WINT_MAX, "WEOF out of wint_t range");
#endif
#endif
#endif

static void
testwcharnorestart(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Not enough memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
	wchar_t w;
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbtowc(&w, c, 1) == 1);
	ASSERT(wctomb(c, w) == 1);
	ASSERT(c[0] == 'C');
	ASSERT(wctob(btowc('C')) == 'C');
	ASSERT(btowc(EOF) == WEOF);
	ASSERT(wctob(WEOF) == EOF);

	ASSERT(wctomb(c, L'W') == 1);
	ASSERT(c[0] == 'W');
#ifdef __STDC_ISO_10646__
	ASSERT(wctomb(c, 0x110000) == -1); // Invalid: Out of 21-bit Unicode range.
	ASSERT(wctomb(c, 0xd800) == -1);   // Invalid: Unpaired UTF-16 surrogate.
	ASSERT(wctomb(c, 0xdfff) == -1);   // Invalid: Unpaired UTF-16 surrogate.
#endif
#endif
#endif
}

static void
testwcharstringnorestart(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L && !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Not enough memory
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

// glibc with _FORTIFY_SOURCE == 2 is not standard-compliant (and fails the tests below)
// However, Ubuntu decided to make _FORTIFY_SOURCE = 2 the default for GCC.
#if !(defined(__GNUC__) && _FORTIFY_SOURCE == 2)
	// Test for 0-terminated strings
	ASSERT(wcstombs(mbs, wcs1, 1000) > 0);
	ASSERT(mbstowcs(wcs2, mbs, 1000) > 0);
	ASSERT(!wcscmp(wcs1, wcs2));
#endif

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
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL))
	static mbstate_t ps;
	wchar_t w;
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbrtowc(&w, c, 1, &ps) == 1);
	ASSERT(w == (L"C")[0]);
	ASSERT(wcrtomb(c, w, &ps) == 1);
	ASSERT(c[0] == 'C');
	ASSERT(mbrtowc(&w, c, 1, &ps) == mbrlen(c, 1, &ps));
#ifdef __STDC_ISO_10646__
	errno = 0;
	ASSERT(wcrtomb(c, 0x110000, 0) == -1); // Invalid: Out of 21-bit Unicode range.
	ASSERT(errno == EILSEQ);
	ASSERT(wcrtomb(c, 0xd800, 0) == -1);   // Invalid: Unpaired UTF-16 surrogate.
	ASSERT(wcrtomb(c, 0xdfff, 0) == -1);   // Invalid: Unpaired UTF-16 surrogate.
#endif
#endif
#endif
}

static void
testchar16restart(void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL))
	static mbstate_t ps;
	char16_t c16[2];
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbrtoc16(c16, c, 1, &ps) == 1);
	ASSERT(c16[0] == (u"C")[0]);
	ASSERT(c16rtomb(c, c16[0], &ps) == 1);
	ASSERT(c[0] == 'C');

	ASSERT(c16rtomb(0, c16[0], 0) == 1);

#ifdef __STDC_UTF_16__
	ASSERT(c16rtomb(c, 0xdc00, 0) == -1);  // Invalid: Unpaired UTF-16 surrogate.
	ASSERT(errno == EILSEQ);

	errno = 0;
	ASSERT(c16rtomb(c, u'\0', 0) == 1);    // Converting a 0 character resets internal state.

#ifdef __SDCC // The standard was defective (fixed in C2X). SDCC always behaves according to the fixed standard.
	ASSERT(c16rtomb(c, 0xd800, 0) == 0);
	ASSERT(c16rtomb(c, 0xd800, 0) == -1);  // Invalid: Unpaired UTF-16 surrogate.

	ASSERT(errno == EILSEQ);
	errno = 0;
	ASSERT(c16rtomb(c, u'\0', 0) == 1);    // Converting a 0 character resets internal state.
#endif
#endif
#endif
#endif
}

static void
testchar32restart(void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL))
	static mbstate_t ps;
	char32_t c32[2];
	char c[MB_LEN_MAX];

	c[0] = 'C';
	ASSERT(mbrtoc32(c32, c, 1, &ps) == 1);
	ASSERT(c32[0] == (U"C")[0]);
	ASSERT(c32rtomb(c, c32[0], &ps) == 1);
	ASSERT(c[0] == 'C');
#endif
#endif
}

