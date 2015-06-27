#include <stdio.h>
#include <pthread.h>

// simulated system
struct app {
  long PC;
  bool simulating;
} sys;

// locks
pthread_mutex_t simulation_lock;
pthread_mutex_t application_lock;


pthread_t sim_th;

struct sim_args {
  long int steps2go;
};

void
step_start(void)
{
  pthread_mutex_lock(&application_lock);
}

void
step_end(void)
{
  pthread_mutex_unlock(&application_lock);
}

void *
sim_thread(void *arg)
{
  printf("Sim: thread started, now trying lock...\n");
  pthread_mutex_lock(&simulation_lock);
  printf("Sim: got lock, now run\n");
  struct sim_args *args= (struct sim_args *)arg;
  bool done= 0;
  long steps2go= args->steps2go, steps= steps2go;
  while (!done)
    {
      step_start();
      sys.PC++;
      if (steps2go > 0)
	{
	  steps--;
	  if (done= steps == 0)
	    printf("Sim: %s steps done\n", steps2go);
	}
      else if (steps2go == 0)
	{
	}
      /*else if (steps2go < 0)
	{*/
	  if (done= !sys.simulating)
	    printf("Sim: requested to stop\n");
	  /*}*/
      step_end();
    }
  printf("Sim: finished, releasing lock...\n");
  pthread_mutex_unlock(&simulation_lock);
  printf("Sim: done\n");
  return(NULL);
}


void
start_command(void)
{
  pthread_mutex_lock(&application_lock);
}

void
end_command(void)
{
  pthread_mutex_unlock(&application_lock);
}


void
g_cmd(FILE *fin, FILE *fout)
{
  start_command();
  fprintf(fout, "PC=%ld, simulating=%d\n", sys.PC, sys.simulating);
  end_command();
}

void
s_cmd(FILE *fin, FILE *fout)
{
  start_command();
  fscanf(fin, " %ld", &sys.PC);
  fprintf(fout, "PC=%ld\n", sys.PC);
  end_command();
}

void
start_simulation(int steps)
{
  struct sim_args sargs;
  sargs.steps2go= steps;
  sys.simulating= 1;
  pthread_attr_t ta;
  pthread_attr_init(&ta);
  pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
  pthread_create(&sim_th, &ta, sim_thread, &sargs);
  pthread_attr_destroy(&ta);
}

void
r_cmd(FILE *fin, FILE *fout)
{
  start_command();
  if (pthread_mutex_trylock(&simulation_lock) != 0)
    {
      fprintf(fout, "Simulation already runing\n");
    }
  else
    {
      fprintf(fout, "Run from PC=%ld\n", sys.PC);
      start_simulation(-1);
      pthread_mutex_unlock(&simulation_lock);
    }
  end_command();
}

void
S_cmd(FILE *fin, FILE *fout)
{
  fprintf(fout, "Trying to get app lock...\n");
  start_command();
  fprintf(fout, "OK, lock is ours, set stopper flag...\n");
  sys.simulating= 0;
  fprintf(fout, "Release lock, to give chance of stop\n");
  end_command();
  fprintf(fout, "Wait for stop...\n");
  pthread_mutex_lock(&simulation_lock);
  fprintf(fout, "We got sim lock, so it stopped\n");
  pthread_mutex_unlock(&simulation_lock);
  fprintf(fout, "Stopped at PC=%ld\n", sys.PC);
}


struct input_args {
  int nr;
  char *fin_name;
  char *fout_name;
};

void *
input_thread(void *arg)
{
  struct input_args *args= (struct input_args *)arg;
  FILE *fin, *fout;
  if (args->fin_name)
    fin= fopen(args->fin_name, "r");
  else
    fin= stdin;
  if (args->fout_name)
    fout= fopen(args->fout_name, "w");
  else
    fout= stdout;
  bool done= 0;
  while (!done)
    {
      char cmd[100];
      fprintf(fout, "%d> ", args->nr); fflush(fout);
      fscanf(fin, " %99s", &cmd[0]);
      fprintf(fout, "%d Got command: %c\n", args->nr, cmd[0]);
      switch (cmd[0])
	{
	case 'q':
	  done= 1;
	  break;
	case 'g':
	  g_cmd(fin, fout);
	  break;
	case 's':
	  s_cmd(fin, fout);
	  break;
	case 'r':
	  r_cmd(fin, fout);
	  break;
	case 'S':
	  S_cmd(fin, fout);
	  break;
	default:
	  fprintf(fout, "%d Unknown command\n", args->nr);
	  break;
	}
    }
  fprintf(fout, "%d Console finished\n", args->nr);
  fclose(fin);
  fclose(fout);
  return(NULL);
}


int
main(int argc, char *argv[])
{
  pthread_t input_th[3];
  int threads= 0;

  pthread_mutex_init(&simulation_lock, NULL);
  pthread_mutex_init(&application_lock, NULL);

  struct input_args iargs= { 0, NULL, NULL };
  pthread_create(&input_th[0], NULL, input_thread, &iargs);
  threads++;
  if (argc > 1)
    {
      struct input_args iargs= { 1, argv[1], argv[1] };
      pthread_create(&input_th[1], NULL, input_thread, &iargs);
      threads++;
    }
  if (argc > 2)
    {
      struct input_args iargs= { 2, argv[2], argv[2] };
      pthread_create(&input_th[2], NULL, input_thread, &iargs);
      threads++;
    }
  int i;
  for (i= 0; i < threads; i++)
    {
      void *ret;
      pthread_join(input_th[i], &ret);
    }
  return(0);
}
