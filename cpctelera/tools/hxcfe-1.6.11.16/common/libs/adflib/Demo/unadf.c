/*
 * unadf 1.0
 *
 *
 * tested under Linux and Win32
 *
 *  This file is part of ADFLib.
 *
 *  ADFLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ADFLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define UNADF_VERSION "1.0"


#include<stdlib.h>
#include<errno.h>
#include<string.h>

#include "adflib.h"

/* The portable way used to create a directory is to call the MKDIR command via the
 * system() function.
 * It is used to create the 'dir1' directory, like the 'dir1/dir11' directory
 */

/* the portable way to check if a directory 'dir1' already exists i'm using is to
 * do fopen('dir1','rb'). NULL is returned if 'dir1' doesn't exists yet, an handle instead
 */

#define MKDIR "mkdir"

#ifdef WIN32
#define DIRSEP '\\'
#else
#define DIRSEP '/'
#endif /* WIN32 */

#define EXTBUFL 1024*8


void help()
{
    puts("unadf [-lrcsp -v n] dumpname.adf [files-with-path] [-d extractdir]");
    puts("    -l : lists root directory contents");
    puts("    -r : lists directory tree contents");
    puts("    -c : use dircache data (must be used with -l)");
    puts("    -s : display entries logical block pointer (must be used with -l)");
    putchar('\n');
    puts("    -v n : mount volume #n instead of default #0 volume");
    putchar('\n');
    puts("    -p : send extracted files to pipe (unadf -p dump.adf Pics/pic1.gif | xv -)");
    puts("    -d dir : extract to 'dir' directory");
}

void printEnt(struct Volume *vol, struct Entry* entry, char *path, BOOL sect)
{
    /* do not print the links entries, ADFlib do not support them yet properly */
    if (entry->type==ST_LFILE || entry->type==ST_LDIR || entry->type==ST_LSOFT)
        return;

    if (entry->type==ST_DIR)
        printf("         ");
    else
        printf("%7ld  ",entry->size);

	printf("%4d/%02d/%02d  %2d:%02d:%02d ",entry->year, entry->month, entry->days,
        entry->hour, entry->mins, entry->secs);
    if (sect)
        printf(" %06ld ",entry->sector);

    if (strlen(path)>0)
        printf(" %s/",path);
    else
        printf(" ");
    if (entry->type==ST_DIR)
        printf("%s/",entry->name);
    else
        printf("%s",entry->name);
    if (entry->comment!=NULL && strlen(entry->comment)>0)
        printf(", %s",entry->comment);
    putchar('\n');

}


void extractFile(struct Volume *vol, char* name, char* path, unsigned char *extbuf,
    BOOL pflag, BOOL qflag)
{
    struct File *file;
    FILE* out;
    long n;
    char *filename;

    filename = NULL;
    if (pflag)
        out = stdout;
    else {
        if (strlen(path)>0) {
            filename=(char*)malloc(sizeof(char)* (strlen(path)+1+strlen(name)+1) );
            if (!filename) return;
            sprintf(filename,"%s%c%s",path,DIRSEP,name);
            out = fopen(filename, "wb");
        }
        else
            out = fopen(name, "wb");
        if (!out) return;
    }

    file = adfOpenFile(vol, name, "r");
    if (!file) { fclose(out); return; }

    n = adfReadFile(file, EXTBUFL, extbuf);
    while(!adfEndOfFile(file)) {
        fwrite(extbuf, sizeof(unsigned char), n, out);
        n = adfReadFile(file, EXTBUFL, extbuf);
    }
    if (n>0)
        fwrite(extbuf, sizeof(unsigned char), n, out);

    if (!pflag)
        fclose(out);

    adfCloseFile(file);

    if (!qflag) {
        if (filename!=NULL)
            printf("x - %s\n", filename);
        else
            printf("x - %s\n", name);
    }
	
    if (filename!=NULL)
        free(filename);
}


