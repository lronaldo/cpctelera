/*-----------------------------------------------------------------------
  SDCC.lex - lexical analyser for use with sdcc (free open source
  compiler for 8/16 bit microcontrollers)
  Written by : Sandeep Dutta . sandeep.dutta@usa.net (1997)

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2, or (at your option) any
  later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

  In other words, you are welcome to use, share and improve this program.
  You are forbidden to forbid anyone else to use, share and improve
  what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

B       [0-1]
D       [0-9]
L       [a-zA-Z_$]
H       [a-fA-F0-9]
E       [Ee][+-]?{D}+
BE      [Pp][+-]?{D}+
FS      (f|F|l|L)
IS      (u|U|l|L)*
CP      (L|u|U|u8)
HASH    (#|%:)
UCN     \\u{H}{4}|\\U{H}{8}

UTF8PART1       \xc2[\xa8\xaa\xad\xaf\xb2-\xb5\xb7-\xba\xbc-\xbe]|\xc3[\x80-\x96\x98-\xb6\xb8-\xbf]|[\xc4-\xcb\xce-\xdf][\x80-\xbf]|\xcd[\xb0-\xbf]
UTF8PART2       \xe0[\xa0-\xbf][\x80-\xbf]|\xe1([\x80-\x99\x9b-\x9f\xa1-\xb6\xb8-\xbf][\x80-\xbf]|\x9a[\x81-\xbf]|\xa0[\x80-\x8d\x8f-\xbf])
UTF8PART3       \xe2(\x80[\x8b-\x8d\xaa-\xae\xbf]|\x81[\x80\x94\xa0-\xbf]|\x82[\x80-\xbf]|[\x83\x86][\x80-\x8f]|[\x84-\x85\x92-\x93\xb0-\xb7\xba-\xbf][\x80-\xbf]|\x91[\xa0-\xbf]|\x9d[\xb6-\xbf]|\x9e[\x80-\x93])
UTF8PART4       \xe3(\x80[\x84-\x87\xa1-\xaf\xb1-\xbf]|[\x81-\xbf][\x80-\xbf])|[\xe4-\xec][\x80-\xbf][\x80-\xbf]|\xed[\x80-\x9f][\x80-\xbf]
UTF8PART5       \xef([\xa4-\xb3\xb5-\xb6\xba-\xbe][\x80-\xbf]|\xb4[\x80-\xbd]|\xb7[\x80-\x8f]|[\xb7-\xb8][\xb0-\xbf]|\xb8[\x80-\x9f]|\xb9[\x80-\x84\x87-\xbf]|\xbf[\x80-\xbd])
UTF8PART6       \xf0([\x90-\x9e\xa0-\xae\xb0-\xbe][\x80-\xbf][\x80-\xbf]|[\x9f\xaf\xbf]([\x80-\xbe][\x80-\xbf]|\xbf[\x80-\xbd]))
UTF8PART7       [\xf1-\xf2]([\x80-\x8e\x90-\x9e\xa0-\xae\xb0-\xbe][\x80-\xbf][\x80-\xbf]|[\x8f\x9f\xaf\xbf]([\x80-\xbe][\x80-\xbf]|\xbf[\x80-\xbd]))
UTF8PART8       \xf3([\x80-\x8e\x90-\x9e\xa0-\xae][\x80-\xbf][\x80-\xbf]|[\x8f\x9f\xaf]([\x80-\xbe][\x80-\xbf]|\xbf[\x80-\xbd]))

UTF8IDF1ST      {UTF8PART1}|{UTF8PART2}|{UTF8PART3}|{UTF8PART4}|{UTF8PART5}|{UTF8PART6}|{UTF8PART7}|{UTF8PART8}
UTF8IDF         {UTF8IDF1ST}|\xcc[\x80-\xbf]|\xcd[\x80-\xaf]|\xe2\x83[\x90-\xbf]|\xef\xb8[\xa0-\xaf]|\xe1\xb7[\x80-\xbf]

%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "newalloc.h"
#include "dbuf_string.h"
/* Some systems, noteably Mac OS, do not have uchar.h. */
/* If it is missing, use our own type definitions. */
#ifdef HAVE_UCHAR_H
#include <uchar.h>
#else
#include <stdint.h>
#define char16_t uint_least16_t
#define char32_t uint_least32_t
#endif
/* Needed by flex 2.5.4 on NetBSD 5.0.1 sparc64 */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
/* MSVC has no unistd.h but has read() declaration in io.h */
#if defined(_MSC_VER)
# include <io.h>
#endif

#define TKEYWORD(token) return (isTargetKeyword(yytext) ? (token) :\
                                check_type())

#define TKEYWORD99(token) return (options.std_c99 ? (token) : check_type())

#define TKEYWORD2X(token) return (options.std_c2x ? (token) : check_type())

int column = 0;         /* current column */

/* global definitions */
char *lexFilename;
int lexLineno = 1;

/* local definitions */
static struct dbuf_s asmbuff; /* reusable _asm buffer */

/* forward declarations */
int yyerror (char *s);
static const char *stringLiteral (char);
static void count (void);
static void count_char (int);
static int process_pragma (const char *);
static int check_type (void);
static void checkCurrFile (const char *s);
%}

