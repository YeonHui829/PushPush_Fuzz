#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <gtk/gtk.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include "../cJSON.h"
#include "client.h"

char user_color[8][20] = {"#faa8a1", "#ffe479", "#dbe87c", "#a19b8b", "#ea9574", "#ffca79", "#c79465", "#e3dbcf"};

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

//GUI functions
GtkWidget *window;
GtkWidget *mat_main, *mat_changed_screen, *mat_board, *label_info, *label_me, *mat_fixed_screen, *mat_screen;
GtkWidget *mat_ans_btn, *mat_sol_btn;
GtkWidget *btn_solve, *btn_exit, *btn_next, *btn_prev;
GtkWidget *btn_auto, *btn_up, *btn_down, *btn_left, *btn_right;
GtkWidget *label_name;
GtkWidget ** label_score;
GdkPixbuf *icon, *icon_block[2], *icon_fruit[11];
GdkPixbuf ** icon_player;

#ifdef MAIN
//this is MAIN

int main(int argc, char *argv[]){

	//get the username from stdin 
	//TODO maybe need change to args
	pthread_mutex_init(&mutx, NULL);
	pthread_cond_init(&cond, NULL);

	signal(SIGALRM, handle_timeout);
	gtk_init(&argc, &argv); //init GTK by args

	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	 }
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	  
	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
		fprintf(stderr, "ERROR: connect() error\n");
		exit(1);
	}

	int game_started;
	if (read_bytes(sock, (void*)&game_started, sizeof(int)) == -1) 
    	return 1;

	if(game_started){
		cannot_enter();
		close(sock);
		return 0;
	}
	
	while(1){
		printf("enter your name: ");
		scanf("%s", buf);
		if(strlen(buf) > NAME_SIZE){
			printf("invalid name. please pick another one.");	
			continue;
		}else break;
	}
	
	int name_size = strlen(buf);
	if(write_bytes(sock, (void *)&name_size, sizeof(int)) == -1)
		return 1;

	if(write_bytes(sock, buf, strlen(buf)) == -1)
		return 1;

    //read my id
	if (read_bytes(sock, (void*)&my_id, sizeof(int)) == -1) 
    	return 1;
		
	fprintf(stderr, "id : %d\n", my_id);
	
    // read json file
    int json_size;
    if (read_bytes(sock, (void*)&(json_size), sizeof(int)) == -1)
		return 1;

    char * json_format = malloc(sizeof(char) * json_size);
    if (read_bytes(sock, json_format, json_size) == -1)
		return 1;

	if(parseJson(json_format))
		return 1;
	
	// receive all player's name size, name 
	// test hardcoding
	for (int i = 0; i < Model.max_user; i++) {
		int name_size;
		if (read_bytes(sock, (void*)&(name_size), sizeof(name_size)) == -1)
			return 1;
	
		if (read_bytes(sock, (void*)(Model.users[i].name), name_size) == -1)
			return 1;
		Model.users[i].name[name_size] = 0x0;
		printf("id : %d name : %s\n",i,Model.users[i].name);
	}


	map = (int **) malloc (sizeof(int *) * Model.map_width);
	for(int i=0;i<Model.map_width;i++){
		map[i] =(int *) malloc(sizeof(int) * Model.map_height);
	} 

	label_score = (GtkWidget **) malloc(Model.max_user* sizeof(GtkWidget *));
	icon_player = (GdkPixbuf **) malloc(Model.max_user * sizeof(GdkPixbuf *));

	update_cell();

	//load icons from icons dir
	if(load_icons()) {
		fprintf(stderr,"failed to load icons\n");
		return 1;
  	}

	srand((unsigned int)time(0));
	set_window();

	alarm(Model.timeout);

	pthread_create(&rcv_thread, NULL, read_msg, (void*)&sock);
	g_timeout_add(50,handle_cmd, NULL);
	
  	gtk_main(); //enter the GTK main loop

	pthread_join(rcv_thread, &thread_return);
	free(map);
	close(sock);  
	pthread_mutex_destroy(&mutx);

 	return 0;
}
#endif


//get pixbuf(for load image) from filename
GdkPixbuf *create_pixbuf(const gchar * filename) {
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if (!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }
   return pixbuf;
}


