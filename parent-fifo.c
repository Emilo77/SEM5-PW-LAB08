#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "err.h"

char message[] = "Hello from your parent!";

int main ()
{
  int desc;
  int wrote = 0;
  int message_len = 23;
  
  char *file_name = "/tmp/fifo_tmp";
  
  if (mkfifo(file_name, 0755) == -1) syserr("Error in mkfifo\n");
    
  switch (fork()) {                     
    case -1: 
      syserr("Error in fork\n");
   
    case 0:                             
      printf("I am a child and my pid is %d\n", getpid());            
      execlp("./child-fifo", "./child-fifo", file_name, NULL); 
      syserr("Error in execlp\n");
    
    default:                            
      break;
  }

  printf("I am a parent and my pid is %d\n", getpid());
    
  desc = open(file_name, O_WRONLY);
  if(desc == -1) syserr("Error in open\n"); 
  
  printf("Parent: writing a message\n");
  wrote = write(desc, message, message_len);
  if (wrote < 0) 
    syserr("Error in write\n");
  else
    printf("Parent: wrote %d byte(s) and waiting\n",wrote);
    
  if (wait(0) == -1) syserr("Error in wait\n");
  
  if(close(desc)) syserr("Error in close\n");
  
  if(unlink(file_name)) syserr("Error in unlink:");
  return 0;
}