%x asm
%%
"__asm"                {
  count ();
  if (asmbuff.buf == NULL)
    dbuf_init (&asmbuff, INITIAL_INLINEASM);
  else
    dbuf_set_length (&asmbuff, 0);

  BEGIN (asm);
}
<asm>"__endasm"        {
  count ();
  yylval.yystr = dbuf_c_str (&asmbuff);
  BEGIN (INITIAL);
  return INLINEASM;
}
<asm>\n                 {
  count ();
  dbuf_append_char(&asmbuff, *yytext);
}
<asm>.                  {
  dbuf_append_char(&asmbuff, *yytext);
}
"__asm__"               { count (); return ASM; }
"__at"                  { count (); TKEYWORD (AT); }
"auto"                  { count (); return AUTO; }
"__bit"                 { count (); TKEYWORD (BIT); }
"bool"                  { count (); TKEYWORD2X (SD_BOOL); }
"_Bool"                 { count (); TKEYWORD99 (SD_BOOL); }
"break"                 { count (); return BREAK; }
"case"                  { count (); return CASE; }
"char"                  { count (); return SD_CHAR; }
"__code"                { count (); TKEYWORD (CODE); }
"const"                 { count (); return SD_CONST; }
"continue"              { count (); return CONTINUE; }
"__critical"            { count (); TKEYWORD (CRITICAL); }
"__data"                { count (); TKEYWORD (DATA); }
"default"               { count (); return DEFAULT; }
"do"                    { count (); return DO; }
"double"                { count (); werror (W_DOUBLE_UNSUPPORTED); return SD_FLOAT; }
"else"                  { count (); return ELSE; }
"enum"                  { count (); return ENUM; }
"extern"                { count (); return EXTERN; }
"__far"                 { count (); TKEYWORD (XDATA); }
"__eeprom"              { count (); TKEYWORD (EEPROM); }
"float"                 { count (); return SD_FLOAT; }
"__fixed16x16"          { count (); TKEYWORD (FIXED16X16); }
"__flash"               { count (); TKEYWORD (CODE); }
"for"                   { count (); return FOR; }
"goto"                  { count (); return GOTO; }
"__idata"               { count (); TKEYWORD (IDATA); }
"if"                    { count (); return IF; }
"int"                   { count (); return SD_INT; }
"__interrupt"           { count (); TKEYWORD (INTERRUPT); }
"__nonbanked"           { count (); TKEYWORD (NONBANKED); }
"__banked"              { count (); TKEYWORD (BANKED); }
"__trap"                { count (); TKEYWORD (TRAP); }
"long"                  { count (); return SD_LONG; }
"__near"                { count (); TKEYWORD (DATA); }
"__pdata"               { count (); TKEYWORD (PDATA); }
"__reentrant"           { count (); TKEYWORD (REENTRANT); }
"__shadowregs"          { count (); TKEYWORD (SHADOWREGS); }
"__wparam"              { count (); TKEYWORD (SD_WPARAM); }
"register"              { count (); return REGISTER; }
"return"                { count (); return RETURN; }
"__sfr"                 { count (); TKEYWORD (SFR); }
"__sfr16"               { count (); TKEYWORD (SFR16); }
"__sfr32"               { count (); TKEYWORD (SFR32); }
"__sbit"                { count (); TKEYWORD (SBIT); }
"short"                 { count (); return SD_SHORT; }
"signed"                { count (); return SIGNED; }
"sizeof"                { count (); return SIZEOF; }
"alignof"               { count (); TKEYWORD2X (ALIGNOF); }
"_Alignof"              { count (); return ALIGNOF; }
"__builtin_offsetof"    { count (); return OFFSETOF; }
"__sram"                { count (); TKEYWORD (XDATA); }
"static"                { count (); return STATIC; }
"struct"                { count (); return STRUCT; }
"switch"                { count (); return SWITCH; }
"_Thread_local"         { count (); return THREAD_LOCAL; }
"typedef"               { count (); return TYPEDEF; }
"union"                 { count (); return UNION; }
"unsigned"              { count (); return UNSIGNED; }
"void"                  { count (); return SD_VOID; }
"volatile"              { count (); return VOLATILE; }
"__using"               { count (); TKEYWORD (USING); }
"__naked"               { count (); TKEYWORD (NAKED); }
"while"                 { count (); return WHILE; }
"__xdata"               { count (); TKEYWORD (XDATA); }
"..."                   { count (); return VAR_ARGS; }
"__typeof"              { count (); return TYPEOF; }
"_JavaNative"           { count (); TKEYWORD (JAVANATIVE); }
"__overlay"             { count (); TKEYWORD (OVERLAY); }
"inline"                { count (); TKEYWORD99 (INLINE); }
"_Noreturn"             { count (); return NORETURN;}
"restrict"              { count (); TKEYWORD99 (RESTRICT); }
"__smallc"              { count (); TKEYWORD (SMALLC); }
"__preserves_regs"      { count (); return PRESERVES_REGS; }
"__z88dk_fastcall"      { count (); TKEYWORD (Z88DK_FASTCALL); }
"__z88dk_callee"        { count (); TKEYWORD (Z88DK_CALLEE); }
"__z88dk_shortcall"     { count (); return Z88DK_SHORTCALL; }
"__z88dk_params_offset" { count (); return Z88DK_PARAMS_OFFSET; }
"__addressmod"          { count (); return ADDRESSMOD; }
"static_assert"         { count (); TKEYWORD2X (STATIC_ASSERT); }
"_Static_assert"        { count (); return STATIC_ASSERT; }
"alignas"               { count (); TKEYWORD2X (ALIGNAS); }
"_Alignas"              { count (); return ALIGNAS; }
"_Generic"              { count (); return GENERIC; }
({L}|{UCN}|{UTF8IDF1ST})({L}|{D}|{UCN}|{UTF8IDF})*  {
  if (!options.dollars_in_ident && strchr (yytext, '$'))
    {
      yyerror ("stray '$' in program");
    }
  if (!options.std_c95)
    {
      bool ucn_check = strchr (yytext, '\\');
      for (char *ptr = yytext; *ptr && !ucn_check; ptr++)
        {
          if ((unsigned char) *ptr >= 0x80)
            ucn_check = true;
        }
      if (ucn_check)
        werror (W_UNIVERSAL_C95);
    }
  count ();
  return check_type();
}
0[bB]('?{B})+{IS}?          {
  if (!options.std_sdcc && !options.std_c2x)
    werror (W_BINARY_INTEGER_CONSTANT_C23);
  count ();
  yylval.val = constIntVal (yytext);
  return CONSTANT;
}
0[xX]('?{H})+{IS}?           { count (); yylval.val = constIntVal (yytext); return CONSTANT; }
0('?[0-7])*{IS}?             { count (); yylval.val = constIntVal (yytext); return CONSTANT; }
[1-9]('?{D})*{IS}?           { count (); yylval.val = constIntVal (yytext); return CONSTANT; }
{CP}?'(\\.|[^\\'])+'         { count (); yylval.val = charVal (yytext); return CONSTANT; /* ' make syntax highlighter happy */ }
{D}+{E}{FS}?                 { count (); yylval.val = constFloatVal (yytext); return CONSTANT; }
{D}*"."{D}+({E})?{FS}?       { count (); yylval.val = constFloatVal (yytext); return CONSTANT; }
{D}+"."{D}*({E})?{FS}?       { count (); yylval.val = constFloatVal (yytext); return CONSTANT; }
0[xX]{H}+{BE}{FS}?           { count (); if (!options.std_c99) werror(E_HEXFLOAT_C99); yylval.val = constFloatVal (yytext); return CONSTANT; }
0[xX]{H}*"."{H}+({BE})?{FS}? { count (); if (!options.std_c99) werror(E_HEXFLOAT_C99); yylval.val = constFloatVal (yytext); return CONSTANT; }
0[xX]{H}+"."{H}*({BE})?{FS}? { count (); if (!options.std_c99) werror(E_HEXFLOAT_C99); yylval.val = constFloatVal (yytext); return CONSTANT; }
\"                           { count (); yylval.yystr = stringLiteral (0); return STRING_LITERAL; }
"L\""                        { count (); if (!options.std_c95) werror(E_WCHAR_STRING_C95); yylval.yystr = stringLiteral ('L'); return STRING_LITERAL; }
"u8\""                       { count (); if (!options.std_c11) werror(E_U8CHAR_STRING_C11); yylval.yystr = stringLiteral ('8'); return STRING_LITERAL; }
"u\""                        { count (); if (!options.std_c11) werror(E_WCHAR_STRING_C11); yylval.yystr = stringLiteral ('u'); return STRING_LITERAL; }
"U\""                        { count (); if (!options.std_c11) werror(E_WCHAR_STRING_C11); yylval.yystr = stringLiteral ('U'); return STRING_LITERAL; }
">>="                   { count (); yylval.yyint = RIGHT_ASSIGN; return RIGHT_ASSIGN; }
"<<="                   { count (); yylval.yyint = LEFT_ASSIGN; return LEFT_ASSIGN; }
"+="                    { count (); yylval.yyint = ADD_ASSIGN; return ADD_ASSIGN; }
"-="                    { count (); yylval.yyint = SUB_ASSIGN; return SUB_ASSIGN; }
"*="                    { count (); yylval.yyint = MUL_ASSIGN; return MUL_ASSIGN; }
"/="                    { count (); yylval.yyint = DIV_ASSIGN; return DIV_ASSIGN; }
"%="                    { count (); yylval.yyint = MOD_ASSIGN; return MOD_ASSIGN; }
"&="                    { count (); yylval.yyint = AND_ASSIGN; return AND_ASSIGN; }
"^="                    { count (); yylval.yyint = XOR_ASSIGN; return XOR_ASSIGN; }
"|="                    { count (); yylval.yyint = OR_ASSIGN; return OR_ASSIGN; }
">>"                    { count (); return RIGHT_OP; }
"<<"                    { count (); return LEFT_OP; }
"++"                    { count (); return INC_OP; }
"--"                    { count (); return DEC_OP; }
"->"                    { count (); return PTR_OP; }
"&&"                    { count (); return AND_OP; }
"||"                    { count (); return OR_OP; }
"<="                    { count (); return LE_OP; }
">="                    { count (); return GE_OP; }
"=="                    { count (); return EQ_OP; }
"!="                    { count (); return NE_OP; }
";"                     { count (); return ';'; }
"{"|"<%"                { count (); ignoreTypedefType = 0; return '{'; }
"}"|"%>"                { count (); return '}'; }
","                     { count (); return ','; }
":"                     { count (); return ':'; }
"="                     { count (); return '='; }
"("                     { count (); ignoreTypedefType = 0; return '('; }
")"                     { count (); return ')'; }
"["|"<:"                { count (); return '['; }
"]"|":>"                { count (); return ']'; }
"."                     { count (); return '.'; }
"&"                     { count (); return '&'; }
"!"                     { count (); return '!'; }
"~"                     { count (); return '~'; }
"-"                     { count (); return '-'; }
"+"                     { count (); return '+'; }
"*"                     { count (); return '*'; }
"/"                     { count (); return '/'; }
"%"                     { count (); return '%'; }
"<"                     { count (); return '<'; }
">"                     { count (); return '>'; }
"^"                     { count (); return '^'; }
"|"                     { count (); return '|'; }
"?"                     { count (); return '?'; }
"::"                    { count (); return ATTRIBCOLON; }
^{HASH}pragma.*         { count (); process_pragma (yytext); }
^{HASH}.*               { count (); checkCurrFile (yytext); }

