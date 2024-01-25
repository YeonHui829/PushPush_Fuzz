#ifndef CLIENT_H
#define CLIENT_H

#define NAME_SIZE 16
#define queue_size 20
#define BUF_SIZE 128
#define GDK_KEY_UP 65362
#define GDK_KEY_DOWN 65364
#define GDK_KEY_LEFT 65361
#define GDK_KEY_RIGHT 65363
#include <gtk/gtk.h>

//object data structures
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

GdkPixbuf *create_pixbuf(const gchar * filename);
GtkWidget* create_entity(int id);
int load_icons();
int check_map_valid();
void set_window();
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void display_screen();
void add_mat_board();
void exit_game(GtkWidget* widget);
void gameover();

//for move handling
int check_validation(int cmd);
int move(int cmd, int movement);
void update_model(int id, int x, int y);
void update_cell();
int item_idxToId(int idx);
int item_idToIdx(int id);
void score_up(int user_idx);
gboolean handle_cmd(gpointer user_data) ;

//for networking
int read_bytes(int sock_fd, void * buf, size_t len);
int write_bytes(int sock_fd, void * buf, size_t len);
void handle_timeout(int signum);
int parseJson(char * jsonfile);
void *read_msg(void * arg);
void cannot_enter();

#endif