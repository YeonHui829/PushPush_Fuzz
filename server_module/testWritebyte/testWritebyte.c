#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

int write_byte(int sock, void * buf, int size)
{

	int write_size = 0;
	int str_len = 0;
	while(write_size < size)
	{
		str_len = write(sock, buf + write_size, size - write_size);
		if( str_len == 0)
		{
			return 0;
		}
		if( str_len == -1)
		{
			// disconnected(sock);
      printf("disconnected(sock);\n");
		}
		write_size += str_len;
	}
	return write_size;
}

int main(int argc, char *argv[]){
  struct stat s;
  char path[32];
  scanf("%s",path);
  if (stat(path, &s) == 0) {
    printf("File size: %lld bytes\n", (long long)s.st_size);
  }
  else {
    printf("Cannot access\n");
  }

  char buf[s.st_size];

  FILE *fp = fopen(path,"rb");
  int fd = open("./testout", O_WRONLY | O_CREAT | O_TRUNC , 0644);

  fread(buf,1,s.st_size,fp);

  write_byte(fd,buf,strlen(buf));

  close(fd);
  fclose(fp);
}