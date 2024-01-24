JSON = "../game_data/map2.json"
IP = "127.0.0.1"
PORT = "9990"

LIBS = `pkg-config --libs --cflags gtk+-2.0`

cJSON_path = ../cJSON.c

compile: server client

server: pushpush_server.c $(cJSON_path)
	gcc -o $@  $^  -pthread
client: pushpush_client.c $(cJSON_path)
	gcc -o $@ $^  $(LIBS) -DDEBUG
rs:
	echo $(JSON) | ./server $(PORT)
rc:
	./client $(IP) $(PORT)
clean:
	rm server client
