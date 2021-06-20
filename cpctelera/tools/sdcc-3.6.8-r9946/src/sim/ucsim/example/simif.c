#include <mcs51reg.h>
#include <stdio.h>


enum sif_command {
  DETECT_SIGN	        = '!',	// answer to detect command
  SIFCM_DETECT		= '_',	// command used to detect the interface
  SIFCM_COMMANDS	= 'i',	// get info about commands
  SIFCM_IFVER		= 'v',	// interface version
  SIFCM_SIMVER		= 'V',	// simulator version
  SIFCM_IFRESET		= '@',	// reset the interface
  SIFCM_CMDINFO		= 'I',	// info about a command
  SIFCM_CMDHELP		= 'h',	// help about a command
  SIFCM_STOP		= 's',	// stop simulation
  SIFCM_PRINT		= 'p',	// print character
  SIFCM_FIN_CHECK	= 'f',	// check input file for input
  SIFCM_READ		= 'r',	// read from input file
  SIFCM_WRITE		= 'w',	// write to output file
};

enum sif_answer_type {
  SIFAT_UNKNOWN		= 0x00,	// we don't know...
  SIFAT_BYTE		= 0x01,	// just a byte
  SIFAT_ARRAY		= 0x02,	// array of some bytes
  SIFAT_STRING		= 0x03,	// a string
  SIFAT_NONE		= 0x04	// no answer at all
};

unsigned char
_sdcc_external_startup (void)
{
  /* copied from device/examples/mcs51/simple2/hi.c */
  PCON = 0x80;  /* power control byte, set SMOD bit for serial port */
  SCON = 0x40;  /* serial control byte, mode 1, RI _NOT_ active */
  TMOD = 0x21;  /* timer control mode, byte operation */
  TCON = 0;     /* timer control register, byte operation */

  TH1  = 0xFA;  /* serial reload value, 9,600 baud at 11.0952Mhz */
  TR1  = 1;     /* start serial timer */

  TI   = 1;     /* enable transmission of first byte */
  return 0;
}

int
putchar (int c)
{
  while (!TI)
    ;
  SBUF = c;
  TI = 0;
  return c;
}

#define SIF_ADDRESS_SPACE_NAME	"xram"
#define SIF_ADDRESS_SPACE	__xdata
#define SIF_ADDRESS		0xffff

unsigned char SIF_ADDRESS_SPACE * volatile sif;

char
sif_get(char cmd)
{
  *sif= cmd;
  return *sif;
}

char
simulated(void)
{
  unsigned char c;
  *sif= 0x55;
  c= *sif;
  if (c != (unsigned char)~0x55)
    return(0);
  *sif= 0xaa;
  if (*sif != (unsigned char)~0xaa)
    return(0);
  return(1);
}

char
detect(void)
{
  *sif= SIFCM_DETECT;
  return *sif == DETECT_SIGN;
}

int nuof_commands;
unsigned char commands[100];

void
get_commands(void)
{
  int i;
  *sif= SIFCM_COMMANDS;
  nuof_commands= *sif;
  for (i= 0; i < nuof_commands; i++)
    commands[i]= *sif;
}

int
get_ifversion(void)
{
  *sif= SIFCM_IFVER;
  return(*sif);
}

unsigned char sim_version[15];

void
get_sim_version()
{
  unsigned char c;
  unsigned char i;
  
  *sif= SIFCM_SIMVER;
  sim_version[0]= 0;
  i= *sif;
  //printf("read answer of len=%d\n", i);
  if (i)
    {
      i= 0;
      c= *sif;
      //printf("  ans[%d]= %02x,%c\n", i, c, c);
      while (c && (i<14))
	{
	  sim_version[i++]= c;
	  c= *sif;
	  //printf("  ans[%d]= %02x,%c\n", i, c, c);
	}
      while (c)
	{
	  c= *sif;
	  //printf("  ans[]= %02x,%c\n", c, c);
	}
      sim_version[i]= 0;
    }
}

void
print_cmd_infos(void)
{
  int i, j;
  unsigned char inf[5];
  for (i= 0; i < nuof_commands; i++)
    {
      printf("Command '%c' info:\n", commands[i]);
      *sif= SIFCM_CMDINFO;
      *sif= commands[i];
      inf[0]= *sif;
      for (j= 0; j < inf[0]; j++)
	{
	  inf[j+1]= *sif;
	  //printf(" 0x%02x", inf[j+1]);
	}
      printf("  need %d params, answers as ", inf[1]);
      switch (inf[2])
	{
	case SIFAT_UNKNOWN	: printf("unknown"); break;
	case SIFAT_BYTE		: printf("byte"); break;
	case SIFAT_ARRAY	: printf("array"); break;
	case SIFAT_STRING	: printf("string"); break;
	case SIFAT_NONE		: printf("none"); break;
	}
      printf(": ");
      *sif= SIFCM_CMDHELP;
      *sif= commands[i];
      if (*sif)
	{
	  j= *sif;
	  while (j)
	    {
	      putchar(j);
	      j= *sif;
	    }
	}
      printf("\n");
    }
  
}

void
sif_putchar(char c)
{
  *sif= SIFCM_PRINT;
  *sif= c;
}

void
sif_print(char *s)
{
  while (*s)
    sif_putchar(*s++);
}

void
fout_demo(char *s)
{
  while (*s)
    {
      *sif= SIFCM_WRITE;
      *sif= *s++;
    }
}

char
sif_fin_avail()
{
  return sif_get(SIFCM_FIN_CHECK);
}

char
sif_read()
{
  return sif_get(SIFCM_READ);
}

void
fin_demo()
{
  char i, c;
  printf("Reading input from SIMIF input file:\n");
  while (i= sif_fin_avail())
    {
      c= sif_read();
      if (c > 31)
	putchar(c);
    }
  printf("\nRead demo finished\n");
}

void
main(void)
{
  sif= (unsigned char SIF_ADDRESS_SPACE *)0xffff;
  printf("Testing simulator interface at %s[0x%x]\n",
	 SIF_ADDRESS_SPACE_NAME, SIF_ADDRESS);
  printf("%s", detect()?"Interface found.":"Interface not found");
  if (detect())
    {
      int i;
      i= get_ifversion();
      get_sim_version();
      printf(" Version %d (of simulator %s)\n", i, sim_version);
      get_commands();
      printf("Interface knows about %d commands:\n", nuof_commands);
      for (i= 0; i < nuof_commands; i++)
	printf("%c", commands[i]);
      printf("\n");
      print_cmd_infos();
      sif_print("Message from simulated program: Hello World!\n");
      fout_demo("Write this message\n"
		"to simif output file.\n"
		"\n"
		"Done.\n");
      fin_demo();
    }
  else
    printf("\n");
  //* (char __idata *) 0 = * (char __xdata *) 0x7654;
  *sif= SIFCM_STOP;
}
