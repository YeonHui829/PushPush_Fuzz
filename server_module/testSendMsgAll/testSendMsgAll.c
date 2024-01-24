#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;

int clnt_cnt = 4;
int clnt_socks[4];

//write_byte: write datas to socket, guarantee that all the byte is sent. 
int write_byte(int sock, void * buf, int size)
{

	int write_size = 0;
	int str_len = 0;
	while(write_size < size)
	{
		str_len = write(sock, buf + write_size, size - write_size);
		if(str_len == -1)
		{
			perror("write: ");
			return 0;
		}
		write_size += str_len;
	}
	return write_size;
}

//send_msg_all: send msg to all connected users
void send_msg_all(void * event, int len)
{
	pthread_mutex_lock(&mutx);
	for (int i = 0; i < clnt_cnt; i++)
	{
		write_byte(clnt_socks[i], event, len);
	}
	pthread_mutex_unlock(&mutx);
}

int main(){
  clnt_socks[0] = open("./testoutput1", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[1] = open("./testoutput2", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[2] = open("./testoutput3", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[3] = open("./testoutput4", O_WRONLY|O_CREAT|O_TRUNC, 0644);

  char event_buf[128];
  fgets(event_buf,128,stdin);
  send_msg_all(event_buf, strlen(event_buf));

  for(int i=0;i<4;i++){
    close(clnt_socks[i]);
  }
}