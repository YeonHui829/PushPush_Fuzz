#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "../server.h"


char input_file_path[MAX_USER][256];
char output_file_path[MAX_USER][256];

int input_fd[MAX_USER];

int main(int argc, char **argv){
    clnt_cnt = MAX_USER;
    loadJson("map1.json");
    for(int i=0;i<MAX_USER;i++){
        strcpy(input_file_path[i], argv[i+1]);
        sprintf(output_file_path[i],"testoutput%d",i+1);
    }

    char buf[256];
    for(int i=0; i<MAX_USER; i++){
		fprintf(stderr, "file path  :%s\n", output_file_path[i]);
		if((clnt_socks[i] = open(output_file_path[i], O_RDWR|O_APPEND|O_CREAT, 0644)) < 0){
			perror("open error\n");
            continue;
		}
        FILE *fp = fopen(input_file_path[i],"rb");
		fprintf(stderr,"input fd[%d]\n",i);
        fread(buf,sizeof(char),1,fp);
        int num = *buf - '0';
        write(clnt_socks[i],&num,sizeof(int));
        fgets(buf, sizeof(buf), fp);
        write_byte(clnt_socks[i],buf,strlen(buf));
        fclose(fp);
	}

    pthread_t t[MAX_USER];

    pthread_mutex_init(&mutx,NULL);

    json_serialize = "This is json_serialize";
    json_size = strlen(json_serialize);

    for(int i=0;i<MAX_USER;i++){
        if((pthread_create((&t[i]),NULL,handle_clnt,(void *)&clnt_socks[i]))!=0){
            perror("error\n\n");
            exit(1);
        }
    }

    for(int i=0;i<MAX_USER;i++){
        pthread_join(t[i],NULL);
    }

    return 0;
}