"\r\n"                  { count (); }
"\n"                    { count (); }
[ \t\v\f]               { count (); }
\\                      {
  int ch = input ();

  if (ch == '\n')
    count_char (ch);
  else
    {
      /* that could have been removed by the preprocessor anyway */
      werror (W_STRAY_BACKSLASH, column);
      unput (ch);
    }
}
.                       { count (); }
%%

/* flex 2.5.31 undefines yytext_ptr, so we have to define it again */
#ifndef yytext_ptr
#define yytext_ptr yytext
#endif

static void
checkCurrFile (const char *s)
{
  int lNum;
  char *tptr;

  /* skip '#' character */
  if (*s++ != '#')
    return;

  /* get the line number */
  lNum = strtol (s, &tptr, 10);
  if (tptr == s || !isspace ((unsigned char) *tptr))
    return;
  s = tptr;

  /* adjust the line number */
  lineno = lexLineno = lNum - 1;

  /* now see if we have a file name */
  while (*s != '"' && *s)
    ++s;

  if (*s)
    {
      /* there is a file name */
      const char *sb;
      struct dbuf_s dbuf;

      dbuf_init (&dbuf, 128);

      /* skip the double quote */
      sb = ++s;

      /* preprocessor emits escaped file names
       * (e.g. double backslashes on MSDOS-ish file systems),
       * so we have to unescape it */
      while (*s && *s != '"')
        {
          if (*s == '\\')
            {
              /* append chars before backslash */
              dbuf_append (&dbuf, sb, s - sb);
              if (*++s)
                {
                  /* append char after backslash */
                  dbuf_append (&dbuf, s, 1);
                  sb = ++s;
                }
            }
          else
            ++s;
        }
      dbuf_append (&dbuf, sb, s - sb);

      /* DON'T free the file name referenced by lexFilename
       * since it will be dereferenced in the future at least
       * by function printCLine(), see struct iCode members
       * filename in SDCCicode.c */

      filename = lexFilename = dbuf_detach_c_str (&dbuf);
    }
}

