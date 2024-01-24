
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../cJSON.h"

#define BUF_SIZE 256
#define MAX_USER 2
#define NAME_SIZE 16
#define PATH_LENGTH 256
#define queue_size 20

typedef struct location{
    int x;
    int y;
} location_t;

typedef struct user{
    char name[NAME_SIZE];
    int score;
    location_t base_loc;
    location_t user_loc;
}user_t;

typedef struct object_data{
    int map_width;
	int map_height;
    int timeout;
    int max_user;
    struct user * users;
    location_t * item_locations;
    location_t * block_locations;
}object_data_t;

enum entity {
	EMPTY = 0,
	BLOCK = -1,
	ITEM = -9, //item will be -10 ~ -110
	USER = 1, //user wil be 1 ~ 3
	BASE = 9, //base will be 10 ~ 30
};

enum spans {
	UP, 
	DOWN, 
	LEFT, 
	RIGHT
};

int ** map; // cell
object_data_t Model; //model
char msg_info[BUF_SIZE] = "";
char buf[BUF_SIZE] = "";
int sock;
int my_id;
int num_item, num_block;
int current_num_item;

pthread_mutex_t mutx;
pthread_cond_t cond;
int event_arry[queue_size];
int rear = 0;
int front = 0;

int usr_cnt = 0; //num of connected user
int game_start = 0;
int max_user;
char ** user_name;
int json_size;
char * json_serialize;

int clnt_cnt = 0;
int clnt_socks[MAX_USER];
pthread_mutex_t mutx;


