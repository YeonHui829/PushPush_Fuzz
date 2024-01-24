#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_USER 4

int clnt_socks[MAX_USER];
int clnt_cnt = 4;
int game_start = 0;
int usr_cnt = 4;

pthread_mutex_t mutx;

char file_path1 [256];
char file_path2 [256];

void disconnected(int sock)
{
	pthread_mutex_lock(&mutx);
	for (int i = 0; i < clnt_cnt; i++)   // remove disconnected client
	{
		if (sock == clnt_socks[i])
		{
			while (i < clnt_cnt-1)
			{
				clnt_socks[i] = clnt_socks[i+1];
				i++;
			}
			break;
		}
	}
	clnt_cnt--;

	if(clnt_cnt == 0)
	{
		game_start = 0;
		usr_cnt = 0;
	}
	pthread_mutex_unlock(&mutx);
	// close(sock);
}

void * thread_func1(){
    FILE * file;
    if((file = fopen(file_path1, "rb")) == NULL){
        fprintf(stderr, "fopen error\n");
        exit(1);
    }

    int input1;
    int input2;
    char buf [256];
    // while(fgets(buf, 256,, file) != NULL)
    if(fgets(buf, 256, file) == NULL){
        sscanf(buf, "%d %d", &input1, &input2);
    }

    disconnected(input1);
    disconnected(input2);

    fclose(file);
}

void * thread_func2(){
    FILE * file;
    if((file = fopen(file_path2, "rb")) == NULL){
        fprintf(stderr, "fopen error\n");
        exit(1);
    }

    int input1;
    int input2;
    char buf [256];
    // while(fgets(buf, 256,, file) != NULL)
    if(fgets(buf, 256, file) == NULL){
        sscanf(buf, "%d %d", &input1, &input2);
    }

    disconnected(input1);
    disconnected(input2);

    fclose(file);
}

//disconnected: check if any user is disconnected, handel clnt_cnt


int main(int argc, char **argv){

    strcpy(file_path1, argv[1]);
    strcpy(file_path2, argv[2]);

    pthread_mutex_init(&mutx,NULL);

    for(int i=0; i<4; i++){
        clnt_socks[i] = i;
    }


    pthread_t t1;
    pthread_t t2;

    if((pthread_create((&t1),NULL,thread_func1,NULL))!=0){
		perror("error\n\n");
		exit(1);
	}
    if((pthread_create((&t2),NULL,thread_func2,NULL))!=0){
		perror("error\n\n");
		exit(1);
	}

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;

}