static void
count_char (int ch)
{
  switch (ch)
    {
    case '\n':
      column = 0;
      lineno = ++lexLineno;
      break;

    case '\t':
      column += 8 - (column % 8);
      break;

    default:
      ++column;
      break;
    }
}

static void
count (void)
{
  const char *p;

  for (p = yytext; *p; ++p)
    count_char(*p);
}

static bool
is_UCN_valid_in_idf (char32_t c, bool is_first)
{
  bool result = false;

  // D.1 Ranges of characters allowed
  if ((c == 0x00A8) || (c == 0x00AA) || (c == 0x00AD) || (c == 0x00AF)
      || (c >= 0x00B2 && c <= 0x00B5) || (c >= 0x00B7 && c <= 0x00BA)
      || (c >= 0x00BC && c <= 0x00BE) || (c >= 0x00C0 && c <= 0x00D6)
      || (c >= 0x00D8 && c <= 0x00F6) || (c >= 0x00F8 && c <= 0x00FF)
      || (c >= 0x0100 && c <= 0x167F) || (c >= 0x1681 && c <= 0x180D)
      || (c >= 0x180F && c <= 0x1FFF) || (c >= 0x200B && c <= 0x200D)
      || (c >= 0x202A && c <= 0x202E) || (c >= 0x203F && c <= 0x2040)
      || (c == 0x2054) || (c >= 0x2060 && c <= 0x206F)
      || (c >= 0x2070 && c <= 0x218F) || (c >= 0x2460 && c <= 0x24FF)
      || (c >= 0x2776 && c <= 0x2793) || (c >= 0x2C00 && c <= 0x2DFF)
      || (c >= 0x2E80 && c <= 0x2FFF) || (c >= 0x3004 && c <= 0x3007)
      || (c >= 0x3021 && c <= 0x302F) || (c >= 0x3031 && c <= 0x303F)
      || (c >= 0x3040 && c <= 0xD7FF) || (c >= 0xF900 && c <= 0xFD3D)
      || (c >= 0xFD40 && c <= 0xFDCF) || (c >= 0xFDF0 && c <= 0xFE44)
      || (c >= 0xFE47 && c <= 0xFFFD) || (c >= 0x10000 && c <= 0x1FFFD)
      || (c >= 0x20000 && c <= 0x2FFFD) || (c >= 0x30000 && c <= 0x3FFFD)
      || (c >= 0x40000 && c <= 0x4FFFD) || (c >= 0x50000 && c <= 0x5FFFD)
      || (c >= 0x60000 && c <= 0x6FFFD) || (c >= 0x70000 && c <= 0x7FFFD)
      || (c >= 0x80000 && c <= 0x8FFFD) || (c >= 0x90000 && c <= 0x9FFFD)
      || (c >= 0xA0000 && c <= 0xAFFFD) || (c >= 0xB0000 && c <= 0xBFFFD)
      || (c >= 0xC0000 && c <= 0xCFFFD) || (c >= 0xD0000 && c <= 0xDFFFD)
      || (c >= 0xE0000 && c <= 0xEFFFD))
    {
      result = true;
      // D.2 Ranges of characters disallowed initially
      if (is_first && ((c >= 0x0300 && c <= 0x036F) || (c >= 0x1DC0 && c <= 0x1DFF)
          || (c >= 0x20D0 && c <= 0x20FF) || (c >= 0xFE20 && c <= 0xFE2F)))
        {
          result = false;
        }
    }

  return result;
}

