#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include "./cJSON.h"

#define BUF_SIZE 256
#define MAX_USER 4
#define NAME_SIZE 16
#define PATH_LENGTH 256

int max_user;
char ** user_name;
int json_size;
char * json_serialize;
int loadJson() 
{
	char filepath[PATH_LENGTH];
	fgets(filepath, PATH_LENGTH-1, stdin);
	filepath[strlen(filepath)-1]=0;
	FILE *file = fopen(filepath,"r");
	if(file == NULL)
	{
		fprintf(stderr,"ERROR: open file");
		return 1;
	}
	struct stat st;
	if(stat(filepath, &st) == -1)
	{
  		fprintf(stderr,"ERROR: stat()\n");
  		return 1;
	}
	int size = st.st_size;

	char* jsonfile = (char*)malloc(size+1);
	if(jsonfile == NULL)
	{
		fprintf(stderr,"ERROR: memory allocation\n");
		return 1;
	}

	int read_size = fread(jsonfile, 1, size, file);
	if(read_size != size)
	{
		fprintf(stderr, "ERROR: read file\n");
		return 1;
	}

	fclose(file);
	jsonfile[size] = '\0';
	
	cJSON* root = cJSON_Parse(jsonfile);
	if (root == NULL) 
	{
		printf("JSON 파싱 오류: %s\n", cJSON_GetErrorPtr());
      	return 1;
	}

	cJSON* num_user = cJSON_GetObjectItem(root, "max_user");
	max_user = num_user->valueint;
	user_name = (char**)malloc(sizeof(char*) * max_user);
	for(int i=0; i< max_user; i++)
	{
		user_name[i] = (char*)malloc(sizeof(char) * NAME_SIZE);
	}

	json_serialize = cJSON_Print(root);
	json_size = strlen(json_serialize);
	
	free(root);
	free(jsonfile);
	return 0;
}

int main(){
    // if(loadJson()){
    //     exit(1);
    // }
    loadJson();
}