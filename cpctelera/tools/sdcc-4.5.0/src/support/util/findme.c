/** \ingroup popt
 * \file popt/findme.c
 */

/* (C) 1998-2002 Red Hat, Inc. -- Licensing details are in the COPYING
   file accompanying popt source distributions, available from 
   ftp://ftp.rpm.org/pub/rpm/dist.

   alloca replaced with malloc()/free() pair */

#include "system.h"
#include "findme.h"

const char * findProgramPath(const char * argv0) {
    char * path = getenv("PATH");
    char * pathbuf;
    char * start, * chptr;
    char * buf;

    if (argv0 == NULL) return NULL;	/* XXX can't happen */
    /* If there is a / in the argv[0], it has to be an absolute path */
    if (strchr(argv0, '/'))
	return xstrdup(argv0);

    if (path == NULL) return NULL;

    start = pathbuf = malloc(strlen(path) + 1);
    if (pathbuf == NULL) return NULL;
    buf = malloc(strlen(path) + strlen(argv0) + sizeof("/"));
    if (buf == NULL) {  /* XXX can't happen */
         free(pathbuf);
         return NULL;
    }      
    strcpy(pathbuf, path);

    chptr = NULL;
    /*@-branchstate@*/
    do {
	if ((chptr = strchr(start, ':')))
	    *chptr = '\0';
	sprintf(buf, "%s/%s", start, argv0);

	if (!access(buf, X_OK)) {
            free(pathbuf);
	    return buf;
        }

	if (chptr) 
	    start = chptr + 1;
	else
	    start = NULL;
    } while (start && *start);
    /*@=branchstate@*/

    free(buf);
    free(pathbuf);

    return NULL;
}
