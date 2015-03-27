/*-------------------------------------------------------------------------
  timeout.c - source file for running ucSim within the regression tests

             Written By -  Bernhard Held . bernhard@bernhardheld.de (2001)
   Native WIN32 port by -  Borut Razem . borut.razem@siol.net (2007)

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

#define PROGNAME "timeout"

#define USAGE PROGNAME " : 1.10\n" \
              "Usage : " PROGNAME " timeout_in_seconds filename [arguments]\n" \
              "  \"filename\" is executed, the arguments are passed to \"filename\".\n" \
              "  When \"filename\" exits before the timeout expires, the\n" \
              "  exit-status of \"filename\" is returned.\n" \
              "  When the timeout expires before \"filename\" exits, \"filename\"\n" \
              "  will be killed and an exit-status of 1 is returned.\n"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <malloc.h>

/*
   Native WIN32 version:

   The program creates a chile process using CreateProcess(). The waits until:
   - the child exits
        The exit status of the child is returned.
   - the timeout elapses
        The child will be killed.
*/

int
makeCmdLine(char *buf, int argc, const char * const *argv)
{
  int len = 0;

  argv += 2;
  while (argc-- > 2)
    {
      int argLen = strlen(*argv);
      len += argLen;
      if (buf)
        {
          memcpy(buf, *argv, argLen);
          buf += argLen;
        }
      if (argc > 2)
        {
          ++len;
          if (buf)
            *buf++ = ' ';
        }
      ++argv;
    }
  if (buf)
    *buf = '\0';

  return len + 1;
}

int
main (int argc, const char * const *argv)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  char *cmdLine;
  long timeout;
  DWORD exitCode;
  HANDLE oldStderr;

  if (argc < 3)
    {
      fprintf (stderr, USAGE);
      return 1;
    }
  timeout = atol (argv[1]);
  if (timeout == 0)
    {
      fprintf (stderr, "Error parameter " PROGNAME ": must be a non-zero dezimal value\n");
      return 1;
    }

  cmdLine = alloca(makeCmdLine(NULL, argc, argv));
  makeCmdLine(cmdLine, argc, argv);

  ZeroMemory(&si, sizeof (si));
  si.cb = sizeof (si);
  ZeroMemory(&pi, sizeof (pi));

  /* We'll redirect here stderr to stdout, which will be redirected  */
  /* to /dev/null by the shell. The shell could also redirect stderr */
  /* to /dev/null, but then this program doesn't have the chance to  */
  /* output any real error. */
  oldStderr = GetStdHandle (STD_ERROR_HANDLE);
  SetStdHandle (STD_ERROR_HANDLE, GetStdHandle (STD_OUTPUT_HANDLE));

  /* Start the child process. */
  if(!CreateProcess (NULL, // No module name (use command line)
    cmdLine,        // Command line
    NULL,           // Process handle not inheritable
    NULL,           // Thread handle not inheritable
    TRUE,           // Set handle inheritance to TRUE
    0,              // No creation flags
    NULL,           // Use parent's environment block
    NULL,           // Use parent's starting directory 
    &si,            // Pointer to STARTUPINFO structure
    &pi)            // Pointer to PROCESS_INFORMATION structure
    ) 
    {
      printf ("CreateProcess failed (%d).\n", GetLastError ());
      return 1;
    }

  /* Restore stderr */
  SetStdHandle (STD_ERROR_HANDLE, oldStderr);

  /* Wait until child process exits or timeout expires. */
  if (WaitForSingleObject (pi.hProcess, timeout * 1000) == WAIT_TIMEOUT)
    {
      TerminateProcess (pi.hProcess, 1);
      WaitForSingleObject (pi.hProcess, INFINITE);
    }

  GetExitCodeProcess (pi.hProcess, &exitCode);

  /* Close process and thread handles. */
  CloseHandle (pi.hProcess);
  CloseHandle (pi.hThread);

  return exitCode;
}

#else

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

