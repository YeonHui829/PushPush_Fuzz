#include "client.h"
#include <string.h>
#include "../cJSON.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

extern char user_color[8][20];

extern int **map;
extern object_data_t Model;
extern char msg_info[BUF_SIZE];
extern char buf[BUF_SIZE];
extern int sock;
extern int my_id;
extern int num_item, num_block;
extern int current_num_item;

extern pthread_mutex_t mutx;
extern pthread_cond_t cond;
extern int event_arry[queue_size];
extern int rear;
extern int front;

// GUI functions
extern GtkWidget *window;
extern GtkWidget *mat_main, *mat_changed_screen, *mat_board, *label_info, *label_me, *mat_fixed_screen, *mat_screen;
extern GtkWidget *mat_ans_btn, *mat_sol_btn;
extern GtkWidget *btn_solve, *btn_exit, *btn_next, *btn_prev;
extern GtkWidget *btn_auto, *btn_up, *btn_down, *btn_left, *btn_right;
extern GtkWidget *label_name;
extern GtkWidget **label_score;
extern GdkPixbuf *icon, *icon_block[2], *icon_fruit[11];
extern GdkPixbuf **icon_player;


// for load Json
#define MAX_USER 4
#define NAME_SIZE 16
#define PATH_LENGTH 256

int usr_cnt = 0; //num of connected user
int game_start = 0;
int max_user;
char ** user_name;
int json_size;
char * json_serialize;

//end

int loadJson(char * filepath);

#ifdef CHECK_VALIDATION 
int main(int argc, char ** argv){
	if(argc != 3){
        fprintf(stderr, "argc error");
        return 1;
    }

    if(loadJson(argv[1]) == 1){
        fprintf(stderr, "loadJson error");
        return 1;
    }
    
    if(parseJson(json_serialize)){
        return 1;
    }

    map = (int **) malloc (sizeof(int *) * Model.map_width);
	for(int i=0;i<Model.map_width;i++){
		map[i] =(int *) malloc(sizeof(int) * Model.map_height);
	} 

    update_cell();

	FILE * fp;
	if((fp = fopen(argv[2], "rb")) == NULL){
		fprintf(stderr, "fopen error\n");
		return 1;
	}

	int input[1];
	for(int i=0; i<1; i++){
		char buf[BUF_SIZE];
		if(fgets(buf, BUF_SIZE, fp) == NULL){
			fprintf(stderr, "fgets error\n");
		}
		input[i] = atoi(buf);
	}

}

#endif 
#ifdef MOVE
int main(int argc, char ** argv){
	if(argc != 3){
        fprintf(stderr, "argc error");
        return 1;
    }

    if(loadJson(argv[1]) == 1){
        fprintf(stderr, "loadJson error");
        return 1;
    }
    
    if(parseJson(json_serialize)){
        return 1;
    }

    map = (int **) malloc (sizeof(int *) * Model.map_width);
	for(int i=0;i<Model.map_width;i++){
		map[i] =(int *) malloc(sizeof(int) * Model.map_height);
	} 

	FILE * fp;
	if((fp = fopen(argv[2], "rb")) == NULL){
		fprintf(stderr, "fopen error\n");
		return 1;
	}

	int input[2];
	for(int i=0; i<2; i++){
		char buf[BUF_SIZE];
		if(fgets(buf, BUF_SIZE, fp) == NULL){
			fprintf(stderr, "fgets error\n");
		}
		input[i] = atoi(buf);
	}

	move(input[0], input[1]);
}
#endif

#ifdef UPDATEMODEL
int main(int argc, char ** argv){
	if(argc != 3){
        fprintf(stderr, "argc error");
        return 1;
    }

    if(loadJson(argv[1]) == 1){
        fprintf(stderr, "loadJson error");
        return 1;
    }
    
    if(parseJson(json_serialize)){
        return 1;
    }

    map = (int **) malloc (sizeof(int *) * Model.map_width);
	for(int i=0;i<Model.map_width;i++){
		map[i] =(int *) malloc(sizeof(int) * Model.map_height);
	} 

	FILE * fp;
	if((fp = fopen(argv[2], "rb")) == NULL){
		fprintf(stderr, "fopen error\n");
		return 1;
	}

	int input[3];
	for(int i=0; i<3; i++){
		char buf[BUF_SIZE];
		if(fgets(buf, BUF_SIZE, fp) == NULL){
			fprintf(stderr, "fgets error\n");
		}
		input[i] = atoi(buf);
	}

	update_model(input[0], input[1], input[2]);
	return 0;
}
#endif

#ifdef UPDATECELL
int main(int argc, char ** argv){
    if(argc != 2){
        fprintf(stderr, "argc error");
        return 1;
    }

    if(loadJson(argv[1]) == 1){
        fprintf(stderr, "loadJson error");
        return 1;
    }
    
    if(parseJson(json_serialize)){
        return 1;
    }

    map = (int **) malloc (sizeof(int *) * Model.map_width);
	for(int i=0;i<Model.map_width;i++){
		map[i] =(int *) malloc(sizeof(int) * Model.map_height);
	} 

    update_cell();
}
#endif

#ifdef LOAD_JSON
int main(int argc, char ** argv){
    if(argc != 2){
        fprintf(stderr, "argc error");
        return 1;
    }

    if(loadJson(argv[1]) == 1){
        fprintf(stderr, "loadJson error");
        return 1;
    }
}

#endif

#ifdef PARSEJSON
int main(int argc, char ** argv){
    if(argc != 2){
        fprintf(stderr, "argc error");
        return 1;
    }
    
    if(loadJson(argv[1]) == 1){
        fprintf(stderr, "loadJson error");
        return 1;
    }

    if(parseJson(json_serialize)){
        return 1;
    }
}

#endif

#ifdef HANDLE_CMD
	int main(int argc, char *argv[]){
		pthread_t rcv_thread;
		loadJson("./map1.json");
		parseJson(json_serialize);

		int fd = open("cmd_sequence",O_CREAT|O_RDWR|O_TRUNC,0644);
		int cmd = 5;
		dprintf(fd,"%d",cmd);

		pthread_create(&rcv_thread, NULL, read_msg, &fd);

		while(1){
			sleep(1);
			handle_cmd(NULL);
		}
	}
#endif


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

