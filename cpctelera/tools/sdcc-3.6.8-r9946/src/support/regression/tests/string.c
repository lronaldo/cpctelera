/** tests for strXXX
*/
#include <testfwk.h>
#include <string.h>
#include <stdlib.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L
#include <wchar.h>
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#include <uchar.h>
#endif

/** tests for strcmp
*/
static void 
do_teststrcmp (void)
{
  int result = strcmp ("", "");
  ASSERT (result == 0);
  
  result = strcmp ("", "a");
  ASSERT (result < 0);

  result = strcmp ("a", "");
  ASSERT (result > 0);

  result = strcmp ("ab", "ab");
  ASSERT (result == 0);

  result = strcmp ("aa", "ab");
  ASSERT (result < 0);
}

/** tests for strcpy
*/
static void 
do_teststrcpy (void)
{
  static char empty[] = "";
  static char string[] = "\1\2\0\3";
  char buf[40] = "abcdefghijklmnopqrstuvwxyz";

  char * result = strcpy (buf, empty);
  ASSERT (strlen (buf) == 0);
  ASSERT (result == buf);

  result = strcpy (buf, string);
  ASSERT (result == buf);
  ASSERT (strlen (buf) == 2);
  ASSERT (buf[0] == '\1');
  ASSERT (buf[1] == '\2');
  ASSERT (buf[3] == 'd');
}

/** tests for strncmp
*/
static void 
do_teststrncmp (void)
{
  ASSERT (strncmp ("", "", 0) == 0);
  ASSERT (strncmp ("ab", "ab", 0) == 0);
  ASSERT (strncmp ("a", "a", 2) == 0);
  ASSERT (strncmp ("aa", "ab", 1) == 0);
  ASSERT (strncmp ("aa", "ab", 2) < 0);
  ASSERT (strncmp ("abc", "abd", 2) == 0);
  ASSERT (strncmp ("abc", "abc", 3) == 0);
}

/** tests for strpbrk
 * related to bug #2908537
*/
static void
do_teststrpbrk (void)
{
  const char *a = "test";

  ASSERT (strpbrk (a, "e")  == &a[1] );
  ASSERT (strpbrk (a, "z")  == NULL );
  ASSERT (strpbrk (a, "et") == &a[0] );
  ASSERT (strpbrk (a, "ze") == &a[1] );
  ASSERT (strpbrk (a, "")   == NULL );
  ASSERT (strpbrk ("", "e") == NULL );
  ASSERT (*strpbrk ("test2", "s") == 's' );
}

/** tests for strrchr
*/
static void
do_teststrrchr (void)
{
  const char *test = "test";

  ASSERT (strrchr (test, 0) == test + 4);
  ASSERT (strrchr (test, 't') == test + 3);
  ASSERT (strrchr (test, 'e') == test + 1);
}

/** tests for strstr
*/
static void 
do_teststrstr (void)
{
  const char *a = "aabbcd";
  ASSERT (strstr (a, "\0\1") == a);
  ASSERT (strstr (a, "") == a);
  ASSERT (strstr (a, "ab") == &a[1]);
  ASSERT (strstr (a, "abc") == NULL);
  ASSERT (strstr (a, "abbc") == &a[1]);
  ASSERT (strstr ("", "abbc") == NULL);
/* ASSERT (strstr ("", "") == a); should work, but it doesn't */
  ASSERT (strstr (a, "cd") == &a[4]);
}

/** tests for strspn
*/
static void 
do_teststrspn (void)
{
  ASSERT (strspn("aabbcd", "ab") == 4);
  ASSERT (strspn("abbacd", "") == 0);
  ASSERT (strspn("abbacd", "ac") == 1);
  ASSERT (strspn("abbacd", "x") == 0);
  ASSERT (strspn("abbacd", "c") == 0);
  ASSERT (strspn("abbacd", "cba") == 5);
  ASSERT (strspn("abbacd", "cdba") == 6);
}

/** tests for strtok
*/
static void
do_teststrtok (void)
{
  static char str[] = "?a???b,,,#c";
  char str2[] = "axaaba";
  char *token = strtok (str, "?"); // 'token' points to the token "a"
  ASSERT (token == &str[1] && 0 == strcmp (token,"a"));
  token = strtok (NULL, ","); // 'token' points to the token "??b"
  ASSERT (token == &str[3] && 0 == strcmp (token,"??b"));
  token = strtok (NULL, "#,"); // 'token' points to the token "c"
  ASSERT (token == &str[10] && 0 == strcmp (token,"c"));
  token = strtok (NULL, "?"); // 'token' is a null pointer
  ASSERT (token == NULL);

  token = strtok (str2, "ab");
  ASSERT (token && 0 == strcmp (token, "x"));
  token = strtok (NULL, "ab");
  ASSERT (token == NULL);
#if !defined (__SUNPRO_C) && !defined (__sun__)
  /* SunPro C compiler and GCC on Solaris have problem with strtok-ing after NULL */
  token = strtok (NULL, "a");
  ASSERT (token == NULL);
#endif
}

#if !defined (__APPLE__) // uchar.h/char16_t/char32_t are not supported on MacOS/Clang

