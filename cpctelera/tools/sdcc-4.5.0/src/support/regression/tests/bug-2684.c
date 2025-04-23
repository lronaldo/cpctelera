/* bug-2684.c
   A bug in the interface between SDCC and tdlib.
 */

#include <testfwk.h>

#pragma disable_warning 85
#pragma disable_warning 196

/* printf.c
 *    Dale Schumacher                         399 Beacon Ave.
 *    (alias: Dalnefre')                      St. Paul, MN  55104
 *    dal@syntel.UUCP                         United States of America
 *
 * Altered to use stdarg, made the core function vfprintf.
 * Hooked into the stdio package using 'inside information'
 * Altered sizeof() assumptions, now assumes all integers except chars
 * will be either
 *  sizeof(xxx) == sizeof(long) or sizeof(xxx) == sizeof(short)
 *
 * -RDB
 */

#include <stdarg.h>
#include <string.h>

typedef unsigned char uchar;
typedef signed long fpos_t;
/* when you add or change fields here, be sure to change the initialization
 * in stdio_init and fopen */
struct __stdio_file {
	uchar	*bufpos;	/* the next byte to write to or read from */
	uchar	*bufread;	/* the end of data returned by last read() */
	uchar	*bufwrite;	/* highest address writable by macro */
	uchar	*bufstart;	/* the start of the buffer */
	uchar	*bufend;	/* the end of the buffer; ie the byte after
				   the last malloc()ed byte */
	int	fd;		/* the file descriptor associated with the stream */
	int	mode;
	char	unbuf[8];	/* The buffer for 'unbuffered' streams */
	struct __stdio_file * next;
};

typedef struct __stdio_file FILE;

int fputc(int c, FILE *stream)
{
	return (0);
}

int fflush(FILE *stream)
{
	return (0);
}

#define _IOLBF		0x01
#define _IONBF		0x02
#define __MODE_BUF	0x03

extern char *__ltostr(long __value, int __radix)
{
	return ("");
}

extern char *__ultostr(unsigned long value, int __radix)
{
	return ("");
}

#if !defined(__SDCC_pic14) // Pseudo-stack size limit
/*
 * Output the given field in the manner specified by the arguments. Return
 * the number of characters output.
 */
static int prtfld(FILE * op, size_t maxlen, size_t ct, unsigned char *buf, int ljustf, char sign,
		  char pad, int width, int preci, int buffer_mode)
{
	return 0;
}

#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL)) && !defined (__SDCC_pdk14) && !defined (__SDCC_pdk15)
int _vfnprintf(FILE * op, size_t maxlen, const char *fmt, va_list ap)
{
	register int i, ljustf, lval, preci, dpoint, width, radix, cnt = 0;
	char pad, sign, hash;
	register char *ptmp, *add;
	unsigned long val;
	char tmp[64];
	int buffer_mode;

	/* This speeds things up a bit for unbuffered */
	buffer_mode = (op->mode & __MODE_BUF);
	op->mode &= (~__MODE_BUF);
	while (*fmt) {
		if (*fmt == '%') {
			if (buffer_mode == _IONBF)
				fflush(op);
			ljustf = 0;	/* left justify flag */
			sign = '\0';	/* sign char & status */
			pad = ' ';	/* justification padding char */
			width = -1;	/* min field width */
			dpoint = 0;	/* found decimal point */
			preci = -1;	/* max data width */
			radix = 10;	/* number base */
			ptmp = tmp;	/* pointer to area to print */
			hash = 0;
			lval = (sizeof(int) == sizeof(long));	/* long value flagged */
		      fmtnxt:for (i = 0, ++fmt;;
			     ++fmt) {
				if (*fmt < '0' || *fmt > '9')
					break;
				i *= 10;
				i += (*fmt - '0');
				if (dpoint)
					preci = i;
				else if (!i && (pad == ' ')) {
					pad = '0';
					goto fmtnxt;
				} else
					width = i;
			}
			switch (*fmt) {
			case '\0':	/* early EOS */
				--fmt;
				goto charout;

			case '-':	/* left justification */
				ljustf = 1;
				goto fmtnxt;

			case ' ':
			case '+':	/* leading sign flag */
				sign = *fmt;
				goto fmtnxt;

			case '#':
				hash = 1;
				goto fmtnxt;

			case '*':	/* parameter width value */
				i = va_arg(ap, int);
				if (dpoint)
					preci = i;
				else if ((width = i) < 0) {
					ljustf = 1;
					width = -i;
				}
				goto fmtnxt;

			case '.':	/* secondary width field */
				dpoint = 1;
				goto fmtnxt;

			case 'l':	/* long data */
				lval = 1;
				goto fmtnxt;

			case 'h':	/* short data */
				lval = 0;
				goto fmtnxt;

			case 'd':	/* Signed decimal */
			case 'i':
				ptmp = __ltostr((long) ((lval) ?
						    va_arg(ap, long) :
						    va_arg(ap, int)), 10);
				goto printit;

			case 'b':	/* Unsigned binary */
				radix = 2;
				goto usproc;

			case 'o':	/* Unsigned octal */
				radix = 8;
				goto usproc;

			case 'p':	/* Pointer */
				lval = (sizeof(char *) == sizeof(long));
				pad = '0';
				width = 5;
				preci = 8;
				/* fall thru */

			case 'X':	/* Unsigned hexadecimal 'ABC' */
				radix = 16;
				goto usproc;

			case 'x':	/* Unsigned hexadecimal 'abc' */
				radix = -16;
				/* fall thru */

			case 'u':	/* Unsigned decimal */
			usproc:
				val = lval ? va_arg(ap, unsigned long) :
				    va_arg(ap, unsigned int);
				ptmp = __ultostr(val, radix < 0 ? -radix : radix);
				add = "";
				if (hash) {
					if (radix == 2)
						add = "0b";
					else if (radix == 8) {
						if (val != 0)
							add = "0";
					} else if (radix == 16)
						add = "0x";
					else if (radix == -16)
						add = "0X";
					if (*add) {
						pad = '\0';
						strcpy(tmp, add);
						ptmp = strcat(tmp, ptmp);
					}
				}
				goto printit;

			case '!':	/* inline Character */
				if ((i = fmt[1]) != 0)
					++fmt;
				goto Chr;

			case 'c':	/* Character */
				i = va_arg(ap, int);
			      Chr:ptmp[0] =
				    i;
				ptmp[1] = '\0';
				if (hash) {
					pad = *ptmp;
					goto chrpad;
				}
				goto nopad;

			case 's':	/* String */
				ptmp = va_arg(ap, char *);
			      nopad:pad =
				    ' ';
			      chrpad:sign =
				    '\0';
			      printit:cnt +=
				    prtfld(op, maxlen, cnt, (unsigned char *) ptmp,
					   ljustf, sign, pad, width, preci,
					   buffer_mode);
				break;

			default:	/* unknown character */
				goto charout;
			}
		} else {
		      charout:
			/* normal char out */
		        if (cnt < maxlen)
		                fputc(*fmt, op);
			++cnt;
			if (*fmt == '\n' && buffer_mode == _IOLBF)
				fflush(op);
		}
		++fmt;
	}
	op->mode |= buffer_mode;
	if (buffer_mode == _IONBF)
		fflush(op);
	if (buffer_mode == _IOLBF)
		op->bufwrite = op->bufstart;
	return (cnt);
}
#endif
#endif

void testBug(void)
{
}