static void
decode_UCNs_to_utf8 (char *dest, const char *src, size_t n)
{
  bool is_first = true;
  const char *s = src;
  size_t chars_left = n - 1;

  while (*src)
    {
      if (*src == '\\')
        {
          ++src;
          char32_t c = 0;
          if (*src == 'u')
            {
              c = universalEscape(&src, 4);
            }
          else  // U - the lexer only accepts \u and \U escapes in identifiers
            {
              c = universalEscape(&src, 8);
            }
          if (!is_UCN_valid_in_idf(c, is_first))
            {
              werror(E_INVALID_UNIVERSAL, s);
            }

          if (c >= 0x10000)
            {
              if (chars_left < 4)
                break;
              *(dest++) = 0xf0 | (c >> 18);
              *(dest++) = 0x80 | ((c >> 12) & 0x3f);
              *(dest++) = 0x80 | ((c >> 6) & 0x3f);
              *(dest++) = 0x80 | (c & 0x3f);
              chars_left -= 4;
            }
          else if (c >= 0x800)
            {
              if (chars_left < 3)
                break;
              *(dest++) = 0xe0 | (c >> 12);
              *(dest++) = 0x80 | ((c >> 6) & 0x3f);
              *(dest++) = 0x80 | (c & 0x3f);
              chars_left -= 3;
            }
          else  // ASCII characters already eliminated by validity check => no further check here
            {
              if (chars_left < 2)
                break;
              *(dest++) = 0xc0 | (c >> 6);
              *(dest++) = 0x80 | (c & 0x3f);
              chars_left -= 2;
            }
        }
      else
        {
          if (chars_left < 1)
            break;
          *(dest++) = *(src++);
          chars_left--;
        }
      is_first = false;
    }
  *dest = '\0';
}

static int
check_type (void)
{
  decode_UCNs_to_utf8(yylval.yychar, yytext, SDCC_NAME_MAX);

  symbol *sym = findSym(SymbolTab, NULL, yylval.yychar);

  /* check if it is in the table as a typedef */
  if (!ignoreTypedefType && sym && IS_SPEC (sym->etype)
      && SPEC_TYPEDEF (sym->etype) && findSym(TypedefTab, NULL, yylval.yychar))
    return (TYPE_NAME);
  /* check if it is a named address space */
  else if (findSym (AddrspaceTab, NULL, yylval.yychar))
    return (ADDRSPACE_NAME);
  else
    return(IDENTIFIER);
}

/*
 * Change by JTV 2001-05-19 to not concatenate strings
 * to support ANSI hex and octal escape sequences in string literals
 */

