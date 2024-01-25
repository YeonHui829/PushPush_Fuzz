#include <pthread.h>

#ifndef SERVER_H
#define SERVER_H

#define BUF_SIZE 256
#define MAX_USER 4
#define NAME_SIZE 16
#define PATH_LENGTH 256

extern int usr_cnt; //num of connected user
extern int game_start;
extern int max_user;
extern char ** user_name;
extern int json_size;
extern char * json_serialize;
extern pthread_mutex_t mutx;

extern int clnt_cnt;
extern int clnt_socks[MAX_USER];

int loadJson();
void disconnected(int sock);
int write_byte(int sock, void * buf, int size);
int read_byte(int sock, void * buf, int size);
void send_msg_all(void * event, int len);
void *handle_clnt(void * arg);
void error_handling(char * msg);

#endif