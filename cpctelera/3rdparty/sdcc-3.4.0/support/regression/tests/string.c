/** tests for strXXX
*/
#include <testfwk.h>
#include <string.h>

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

/** tests for multibyte character sets
 * related to bug #3506236
*/
static void
do_multibyte (void)
{
  const char *str = "ÔÂ";

  ASSERT (str[0] == '\xd4');
  ASSERT (str[1] == '\xc2');
}

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
  do_multibyte ();
}