//load icons needed
//0 on success, 1 on failure
int load_icons(){
   	GdkPixbuf *pixbuf;
	for(int i = 0; i < Model.max_user; i++){
		sprintf(buf, "../icons/user%d.png", i);
		if((pixbuf = create_pixbuf(buf)) == NULL) return 1;
		else fprintf(stderr,"loading %s...\n", buf);
		icon_player[i] = gdk_pixbuf_scale_simple(pixbuf, 32, 32, GDK_INTERP_BILINEAR);
		g_object_unref(pixbuf);
	}
	for(int i = 0; i < 2; i++){
		sprintf(buf, "../icons/block%d.png", i);
		if((pixbuf = create_pixbuf(buf)) == NULL) return 1;
		else fprintf(stderr,"loading %s...\n", buf);
		icon_block[i] = gdk_pixbuf_scale_simple(pixbuf, 32, 32, GDK_INTERP_BILINEAR);
		g_object_unref(pixbuf);
	}
	for(int i = 0; i < 12; i++){
		sprintf(buf, "../icons/fruit%d.png", i);
		if((pixbuf = create_pixbuf(buf)) == NULL) return 1;
		else fprintf(stderr,"loading %s...\n", buf);
		icon_fruit[i] = gdk_pixbuf_scale_simple(pixbuf, 32, 32, GDK_INTERP_BILINEAR);
		g_object_unref(pixbuf);
	}

	fprintf(stderr,"success to load all icons!\n");
	return 0;

}

//GUI: set the main window
void set_window(){

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);//make window
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);//for termination
  g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_press), NULL);
 
  //set the window
  gtk_window_set_title(GTK_WINDOW(window), "pushpush HK");
  gtk_window_set_default_size(GTK_WINDOW(window), 1024, 512);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(window), 5);

  //change the icon of page(for cuteness)
  icon = create_pixbuf("../icons/catIcon.png");
  gtk_window_set_icon(GTK_WINDOW(window), icon);
  
  //set main matrix
  mat_main = gtk_table_new(8, 8-1, TRUE);
	
  strcpy(msg_info, "Welcome to PushPush HK!");
  label_info = gtk_label_new(msg_info);
  gtk_table_attach_defaults(GTK_TABLE(mat_main), label_info, 0, 11, 0, 1);
  
  display_screen();
  add_mat_board();
 
  label_me = gtk_label_new("23-winter capston study#2 leeejjju");
  gtk_misc_set_alignment(GTK_MISC(label_me), 0.0, 1.0);
  gtk_table_attach_defaults(GTK_TABLE(mat_main), label_me, 0, 8+1, 6, 8);

  gtk_container_add(GTK_CONTAINER(window), mat_main);
  gtk_widget_show_all(window); //is it dup with above
  g_object_unref(icon);


}

//create entity Widget from ids, return widget* or NULL on id-empty
GtkWidget* create_entity(int id){

	GtkWidget* sprite;
	GdkColor color;
	int idx;

	if(id == EMPTY) return NULL;
	else if(id == BLOCK){
		idx = rand() % 2;
      	sprite = gtk_image_new_from_pixbuf(icon_block[idx]); 
	}else if(id < ITEM){
		idx = (0-id)/10-1;
      	sprite = gtk_image_new_from_pixbuf(icon_fruit[idx]); 
	}else if(id > BASE){
		idx = id/10 -1;
		sprite = gtk_event_box_new();
		//gtk_widget_set_size_request(sprite, 32, 32);		
		gdk_color_parse(user_color[idx], &color);
		gtk_widget_modify_bg(sprite, GTK_STATE_NORMAL, &color);
	}else{
		idx = id-1;
      	sprite = gtk_image_new_from_pixbuf(icon_player[idx]); 
	}
	return sprite;

}

