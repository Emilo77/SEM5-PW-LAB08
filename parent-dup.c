#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "err.h"

char message[] = "Hello from your parent!\n";

int main (int argc, char *argv[])
{
  int pipe_dsc[2];
  
  if (pipe(pipe_dsc) == -1) syserr("Error in pipe\n");

  switch (fork ()) {
    case -1: 
      syserr("Error in fork\n");

    case 0:
      if (close (0) == -1)            syserr("Error in child, close (0)\n");
      if (dup (pipe_dsc [0]) != 0)    syserr("Error in child, dup (pipe_dsc [0])\n");
      if (close (pipe_dsc [0]) == -1) syserr("Error in child, close (pipe_dsc [0])\n");
      if (close (pipe_dsc [1]) == -1) syserr("Error in child, close (pipe_dsc [1])\n");

      if (argc >= 2) {
	
        /* argv + 1 to argv bez pierwszego elementu - nazwy programu parent_dup */
	execvp(argv[1], argv + 1);
	syserr("Error in execvp\n");
      }
      exit (0);

    default:
      if (close (pipe_dsc [0]) == -1) syserr("Error in parent, close (pipe_dsc [0])\n");

      if (write (pipe_dsc [1], message, sizeof(message) - 1) != sizeof(message) - 1)
	syserr("Error in write\n");

      if (close (pipe_dsc [1]) == -1) syserr("Error in parent, close (pipe_dsc [1])\n");

      if (wait (0) == -1) 
	syserr("Error in wait\n");
      exit (0);
  } 
}
