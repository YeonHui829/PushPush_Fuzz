#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "./server.h"

#ifdef disconnected
char file_path1 [256];
char file_path2 [256];

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

int main(int argc, char *argv[]){
    clnt_cnt = MAX_USER;
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
#endif

#ifdef handleclnt
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
#endif

#ifdef loadJson
int main(int argc, char *argv[]){
  loadJson(argv[1]);
}
#endif

#ifdef readbyte
int main(int argc, char *argv[]){
  char buf[32];
	char *path = argv[1];
  int fd = open(path,O_RDONLY,0664);
  read_byte(fd,buf,sizeof(buf));
	printf("%s\n",buf);
  close(fd);
}
#endif

#ifdef sendmsgall
int main(){
  clnt_cnt = MAX_USER;
  clnt_socks[0] = open("./testoutput1", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[1] = open("./testoutput2", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[2] = open("./testoutput3", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  clnt_socks[3] = open("./testoutput4", O_WRONLY|O_CREAT|O_TRUNC, 0644);

  char event_buf[128];
  fgets(event_buf,128,stdin);
  send_msg_all((void*)event_buf, strlen(event_buf));

  for(int i=0;i<4;i++){
    close(clnt_socks[i]);
  }
}
#endif