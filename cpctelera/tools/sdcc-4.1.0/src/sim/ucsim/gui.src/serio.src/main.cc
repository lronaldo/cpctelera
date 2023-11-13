/******************************************************************************
 * to emulate the serial input and output of an 8051 controller               *
 * main.cc - the main stuff                                                   *
 ******************************************************************************/
//#include "ddconfig.h"

#include <stdio.h>
//#include <sys/types.h>
#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#if defined(HAVE_GETOPT_H)
# include <getopt.h>
#endif

#include "fileio.hh"
#include "frontend.hh"
#include "posix_signal.hh"

Viewer *view;
FileIO *fobj;

struct auto_answer_t {
  char *pattern;
  int p_len;
  char *answer;
  int a_len;
};

enum
 {
   aa_len= 100
 };


struct auto_answer_t aa[aa_len];
int max_pattern_len= 0;
int aa_p_num= 0;
int aa_a_num= 0;
char *aa_queue;
int aa_q_len= 0;


void
init_aa()
{
  int i;
  for (i= 0; i < aa_len; i++)
    {
      aa[i].pattern= 0;
      aa[i].p_len= 0;
      aa[i].answer= 0;
      aa[i].a_len= 0;
    }
}

void
start_aa()
{
  if (max_pattern_len > 0)
    {
      aa_queue= (char*)malloc(max_pattern_len);
      aa_queue[0]= 0;
    }
  else
    aa_queue= NULL;
}

void
new_aa_pattern(char *s, int len)
{
  if (aa_p_num < aa_len)
    {
      aa[aa_p_num].pattern= s;
      aa[aa_p_num].p_len= len;
      aa_p_num++;
      if (len > max_pattern_len)
	max_pattern_len= len;
    }
}

void
new_aa_answer(char *s, int len)
{
  if (aa_a_num < aa_len)
    {
      aa[aa_a_num].answer= s;
      aa[aa_a_num].a_len= len;
      aa_a_num++;
    }
}

// 0: no match
// 1: partial match
// 2: full match
int
check_pattern(char *p, int pl, char *q, int ql)
{
  int m= pl, i;
  if (ql < m)
    m= ql;
  if (m == 0)
    return 0;
  for (i= 0; i<m; i++)
    {
      if (p[i] != q[i])
	break;
    }
  if (i != m)
    return 0;
  if (pl == ql)
    return 2;
  return 1;
}

struct auto_answer_t *
get_aa(int *partial)
{
  
  return 0;
}

void
shift_queue()
{
  int i;
  if (aa_queue)
    {
      for (i= 1; i<aa_q_len; i++)
	aa_queue[i-1]= aa_queue[i];
      aa_queue[--aa_q_len]= 0;
    }
}

void
aa_proc(char c)
{
  int i, p;
  // add to queue
  if (!aa_queue)
    return;
  if (aa_q_len >= max_pattern_len)
    shift_queue();
  aa_queue[aa_q_len++]= c;
  // search
  for (i=p= 0; i < aa_p_num; i++)
    {
      int m= check_pattern(aa[i].pattern, aa[i].p_len, aa_queue, aa_q_len);
      if (m == 2)
	{
	  // found a full match
	  int j;
	  for (j=0; j < aa[i].a_len; j++)
	    {
	      fobj->SendByte(aa[i].answer[j]);
	    }
	  shift_queue();
	  return;
	}
      if (m == 1)
	p++;
    }
  if (!p)
    shift_queue();
}

// globals
int doloop = 1;

// the signal handler
void HandleSig(int info)
{
  doloop = 0;
}

int
hex2bin(char h)
{
  int cv= 0;
  if ((h >= '0') && (h <= '9'))
    cv= h-'0';
  else if ((h >= 'A') && (h <= 'F'))
    cv= 10 + h-'A';
  else if ((h >= 'a') && (h <= 'f'))
    cv= 10 + h-'a';
  return cv;
}

/* Convert C escapes into bin chars */

