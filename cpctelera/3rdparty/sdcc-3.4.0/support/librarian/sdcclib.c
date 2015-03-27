/* sdcclib.c: sdcc librarian
   Copyright (C) 2003, Jesus Calvino-Fraga jesusc(at)ece.ubc.ca

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
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#define _POSIX_
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if !defined(__BORLANDC__) && !defined(_MSC_VER)
#include <unistd.h>
#endif

char ProgName[FILENAME_MAX];
char LibName[FILENAME_MAX];
char LibNameTmp[FILENAME_MAX];
char IndexName[FILENAME_MAX];
char ListName[FILENAME_MAX];

char **RelName;
int NumRelFiles=0;

#define version "1.2"

#define OPT_NONE     0
#define OPT_ADD_REL  1
#define OPT_EXT_REL  2
#define OPT_DEL_REL  3
#define OPT_ADD_LIST 4
#define OPT_DUMP_SYM 5
#define OPT_DUMP_MOD 6

#define MAXLINE 254
#define EQ(A,B) !strcmp((A),(B))
#define NEQ(A,B) strcmp((A),(B))

int action=OPT_NONE;
FILE *lib, *newlib, *rel, *adb, *libindex;
char FLine[MAXLINE+1];
char ModName[MAXLINE+1];

void GetNameFromPath(char * path, char * name)
{
    int i, j;

    for(i=0; path[i]!=0; i++);
    for(; i>=0; i--)
    {
    	if((path[i]=='\\') || (path[i]=='/')) break;
    }
    for(j=0, i++; (path[i]!='.')&&(path[i]!=0); i++, j++) name[j]=path[i];
    name[j]=0;
}

void ChangeExtension(char * path, char * ext)
{
    int i;

    for(i=0; path[i]!=0; i++);
    for(; i>=0; i--)
    {
    	if( (path[i]=='.') || (path[i]=='\\') || (path[i]=='/') ) break;
    }
    if(i>=0)
    {
	    if(path[i]=='.')
	    {
	        path[i+1]=0;
	        strcat(path, ext);
	    }
	    else
	    {
	        printf("ERROR: Filename '%s' must have an extension\n", path);
	        exit(1);
	    }
    }
    else
    {
        printf("ERROR: Empty filename\n");
        exit(1);
    }
}

void CleanLine(char * buff)
{
    int j, l;
    l=strlen(buff);
    for(j=0; j<l; j++)
    {
        if((buff[j]=='\n')||(buff[j]=='\r')) buff[j]=0;
    }
}

int set_options (char * opt)
{
    int rvalue=0, unknown=0;
    static char Help[] =
    "Usage: %s [option|-options] library relfile1 relfile2 relfile3 ...\n\n"
    "available options:\n"
    "a,r - Adds relfile(s) to library.  If relfile exists, replaces it.\n"
    "  l - Adds relfile list to library.\n"
    "  d - Deletes relfile(s) from library.\n"
    "e,x - Extracts relfile(s) from library.\n"
    "  s - Dumps symbols of library.\n"
    "  m - Dumps modules of library.\n"
    "  v - Displays program version.\n"
    "h,? - This help.\n";

    switch (opt[0])
    {
        default:
            unknown=1;
        case 'h':
        case '?':
        case 'v':
            printf("%s: SDCC librarian version %s\n", ProgName, version);
            printf("by Jesus Calvino-Fraga\n\n");
            if (unknown) printf("Unknown option: %c\n", opt[0]);
            if (opt[0]=='v') exit(0);
            printf(Help, ProgName);
            exit(1);
        break;
        case 'a':
        case 'r':
            action=OPT_ADD_REL;
        break;
        case 'l':
            action=OPT_ADD_LIST;
        break;
        case 'e':
        case 'x':
            action=OPT_EXT_REL;
        break;
        case 'd':
            action=OPT_DEL_REL;
        break;
        case 's':
            action=OPT_DUMP_SYM;
        break;
        case 'm':
            action=OPT_DUMP_MOD;
        break;
    }
    return (rvalue);
}

void ProcLineOptions (int argc, char **argv)
{
    int cont_par=0;
    int i, j;

    /*Get the program name*/
    GetNameFromPath(argv[0], ProgName);

    for (j=1; j<argc; j++)
    {
        if(argv[j][0]=='-')
        {
            for(i=1; argv[j][i]!=0 ; i++)
                if (set_options(&argv[j][i])) break;
        }
        else
        {
            switch(cont_par)
            {
                case 0:
                    if ((strlen(argv[j])==1) && (action==OPT_NONE))
                        set_options(argv[j]);
                    else
                    {
                        cont_par++;
                        if(strlen(LibName) > FILENAME_MAX - 1)
                        {
                            printf("ERROR: Library name too long.\n");
                            exit(2);
                        }
                        strcpy(LibName, argv[j]);
                    }
                break;

                case 1:
                    cont_par++;
                    if(action==OPT_ADD_LIST)
                        strcpy(ListName, argv[j]);
                    else
                    {
                        NumRelFiles=1;
                        RelName = (char **) calloc (1, sizeof (char *));
                        if(RelName==NULL)
                        {
                            printf("ERROR: Insufficient memory.\n");
                            exit(2);
                        }
                        RelName[0]=strdup(argv[j]);
                        if(RelName[0]==NULL)
                        {
                            printf("ERROR: Insufficient memory.\n");
                            exit(2);
                        }
                    }
                break;

                default:
                    cont_par++;
                    NumRelFiles++;
                    RelName = (char **) realloc (RelName, NumRelFiles * sizeof (char *));
                    if(RelName==NULL)
                    {
                        printf("ERROR: Insufficient memory.\n");
                        exit(2);
                    }
                    RelName[NumRelFiles-1]=strdup(argv[j]);
                    if(RelName[NumRelFiles-1]==NULL)
                    {
                        printf("ERROR: Insufficient memory.\n");
                        exit(2);
                    }
                break;
            }
        }
    }

    if ( (cont_par<2) && (action<OPT_DUMP_SYM) )
    {
        printf("ERROR: Too few arguments.\n");
        set_options("h"); /*Show help and exit*/
    }
    else if ( (cont_par!=1) && (action>=OPT_DUMP_SYM) )
    {
        printf("ERROR: Too %s arguments.\n", cont_par<1?"few":"many");
        set_options("h"); /*Show help and exit*/
    }
}