void extractTree(struct Volume *vol, struct List* tree, char *path, unsigned char *extbuf,
    BOOL pflag, BOOL qflag)
{
	struct Entry* entry;
    char *buf;
    char sysbuf[200];

    while(tree) {
        entry = (struct Entry*)tree->content;
        if (entry->type==ST_DIR) {
            buf = NULL;
            if (strlen(path)>0) {
                buf=(char*)malloc(strlen(path)+1+strlen(entry->name)+1);
                if (!buf) return;
                sprintf(buf,"%s%c%s",path,DIRSEP,entry->name);
                sprintf(sysbuf,"%s %s",MKDIR,buf);
                if (!qflag) printf("x - %s%c\n",buf,DIRSEP);
            }
            else {
                sprintf(sysbuf,"%s %s",MKDIR,entry->name);
                if (!qflag) printf("x - %s%c\n",entry->name,DIRSEP);
            }

            if (!pflag) system(sysbuf);

	        if (tree->subdir!=NULL) {
                if (adfChangeDir(vol,entry->name)==RC_OK) {
                    if (buf!=NULL)
                        extractTree(vol,tree->subdir,buf,extbuf, pflag, qflag);
                    else
                        extractTree(vol,tree->subdir,entry->name,extbuf, pflag, qflag);
                    adfParentDir(vol);
                }
                else {
                    if (strlen(path)>0)
                        fprintf(stderr,"ExtractTree : dir \"%s/%s\" not found.\n",path,entry->name);
                    else
                        fprintf(stderr,"ExtractTree : dir \"%s\" not found.\n",entry->name);
                }
            }

            if (buf!=NULL)
                free(buf);
        }
        else if (entry->type==ST_FILE) {
            extractFile(vol,entry->name,path,extbuf, pflag, qflag);
        }
        tree = tree->next;
    }
}


void printTree(struct Volume *vol, struct List* tree, char* path, BOOL sect)
{
    char *buf;
    struct Entry* entry;

    while(tree) {
        printEnt(vol, tree->content, path, sect);
        if (tree->subdir!=NULL) {
            entry = (struct Entry*)tree->content;
            if (strlen(path)>0) {
                buf=(char*)malloc(sizeof(char)* (strlen(path)+1+strlen(entry->name)+1) );
                if (!buf) {
                    fprintf(stderr,"printTree : malloc error\n");
                    return;
                }
                sprintf(buf,"%s/%s", path, entry->name);
                printTree(vol, tree->subdir, buf, sect);
                free(buf);
            }
            else
                printTree(vol, tree->subdir, entry->name, sect);
        }
        tree = tree->next;
    }
}


void printDev(struct Device* dev)
{
    printf("Device : ");

    switch(dev->devType){
    case DEVTYPE_FLOPDD:
        printf("Floppy DD"); break;
    case DEVTYPE_FLOPHD:
        printf("Floppy HD"); break;
    case DEVTYPE_HARDDISK:
        printf("Harddisk"); break;
    case DEVTYPE_HARDFILE:
        printf("Hardfile"); break;
    default:
        printf("???"); break;
    }

    printf(". Cylinders = %ld, Heads = %ld, Sectors = %ld",dev->cylinders,dev->heads,dev->sectors);

    printf(". Volumes = %d\n",dev->nVol);
}


