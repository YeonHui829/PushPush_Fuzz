#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


//read_byte: read datas from socket, guarantee that all byte is accepted.
int read_byte(int sock, void * buf, int size)
{
	int read_size = 0;
	int str_len = 0;
	while(read_size < size)
	{
		str_len = read(sock, buf + read_size, size - read_size);
		if( str_len == 0 || str_len == -1)
		{
			printf("disconnected(sock);\n");
			//disconnected(sock);
			return 0;
		}
		read_size += str_len;
	}
	return read_size;
}

int main(int argc, char *argv[]){
  char buf[32];
	char *path = argv[1];
  int fd = open(path,O_RDONLY,0664);
  read_byte(fd,buf,sizeof(buf));
	printf("%s\n",buf);
  close(fd);
}