void AddRel(char * RelName)
{
    int inrel=0;
    int state=0;
    long newlibpos, indexsize;
    char symname[MAXLINE+1];
    char c;
    int IsDOSStyle=0;
    char *AdbName = 0;

    strcpy(LibNameTmp, LibName);
    ChangeExtension(LibNameTmp, "__L");

    strcpy(IndexName, LibName);
    ChangeExtension(IndexName, "__I");

    AdbName = strdup (RelName);
    ChangeExtension(AdbName, "adb");

    lib=fopen(LibName, "r");

    if(action==OPT_ADD_REL)
    {
        rel=fopen(RelName, "r");
        if(rel==NULL)
        {
            printf("ERROR: Couldn't open file '%s'\n", RelName);
            if(lib!=NULL) fclose(lib);
            free(AdbName);
            return;
        }
    }
    GetNameFromPath(RelName, ModName);

    newlib=fopen(LibNameTmp, "w");
    if(newlib==NULL)
    {
        printf("ERROR: Couldn't create temporary file '%s'\n", LibNameTmp);
        if(lib!=NULL) fclose(lib);
        fclose(rel);
        free(AdbName);
        return;
    }
    fprintf(newlib, "<FILES>\n\n");

    libindex=fopen(IndexName, "w");
    if(libindex==NULL)
    {
        printf("ERROR: Couldn't create temporary file '%s'\n", IndexName);
        if(lib!=NULL) fclose(lib);
        fclose(newlib);
        fclose(rel);
        free(AdbName);
        return;
    }

    if(lib!=NULL) while(!feof(lib))
    {
        FLine[0]=0;
        if (fgets(FLine, MAXLINE, lib) == NULL)
          break;
        CleanLine(FLine);

        switch(state)
        {
            case 0:
                if(EQ(FLine, "<FILE>"))
                {
                    FLine[0]=0;
                    if (fgets(FLine, MAXLINE, lib) == NULL)
                      break;
                    CleanLine(FLine);
                    if(NEQ(FLine, ModName))
                    {
                        newlibpos=ftell(newlib);
                        fprintf(newlib, "<FILE>\n%s\n", FLine);
                        fprintf(libindex, "<MODULE>\n%s %ld\n", FLine, newlibpos);
                        state++;
                    }
                }
            break;
            case 1:
                fprintf(newlib, "%s\n", FLine);
                if(EQ(FLine, "</FILE>"))
                {
                    fprintf(newlib, "\n");
                    fprintf(libindex, "</MODULE>\n\n");
                    state=0;
                    inrel=0;
                }
                else if(EQ(FLine, "<REL>")) inrel=1;
                else if(EQ(FLine, "</REL>")) inrel=0;
                if(inrel)
                {
                    if(FLine[0]=='S')
                    {
                        sscanf(FLine, "S %s %c", symname, &c);
                        if(c=='D') fprintf(libindex, "%s\n", symname);
                    }
                }
            break;
        }
    }

    if(action==OPT_ADD_REL)
    {
        newlibpos=ftell(newlib);
        fprintf(newlib, "<FILE>\n%s\n<REL>\n", ModName);
        fprintf(libindex, "<MODULE>\n%s %ld\n", ModName, newlibpos);
        while(!feof(rel))
        {
            FLine[0]=0;
            if (fgets(FLine, MAXLINE, rel) == NULL)
              break;
            CleanLine(FLine);
            if(strlen(FLine)>0)
            {
                fprintf(newlib, "%s\n", FLine);
            }
            if(FLine[0]=='S')
            {
                sscanf(FLine, "S %s %c", symname, &c);
                if(c=='D') fprintf(libindex, "%s\n", symname);
            }
        }
        fclose(rel);
        fprintf(libindex, "</MODULE>\n");
        fprintf(newlib, "</REL>\n<ADB>\n");

        adb=fopen(AdbName, "r");
        if(adb!=NULL)
        {
            while(!feof(rel))
            {
                FLine[0]=0;
                if (fgets(FLine, MAXLINE, adb) == NULL)
                  break;
                CleanLine(FLine);
                if(strlen(FLine)>0)
                {
                    fprintf(newlib, "%s\n", FLine);
                }
            }
            fclose(adb);
        }
        fprintf(newlib, "</ADB>\n</FILE>\n");
    }

    /*Put the temporary files together as a new library file*/
    indexsize=ftell(libindex);
    fflush(libindex);
    fflush(newlib);
    fclose(newlib);
    if(lib!=NULL) fclose(lib);
    fclose(libindex);

    newlib=fopen(LibNameTmp, "r");
    lib=fopen(LibName, "w");
    libindex=fopen(IndexName, "r");

    fprintf(lib, "<SDCCLIB>\n\n<INDEX>\n");

    /*Find out if the \n is expanded to \r\n or not*/
    if(ftell(lib)!=(long)strlen("<SDCCLIB>\n\n<INDEX>\n"))
    {
        IsDOSStyle=1;
    }

    indexsize+=ftell(lib)+strlen("0123456789\n\n</INDEX>\n\n");
    if(IsDOSStyle) indexsize+=4;

    fprintf(lib, "%10ld\n", indexsize);

    while(!feof(libindex))
    {
        FLine[0]=0;
        if (fgets(FLine, MAXLINE, libindex) == NULL)
          break;
        fprintf(lib, "%s", FLine);
    }
    fprintf(lib, "\n</INDEX>\n\n");

    while(!feof(newlib))
    {
        FLine[0]=0;
        if (fgets(FLine, MAXLINE, newlib) == NULL)
          break;
        fprintf(lib, "%s", FLine);
    }
    fprintf(lib, "\n</FILES>\n\n");
    fprintf(lib, "</SDCCLIB>\n");

    fclose(newlib);
    fclose(lib);
    fclose(libindex);

    remove(LibNameTmp);
    remove(IndexName);

    free(AdbName);
}