/* First the program tries to limit the maximum CPU-time to the timeout-value.
   Then the child is run with execvp().

   It's not possible to limit the CPU-time under Cygwin (V1.3.3). If setrlimit (RLIMIT_CPU, rlp)
   fails, the program will fork() and run the child with execvp(). The fork/exec pair is slow on
   Cygwin, but what else can we do? The parent sleeps until:
   - a signal shows the child´s exitus
        The exit status of the child is returned.
   - the timeout elapses
        The child will be killed.
*/

/* Get the status from all child processes that have terminated, without ever waiting.
   This function is designed to be a handler for SIGCHLD, the signal that indicates
   that at least one child process has terminated.
   http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_23.html#SEC401
*/

#ifndef WAIT_ANY
  #define WAIT_ANY -1
#endif

void
sigchld_handler (int signum)
{
  int pid;
  int status;
  int exit_status = 0;

  while (1)
    {
      pid = waitpid (WAIT_ANY, &status, WNOHANG);
      if (WEXITSTATUS (status))
        exit_status = 1; // WEXITSTATUS(status);
      /* pid == -1: no children               */
      /* pid ==  0: no children to be noticed */
      if (pid <= 0)
        break;
    }
  exit (exit_status);
}

int
main (int argc, char * const *argv)
{
  /* if getrlimit() / setrlimit() succeed, then no fork is neeeded */
  int flagNoFork = 0;
  int old_stderr;
  long timeout;
  pid_t pid_child;
  struct rlimit rl;

  if (argc < 3)
    {
      fprintf (stderr, USAGE);
      return 1;
    }
  timeout = atol (argv[1]);
  if (timeout == 0)
    {
      fprintf (stderr, "Error parameter " PROGNAME ": must be a non-zero dezimal value\n");
      return 1;
    }

  /* try to use getrlimit() / setrlimit() for RLIMIT_CPU */
  /* to limit the CPU-time                               */
  if (getrlimit (RLIMIT_CPU, &rl) == 0)
    {
      rl.rlim_cur = timeout;
      if (setrlimit (RLIMIT_CPU, &rl) == 0)
        flagNoFork = 1;
    }

  if (flagNoFork)
    { /* the CPU-time is limited: simple execvp */

      /* s51 prints warnings on stderr:                                  */
      /* serial input/output interface connected to a non-terminal file. */
      /* We'll redirect here stderr to stdout, which will be redirected  */
      /* to /dev/null by the shell. The shell could also redirect stderr */
      /* to /dev/null, but then this program doesn't have the chance to  */
      /* output any real error. */
      old_stderr = dup (STDERR_FILENO);
      dup2 (STDOUT_FILENO, STDERR_FILENO);
      /* shouldn't return */
      execvp (argv[2], argv + 2);
      /* restore stderr */
      dup2 (old_stderr, STDERR_FILENO);
      perror (argv[2]);
      return 1; /* Error */
    }
  else
    {
      /* do it the hard way: fork/exec */
      signal (SIGCHLD, sigchld_handler);
      pid_child = fork();
      if (pid_child == 0)
        {
           /* s51 prints warnings on stderr:                                  */
           /* serial input/output interface connected to a non-terminal file. */
           /* We'll redirect here stderr to stdout, which will be redirected  */
           /* to /dev/null by the shell. The shell could also redirect stderr */
           /* to /dev/null, but then this program doesn't have the chance to  */
           /* output any real error. */
           old_stderr = dup (STDERR_FILENO);
           dup2 (STDOUT_FILENO, STDERR_FILENO);
           /* shouldn't return */
           execvp (argv[2], argv + 2);
           /* restore stderr */
           dup2 (old_stderr, STDERR_FILENO);
           perror (argv[2]);
           return 1; /* Error */
        }
      else
        {
          /* this timeout is hopefully aborted by a SIGCHLD */
          sleep (timeout);
          fprintf (stderr, PROGNAME ": timeout, killing child %s\n", argv[2]);
          kill (pid_child, SIGTERM);
          return 1; /* Error */
        }
    }
}
#endif