static const char *
stringLiteral (char enc)
{
#define STR_BUF_CHUNCK_LEN  1024
  int ch;
  static struct dbuf_s dbuf;  /* reusable string literal buffer */

  if (dbuf.alloc == 0)
    dbuf_init(&dbuf, STR_BUF_CHUNCK_LEN);
  else
    dbuf_set_length(&dbuf, 0);

  switch (enc)
    {
    case 'u': // UTF-16
      dbuf_append_str(&dbuf, "u\"");
      break;
    case 'L': // UTF-32
      enc = 'L';
      dbuf_append_str(&dbuf, "L\"");
      break;
    case 'U': // UTF-32
      enc = 'U';
      dbuf_append_str(&dbuf, "U\"");
      break;
    case '8': // UTF-8
      enc = '8';
      dbuf_append_str(&dbuf, "u8\"");
    default: // UTF-8 or whatever else the source character set is encoded in
      dbuf_append_char(&dbuf, '"');
    }

  /* put into the buffer till we hit the first \" */

  for (; ; )
    {
      ch = input();
      count_char(ch);
      if (ch == EOF)
        break;

      switch (ch)
        {
        case '\\':
          /* if it is a \ then escape char's are allowed */
          ch = input();
          count_char(ch);
          if (ch == '\n')
            {
              /* \<newline> is a continuator */
            }
          else
            {
              char buf[2];

              if (ch == EOF)
                goto out;

              buf[0] = '\\';
              buf[1] = ch;
              dbuf_append(&dbuf, buf, 2); /* get the escape char, no further check */
            }
          break; /* carry on */

        case '\n':
          /* if new line we have a new line break, which is illegal */
          werror(W_NEWLINE_IN_STRING);
          dbuf_append_char(&dbuf, '\n');
          break;

        case '"':
          /* if this is a quote then we have work to do */
          /* find the next non whitespace character     */
          /* if that is a double quote then carry on    */
          dbuf_append_char(&dbuf, '"');  /* Pass end of this string or substring to evaluator */
          while ((ch = input()) && (isspace(ch) || ch == '\\' || ch == '#'))
            {
              switch (ch)
                {
                case '\\':
                  count_char(ch);
                  if ((ch = input()) != '\n')
                    {
                      werror(W_STRAY_BACKSLASH, column);
                      if (ch != EOF)
                        unput(ch);
                      else
                        count_char(ch);
                    }
                  else
                    count_char(ch);
                  break;

                case '\n':
                  count_char(ch);
                  break;

                case '#':
                  if (column == 0)
                    {
                      /* # at the beginning of the line: collect the entire line */
                      struct dbuf_s linebuf;
                      const char *line;

                      count_char(ch);

                      dbuf_init(&linebuf, STR_BUF_CHUNCK_LEN);
                      dbuf_append_char(&linebuf, '#');

                      while ((ch = input()) != EOF && ch != '\n')
                        dbuf_append_char(&linebuf, (char)ch);

                      if (ch == '\n')
                        count_char(ch);

                      line = dbuf_c_str(&linebuf);

                      /* process the line */
                      if (startsWith(line, "#pragma"))
                        process_pragma(line);
                      else
                        checkCurrFile(line);

                      dbuf_destroy(&linebuf);
                    }
                  else
                    {
                      unput(ch);
                      goto out;
                    }

                default:
                  count_char(ch);
                  break;
                }
            }

          if (ch == EOF)
            goto out;

          if (ch == 'u' || ch == 'U' || ch == 'L') /* Could be an utf-16 or utf-32 wide string literal prefix or an utf-8 prefix */
            {
              int ch2;

              if (!(options.std_c11 || options.std_c95 && ch == 'L'))
                {
                  werror (ch == 'L' ? E_WCHAR_STRING_C95 : E_WCHAR_STRING_C11);
                  unput(ch);
                  goto out;
                }

              ch2 = input();
              if (ch2 != '"')
                unput (ch2);
              else /* It is an utf-16 or utf-32 wide string literal prefix */
                {
                  if (!enc) 
                    {
                      dbuf_prepend_char(&dbuf, ch == 'L' ? 'U' : ch);
                      enc = ch;
                    }
                  else if (enc != ch)
                    werror (W_PREFIXED_STRINGS);
                  count_char(ch);
                  count_char(ch2);
                  break;
                }
            }

          if (ch == 'u') /* Could be an utf-8 wide string literal prefix */
            {
              ch = input();
              if (ch != '8')
                {
                  unput(ch);
                  unput('u');
                  goto out;
                }
              ch = input();
              if (ch != '"')
                {
                  unput(ch);
                  unput('8');
                  unput('u');
                  goto out;
                }
              if (!enc)
                enc = '8';
              else if (enc != '8')
                werror (W_PREFIXED_STRINGS);
            }
          if (ch != '"')
            {
              unput(ch);
              goto out;
            }
          count_char(ch);
          break;

        default:
          dbuf_append_char(&dbuf, (char)ch);  /* Put next substring introducer into output string */
        }
    }

out:
  return dbuf_c_str(&dbuf);
}


enum {
   P_SAVE = 1,
   P_RESTORE,
   P_INDUCTION,
   P_NOINDUCTION,
   P_NOINVARIANT,
   P_STACKAUTO,
   P_OVERLAY_,     /* I had a strange conflict with P_OVERLAY while */
                   /* cross-compiling for MINGW32 with gcc 3.2 */
   P_NOOVERLAY,
   P_LESSPEDANTIC,
   P_NOGCSE,
   P_CALLEE_SAVES,
   P_EXCLUDE,
   P_NOIV,
   P_LOOPREV,
   P_DISABLEWARN,
   P_OPTCODESPEED,
   P_OPTCODESIZE,
   P_OPTCODEBALANCED,
   P_STD_C89,
   P_STD_C99,
   P_STD_C11,
   P_STD_C2X,
   P_STD_SDCC89,
   P_STD_SDCC99,
   P_CODESEG,
   P_CONSTSEG
};


/* SAVE/RESTORE stack */
#define SAVE_RESTORE_SIZE 128

STACK_DCL(options_stack, struct options *, SAVE_RESTORE_SIZE)
STACK_DCL(optimize_stack, struct optimize *, SAVE_RESTORE_SIZE)
STACK_DCL(SDCCERRG_stack, struct SDCCERRG *, SAVE_RESTORE_SIZE)

/*
 * cloneXxx functions should be updated every time a new set is
 * added to the options or optimize structure!
 */

static struct options *
cloneOptions (struct options *opt)
{
  struct options *new_opt;

  new_opt = Safe_malloc(sizeof (struct options));

  /* clone scalar values */
  *new_opt = *opt;

  /* clone sets */
  new_opt->calleeSavesSet = setFromSetNonRev(opt->calleeSavesSet);
  new_opt->excludeRegsSet = setFromSetNonRev(opt->excludeRegsSet);
  /* not implemented yet: */
  /* new_opt->olaysSet = setFromSetNonRev(opt->olaysSet); */

  return new_opt;
}