void ExtractRel(char * RelName)
{
    int state=0;
    char *AdbName = 0;

    AdbName = strdup(RelName);
    ChangeExtension(AdbName, "adb");

    lib=fopen(LibName, "r");
    if(lib==NULL)
    {
        printf("ERROR: Couldn't open file '%s'\n", LibName);
        free(AdbName);
        return;
    }

    rel=fopen(RelName, "w");
    if(rel==NULL)
    {
        printf("ERROR: Couldn't create file '%s'\n", RelName);
        fclose(lib);
        free(AdbName);
        return;
    }
    GetNameFromPath(RelName, ModName);

    adb=fopen(AdbName, "w");
    if(adb==NULL)
    {
        printf("ERROR: Couldn't create file '%s'\n", AdbName);
        fclose(lib);
        fclose(rel);
        free(AdbName);
        return;
    }

    while(!feof(lib))
    {
        if(state==5) break;
        FLine[0]=0;
        if (fgets(FLine, MAXLINE, lib) == NULL)
          break;
        CleanLine(FLine);

        switch(state)
        {
            case 0:
                if(EQ(FLine, "<FILE>"))
                {
                    FLine[0]=0;
                    if (fgets(FLine, MAXLINE, lib) == NULL)
                      break;
                    CleanLine(FLine);
                    if(EQ(FLine, ModName)) state=1;
                }
            break;
            case 1:
                if(EQ(FLine, "<REL>")) state=2;
            break;
            case 2:
                if(EQ(FLine, "</REL>"))
                    state=3;
                else
                    fprintf(rel, "%s\n", FLine);
            break;
            case 3:
                if(EQ(FLine, "<ADB>")) state=4;
            break;
            case 4:
                if(EQ(FLine, "</ADB>"))
                    state=5;
                else
                    fprintf(adb, "%s\n", FLine);
            break;
        }
    }

    fclose(rel);
    fclose(lib);
    fclose(adb);

    free(AdbName);
}