//GUI: display screen from map[] model
void display_screen(){
	
  //set screen matrix
  if(mat_changed_screen == NULL){ //initially once
	mat_screen = gtk_fixed_new();
	mat_changed_screen = gtk_table_new(Model.map_width, Model.map_height, TRUE);
	mat_fixed_screen = gtk_table_new(Model.map_width, Model.map_height, TRUE);
  for (int i = 0; i < Model.map_width; i++) {
    for (int j = 0; j < Model.map_height; j++) {
			if(map[j][i] == BLOCK || map[j][i] > BASE){
				GtkWidget* sprite = create_entity(map[j][i]);
				if(sprite != NULL) gtk_table_attach_defaults(GTK_TABLE(mat_fixed_screen), sprite, i, i+1, j, j+1);
			}	
		}
  }
	gtk_fixed_put(GTK_FIXED(mat_screen), mat_fixed_screen, 0, 0);
	gtk_fixed_put(GTK_FIXED(mat_screen), mat_changed_screen, 0, 0);
  }else gtk_container_foreach(GTK_CONTAINER(mat_changed_screen), (GtkCallback)gtk_widget_destroy, NULL); 

  for (int i = 0; i < Model.map_width; i++) {
    for (int j = 0; j < Model.map_height; j++) {
		if(map[j][i] == BLOCK || map[j][i] > BASE) 
			continue;
		GtkWidget* sprite = create_entity(map[j][i]);
		if(sprite != NULL) gtk_table_attach_defaults(GTK_TABLE(mat_changed_screen), sprite, i, i+1, j, j+1);
    }
  }

  if(!gtk_widget_get_parent(mat_screen)) gtk_table_attach_defaults(GTK_TABLE(mat_main), mat_screen, 0, 9, 1, 10);
  gtk_widget_show_all(window); 

}

//GUI: set the score board
void add_mat_board(){

  //set board vbox
  int board_width = 8;
  mat_board = gtk_table_new(board_width, 10, TRUE);

  GtkWidget* line1 = gtk_hseparator_new();
  sprintf(buf, "Good luck, %s!", Model.users[my_id].name);
  label_name = gtk_label_new(buf);
  GtkWidget* sprite = gtk_image_new_from_pixbuf(icon_player[my_id]);
  GtkWidget* line2 = gtk_hseparator_new();
  GtkWidget *label_title = gtk_label_new(":: SCORE ::");
  
  gtk_table_attach_defaults(GTK_TABLE(mat_board), line1, 0, board_width+1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(mat_board), label_name, 0, board_width+1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(mat_board), line2, 0, board_width+1, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(mat_board), sprite, 0, board_width+1, 3, 4);
  gtk_table_attach_defaults(GTK_TABLE(mat_board), label_title, 0, board_width+1, 4, 5);
	
  GtkWidget* score_board = gtk_vbox_new(TRUE, 10);
  for(int i = 0; i < Model.max_user; i++){
	sprintf(msg_info, "%s: %d", Model.users[i].name, Model.users[i].score);		
	label_score[i] = gtk_label_new(msg_info);
	gtk_container_add(GTK_CONTAINER(score_board), label_score[i]);
  } 
  gtk_container_add(GTK_CONTAINER(score_board), gtk_label_new(""));
  gtk_table_attach_defaults(GTK_TABLE(mat_board), score_board, 0, board_width+1, 5, 10);

  btn_exit = gtk_button_new_with_label("exit game");
  gtk_table_attach_defaults(GTK_TABLE(mat_board), btn_exit, 0, board_width+1, 10, 11);
  g_signal_connect(G_OBJECT(btn_exit), "clicked", G_CALLBACK(exit_game), NULL);
  gtk_table_attach_defaults(GTK_TABLE(mat_main), mat_board, 9, 11, 0, 8); 

}

void exit_game(GtkWidget* widget){
	printf("See you again!\n");
	exit(EXIT_SUCCESS);
}