static struct optimize *
cloneOptimize (struct optimize *opt)
{
  struct optimize *new_opt;

  new_opt = Safe_malloc(sizeof (struct optimize));

  /* clone scalar values */
  *new_opt = *opt;

  return new_opt;
}

static struct SDCCERRG *
cloneSDCCERRG (struct SDCCERRG *val)
{
  struct SDCCERRG *new_val;

  new_val = Safe_malloc(sizeof (struct SDCCERRG));

  /* clone scalar values */
  *new_val = *val;

  return new_val;
}

static void
copyAndFreeOptions (struct options *dest, struct options *src)
{
  /* delete dest sets */
  deleteSet(&dest->calleeSavesSet);
  deleteSet(&dest->excludeRegsSet);
  /* not implemented yet: */
  /* deleteSet(&dest->olaysSet); */

  /* copy src to dest */
  *dest = *src;

  Safe_free(src);
}

static void
copyAndFreeOptimize (struct optimize *dest, struct optimize *src)
{
  /* copy src to dest */
  *dest = *src;

  Safe_free(src);
}

static void
copyAndFreeSDCCERRG (struct SDCCERRG *dest, struct SDCCERRG *src)
{
  /* copy src to dest */
  *dest = *src;

  Safe_free(src);
}

/*
 * returns 1 if the pragma was processed, 0 if not
 */
static int
doPragma (int id, const char *name, const char *cp)
{
  struct pragma_token_s token;
  int err = 0;
  int processed = 1;

  init_pragma_token(&token);

  switch (id)
    {
    case P_SAVE:
      {
        cp = get_pragma_token(cp, &token);
        if (TOKEN_EOL != token.type)
          {
            err = 1;
            break;
          }

        STACK_PUSH(options_stack, cloneOptions(&options));
        STACK_PUSH(optimize_stack, cloneOptimize(&optimize));
        STACK_PUSH(SDCCERRG_stack, cloneSDCCERRG(&_SDCCERRG));
      }
      break;

    case P_RESTORE:
      {
        struct options *optionsp;
        struct optimize *optimizep;
        struct SDCCERRG *sdccerrgp;

        cp = get_pragma_token(cp, &token);
        if (TOKEN_EOL != token.type)
          {
            err = 1;
            break;
          }

        optionsp = STACK_POP(options_stack);
        copyAndFreeOptions(&options, optionsp);

        optimizep = STACK_POP(optimize_stack);
        copyAndFreeOptimize(&optimize, optimizep);

        sdccerrgp = STACK_POP(SDCCERRG_stack);
        copyAndFreeSDCCERRG(&_SDCCERRG, sdccerrgp);
      }
      break;

    case P_INDUCTION:
    case P_NOINDUCTION:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      optimize.loopInduction = (id == P_INDUCTION) ? 1 : 0;
      break;

    case P_NOINVARIANT:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      optimize.loopInvariant = 0;
      break;

    case P_STACKAUTO:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.stackAuto = 1;
      break;

    case P_NOGCSE:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      optimize.global_cse = 0;
      break;

    case P_OVERLAY_:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      break; /* notyet */

    case P_NOOVERLAY:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.noOverlay = 1;
      break;

    case P_LESSPEDANTIC:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.lessPedantic = 1;
      setErrorLogLevel(ERROR_LEVEL_WARNING);
      break;

    case P_CALLEE_SAVES:
      /* append to the functions already listed
         in callee-saves */
      setParseWithComma(&options.calleeSavesSet, cp);
      err = -1;
      break;

    case P_EXCLUDE:
      {
        deleteSet(&options.excludeRegsSet);
        setParseWithComma(&options.excludeRegsSet, cp);
        err = -1;
      }
      break;

    case P_NOIV:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.noiv = 1;
      break;

    case P_LOOPREV:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      optimize.noLoopReverse = 1;
      break;

    case P_DISABLEWARN:
      {
        int warn;

        cp = get_pragma_token(cp, &token);
        if (TOKEN_INT != token.type)
          {
            err = 1;
            break;
          }
        warn = token.val.int_val;

        cp = get_pragma_token(cp, &token);
        if (TOKEN_EOL != token.type)
          {
            err = 1;
            break;
          }

        setWarningDisabled(warn);
      }
      break;

    case P_OPTCODESPEED:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      optimize.codeSpeed = 1;
      optimize.codeSize = 0;
      break;

    case P_OPTCODESIZE:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      optimize.codeSpeed = 0;
      optimize.codeSize = 1;
      break;

    case P_OPTCODEBALANCED:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      optimize.codeSpeed = 0;
      optimize.codeSize = 0;
      break;

    case P_STD_C89:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.std_c99 = 0;
      options.std_c11 = 0;
      options.std_c2x = 0;
      options.std_sdcc = 0;
      break;

    case P_STD_C99:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.std_c99 = 1;
      options.std_c11 = 0;
      options.std_c2x = 0;
      options.std_sdcc = 0;
      break;

    case P_STD_C11:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.std_c99 = 1;
      options.std_c11 = 1;
      options.std_c2x = 0;
      options.std_sdcc = 0;
      break;

    case P_STD_C2X:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.std_c99 = 1;
      options.std_c11 = 1;
      options.std_c2x = 1;
      options.std_sdcc = 0;
      break;

    case P_STD_SDCC89:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.std_c99 = 0;
      options.std_c11 = 0;
      options.std_c2x = 0;
      options.std_sdcc = 1;
      break;

    case P_STD_SDCC99:
      cp = get_pragma_token(cp, &token);
      if (TOKEN_EOL != token.type)
        {
          err = 1;
          break;
        }

      options.std_c99 = 1;
      options.std_c11 = 0;
      options.std_c2x = 0;
      options.std_sdcc = 1;
      break;

    case P_CODESEG:
    case P_CONSTSEG:
      {
        struct dbuf_s segname;

        cp = get_pragma_token(cp, &token);
        if (token.type == TOKEN_EOL)
          {
            err = 1;
            break;
          }

        dbuf_init(&segname, 16);
        dbuf_printf(&segname, "%-8s(CODE)", get_pragma_string(&token));

        cp = get_pragma_token(cp, &token);
        if (token.type != TOKEN_EOL)
          {
            dbuf_destroy(&segname);
            err = 1;
            break;
          }

        if (id == P_CODESEG)
          options.code_seg = dbuf_detach(&segname);
        else
          options.const_seg = dbuf_detach(&segname);
      }
      break;

    default:
      processed = 0;
      break;
    }

  get_pragma_token(cp, &token);

  if (1 == err || (0 == err && token.type != TOKEN_EOL))
    werror(W_BAD_PRAGMA_ARGUMENTS, name);

  free_pragma_token(&token);
  return processed;
}