char *
unesc_cstr(char *s, int *len)
{
  char *d= (char*)malloc(strlen(s));
  int i, j;
  d[j= 0]= 0;
  for (i= 0; s[i]!=0; i++)
    {
      if (s[i]=='\\')
	{
	  if (s[++i]==0)
	    break;
	  switch (s[i])
	    {
	    case 'a': d[j++]= '\a'; d[j]= 0; break;
	    case 'b': d[j++]= '\b'; d[j]= 0; break;
	    case 'e': d[j++]= 27; d[j]= 0; break;
	    case 'f': d[j++]= '\f'; d[j]= 0; break;
	    case 'n': d[j++]= '\n'; d[j]= 0; break;
	    case 'r': d[j++]= '\r'; d[j]= 0; break;
	    case 't': d[j++]= '\t'; d[j]= 0; break;
	    case 'v': d[j++]= '\v'; d[j]= 0; break;
	    case '\\': d[j++]= '\\'; d[j]= 0; break;
	    case '\'': d[j++]= '\''; d[j]= 0; break;
	    case '\"': d[j++]= '\"'; d[j]= 0; break;
	    case '?': d[j++]= '?'; d[j]= 0; break;
	    case 'x':
	      {
		if (s[++i]==0)
		  break;
		int n= 0, l= strspn(&s[i], "0123456789abcdefABCDEF"), v= 0, cv;
		do
		  {
		    cv= hex2bin(s[i]);
		    v= (v*16) + cv;
		    if (s[++i]==0)
		      break;
		  }
		while (++n < l);
		d[j++]= v&0xff;
		d[j]= 0;
		i--;
		break;
	      }
	    case 'u':
	      {
		if (s[++i]==0)
		  break;
		int n= 0, l= 4, v= 0, cv;
		do
		  {
		    cv= hex2bin(s[i]);
		    v= (v*16) + cv;
		    if (s[++i]==0)
		      break;
		  }
		while (++n < l);
		d[j++]= v&0xff;
		d[j]= 0;
		i--;
		break;
	      }
	    case 'U': 
	      {
		if (s[++i]==0)
		  break;
		int n= 0, l= 8, v= 0, cv;
		do
		  {
		    cv= hex2bin(s[i]);
		    v= (v*16) + cv;
		    if (s[++i]==0)
		      break;
		  }
		while (++n < l);
		d[j++]= v&0xff;
		d[j]= 0;
		i--;
		break;
	      }
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      {
		int n= 0, l= strspn(&s[i], "01234567"), v= 0;
		if (l>3)
		  l= 3;
		do
		  {
		    v= (v*8) + (s[i]-'0');
		    if (s[++i]==0)
		      break;
		  }
		while (++n < l);
		d[j++]= v&0xff;
		d[j]= 0;
		i--;
		break;
	      }
	    default:
	      d[j++]= s[i];
	      d[j]= 0;
	      break;
	    }
	  if (s[i]==0)
	    break;
	}
      else
	{
	  d[j++]= s[i];
	  d[j]= 0;
	}
    }
  if (len)
    *len= j;
  return d;
}

// usage
void PrintUsage(char *progname)
{
  std::cout << "Usage: " << progname << " [-i <filename>] [-o <filename>] [-hIO]\n";
  std::cout << "-i <filename>\t<filename> is the pipe to the controllers' serial input\n";
  std::cout << "-o <filename>\t<filename> is the pipe to the controllers' serial output\n";
  std::cout << "-I \t\thexa filter on input\n";
  std::cout << "-O \t\thexa filter on output\n";
  std::cout << "-L n\t\tSet line length of hex dump in output panel (def=8)\n";
  std::cout << "-a string\tPattern for autoanswer\n";
  std::cout << "-A string\tAnswer for autoanswer\n";
  std::cout << "-h\t\tshow the help\n";
  std::cout << "\nTim Hurman - t.hurman@virgin.net\n";
  exit(0);
}


// the main function
int main(int argc, char **argv)
{
  char *string = new char[MAX_SIZ];
  extern char *optarg;
  int errflg=0;
  int c, l= 8;
  enum filter_t fi= flt_none, fo= flt_none;
  const char *infile = DEF_INFILE;
  const char *outfile = DEF_OUTFILE;
  
  // sort out any command line params
  while ((c = getopt(argc, argv, "i:o:hOIL:a:A:")) != EOF)
    switch(c) {
    case 'i':
      infile = optarg;
      break;
    case 'o':
      outfile = optarg;
      break;
    case 'I':
      fi= flt_hex;
      break;
    case 'O':
      fo= flt_hex;
      break;
    case 'L':
      l= strtol(optarg, 0, 0);
      break;
    case 'h':
      errflg++;
      break;
    case 'a':
      {
	int l;
	char *s= unesc_cstr(optarg, &l);
	new_aa_pattern(s, l);
	break;
      }
    case 'A':
      {
	int l;
	char *s= unesc_cstr(optarg, &l);
	new_aa_answer(s, l);
	break;
      }
    default:
      std::cerr << "Invalid or unknown switch\n";
      errflg++;
      break;
    }
  
  // was there a problem
  if(errflg)
    PrintUsage(argv[0]);
  
  // the main objects needed
  view = new Viewer();
  fobj = new FileIO(infile, outfile);
  SigHandler *sig = new SigHandler();
  
  view->iflt_mode(fi);
  view->oflt_mode(fo);
  view->set_length(l);
  
  // add a signal handler for ^C
  sig->SetSignal(SIGINT, HandleSig);

  start_aa();
  // set the timeout for waiting for a char
  fd_set s;
  while(doloop)
    {
      int ret, i;
      FD_ZERO(&s);
      FD_SET(fileno(stdin), &s);
      FD_SET(fobj->infile_id(), &s);

      i= select(FD_SETSIZE, &s, NULL, NULL, NULL);
      if (i >= 0)
	{
	  if (FD_ISSET(fileno(stdin), &s))
	    {
	      ret= view->GetChInWin(&string[0]);
	      if (ret > 0)
		{
		  fobj->SendByte(string[0]);
		}
	      else if (ret < 0)
		break;
	    }

	  if (FD_ISSET(fobj->infile_id(), &s))
	    {
	      if (fobj->RecvByte(string) > 0)
		{
		  aa_proc(string[0]);
		  view->AddChOutWin(string[0]);
		}
	    }
	}
      //usleep(5000);
    }
  
  delete fobj;
  delete view;
  delete sig;
  delete string;
  return(0);
}