// Test C11 UTF-8 behaviour.
static void
do_utf_8 (void)
{
#if defined(__STDC_VERSION) && __STDC_VERSION >= 201112L
  const char *str1 = u8"Ä ä";
  const char *str2 = u8"\u00c4 ä";
  const char *str3 = u8"Ä " "ä";
  const char *str4 = u8"Ä " u8"ä";
  const char *str5 = "Ä " u8"ä";

  ASSERT (str1[0] == 0xc3);
  ASSERT (str2[1] == 0x84);
  ASSERT (!strcmp (str1, str2));
  ASSERT (!strcmp (str1, str3));
  ASSERT (!strcmp (str1, str4));
  ASSERT (!strcmp (str1, str5));
#endif
}

// Test SDCC implementation-defined UTF-8 behaviour
// string literals are UTF-8 (as nearly all implementations out there)
static void
do_utf_8_sdcc (void)
{
#ifdef __SDCC
  const char *str1 = "Ä ä";
  const char *str2 = "\u00c4 ä";
  const char *str3 = u8"Ä " "ä";
  const char *str4 = "Ä " "ä";
  const char *str5 = u8"Ä " u8"ä";

  ASSERT (str1[0] == 0xc3);
  ASSERT (str2[1] == 0x84);
  ASSERT (!strcmp (str1, str2));
  ASSERT (!strcmp (str1, str3));
  ASSERT (!strcmp (str1, str4));
  ASSERT (!strcmp (str1, str5));

  ASSERT (!mblen(0, 0));
  ASSERT (mblen(str1, 3) == 2);
  ASSERT (mblen("test", 3) == 1);
  ASSERT (mblen("", 3) == 0);
#endif
}

// Test C11 UTF-16 behaviour
static void
do_utf_16 (void)
{
#ifdef __STDC_UTF_16__
  const char16_t *str1 = u"Ä ä";
  const char16_t *str2 = u"\u00c4 ä";
  const char16_t *str3 = u"Ä " "ä";
  const char16_t *str4 = "Ä " u"ä";
  const char16_t *str5 = u"Ä " u"ä";

  ASSERT (str1[0] == 0xc4);
  ASSERT (str2[2] == 0xe4);
  ASSERT (!memcmp (str1, str2, 4 * sizeof(char16_t)));
  ASSERT (!memcmp (str1, str3, 4 * sizeof(char16_t)));
  ASSERT (!memcmp (str1, str4, 4 * sizeof(char16_t)));
  ASSERT (!memcmp (str1, str5, 4 * sizeof(char16_t)));
#endif
}

// Test C95 UTF-32 behaviour
static void
do_utf_32_c95 (void)
{
#ifdef __STDC_ISO_10646__
  const wchar_t *str1 = L"Ä ä";
  const wchar_t *str2 = L"\u00c4 ä";
  const wchar_t *str3 = L"Ä " "ä";
  const wchar_t *str4 = "Ä " L"ä";
  const wchar_t *str5 = L"Ä " L"ä";

  ASSERT (str1[0] == 0xc4);
  ASSERT (str2[2] == 0xe4);
  ASSERT (wcslen (str1) == 3);
  ASSERT (!memcmp (str1, str2, 4 * sizeof(wchar_t)));
  ASSERT (!memcmp (str1, str3, 4 * sizeof(wchar_t)));
  ASSERT (!memcmp (str1, str4, 4 * sizeof(wchar_t)));
  ASSERT (!memcmp (str1, str5, 4 * sizeof(wchar_t)));
#endif
}

// Test C11 UTF-32 behaviour
static void
do_utf_32_c11 (void)
{
#ifdef __STDC_UTF_32__
  const char32_t *str1 = U"Ä ä";
  const char32_t *str2 = U"\u00c4 ä";
  const char32_t *str3 = U"Ä " "ä";
  const char32_t *str4 = "Ä " U"ä";
  const char32_t *str5 = U"Ä " U"ä";

  ASSERT (str1[0] == 0xc4);
  ASSERT (str2[2] == 0xe4);
  ASSERT (!memcmp (str1, str2, 4 * sizeof(char32_t)));
  ASSERT (!memcmp (str1, str3, 4 * sizeof(char32_t)));
  ASSERT (!memcmp (str1, str4, 4 * sizeof(char32_t)));
  ASSERT (!memcmp (str1, str5, 4 * sizeof(char32_t)));
#endif
}

static void
do_chinese (void)
{
#ifdef __STDC_UTF_32__
  const char32_t *p0 = U"史斌";
#endif
#ifdef __STDC_ISO_10646__
  const wchar_t *p1 = L"史庭芳";
#endif
#ifdef __STDC_UTF_16__
  const char16_t *p2 = u"天津";
#endif
#ifdef __STDC_UTF_32__
  ASSERT (p0[0] == 0x53f2);
#endif
#ifdef __STDC_ISO_10646__
  ASSERT (p1[2] == 0x82b3);
#endif
#ifdef __STDC_UTF_16__
  ASSERT (p2[1] == 0x6d25);
#endif
}

#endif // __APPLE__

static void
teststr (void)
{
  do_teststrcmp ();
  do_teststrcpy ();
  do_teststrncmp ();
  do_teststrpbrk ();
  do_teststrrchr ();
  do_teststrstr ();
  do_teststrspn ();
  do_teststrtok ();
#if !defined (__APPLE__)
  do_utf_8 ();
  do_utf_8_sdcc ();
  do_utf_16 ();
  do_utf_32_c95 ();
  do_utf_32_c11 ();
  do_chinese ();
#endif // __APPLE__
}