void printVol(struct Volume* vol, int volNum)
{
    printf("Volume : ");
	
    switch(vol->dev->devType) {
    case DEVTYPE_FLOPDD:
        printf ("Floppy 880 KBytes,");
        break;
    case DEVTYPE_FLOPHD:
        printf ("Floppy 1760 KBytes,");
        break;
    case DEVTYPE_HARDDISK:
        printf ("HD partition #%d %3.1f KBytes,", volNum, (vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
        break;
    case DEVTYPE_HARDFILE:
        printf ("HardFile %3.1f KBytes,", (vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
        break;
    default:
        printf ("???,");
    }

    if (vol->volName!=NULL)
        printf(" \"%s\"", vol->volName);

    printf(" between sectors [%ld-%ld].",vol->firstBlock, vol->lastBlock);

    printf(" %s ",isFFS(vol->dosType) ? "FFS" : "OFS");
    if (isINTL(vol->dosType))
        printf ("INTL ");
    if (isDIRCACHE(vol->dosType))
        printf ("DIRCACHE ");

    printf(". Filled at %2.1f%%.\n", 100.0-
	(adfCountFreeBlocks(vol)*100.0)/(vol->lastBlock - vol->firstBlock +1) );

}


void processFile(struct Volume *vol, char* name, char* path, unsigned char *extbuf,
    BOOL pflag, BOOL qflag)
{
	char *sepptr, *cdstr, *fullname, *filename;
	char *bigstr;
    FILE *tfile;

    adfToRootDir(vol);

    sepptr = strchr(name, '/');
    if (sepptr==NULL) {
        extractFile(vol, name, path, extbuf, pflag, qflag);
    }
    else {
        /* the all-in-one string : to call system(), to find the filename, the convert dir sep char ... */
        bigstr=(char*)malloc(strlen(MKDIR)+1+strlen(path)+1+strlen(name)+1);
        if (!bigstr) { fprintf(stderr,"processFile : malloc"); return; }

        /* to build to extract path */
        if (strlen(path)>0) {
            sprintf(bigstr,"%s %s%c%s",MKDIR,path,DIRSEP,name);
            cdstr = bigstr+strlen(MKDIR)+1+strlen(path)+1;
        }
        else {
            sprintf(bigstr,"%s %s",MKDIR,name);
            cdstr = bigstr+strlen(MKDIR)+1;
        }
        /* the directory in which the file will be extracted */
        fullname =  bigstr+strlen(MKDIR)+1;

        /* finds the filename, and separates it from the path */
        filename = strrchr(bigstr,'/')+1;
        filename[-1]='\0';

        sepptr = cdstr;
        /* find the end of the first dir to create */
        while(sepptr[0]!='/' && sepptr[0]!='\0')
            sepptr++;

        while(strlen(cdstr)>0) {
            if (sepptr[0]=='/') { /* not the last one */
                sepptr[0]='\0';
                if (adfChangeDir(vol,cdstr)!=RC_OK)
                    return;
                tfile = fopen(fullname,"r"); /* the only portable way to test if the dir exists */
                if (tfile==NULL) { /* does't exist : create it */
                    if (!pflag) system(bigstr);
                    if (!qflag) printf("x - %s%c\n",fullname,DIRSEP);
                }
                else
                    fclose(tfile);
                sepptr[0] = DIRSEP; /* converts the '/' to '/' or '\' */
                cdstr = sepptr+1; /* next beginning of the next dir to create */
                /* to find the end of the next dir */
                sepptr++;
                while(sepptr[0]!='/' && sepptr[0]!='\0')
                    sepptr++;
            }
            else { /* the last one */
                if (adfChangeDir(vol,cdstr)!=RC_OK)
                    return;
                tfile = fopen(fullname,"r");
                if (tfile==NULL) {
                    if (!pflag) system(bigstr);
                    if (!qflag) printf("x - %s%c\n",fullname,DIRSEP);
                }
                else
                    fclose(tfile);
                cdstr = cdstr+strlen(cdstr); /* at the end, ends the while loop */
            }
        }
        extractFile(vol, filename, fullname, extbuf, pflag, qflag);

        free(bigstr);
    }


}


int main(int argc, char* argv[])
{
    int i, j;
    BOOL rflag, lflag, xflag, cflag, vflag, sflag, dflag, pflag, qflag;
    struct List* files, *rtfiles;
    char *devname, *dirname;
    char strbuf[80];
    unsigned char *extbuf;
    int vInd, dInd, fInd, aInd;
    BOOL nextArg;

    struct Device *dev;
    struct Volume *vol;
    struct List *list, *cell;
    int volNum;
    BOOL true = TRUE;

    if (argc<2) {
        help();
        exit(0);
    }

    rflag = lflag = cflag = vflag = sflag = dflag = pflag = qflag = FALSE;
    vInd = dInd = fInd = aInd = -1;
    xflag = TRUE;
    dirname = NULL;
    devname = NULL;
    files = rtfiles = NULL;
    volNum = 0;

    fprintf(stderr,"unADF v%s : a unzip like for .ADF files, powered by ADFlib (v%s - %s)\n\n",
        UNADF_VERSION, adfGetVersionNumber(),adfGetVersionDate());

    /* parse options */
    i=1;
    while(i<argc) {
        if (argv[i][0]=='-') {
            j=1;
            nextArg = FALSE;
            while(j<(int)strlen(argv[i]) && !nextArg) {
                switch(argv[i][j]) {
                case 'v':
                    vflag = TRUE;
                    if ((i+1)<(argc-1)) {
                        i++;
                        nextArg = TRUE;
                        errno = 0;
                        volNum = atoi(argv[i]);
                        if (errno!=0 || volNum<0) {
                            fprintf(stderr,"invalid volume number, aborting.\n");
                            exit(1);
                        }
                    }
                    else
                        fprintf(stderr,"no volume number, -v option ignored.\n");
                    break;
                case 'l': 
                    lflag = TRUE;
                    xflag = FALSE;
                    break;
                case 's': 
                    sflag = TRUE;
                    break;
                case 'c': 
                    cflag = TRUE;
                    break;
                case 'r':
                    rflag = TRUE;
                    break;
                case 'd':
                    if (devname!=NULL && xflag && (i+1)==(argc-1)) {
                        i++;
                        dirname = argv[i];
                        if (dirname[strlen(dirname)-1]==DIRSEP)
                            dirname[strlen(dirname)-1]='\0';
                        nextArg = TRUE;
                        dflag = TRUE;
                    }
                    break;
                case 'p':
                    if (xflag) {
                        fprintf(stderr,"sending files to pipe.\n");
                        pflag = TRUE;
                        qflag = TRUE;
                    }
                    else
                        fprintf(stderr,"-p option must be used with extraction, ignored.\n");
                    break;
                case 'h':
                default:
                    help();
                    exit(0);
                } /* switch */
            j++;
            } /* while */
        } /* if */
        else {
			/* the last non option string is taken as a filename */
            if (devname==NULL) /* if the device name has been already given */
                devname = argv[i];
            else {
                if (xflag) {
                    if (rtfiles==NULL)
                        rtfiles = files = newCell(NULL, (void*)argv[i]);
                    else
                        files = newCell(files, (void*)argv[i]);
                }
                else
                    fprintf(stderr,"Must be used with extraction, ignored.\n");
            }
        }
        i++;
    } /* while */

    extbuf =(unsigned char*)malloc(EXTBUFL*sizeof(char));
    if (!extbuf) { fprintf(stderr,"malloc error\n"); exit(1); }

    /* initialize the library */
    adfEnvInitDefault();

    dev = adfMountDev( devname,TRUE );
    if (!dev) {
        sprintf(strbuf,"Can't mount the dump device '%s'.\n", devname);
        fprintf(stderr, strbuf);
        adfEnvCleanUp(); exit(1);
    }
    if (!qflag)
        printDev(dev);

    if (volNum>=dev->nVol) {
        fprintf(stderr,"This device has only %d volume(s), aborting.\n",dev->nVol);
        exit(1);
    }

    vol = adfMount(dev, volNum, TRUE);
    if (!vol) {
        adfUnMountDev(dev);
        fprintf(stderr, "Can't mount the volume\n");
        adfEnvCleanUp(); exit(1);
    }

    if (!qflag) {
        printVol(vol, volNum);
        putchar('\n');
    }

    if (cflag && isDIRCACHE(vol->dosType) && lflag) {
        adfChgEnvProp(PR_USEDIRC,&true);
        if (!qflag)
            puts("Using dir cache blocks.");
    }

    if (lflag) {
        if (!rflag) {
            cell = list = adfGetDirEnt(vol,vol->curDirPtr);
            while(cell) {
                printEnt(vol,cell->content,"", sflag);
                cell = cell->next;
            }
            adfFreeDirList(list);
        } else {
            cell = list = adfGetRDirEnt(vol,vol->curDirPtr,TRUE);
            printTree(vol,cell,"", sflag);
            adfFreeDirList(list);
        }
    }else if (xflag) {
        if (rtfiles!=NULL) {
            files = rtfiles;
            while(files!=NULL) {
                if (dirname!=NULL)
                    processFile(vol, (char*)files->content, dirname, extbuf, pflag, qflag);
                else
                    processFile(vol, (char*)files->content, "", extbuf, pflag, qflag);
                files = files->next;
            }
            freeList(rtfiles);
        }
        else {
            cell = list = adfGetRDirEnt(vol,vol->curDirPtr,TRUE);
            if (dirname==NULL)
                extractTree(vol, cell, "", extbuf, pflag, qflag);
            else
                extractTree(vol, cell, dirname, extbuf, pflag, qflag);
            adfFreeDirList(list);
        }
    }
    else
        help();

    free(extbuf);

    adfUnMount(vol);
    adfUnMountDev(dev);

    adfEnvCleanUp();

    return(0);
}
