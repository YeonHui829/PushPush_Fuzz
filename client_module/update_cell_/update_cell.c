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
#define MAX_USER 4
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

int item_idxToId(int idx){ return ((0-(idx+1))*10); } //0 -> -10


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

// update cell using Model structure 
void update_cell(){

	//init
    fprintf(stderr, "init start\n");
	for(int i = 0; i < Model.map_width; i++){
		for(int j = 0; j < Model.map_height; j++){
			map[j][i] = EMPTY;
		}
	}
    fprintf(stderr, "init finish\n");
	//base and user
	for(int i = 0; i < Model.max_user; i++){
		int id = i+1;
		map[Model.users[i].user_loc.y][Model.users[i].user_loc.x] = id;
		map[Model.users[i].base_loc.y][Model.users[i].base_loc.x] = id*10;

		fprintf(stderr,"user %d  x : %d, y : %d, base %d, %d\n",i, Model.users[i].user_loc.x,Model.users[i].user_loc.y, Model.users[i].base_loc.x,Model.users[i].base_loc.y);
	}
    fprintf(stderr, "user finish\n");
	//block
	for (int i = 0; i < num_block; i++) {
		map[Model.block_locations[i].y][Model.block_locations[i].x] = BLOCK;
    }
    fprintf(stderr, "block finish\n");
	//item
	for (int i = 0; i < num_item; i++) {
		int item_id = item_idxToId(i);
        if(Model.item_locations[i].x == -1){
            fprintf(stderr, "here");
        }
        else{
            fprintf(stderr, "not!\n");
        }
		if(Model.item_locations[i].x == -1 && Model.item_locations[i].y == -1) continue; //skip removed item
		map[Model.item_locations[i].y][Model.item_locations[i].x] = item_id;
    }
    fprintf(stderr, "item finish\n");

}
/*
update cell의 input값은 새로워진 Model값 그럼 init할 때 update cell을 쓴 것처럼 json file을 통해 Model의 값들을 채워주고,
update cell을 이용해 map을 update해주자 ! 그렇다면 여기서 input 값은 json file!
*/

int main(int argc, char ** argv){

    // char * json_input = "map1.json";
    fprintf(stderr, "argv : %s\n", argv[1]);
	loadJson(argv[1]);
    parseJson(json_serialize);
    map = (int **) malloc (sizeof(int *) * Model.map_width);
	for(int i=0;i<Model.map_width;i++){
		map[i] =(int *) malloc(sizeof(int) * Model.map_height);
	} 

    update_cell();
}