cJSON_path = ../cJSON.c

% : %.c $(cJSON_path)
	gcc -O0 -fprofile-arcs -ftest-coverage -static -o $@ $^ -pthread

clean_move_gcov:
	rm move.gcda
	rm move.gcno
	rm move.c.gcov
	

# clean: 
# 	rm move

clean:
	rm $(TARGET)
	rm $(TARGET).gcda
	rm $(TARGET).gcno
	rm $(TARGET).c.gcov
	