int loadJson(char * filepath) 
{
	// char filepath[PATH_LENGTH];
	// fgets(filepath, PATH_LENGTH-1, stdin);
	// filepath[strlen(filepath)-1]=0;
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
	if(jsonfile 	== NULL)
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

int parseJson(char * jsonfile) {

    cJSON* root;
	root = cJSON_Parse(jsonfile);
	if (root == NULL) {
		printf("JSON parsing error: %s\n", cJSON_GetErrorPtr());
        return 1;
	}
        
	cJSON* timeout = cJSON_GetObjectItem(root, "timeout");
	Model.timeout = timeout->valueint;
	cJSON* max_user = cJSON_GetObjectItem(root, "max_user");
	Model.max_user = max_user->valueint;

	cJSON* map = cJSON_GetObjectItem(root, "map");
	cJSON* map_width = cJSON_GetObjectItem(map, "map_width");
	Model.map_width = map_width->valueint;	
	cJSON* map_height = cJSON_GetObjectItem(map, "map_height");
	Model.map_height = map_height->valueint;

	cJSON* user = cJSON_GetObjectItem(root, "user");
	Model.users = (struct user *)malloc(sizeof(struct user) * Model.max_user);
	for(int i = 0; i < Model.max_user; i++){
		memset(Model.users[i].name, 0, sizeof(NAME_SIZE));
		Model.users[i].score = 0;
		cJSON* user_array = cJSON_GetArrayItem(user,i);
	    cJSON* base = cJSON_GetObjectItem(user_array,"base"); 
		cJSON* base_x = cJSON_GetArrayItem(base, 0);
		cJSON* base_y = cJSON_GetArrayItem(base, 1);
		cJSON* user_location = cJSON_GetObjectItem(user_array,"location"); 
		cJSON* user_x = cJSON_GetArrayItem(user_location, 0);
		cJSON* user_y = cJSON_GetArrayItem(user_location, 1);
		Model.users[i].user_loc.x = user_x->valueint;
		Model.users[i].user_loc.y = user_y->valueint;
		Model.users[i].base_loc.x = base_x->valueint;
		Model.users[i].base_loc.y = base_y->valueint;
	#ifdef DEBUG
		printf("name: %s\n",Model.users[i].name);
		printf("base x: %d\n",Model.users[i].base_loc.x);
		printf("base y: %d\n",Model.users[i].base_loc.y);
	#endif
	}
	
	cJSON * item = cJSON_GetObjectItem(root, "item_location");
	num_item = cJSON_GetArraySize(item);
	current_num_item = num_item;
	Model.item_locations = (struct location *)malloc(sizeof(struct location) * num_item); 
	for(int i = 0; i < num_item; i++){
		cJSON* item_array = cJSON_GetArrayItem(item,i);
		cJSON* item_x = cJSON_GetArrayItem(item_array, 0);
		cJSON* item_y = cJSON_GetArrayItem(item_array, 1);
		Model.item_locations[i].x = item_x->valueint;
		Model.item_locations[i].y = item_y->valueint;
	#ifdef DEBUG
		printf("item x: %d\n",Model.item_locations[i].x);
		printf("item y: %d\n",Model.item_locations[i].y);
		#endif
	}	

	cJSON * block = cJSON_GetObjectItem(root, "block_location");
	num_block = cJSON_GetArraySize(block);
	Model.block_locations = (struct location *)malloc(sizeof(struct location) * num_block); 
	for(int i = 0; i < num_block; i++){
		cJSON* block_array = cJSON_GetArrayItem(block,i);
		cJSON* block_x = cJSON_GetArrayItem(block_array, 0);
		cJSON* block_y = cJSON_GetArrayItem(block_array, 1);
		Model.block_locations[i].x = block_x->valueint;
		Model.block_locations[i].y = block_y->valueint;
	#ifdef DEBUG
		printf("block x: %d\n",Model.block_locations[i].x);
		printf("block y: %d\n",Model.block_locations[i].y);
	#endif
	}	
		
	return 0;
}

int move(int cmd, int movement){
	fprintf(stderr, "cmd : %d\n", cmd);
	int user_idx = cmd/Model.max_user;
	int span = cmd%4;	
	int curr_x, curr_y, target_x, target_y, item_target_x, item_target_y;
	curr_x = target_x = item_target_x = Model.users[user_idx].user_loc.x;
	curr_y = target_y = item_target_y = Model.users[user_idx].user_loc.y;
	switch(span){
		case UP:		
			target_y = curr_y - 1;
			item_target_y = target_y - 1;
			break;	
		case DOWN:
			target_y = curr_y + 1;
			item_target_y = target_y + 1;
			break;	
		case LEFT:
			target_x = curr_x - 1;
			item_target_x = target_x - 1;
			break;	
		case RIGHT:
			fprintf(stderr, "right\n");
			target_x = curr_x + 1;
			item_target_x = target_x + 1;
			break;	
	}
	
	if(movement < ITEM){ //valid and item-empty
		fprintf(stderr,"move for item %d!!!\n", movement);	
		// update_model(movement, item_target_x, item_target_y);	
	}else if(movement > (0-ITEM)){ //valid and item-scoreup
		fprintf(stderr,"move for success %d!!!\n", movement);	
		// update_model((0-movement), -1, -1);	
		// score_up(user_idx);
		current_num_item--;
	}
	// update_model(user_idx+1, target_x,target_y);	
	fprintf(stderr,"move finish!\n");
}

int main(int argc, char ** argv){
    /*
    1. Model을 정의
    */
    if(argc != 2){
        // return 1;
    }
    
    char * json_input = "map1.json";
	loadJson(json_input);
    parseJson(json_serialize);

    int fd;
    if((fd = open(argv[1], O_RDONLY)) < 0){
        fprintf(stderr, "%s open error\n",argv[1]);
        return 1;
    }
    char cmd;
	char movement[2];
	
    read(fd, (void *)&cmd , 1);
    fprintf(stderr, "cmd :%c\n", cmd);
	int a = cmd - '0';

	read(fd, (void *)&movement , 2);
    fprintf(stderr, "movement :%s\n", movement);
	int b = atoi(movement);

	move(a,b);


    return 0;
}