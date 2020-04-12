/*
    bug 2756
    A non-connected live-range not caught by the live-range splitter resulted
    in an invalid register allocation and an assertion failure in code generation.
*/

#include <testfwk.h>

#pragma disable_warning 84
#pragma disable_warning 85

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __SDCC

struct _bstdio_file {int i;};
typedef struct _bstdio_file FILE;

int bfprintf(FILE *_bstream, const char *__fmt, ...)
{
	return 0;
}

int bprintf(const char *_bfmt, ...)
{
	return 0;
}

FILE *__fopen(const char *_bpath, int _bfd, FILE * _bstream, const char *_bmode)
{
	return 0;
}

#define fopen(__file, __mode)	      __fopen((__file), -1, (FILE*)0, (__mode))

char *bgetenv(char *__name)
{
	return "env";
}

char *bfgets(char *_bs, size_t _bsize, FILE *_bstream)
{
	return 0;
}

FILE bstderr[1];

void err(int _beval, const char *_bfmt, ...)
{
}

void errx(int _beval, const char *_bfmt, ...)
{
}

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Lack of memory
char *argv0;
int all = 0;
int wday;
int advance;
char CurLine[64];
char *CurLinep;
#endif

enum {NONE, DAY, WEEK, MONTH} mflag;

#define	CALFILE	"/.calendar"

int pnmatch(const char *s, const char *p, int unanch)
{
	return 0;
}

void usage(void)
{
}

int findmon1(void)
{
	return 0;
}

int findmon2(void)
{
	return 0;
}

int findmon3(void)
{
	return 0;
}

int findday(void)
{
	return 0;
}

int findyear(void)
{
	return 0;
}

int current(int opt)
{
	return 0;
}

int date(int day, int month, int year)
{
	return 0;
}

void doall(void)
{
}

int m(int argc, char *argv[])
{
#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Not enough memory
#if !(defined (__SDCC_mcs51) && (defined (__SDCC_MODEL_LARGE) || defined (__SDCC_MODEL_HUGE))) // build failure
	int arg = 1;
	char *cp;
	char *thisline;
	char *matchstr;
	char *filename[10];
	FILE *fp[10];
	int matchdate;
	int foundfiles;
	int nfiles;
	int thismonth, thisday, thisyear;
	int thisdate;
	char *atsign;

	argv0 = argv[0];
	mflag = NONE; 				/* Default to no match */

	for (arg = 1; arg < argc; arg++)  { 	/* Read option string */
		cp = argv[arg];
	sw:	switch (*cp) {
			case '-':
				cp++;
				goto sw;
			case 'a':
				all = 1;
				break;
			case 'f':
				filename[arg-1] = ++cp;
				nfiles++;
				continue;
			case 'd':
				matchstr = ++cp;
				mflag = DAY;
				continue;
			case 'w':
				matchstr = ++cp;
				mflag = WEEK;
				continue;
			case 'm':
				matchstr = ++cp;
				mflag = MONTH;
				continue;
			default:
				bfprintf(bstderr, "%s: unrecognized option '%c'\n", argv0, *cp);
				usage();
		}
	}
	if (all)
		doall();

	/*
	 * Open files.
	 */
	if (nfiles)  {
		for (arg = 0; arg < nfiles; arg++ )  {
			if ((fp[arg] = fopen(filename[arg], "r")) == NULL)
				bfprintf(bstderr, "cannot open file %s\n", filename[arg]);
			else
				foundfiles++;
		}
		if (!foundfiles)
			err(1, "cannot open any files specified");
	} else {
		char *hp;

		nfiles = 1;
		if ((hp = bgetenv("HOME")) == NULL)
			errx(1, "can't find my way back HOME");
		filename[0] = malloc(strlen(hp) + strlen(CALFILE) + 1);
		if (filename[0] == NULL)
			errx(1, "out of memory");
		strcpy(filename[0], hp);
		strcat(filename[0], CALFILE);
		if ((fp[0] = fopen(filename[0], "r")) == NULL)
			err(1, "cannot open file $HOME/.calendar");
	}
	/*
	 * Find match condition from options or current date
	 */
	switch (mflag)  {
		case NONE:
			matchdate = current(0);
			break;
		case DAY:
		case WEEK:
			if (*matchstr == '\0') 
				matchdate = current(0);
			else {
				strncpy(CurLine, matchstr, sizeof(CurLine));
				CurLinep = &CurLine[0];
				if ((thismonth = findmon1()) == -1) 
					errx(1, "invalid month in match date");
				if ((thisday = findday()) == -1)  
					errx(1, "invalid day in match date");
				if ((thisyear = findyear()) == -1)  
					thisyear = current(1);
				matchdate = date(thisday, thismonth, thisyear);
			}
			break;
		case MONTH:
			if (*matchstr == '\0')
				matchdate = current(2);
			else  {
				strncpy(CurLine, matchstr, sizeof(CurLine));
				CurLinep = &CurLine[0];
				if ((matchdate = findmon2()) == -1)
					errx(1, "invalid month in match date");
			}
			break;
	}
	/*
	 * Read the calendar files, print matched lines.
	 */
	for (arg = 0; arg < nfiles; arg++) {
		if (fp[arg] == NULL)
			continue;
		while ((thisline = bfgets(CurLine,sizeof(CurLine),fp[arg]))!=NULL) {
			CurLinep = &CurLine[0];
			advance = 0;
			if ((atsign = strchr(CurLinep, '@')) != NULL)
				advance = atoi(atsign + 1);
			if ((thismonth = findmon3()) == -1)
				thismonth = 0;
			if ((thisday = findday()) == -1)
				thisday = 0;
			if ((thisyear = findyear()) == -1)
				thisyear = current(1);
			thisdate = date(thisday, thismonth, thisyear);
			if (thisdate >= matchdate &&
			    thisdate <= matchdate + advance)
				bprintf("%s", thisline);
			else switch (mflag)  {
				case NONE:
					if (wday == 6)
						if (thisdate == matchdate ||
						  thisdate == matchdate + 1 ||
						  thisdate == matchdate + 2 ||
						  thisdate == matchdate + 3)
							bprintf("%s", thisline);
					if (wday == 7)
						if (thisdate == matchdate ||
						  thisdate == matchdate + 1 ||
						  thisdate == matchdate + 2)
							bprintf("%s", thisline);
					if (0 <= wday && wday < 6)
						if (thisdate == matchdate ||
						   thisdate == matchdate + 1)
							bprintf("%s", thisline);
					break;
				case DAY:
					if (thisdate == matchdate)
						bprintf("%s", thisline);
					break;
				case WEEK:
					if (matchdate <= thisdate &&
						    thisdate <= matchdate+7)
						bprintf("%s", thisline);
					break;
				case MONTH:
					if (thismonth == matchdate)
						bprintf("%s", thisline);
					break;
			}
			thisline = NULL;
		}
	}
#endif
#endif
	return 0;
}

#endif // __SDCC

void testBug(void)
{
}