void DumpSymbols(void)
{
    int state=0;

    lib=fopen(LibName, "r");
    if(lib==NULL)
    {
        printf("ERROR: Couldn't open file '%s'\n", LibName);
        return;
    }

    FLine[0]=0;
    if (fgets(FLine, MAXLINE, lib) == NULL)
      return;
    CleanLine(FLine);
    if(NEQ(FLine, "<SDCCLIB>"))
    {
        printf("ERROR: File '%s' was not created with '%s'\n", LibName, ProgName);
        return;
    }

    while(!feof(lib))
    {
        if(state==3) break;
        FLine[0]=0;
        if (fgets(FLine, MAXLINE, lib) == NULL)
          break;
        CleanLine(FLine);

        switch(state)
        {
            case 0:
                if(EQ(FLine, "<INDEX>")) state=1;
            break;
            case 1:
                if(EQ(FLine, "<MODULE>"))
                {
                    FLine[0]=0;
                    if (fgets(FLine, MAXLINE, lib) == NULL)
                      break;
                    sscanf(FLine, "%s", ModName);
                    if(action==OPT_DUMP_SYM)
                    {
                        printf("%s.rel:\n", ModName);
                        state=2;
                    }
                    else
                    {
                        printf("%s.rel\n", ModName);
                    }
                }
                else if(EQ(FLine, "</INDEX>")) state=3;
            break;
            case 2:
                if(EQ(FLine, "</MODULE>"))
                {
                    state=1;
                    printf("\n");
                }
                else printf("   %s\n", FLine);
            break;
            default:
                state=3;
            case 3:
            break;
        }
    }

    fclose(lib);
}

int fileexist(char * fname)
{
    FILE * fp;

    fp=fopen(fname, "r");
    if(fp==NULL) return 0;
    fclose(fp);
    return 1;
}

void AddList(void)
{
    FILE * list;
    char *cc;
    char *as;
    char CmdLine[1024];
    char SrcName[FILENAME_MAX];
    char RelName[FILENAME_MAX];

    list=fopen(ListName, "r");
    if(list==NULL)
    {
        printf("ERROR: Couldn't open list file '%s'\n", ListName);
        return;
    }

    cc = getenv("SDCCLIB_CC");
    as = getenv("SDCCLIB_AS");

    action=OPT_ADD_REL;
    while(!feof(list))
    {
        RelName[0]=0;
        if (fgets(RelName, FILENAME_MAX, list) == NULL)
          break;
        CleanLine(RelName);
        if(strlen(RelName)>0) //Skip empty lines
        {
            if((cc!=NULL)||(as!=NULL))
            {
                strcpy(SrcName, RelName);
                if(strchr(SrcName,'.')==NULL)
                    strcat(SrcName,".src");
            }

            if(cc!=NULL)
            {
                ChangeExtension(SrcName, "c");
                if(fileexist(SrcName))
                {
                    sprintf(CmdLine, "%s %s", cc, SrcName);
                    printf("%s\n", CmdLine);
                    if (system(CmdLine))
                      continue;
                }
            }
            else if(as!=NULL)
            {
                ChangeExtension(SrcName, "asm");
                if(fileexist(SrcName))
                {
                    sprintf(CmdLine, "%s %s", as, SrcName);
                    printf("%s\n", CmdLine);
                    if (system(CmdLine))
                      continue;
                }
            }

            if(strchr(RelName,'.')==NULL)
            {
                //Try adding the default sdcc extensions
                strcat(RelName,".o");
                if(!fileexist(RelName))
                    ChangeExtension(RelName, "rel");
            }

            printf("Adding: %s\n", RelName);
            AddRel(RelName);
        }
    }
    action=OPT_ADD_LIST;
    fclose(list);
}

int main(int argc, char **argv)
{
    int j;
    ProcLineOptions (argc, argv);

    switch(action)
    {
        default:
            action=OPT_ADD_REL;
        case OPT_ADD_REL:
        case OPT_DEL_REL:
            for(j=0; j<NumRelFiles; j++) AddRel(RelName[j]);
            //Clean up
            for(j=0; j<NumRelFiles; j++) free(RelName[j]);
            free(RelName);
        break;

        case OPT_ADD_LIST:
            AddList();
        break;

        case OPT_EXT_REL:
            for(j=0; j<NumRelFiles; j++) ExtractRel(RelName[j]);
            //Clean up
            for(j=0; j<NumRelFiles; j++) free(RelName[j]);
            free(RelName);
        break;

        case OPT_DUMP_SYM:
        case OPT_DUMP_MOD:
            DumpSymbols();
        break;
    }
    return 0; //Success!!!
}

