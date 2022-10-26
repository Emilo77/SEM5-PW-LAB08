#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "err.h"

#define BUF_SIZE                1024

int main (int argc, char *argv[])
{
  int desc, buf_len;
  char buf[BUF_SIZE];
  
  if (argc != 2)
    fatal("Usage: %s <fifo-name>\n", argv[0]);
  
  printf("Child: trying to open %s\n", argv[1]);
  desc = open(argv[1], O_RDWR);
  if(desc == -1) syserr("Child, error in open:");
  
  printf("Child: reading data from descriptor %d\n", desc);
  if ((buf_len = read(desc, buf, BUF_SIZE - 1)) == -1)
    syserr("Error in read\n");;
  
  buf[buf_len < BUF_SIZE - 1 ? buf_len : BUF_SIZE - 1] = '\0';

  if (buf_len == 0)                         
    fatal("Unexpected end-of-file\n");
  else      
    printf("Child: read %d byte(s): \"%s\"\n", buf_len, buf);
  
  if(close(desc)) syserr("Child, error in close\n");

  exit(0);
}