int check_validation(int cmd){
	int user_idx = cmd/4;
	int span = cmd%4;	
	
	int curr_x, curr_y, target_x, target_y, item_target_x, item_target_y;
	curr_x = target_x = item_target_x = Model.users[user_idx].user_loc.x;
	curr_y = target_y = item_target_y = Model.users[user_idx].user_loc.y;
	switch(span){
		case UP:		
			if((target_y = (curr_y - 1)) < 0) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_y = (target_y - 1)) < 0) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1) * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base
				else return 0;	//others (block, user, base)
			}else return 0;	
			break;

		case DOWN:
			if((target_y = (curr_y + 1)) > Model.map_height -1 ) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_y = (target_y + 1)) > Model.map_height - 1) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1) * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base
				else return 0;	//others (block, user, base)
			}else return 0;	
			break;


		case LEFT:
			if((target_x = (curr_x - 1)) < 0) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_x = (target_x - 1)) < 0) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1)  * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base

				else return 0;	//others (block, user, base)
			}else return 0;	
			break;

		case RIGHT:
			if((target_x = (curr_x + 1)) > Model.map_width  - 1) return 0;//out of array
			else if(map[target_y][target_x] == EMPTY) return 1; //empty
			else if(map[target_y][target_x] > BASE) return 1; //base
			else if(map[target_y][target_x] < ITEM){ 
				if((item_target_x = (target_x + 1)) > Model.map_width -1) return 0; //item and non-movabel
				if(map[item_target_y][item_target_x] == EMPTY) return map[target_y][target_x]; //item and movable
				if((map[item_target_y][item_target_x] > BASE) && ((map[item_target_y][item_target_x]) == ((user_idx + 1) * 10))) return (0 - map[target_y][target_x]);
				if(map[item_target_y][item_target_x] > BASE) return map[target_y][target_x]; //item and movable as other's base
				else return 0;	//others (block, user, base)
			}else return 0;	
			break;
	}

}

//update cells by cmd(0-15), 
//return 0 on normal moving, return 1 on get-score moving
//TODO 
int move(int cmd, int movement){
	int user_idx = cmd/4;
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
			target_x = curr_x + 1;
			item_target_x = target_x + 1;
			break;	
	}
	
	if(movement < ITEM){ //valid and item-empty
		fprintf(stderr,"move for item %d!!!\n", movement);	
		update_model(movement, item_target_x, item_target_y);	
	}else if(movement > (0-ITEM)){ //valid and item-scoreup
		fprintf(stderr,"move for success %d!!!\n", movement);	
		update_model((0-movement), -1, -1);	
		// score_up(user_idx);
		current_num_item--;
	}
	update_model(user_idx+1, target_x,target_y);	
	fprintf(stderr,"move finish!\n");
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    
	const gchar *greeting = NULL;
	int cmd = my_id*4;
    switch (event->keyval) {
        case GDK_KEY_UP:
            greeting = "up key pressed...";
			cmd += 0;
            break;
        case GDK_KEY_DOWN:
            greeting = "down key pressed...";
			cmd += 1;
            break;
        case GDK_KEY_LEFT:
            greeting = "left key pressed...";
			cmd += 2;
            break;
        case GDK_KEY_RIGHT:
            greeting = "right key pressed...";
			cmd += 3;
            break;
	default:
		return TRUE;    
    }
	fprintf(stderr,"keyboard :%d player id : %d, cmd : %d\n", event->keyval, my_id ,cmd);
	write_bytes(sock, (void*)&cmd, sizeof(int));

	return TRUE;
}

void update_model(int id, int x, int y){

	printf("update model\n");

	int idx;
	if(id < ITEM){ //item
		idx = item_idToIdx(id);
		Model.item_locations[idx].x = x;	
		Model.item_locations[idx].y = y;	
		fprintf(stderr,"item model updated\n");
	}else{ //user
		idx = id - 1; 
		Model.users[idx].user_loc.x = x;	
		Model.users[idx].user_loc.y = y;	
		fprintf(stderr,"user model updated\n");
	}
	update_cell();
	//for debug
	for (int i = 0; i < Model.map_width; i++) {
      for (int j = 0; j < Model.map_height; j++) {
		fprintf(stderr,"%3d ",map[i][j]);
	  }
	  fprintf(stderr,"\n");
    }

}

