#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h> 



#define MAX_USER 2

int usr_cnt = 0;
int clnt_socks[MAX_USER];
int clnt_cnt = 2;
int game_start = 0;
int output_fd[2];
int json_size ;
char * json_serialize;
char file_path[MAX_USER][256];
char user_name[MAX_USER][256];

pthread_mutex_t mutx;

int input_fd[2];

void *handle_clnt(void * arg)
{
	fprintf(stderr, "enter thread\n");
	int clnt_sock = *((int*)arg);
	int read_fd = input_fd[clnt_sock];
	int write_fd = output_fd[clnt_sock];
	int str_len = 0;
	char event [256];
	char name_size [256];
	fprintf(stderr, "thread info[%d] read fd : %d output fd : %d\n",clnt_sock, read_fd, write_fd );

	//recive name size
	
	// str_len = read_byte(clnt_sock, (void *)&name_size, sizeof(int));
	fprintf(stderr, "first read\n");
	read(read_fd, name_size, 1);
    // fscanf(file, "%d", &name_size);
	fprintf(stderr, "read finish, name size : %s\n", name_size);
	//reciev name, send id
	pthread_mutex_lock(&mutx);
	for (int i = 0; i < clnt_cnt; i++) 
	{
		if (clnt_sock == clnt_socks[i])
		{
			// read_byte(clnt_sock, (void *)user_name[i], name_size);
            // fgets(user_name[i], name_size, file);
			fprintf(stderr, "read wait2\n");
			read(read_fd, (void *)user_name[i], atoi(name_size));
			fprintf(stderr, "read finish2\n");
			printf("[start]%s is enter\n",user_name[i]);
			// write_byte(clnt_sock, (void *)&i, sizeof(int));
			write(write_fd, (void *)&i, sizeof(int));
			usr_cnt++;
			break;
		}
	}
	pthread_mutex_unlock(&mutx);


    // FILE * output_fp;
    // if((output_fp= fopen(output_file[clnt_sock], "wb")) == NULL){
    //     fprintf(stderr, "fopen error\n");
    //     exit(1);
    // }

	//send json
    // fwrite(json_size, 1, sizeof(int), output_fp);
    // fwrite(json_serialize, 1, json_size, output_fp);

	// write_byte(clnt_sock, (void *)&json_size, sizeof(int));
	// write_byte(clnt_sock, json_serialize, json_size);

	write(write_fd, (void *)&json_size, sizeof(int));
	write(write_fd, json_serialize, json_size);



	while(usr_cnt < MAX_USER); //wait untill all user is connected
	
	//send connected user information
	for(int i=0; i< MAX_USER; i++)
	{
		int len = strlen(user_name[i]);
		// write_byte(clnt_sock, &len,sizeof(int));
		// write_byte(clnt_sock,user_name[i], len);

		write(write_fd, &len, sizeof(int));
		write(write_fd, user_name[i], len);
        // fwrite(len, 1, sizeof(int), output_fp);
		// fwrite(user_name, 1, len, output_fp);
	}

	//receive and echo command
	// char * event = malloc(1);
	
	while (read(read_fd, event, 1)) //stdin으로 file로  
	{
		printf("thread [%d] move: %s\n", clnt_sock, event);
		// send_msg_all((void *)&event, sizeof(int)); //stdout이나 file에 작성하는 식으로 변경 
		//detect end flag
		if(atoi(event) == MAX_USER*4)
		{
			printf("end game!\n");
			break;
			// disconnected(clnt_sock);
		}
	}

    // fclose(file_path1);
	close(read_fd);
	close(write_fd);

	return NULL;
}

int main(int argc, char **argv){
    strcpy(file_path[0], argv[1]);
    strcpy(file_path[1], argv[2]);

	char buf[256];
    for(int i=0; i<MAX_USER; i++){
        sprintf(buf, "handle_clnt_output%d.txt", i+1);
		output_fd[i] = open(buf, O_WRONLY|O_CREAT|O_TRUNC, 0644);
		fprintf(stderr, "file path  :%s\n", file_path[i]);
		if((input_fd[i] = open(file_path[i], O_RDONLY)) <= 0){
			fprintf(stderr, "open error\n");
		}
		fprintf(stderr,"input fd[%d] : %d\n", i ,input_fd[i]);
		clnt_socks[i] = i;
	}

    pthread_t t1;
    pthread_t t2;

    // for(int i=0; i<MAX_USER; i++){
    //     clnt_socks[i] = i;
    // }

    pthread_mutex_init(&mutx,NULL);

    json_serialize = "This is json_serialize";
    json_size = strlen(json_serialize);

    int clnt_sock = 0;
    if((pthread_create((&t1),NULL,handle_clnt,(void *)&clnt_sock))!=0){
		perror("error\n\n");
		exit(1);
	}
    int clnt_sock2 = 1;

    if((pthread_create((&t2),NULL,handle_clnt,(void *)&clnt_sock2))!=0){
		perror("error\n\n");
		exit(1);
	}

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}