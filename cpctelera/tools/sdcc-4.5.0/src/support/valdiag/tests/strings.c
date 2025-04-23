#include <wchar.h>
#include <uchar.h>

// Concatenation of string literals, some prefixed, all prefixed ones have same prefix. Allowed.
#ifdef TEST1
const char *str1 = u8"testu8" "test" u8"testu8";
const char *str2 = u8"testu8" u8"testu8";
const char *str3 = u8"testu8" u8"testu8" u8"testu8";
const char16_t *str4 = u"testu8" "test";
const char32_t *str5 = U"testu8" "test";
#endif

// Concatenation of string literals, some prefixed, prefixed have different prefixes. Implementation-defined in C11, C17. Requires diagnostic in C23.
#ifdef TEST2
const char *str1 = u8"testu8" "test" u"testu"; /* WARNING(SDCC) */ /* IGNORE(GCC) */
#endif

#ifdef TEST3
const char *str1 = u8"testu8" "test" L"testL"; /* WARNING(SDCC) */ /* IGNORE(GCC) */
#endif

#ifdef TEST4
const char *str1 = L"testuL" "test" u"testu"; /* WARNING(SDCC) */ /* IGNORE(GCC) */
#endif

#ifdef TEST5
const char32_t *str1 = U"testU" "test" u"testu"; /* WARNING(SDCC) */ /* IGNORE(GCC) */
#endif

#ifdef TEST6
const char32_t *str1 = U"testU" u"testu"; /* WARNING(SDCC) */ /* IGNORE(GCC) */
#endif

#ifdef TEST7
const char32_t *str1 = U"testU" L"testL"; /* WARNING(SDCC) */ /* IGNORE(GCC) */
#endif

#ifdef TEST8
const wchar_t *str1 = L"testL" u"testu"; /* WARNING(SDCC) */ /* IGNORE(GCC) */
#endif