static struct pragma_s pragma_tbl[] = {
  { "save",              P_SAVE,            0, doPragma },
  { "restore",           P_RESTORE,         0, doPragma },
  { "induction",         P_INDUCTION,       0, doPragma },
  { "noinduction",       P_NOINDUCTION,     0, doPragma },
  { "noinvariant",       P_NOINVARIANT,     0, doPragma },
  { "noloopreverse",     P_LOOPREV,         0, doPragma },
  { "stackauto",         P_STACKAUTO,       0, doPragma },
  { "nogcse",            P_NOGCSE,          0, doPragma },
  { "overlay",           P_OVERLAY_,        0, doPragma },
  { "nooverlay",         P_NOOVERLAY,       0, doPragma },
  { "callee_saves",      P_CALLEE_SAVES,    0, doPragma },
  { "exclude",           P_EXCLUDE,         0, doPragma },
  { "noiv",              P_NOIV,            0, doPragma },
  { "less_pedantic",     P_LESSPEDANTIC,    0, doPragma },
  { "disable_warning",   P_DISABLEWARN,     0, doPragma },
  { "opt_code_speed",    P_OPTCODESPEED,    0, doPragma },
  { "opt_code_size",     P_OPTCODESIZE,     0, doPragma },
  { "opt_code_balanced", P_OPTCODEBALANCED, 0, doPragma },
  { "std_c89",           P_STD_C89,         0, doPragma },
  { "std_c99",           P_STD_C99,         0, doPragma },
  { "std_c11",           P_STD_C11,         0, doPragma },
  { "std_c2x",           P_STD_C2X,         0, doPragma },
  { "std_sdcc89",        P_STD_SDCC89,      0, doPragma },
  { "std_sdcc99",        P_STD_SDCC99,      0, doPragma },
  { "codeseg",           P_CODESEG,         0, doPragma },
  { "constseg",          P_CONSTSEG,        0, doPragma },
  { NULL,                0,                 0, NULL },
};

/*
 * returns 1 if the pragma was processed, 0 if not
 */
int
process_pragma_tbl (const struct pragma_s *pragma_tbl, const char *s)
{
  struct pragma_token_s token;
  int i;
  int ret = 0;

  init_pragma_token(&token);

  s = get_pragma_token(s, &token);

  /* skip separating whitespace */
  while ('\n' != *s && isspace((unsigned char)*s))
    s++;

  for (i = 0; NULL != pragma_tbl[i].name; ++i)
    {
      /* now compare and do what needs to be done */
      if (strcmp(get_pragma_string(&token), pragma_tbl[i].name) == 0)
        {
          if (pragma_tbl[i].deprecated != 0)
            werror(W_DEPRECATED_PRAGMA, pragma_tbl[i].name);

          ret = (*pragma_tbl[i].func)(pragma_tbl[i].id, pragma_tbl[i].name, s);
          break;
        }
    }

  free_pragma_token(&token);
  return ret;
}

static int
process_pragma (const char *s)
{
  struct pragma_token_s token;

  init_pragma_token(&token);

  s = get_pragma_token(s, &token);
  if (0 != strcmp("#pragma", get_pragma_string(&token)))
    {
      /* Oops, something went totally wrong - internal error */
      wassertl(0, "pragma parser internal error");
    }

  /* skip spaces */
  while ('\n' != *s && isspace((unsigned char)*s))
    ++s;

  /* First give the port a chance */
  if (port->process_pragma && port->process_pragma(s))
    return 1;

  if (process_pragma_tbl(pragma_tbl, s))
    {
      return 1;
    }
  else
    {
      werror(W_UNKNOWN_PRAGMA, s);
      return 0;
    }
}

int
yywrap (void)
{
  if (!STACK_EMPTY(options_stack) || !STACK_EMPTY(optimize_stack))
    werror(W_SAVE_RESTORE);

  return 1;
}

int
yyerror (char *s)
{
  fflush(stdout);

  werror (S_SYNTAX_ERROR, yytext, column);

  return 0;
}