void update_cell(){

	//init
	for(int i = 0; i < Model.map_width; i++){
		for(int j = 0; j < Model.map_height; j++){
			map[j][i] = EMPTY;
		}
	}

	//base and user
	for(int i = 0; i < Model.max_user; i++){
		int id = i+1;
		map[Model.users[i].user_loc.y][Model.users[i].user_loc.x] = id;
		map[Model.users[i].base_loc.y][Model.users[i].base_loc.x] = id*10;

		fprintf(stderr,"user %d  x : %d, y : %d, base %d, %d\n",i, Model.users[i].user_loc.x,Model.users[i].user_loc.y, Model.users[i].base_loc.x,Model.users[i].base_loc.y);
	}
	//block
	for (int i = 0; i < num_block; i++) {
		map[Model.block_locations[i].y][Model.block_locations[i].x] = BLOCK;
    }

	//item
	for (int i = 0; i < num_item; i++) {
		int item_id = item_idxToId(i);
		if(Model.item_locations[i].x == -1 && Model.item_locations[i].y == -1) continue; //skip removed item
		map[Model.item_locations[i].y][Model.item_locations[i].x] = item_id;
    }

}
int item_idxToId(int idx){ return ((0-(idx+1))*10); } //0 -> -10
int item_idToIdx(int id){ return (((0-id)/10)-1); }//-10 -> 0


void score_up(int user_idx){
	
	Model.users[user_idx].score ++;
	sprintf(msg_info, "%s got the score!", Model.users[user_idx].name);
	fprintf(stderr,"%s got the score!\n", Model.users[user_idx].name);
	gtk_label_set_text((GtkLabel*)label_info, msg_info);
	sprintf(msg_info, "%s: %d", Model.users[user_idx].name, Model.users[user_idx].score);
	gtk_label_set_text((GtkLabel*)label_score[user_idx], msg_info);

}

void gameover(){
	
	sprintf(msg_info, "GAME OVER!!!");
	fprintf(stderr,"GAME OVER\n");
	gtk_label_set_text((GtkLabel*)label_info, msg_info);

	gtk_widget_show_all(window);
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

void handle_timeout(int signum) {
    // 이 함수가 호출되면 10초가 경과했음을 의미
	int game_over = Model.max_user*4;

	write_bytes(sock,(void *)&game_over,sizeof(game_over));
    //gameover 신호 보내기 
	gameover();
}
void * read_msg(void * arg)   // read thread main
{
	int sock = *((int*)arg);
	int read_cmd;

	//now enter new move 
	while(1){
		if(read_bytes(sock, (void *)&read_cmd, sizeof(read_cmd)) == -1)
			return (void*)-1;

        fprintf(stderr, "From Server : %d\n", read_cmd);

		pthread_mutex_lock(&mutx);
		while((rear+1)%queue_size == front)
			pthread_cond_wait(&cond, &mutx);
		rear = (rear + 1) % queue_size;
		event_arry[rear] = read_cmd;
		pthread_cond_signal(&cond);
  		pthread_mutex_unlock(&mutx);
	}
	return NULL;
}

int read_bytes(int sock_fd, void * buf, size_t len){
    char * p = (char *)buf;
    size_t acc = 0;

    while(acc < len)
    {
        size_t readed;
        readed = read(sock_fd, p, len - acc);
        if(readed  == -1 || readed == 0){
            return -1;
        }
        p+= readed ;
        acc += readed ;
    }
    return 0;
}

int write_bytes(int sock_fd, void * buf, size_t len){
    char * p = (char *) buf;
    size_t acc = 0;

    while(acc < len){
        size_t sent;
        sent = write(sock_fd, p, len - acc);
        if(sent == -1){
            return -1;
        }

        p+= sent;
        acc += sent;
    }
    return 0;
}

gboolean handle_cmd(gpointer user_data) {
	int event;
    pthread_mutex_lock(&mutx);
	if(rear == front)
	{
		pthread_mutex_unlock(&mutx);
		return TRUE;
	}

	front = (front + 1) % queue_size;
	event = event_arry[front];
	
	pthread_cond_signal(&cond);
  	pthread_mutex_unlock(&mutx);
	
	if(event == Model.max_user*4) 
	{
		return FALSE;
	}

	int movement;
	if((movement = check_validation(event)) == 0) fprintf(stderr,"invalid movement!\n");
	else{	
		move(event, movement);
		display_screen();
		if( current_num_item <= 0) {
			gameover();
		}
	} 

    return TRUE;
}

void cannot_enter(){
	// sprintf(msg_info, "Cannot enter the game.\n");
	fprintf(stderr,"Cannot enter the game.\n");
}