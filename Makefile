cJSON_path = ./cJSON.c

loadJson : loadJson_test.c $(cJSON_path)
	gcc -O0 -fprofile-arcs -ftest-coverage -static -o $@ $^ -pthread

% : %.c
	gcc -O0 -fprofile-arcs -ftest-coverage -static -o $@ $^ -pthread

handle_clnt : handle_clnt.c;
	gcc -O0 -fprofile-arcs -ftest-coverage -static -o $@ $^ -pthread

clean:
	rm handle_clnt
	rm loadJson 
	