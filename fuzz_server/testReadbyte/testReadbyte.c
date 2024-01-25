#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "../server.h"

int main(int argc, char *argv[]){
  char buf[32];
	char *path = argv[1];
  int fd = open(path,O_RDONLY,0664);
  read_byte(fd,buf,sizeof(buf));
	printf("%s\n",buf);
  close(fd);
}